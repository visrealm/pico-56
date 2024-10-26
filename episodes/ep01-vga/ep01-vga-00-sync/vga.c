/*
 * Project: pico-56 - episode 1
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "vga.h"
#include "vga.pio.h"

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include <math.h>
#include <string.h>

#define SYNC_PINS_START 0        // first sync pin gpio number
#define SYNC_PINS_COUNT 2        // number of sync pins (h and v)

#define VGA_PIO         pio0_hw  // which pio are we using for vga?
#define SYNC_SM         0        // vga sync state machine index


 /*
  * timings for 640 x 480 @ 60Hz (from http://tinyvga.com/vga-timing/640x480@60Hz)
  */
#define PIXEL_CLOCK_KHZ 25175.0f // true pixel clock for the given mode (640 x 480 x 60Hz)
#define ACTIVE_PIXELS_X   640    // number of horizontal pixels for active area
#define FPORCH_PIXELS_X    16    // number of horizontal pixels for front porch
#define HSYNC_PIXELS_X     96    // number of horizontal pixels for sync
#define BPORCH_PIXELS_X    48    // number of horizontal pixels for back porch

#define ACTIVE_PIXELS_Y   480    // number of vertical pixels for active area
#define FPORCH_PIXELS_Y    10    // number of vertical pixels for front porch
#define VSYNC_PIXELS_Y      2    // number of vertical pixels for sync
#define BPORCH_PIXELS_Y    33    // number of vertical pixels for back porch


#define TOTAL_PIXELS_X    (ACTIVE_PIXELS_X + FPORCH_PIXELS_X + HSYNC_PIXELS_X + BPORCH_PIXELS_X)
#define TOTAL_PIXELS_Y    (ACTIVE_PIXELS_Y + FPORCH_PIXELS_Y + VSYNC_PIXELS_Y + BPORCH_PIXELS_Y)

#define PIXEL_SCALE_X     (ACTIVE_PIXELS_X / VGA_VIRTUAL_WIDTH)
#define PIXEL_SCALE_Y     (ACTIVE_PIXELS_Y / VGA_VIRTUAL_HEIGHT)

#define HSYNC_PULSE_POSITIVE 0
#define VSYNC_PULSE_POSITIVE 0

  /*
   * sync pio dma data buffers (must be word aligned)
   */
uint32_t __aligned(4) syncDataActive[4];  // active display area
uint32_t __aligned(4) syncDataPorch[4];   // vertical porch
uint32_t __aligned(4) syncDataSync[4];    // vertical sync

#define SYNC_LINE_ACTIVE 0
#define SYNC_LINE_FPORCH 1
#define SYNC_LINE_HSYNC  2
#define SYNC_LINE_BPORCH 3

/*
 * file scope
 */
static int syncDmaChan = 0;

/*
 * build the sync data buffers
 */
static void buildSyncData(void)
{
  const uint32_t sysClockKHz = clock_get_hz(clk_sys) / 1000;
  const uint32_t pioClockKHz = sysClockKHz;

  // compute number of pio ticks for a single pixel
  const float pioClocksPerPixel = pioClockKHz / PIXEL_CLOCK_KHZ;

  // compute pio ticks for each phase of the scanline
  const uint32_t activeTicks = round(pioClocksPerPixel * (float)ACTIVE_PIXELS_X) - vga_sync_SETUP_OVERHEAD;
  const uint32_t fPorchTicks = round(pioClocksPerPixel * (float)FPORCH_PIXELS_X) - vga_sync_SETUP_OVERHEAD;
  const uint32_t syncTicks = round(pioClocksPerPixel * (float)HSYNC_PIXELS_X) - vga_sync_SETUP_OVERHEAD;
  const uint32_t bPorchTicks = round(pioClocksPerPixel * (float)BPORCH_PIXELS_X) - vga_sync_SETUP_OVERHEAD;

  // compute sync bits
  const uint32_t hSyncOff = (HSYNC_PULSE_POSITIVE ? 0b00 : 0b01) << vga_sync_WORD_SYNC_OFFSET;
  const uint32_t hSyncOn = (HSYNC_PULSE_POSITIVE ? 0b01 : 0b00) << vga_sync_WORD_SYNC_OFFSET;
  const uint32_t vSyncOff = (VSYNC_PULSE_POSITIVE ? 0b00 : 0b10) << vga_sync_WORD_SYNC_OFFSET;
  const uint32_t vSyncOn = (VSYNC_PULSE_POSITIVE ? 0b10 : 0b00) << vga_sync_WORD_SYNC_OFFSET;

  // compute exec instructions
  const uint32_t instNop = pio_encode_nop() << vga_sync_WORD_EXEC_OFFSET;

  // sync data for an active display scanline
  syncDataActive[SYNC_LINE_ACTIVE] = instNop | vSyncOff | hSyncOff | activeTicks;
  syncDataActive[SYNC_LINE_FPORCH] = instNop | vSyncOff | hSyncOff | fPorchTicks;
  syncDataActive[SYNC_LINE_HSYNC] = instNop | vSyncOff | hSyncOn | syncTicks;
  syncDataActive[SYNC_LINE_BPORCH] = instNop | vSyncOff | hSyncOff | bPorchTicks;

  // sync data for a front or back porch scanline
  syncDataPorch[SYNC_LINE_ACTIVE] = instNop | vSyncOff | hSyncOff | activeTicks;
  syncDataPorch[SYNC_LINE_FPORCH] = instNop | vSyncOff | hSyncOff | fPorchTicks;
  syncDataPorch[SYNC_LINE_HSYNC] = instNop | vSyncOff | hSyncOn | syncTicks;
  syncDataPorch[SYNC_LINE_BPORCH] = instNop | vSyncOff | hSyncOff | bPorchTicks;

  // sync data for a vsync scanline
  syncDataSync[SYNC_LINE_ACTIVE] = instNop | vSyncOn | hSyncOff | activeTicks;
  syncDataSync[SYNC_LINE_FPORCH] = instNop | vSyncOn | hSyncOff | fPorchTicks;
  syncDataSync[SYNC_LINE_HSYNC] = instNop | vSyncOn | hSyncOn | syncTicks;
  syncDataSync[SYNC_LINE_BPORCH] = instNop | vSyncOn | hSyncOff | bPorchTicks;
}


