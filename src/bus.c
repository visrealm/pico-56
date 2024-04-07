/*
 * Project: pico-56 - the bus
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "vrEmu6502.h"
#include "vrEmu6522.h"
#include "tms9918.h"
#include "audio.h"
#include "nes-ctrl.h"
#include "ps2-kbd.h"
#include "sdcard.h"

#include "interrupts.h"
#include "config.h"

#include "bus.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include <stdlib.h>
#include <stdio.h>

 // HBC-56 RAM
static uint8_t __aligned(4) ram[HBC56_RAM_SIZE];

// HBC-56 ROM
extern uint8_t pico56rom[];

// ROM access
uint8_t* romPtr()
{
  return pico56rom;
}
size_t romSize()
{
  return HBC56_ROM_SIZE;
}

static VrEmu6502* cpu = NULL;
static VrEmu6522* via = NULL;
static VrEmuTms9918* tms9918 = NULL;

#define HBC56_CLOCK_FREQ_MHZ 3.686400 /* half of 7.3728*/
#define MICROSECONDS_PER_BURST  50
#define TICKS_PER_BURST (int)(MICROSECONDS_PER_BURST * HBC56_CLOCK_FREQ_MHZ)

#define FOPEN_PORT 0x04
#define FCLOSE_PORT 0x04
#define FREAD_PORT 0x05
#define FWRITE_PORT 0x05

FIL fil;

#define CPU_6502_WAI 0xcb

#define UART_STATUS_RX_REG_FULL       0b00000001
#define UART_STATUS_TX_REG_EMPTY      0b00000010

static uint8_t uartControl = 0;
static uint8_t uartStatus = UART_STATUS_TX_REG_EMPTY;
static uint8_t uartBuffer = 0;


/*
 * 65c02 bus read/write callbacks
 */
void busWrite(uint16_t addr, uint8_t val);
uint8_t busRead(uint16_t addr, bool isDbg);

static bool capsOn = false;   // 4
static bool numOn = false;    // 2
static bool scrollOn = false; // 1

/*
 * called at the end of each frame
 */
static void endOfFrameCb(uint64_t frameNumber)
{
  static uint8_t lastCode = 0;

  static uint8_t writeQueue[2] = { 0, 0 };
  static uint8_t writeQueueSize = 0;

  if (writeQueueSize)
  {
    ps2kbd_write(writeQueue[--writeQueueSize]);
    writeQueue[writeQueueSize] = 0;
  }
  else
  {
    // update keyboard state
    uint8_t kbdScancode = ps2kbd_read();
    if (kbdScancode != 0)
    {
      kbdQueuePush(kbdScancode);

      if (lastCode != 0xf0)
      {
        if (kbdScancode == 0x58 ||  // caps
          kbdScancode == 0x7e ||  // scroll
          kbdScancode == 0x77)  // num
        {
          if (kbdScancode == 0x58) capsOn = !capsOn;
          if (kbdScancode == 0x7e) scrollOn = !scrollOn;
          if (kbdScancode == 0x77) numOn = !numOn;
          writeQueue[0] = (capsOn ? 0x04 : 0x00) | (scrollOn ? 0x01 : 0x00) | (numOn ? 0x02 : 0x00);
          writeQueue[1] = 0xed;
          writeQueueSize = 2;
        }
      }
      lastCode = kbdScancode;
    }
  }

  // update nes state
  nes_read_finish();
  nes_read_start();
}

/*
 * initialize the bus / hardware / devices
*/
void busInit()
{
  // 65C02 cpu
  cpu = vrEmu6502New(CPU_W65C02, busRead, busWrite);

  // 65C22 VIA
  via = vrEmu6522New(VIA_65C22);

  // TMS9918A VDP
  tms9918 = tmsInit();
  tmsSetFrameCallback(endOfFrameCb);

  // Dual AY-3-8910 PSGs
  audioInit(HBC56_AY38910_CLOCK, tmsGetHsyncFreq());
  tmsSetHsyncCallback(audioUpdate);

  // PS/2 keyboard
  ps2kbd_begin();

  // dual NES controllers
  nes_begin();
  nes_read_start();
}

/*
 * the main loop
 */
void __not_in_flash_func(busMainLoop)()
{
  /* reset the cpu (technically don't need to do this as vrEmu6502New does reset it) */
  vrEmu6502Reset(cpu);

  capsOn = numOn = scrollOn = false;

  // revert keyboard 'locks'
  ps2kbd_write(0xed);
  sleep_ms(10);
  ps2kbd_write(0x00);

  absolute_time_t startTime = get_absolute_time();
  absolute_time_t currentTime = startTime;
  absolute_time_t nextUartTime = startTime;

  int i = 0;
  int prevViaInt = IntCleared;

  // loop forever
  while (1)
  {
    // run the cpu for a number of ticks
    while (i < TICKS_PER_BURST)
    {
      int cycleTicks = vrEmu6502InstCycle(cpu);
      if (vrEmu6502GetCurrentOpcode(cpu) == CPU_6502_WAI)
      {
        i += TICKS_PER_BURST;
        break;
      }
      i += cycleTicks;
    }
    i -= TICKS_PER_BURST;

    // run the via for a number of ticks
    vrEmu6522Ticks(via, TICKS_PER_BURST);

    if (uartBuffer == 0)
    {
      nextUartTime = delayed_by_us(currentTime, 100);
      int c = getchar_timeout_us(0);
      if (c != PICO_ERROR_TIMEOUT)
      {
        //        putchar(c);
        uartBuffer = c;// & 0xff;
        raiseInterrupt(HBC56_UART_IRQ);
        uartStatus = UART_STATUS_TX_REG_EMPTY;
        uartStatus |= UART_STATUS_RX_REG_FULL;
      }
      else
      {
        releaseInterrupt(HBC56_UART_IRQ);
        uartStatus &= ~(UART_STATUS_RX_REG_FULL);
      }
    }


    // has the via interrupted?
    setOrClearInterrupt(HBC56_VIA_IRQ, *vrEmu6522Int(via) == IntRequested);

    // set cpu interrupt
    *(vrEmu6502Int(cpu)) = intReg() ? IntRequested : IntCleared;

    // delay or continue immediately to keep cpu clock
    currentTime = delayed_by_us(currentTime, MICROSECONDS_PER_BURST);
    if (to_us_since_boot(currentTime) < to_us_since_boot(get_absolute_time()))
    {
      currentTime = get_absolute_time();
    }
    else
    {
      busy_wait_until(currentTime);
    }
  }
}


