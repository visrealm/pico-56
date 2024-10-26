/*
 * Project: pico-56 - vga
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
#include "pio_utils.h"

#include "pico/divider.h"
#include "pico/multicore.h"

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include <math.h>
#include <string.h>

#define SYNC_PINS_START 0        // first sync pin gpio number
#define SYNC_PINS_COUNT 2        // number of sync pins (h and v)

#define RGB_PINS_START  2        // first rgb pin gpio number
#define RGB_PINS_COUNT 12        // number of rgb pins

#define VGA_PIO         pio0_hw  // which pio are we using for vga?
#define SYNC_SM         0        // vga sync state machine index
#define RGB_SM          1        // vga rgb state machine index


 /*
  * timings for 800 x 600 @ 60Hz (from http://tinyvga.com/vga-timing/800x600@60Hz)
  */
#define PIXEL_CLOCK_KHZ 40000    // true pixel clock for the given mode (640 x 480 x 60Hz)
#define ACTIVE_PIXELS_X   800    // number of horizontal pixels for active area
#define FPORCH_PIXELS_X    40    // number of horizontal pixels for front porch
#define HSYNC_PIXELS_X    128    // number of horizontal pixels for sync
#define BPORCH_PIXELS_X    88    // number of horizontal pixels for back porch

#define ACTIVE_PIXELS_Y   600    // number of vertical pixels for active area
#define FPORCH_PIXELS_Y     1    // number of vertical pixels for front porch
#define VSYNC_PIXELS_Y      4    // number of vertical pixels for sync
#define BPORCH_PIXELS_Y    23    // number of vertical pixels for back porch

#define TOTAL_PIXELS_X    (ACTIVE_PIXELS_X + FPORCH_PIXELS_X + HSYNC_PIXELS_X + BPORCH_PIXELS_X)
#define TOTAL_PIXELS_Y    (ACTIVE_PIXELS_Y + FPORCH_PIXELS_Y + VSYNC_PIXELS_Y + BPORCH_PIXELS_Y)

#define PIXEL_SCALE_X     (ACTIVE_PIXELS_X / VGA_VIRTUAL_WIDTH)
#define PIXEL_SCALE_Y     (ACTIVE_PIXELS_Y / VGA_VIRTUAL_HEIGHT)

#define HSYNC_PULSE_POSITIVE 0
#define VSYNC_PULSE_POSITIVE 0

  /*
   * sync pio dma data buffers
   */
uint32_t __aligned(4) syncDataActive[4];  // active display area
uint32_t __aligned(4) syncDataPorch[4];   // vertical porch
uint32_t __aligned(4) syncDataSync[4];    // vertical sync

uint16_t __aligned(4) rgbDataBufferEven[VGA_VIRTUAL_WIDTH];
uint16_t __aligned(4) rgbDataBufferOdd[VGA_VIRTUAL_WIDTH];

#define SYNC_LINE_ACTIVE 0
#define SYNC_LINE_FPORCH 1
#define SYNC_LINE_HSYNC  2
#define SYNC_LINE_BPORCH 3

/*
 * file scope
 */
static int syncDmaChan = 0;
static int rgbDmaChan = 0;
static VgaInitParams vgaParams;


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
  const uint32_t instIrq = pio_encode_irq_set(false, vga_rgb_RGB_IRQ) << vga_sync_WORD_EXEC_OFFSET;
  const uint32_t instNop = pio_encode_nop() << vga_sync_WORD_EXEC_OFFSET;

  // sync data for an active display scanline
  syncDataActive[SYNC_LINE_ACTIVE] = instIrq | vSyncOff | hSyncOff | activeTicks;
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
  sm_config_set_out_pins(&syncConfig, SYNC_PINS_START, 2);
  sm_config_set_clkdiv(&syncConfig, 1);
  sm_config_set_out_shift(&syncConfig, true, true, 32); // R shift, autopull @ 32 bits
  sm_config_set_fifo_join(&syncConfig, PIO_FIFO_JOIN_TX); // Join FIFOs together to get an 8 entry TX FIFO
  pio_sm_init(VGA_PIO, SYNC_SM, syncProgOffset, &syncConfig);

  // initialise sync dma
  syncDmaChan = dma_claim_unused_channel(true);
  dma_channel_config syncDmaChanConfig = dma_channel_get_default_config(syncDmaChan);
  channel_config_set_transfer_data_size(&syncDmaChanConfig, DMA_SIZE_32);           // transfer 32 bits at a time
  channel_config_set_read_increment(&syncDmaChanConfig, true);                       // increment read
  channel_config_set_write_increment(&syncDmaChanConfig, false);                     // don't increment write 
  channel_config_set_dreq(&syncDmaChanConfig, pio_get_dreq(VGA_PIO, SYNC_SM, true)); // transfer when there's space in fifo

  // setup the dma channel and set it going
  dma_channel_configure(syncDmaChan, &syncDmaChanConfig, &VGA_PIO->txf[SYNC_SM], syncDataSync, 4, false);
  dma_channel_set_irq0_enabled(syncDmaChan, true);
}


/*
 * initialise the vga sync pio
 */