/*
 * initialise the vga sync pio
 */
static void vgaInitSync(void)
{
  buildSyncData();

  // initalize sync pins for pio
  for (uint i = 0; i < SYNC_PINS_COUNT; ++i)
  {
    pio_gpio_init(VGA_PIO, SYNC_PINS_START + i);
  }

  // add sync pio program
  uint syncProgOffset = pio_add_program(VGA_PIO, &vga_sync_program);
  pio_sm_set_consecutive_pindirs(VGA_PIO, SYNC_SM, SYNC_PINS_START, SYNC_PINS_COUNT, true);

  // configure sync pio
  pio_sm_config syncConfig = vga_sync_program_get_default_config(syncProgOffset);
  sm_config_set_out_pins(&syncConfig, SYNC_PINS_START, SYNC_PINS_COUNT);
  sm_config_set_clkdiv(&syncConfig, 1);
  sm_config_set_out_shift(&syncConfig, true, true, 32);   // right shift, auto-pull 32 bits
  sm_config_set_fifo_join(&syncConfig, PIO_FIFO_JOIN_TX);
  pio_sm_init(VGA_PIO, SYNC_SM, syncProgOffset, &syncConfig);

  // initialise sync dma
  syncDmaChan = dma_claim_unused_channel(true);
  dma_channel_config syncDmaChanConfig = dma_channel_get_default_config(syncDmaChan);
  channel_config_set_transfer_data_size(&syncDmaChanConfig, DMA_SIZE_32);            // 32 bits at a time
  channel_config_set_read_increment(&syncDmaChanConfig, true);                       // increment read
  channel_config_set_write_increment(&syncDmaChanConfig, false);                     // don't increment write 
  channel_config_set_dreq(&syncDmaChanConfig, pio_get_dreq(VGA_PIO, SYNC_SM, true)); // transfer when there's space in fifo

  // setup the dma channel and set it going
  dma_channel_configure(syncDmaChan, &syncDmaChanConfig, &VGA_PIO->txf[SYNC_SM], syncDataSync, 4, false);
  dma_channel_set_irq0_enabled(syncDmaChan, true);
}

/*
 * dma interrupt handler
 */
static void __time_critical_func(dmaIrqHandler)(void)
{
  static int currentTimingLine = -1;

  if (dma_hw->ints0 & (1u << syncDmaChan))
  {
    dma_hw->ints0 = 1u << syncDmaChan;

    if (++currentTimingLine >= TOTAL_PIXELS_Y)
    {
      currentTimingLine = 0;
    }

    if (currentTimingLine < VSYNC_PIXELS_Y)
    {
      dma_channel_set_read_addr(syncDmaChan, syncDataSync, true);
    }
    else if (currentTimingLine < (VSYNC_PIXELS_Y + BPORCH_PIXELS_Y))
    {
      dma_channel_set_read_addr(syncDmaChan, syncDataPorch, true);
    }
    else if (currentTimingLine < (TOTAL_PIXELS_Y - FPORCH_PIXELS_Y))
    {
      dma_channel_set_read_addr(syncDmaChan, syncDataActive, true);
    }
    else
    {
      dma_channel_set_read_addr(syncDmaChan, syncDataPorch, true);
    }
  }
}

/*
 * initialise the pio dma
 */
static void initDma()
{
  irq_set_exclusive_handler(DMA_IRQ_0, dmaIrqHandler);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_start(syncDmaChan);
}


/*
 * initialise the vga
 */
void vgaInit()
{
  vgaInitSync();
  initDma();

  // enable the sync state machine
  pio_sm_set_enabled(VGA_PIO, SYNC_SM, true);
}