/*
 * 65c02 write to the bus
 */
void __not_in_flash_func(busWrite)(uint16_t addr, uint8_t val)
{
  // ram?
  if (addr < HBC56_IO_START)
  {
    ram[addr] = val;
  }

  // io?
  else if (addr <= HBC56_ROM_START)
  {
    if ((addr & HBC56_VIA_PORT) == HBC56_VIA_PORT)
    {
      vrEmu6522Write(via, addr & 0x0f, val);
      setOrClearInterrupt(HBC56_VIA_IRQ, *vrEmu6522Int(via) == IntRequested);
    }
    else
    {
      switch (addr & 0xff)
      {
        case HBC56_TMS9918_PORT:
          vrEmuTms9918WriteData(tms9918, val);
          break;

        case HBC56_TMS9918_PORT | 0x01:
          vrEmuTms9918WriteAddr(tms9918, val);
          break;

        case HBC56_AY38910_A_PORT:
        case HBC56_AY38910_A_PORT | 0x01:
          audioWritePsg0(addr, val);
          break;

        case HBC56_AY38910_B_PORT:
        case HBC56_AY38910_B_PORT | 0x01:
          audioWritePsg1(addr, val);
          break;

        case HBC56_UART_PORT:
          uartControl = val;
          if ((val & 0x03) == 0x03)   // reset
          {
            uartStatus = UART_STATUS_TX_REG_EMPTY;
          }
          else
          {
            releaseInterrupt(HBC56_UART_IRQ);
          }
          break;

        case HBC56_UART_PORT | 0x01:
          putchar(val);
          break;

        case FOPEN_PORT:
          {
            uint16_t addr = ram[val] | (ram[val + 1] << 8);
            char* strAddr = ram + addr;
            f_open(&fil, strAddr, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
          }
          break;

        case FWRITE_PORT:
          {
            uint bw = 0;
            f_write(&fil, &val, 1, &bw);
          }
          break;

        default:
          // unknown device
          break;
      }
    }
  }
}

#define KB_INT_FLAG 0x02
#define KB_RDY_FLAG 0x04

/*
 * 65c02 read from the bus
 */
uint8_t __not_in_flash_func(busRead)(uint16_t addr, bool isDbg)
{
  // rom?
  if (addr & 0x8000)
  {
    return pico56rom[addr & (HBC56_ROM_SIZE - 1)];
  }

  // ram?
  else if (addr < HBC56_IO_START)
  {
    return ram[addr];
  }

  // io?
  else
  {
    if ((addr & HBC56_VIA_PORT) == HBC56_VIA_PORT)
    {
      uint8_t value = vrEmu6522Read(via, addr & 0x0f);
      setOrClearInterrupt(HBC56_VIA_IRQ, *vrEmu6522Int(via) == IntRequested);
      return value;
    }
    else
    {
      switch (addr & 0xff)
      {
        case HBC56_TMS9918_PORT:
          {
            return vrEmuTms9918ReadData(tms9918);
          }

        case HBC56_TMS9918_PORT | 0x01:
          {
            uint8_t value = vrEmuTms9918ReadStatus(tms9918);
            releaseInterrupt(HBC56_TMS9918_IRQ);
            return value;
          }

        case HBC56_KB_PORT:
          {
            uint8_t value = 0;
            if (!kbdQueueEmpty())
            {
              value = kbdQueuePop();
            }
            return value;
          }
        case HBC56_KB_PORT | 0x01:
          {
            return (!kbdQueueEmpty())
              ? (KB_INT_FLAG | KB_RDY_FLAG)
              : 0;
          }
        case HBC56_NES_PORT:
          return nes_get_state_1();

        case HBC56_NES_PORT | 0x01:
          return nes_get_state_2();

        case HBC56_IRQ_PORT:
          return intReg();

        case HBC56_AY38910_A_PORT | 0x02:
          return audioReadPsg0();

        case HBC56_AY38910_B_PORT | 0x02:
          return audioReadPsg1();

        case HBC56_UART_PORT:
          return uartStatus;

        case HBC56_UART_PORT | 0x01:
          {
            int c = getchar_timeout_us(0);
            if (c == PICO_ERROR_TIMEOUT)
            {
              releaseInterrupt(HBC56_UART_IRQ);
              uartStatus &= ~(UART_STATUS_RX_REG_FULL);
              c = 0;
            }
            uint8_t val = uartBuffer;
            uartBuffer = c;
            return val;
          }

        case FCLOSE_PORT:
          f_close(&fil);
          return 0;

        case FREAD_PORT:
          {
            uint br = 0;
            uint8_t value = 0;
            if (f_read(&fil, &value, 1, &br) != FR_OK)
            {
              value = 0;
            }
            return value;
          }

        default:
          return 0x00;
      }
    }
  }
}