static void vgaInitRgb(void)
{
  const uint32_t sysClockKHz = clock_get_hz(clk_sys) / 1000;
  uint32_t pioClockKHz = sysClockKHz;
  float pioClkDiv = 1.0f;

  float pioClocksPerScaledPixel = pioClockKHz * PIXEL_SCALE_X / PIXEL_CLOCK_KHZ;
  while (pioClocksPerScaledPixel > 31)
  {
    ++pioClkDiv;
    pioClockKHz = sysClockKHz / pioClkDiv;
    pioClocksPerScaledPixel = pioClockKHz * PIXEL_SCALE_X / PIXEL_CLOCK_KHZ;
  }

  // compute number of pio ticks for a single pixel
  const float pioClocksPerPixel = pioClockKHz / PIXEL_CLOCK_KHZ;

  const uint32_t rgbCyclesPerPixel = round(pioClocksPerScaledPixel);

  // copy the rgb program and set the appropriate pixel delay
  uint16_t rgbProgramInstr[vga_rgb_program.length];
  memcpy(rgbProgramInstr, vga_rgb_program.instructions, sizeof(rgbProgramInstr));
  rgbProgramInstr[vga_rgb_DELAY_INSTR] |= pio_encode_delay(rgbCyclesPerPixel - vga_rgb_LOOP_TICKS);

  pio_program_t rgbProgram = {
    .instructions = rgbProgramInstr,
    .length = vga_rgb_program.length,
    .origin = vga_rgb_program.origin
  };

  // initalize sync pins for pio
  for (uint i = 0; i < RGB_PINS_COUNT; ++i)
  {
    pio_gpio_init(VGA_PIO, RGB_PINS_START + i);
  }

  // add rgb pio program
  pio_sm_set_consecutive_pindirs(VGA_PIO, RGB_SM, RGB_PINS_START, RGB_PINS_COUNT, true);
  pio_set_y(VGA_PIO, RGB_SM, VGA_VIRTUAL_WIDTH - 1);

  uint rgbProgOffset = pio_add_program(VGA_PIO, &rgbProgram);
  pio_sm_config rgbConfig = vga_rgb_program_get_default_config(rgbProgOffset);

  sm_config_set_out_pins(&rgbConfig, RGB_PINS_START, RGB_PINS_COUNT);
  sm_config_set_clkdiv(&rgbConfig, pioClkDiv);

  sm_config_set_fifo_join(&rgbConfig, PIO_FIFO_JOIN_TX);

  sm_config_set_out_shift(&rgbConfig, true, true, 16); // R shift, autopull @ 16 bits
  pio_sm_init(VGA_PIO, RGB_SM, rgbProgOffset, &rgbConfig);

  // initialise rgb dma
  rgbDmaChan = dma_claim_unused_channel(true);
  dma_channel_config rgbDmaChanConfig = dma_channel_get_default_config(rgbDmaChan);
  channel_config_set_transfer_data_size(&rgbDmaChanConfig, DMA_SIZE_16);  // transfer 16 bits at a time
  channel_config_set_read_increment(&rgbDmaChanConfig, true);             // increment read
  channel_config_set_write_increment(&rgbDmaChanConfig, false);           // don;t increment write
  channel_config_set_dreq(&rgbDmaChanConfig, pio_get_dreq(VGA_PIO, RGB_SM, true));

  // setup the dma channel and set it going
  dma_channel_configure(rgbDmaChan, &rgbDmaChanConfig, &VGA_PIO->txf[RGB_SM], rgbDataBufferEven, VGA_VIRTUAL_WIDTH, false);
  dma_channel_set_irq0_enabled(rgbDmaChan, true);
}

/*
 * dma interrupt handler
 */
static void __time_critical_func(dmaIrqHandler)(void)
{
  static int currentTimingLine = -1;
  static int currentDisplayLine = -1;

  if (dma_hw->ints0 & (1u << syncDmaChan))
  {
    dma_hw->ints0 = 1u << syncDmaChan;

    if (++currentTimingLine >= TOTAL_PIXELS_Y)
    {
      currentTimingLine = 0;
      currentDisplayLine = 0;
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


  if (dma_hw->ints0 & (1u << rgbDmaChan))
  {
    dma_hw->ints0 = 1u << rgbDmaChan;

    divmod_result_t pxLineVal = divmod_u32u32(++currentDisplayLine, PIXEL_SCALE_Y);
    uint32_t pxLine = to_quotient_u32(pxLineVal);
    uint32_t pxLineRpt = to_remainder_u32(pxLineVal);

    dma_channel_set_read_addr(rgbDmaChan, (pxLine & 1) ? rgbDataBufferOdd : rgbDataBufferEven, true);

    // need a new line every X display lines
    if ((pxLineRpt == 0))
    {
      uint32_t requestLine = pxLine + 1;
      if (requestLine >= VGA_VIRTUAL_HEIGHT) requestLine -= VGA_VIRTUAL_HEIGHT;

      multicore_fifo_push_timeout_us(requestLine, 0);
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
  dma_channel_start(rgbDmaChan);
}

/*
 * main vga loop
 */
static void vgaLoop()
{
  while (1)
  {
    uint32_t message = multicore_fifo_pop_blocking();

    if (message & 0x01)
    {
      vgaParams.scanlineFn(message & 0xfff, rgbDataBufferOdd);
    }
    else
    {
      vgaParams.scanlineFn(message & 0xfff, rgbDataBufferEven);
    }
  }
}


/*
 * initialise the vga
 */
void vgaInit(VgaInitParams params)
{
  vgaParams = params;

  vgaInitSync();
  vgaInitRgb();

  initDma();

  pio_sm_set_enabled(VGA_PIO, SYNC_SM, true);
  pio_sm_set_enabled(VGA_PIO, RGB_SM, true);

  multicore_launch_core1(vgaLoop);
}

