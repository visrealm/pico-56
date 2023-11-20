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

#include "interrupts.h"
#include "config.h"

#include "bus.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include <stdlib.h>

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

#define NES_LATCH_GPIO 26
#define NES_CLOCK_GPIO 22
#define NES1_DATA_GPIO 27
#define NES2_DATA_GPIO 28

#define HBC56_CLOCK_FREQ_MHZ 3.686400 /* half of 7.3728*/
#define MICROSECONDS_PER_BURST  50
#define TICKS_PER_BURST (int)(MICROSECONDS_PER_BURST * HBC56_CLOCK_FREQ_MHZ)

#define CPU_6502_WAI 0xcb

void busWrite(uint16_t addr, uint8_t val);
uint8_t busRead(uint16_t addr, bool isDbg);

/* called at the end of each frame */
static void endOfFrameCb(uint64_t frameNumber)
{
  nes_read_finish();
  nes_read_start();
}

/*
 * initialize the bus / hardware / devices
*/
void busInit()
{
  cpu = vrEmu6502New(CPU_W65C02, busRead, busWrite);

  via = vrEmu6522New(VIA_65C22);

  tms9918 = tmsInit();
  tmsSetFrameCallback(endOfFrameCb);
  tmsSetHsyncCallback(audioUpdate);

  audioInit(HBC56_AY38910_CLOCK, tmsGetHsyncFreq());

  ps2kbd_begin(15, 14);
  nes_begin(NES_CLOCK_GPIO, NES1_DATA_GPIO, NES_LATCH_GPIO);
  nes_read_start();
}

/*
 * the main loop
 */
void busMainLoop()
{
  vrEmu6502Reset(cpu);

  double burstCount = 0;
  int charPos = 0;
  bool hasConnected = false;

  if (cpu)
  {
    /* reset the cpu (technically don't need to do this as vrEmu6502New does reset it) */
    vrEmu6502Reset(cpu);

    absolute_time_t startTime = get_absolute_time();
    absolute_time_t currentTime = startTime;

    int i = 0;
    int prevViaInt = IntCleared;

    while (1)
    {
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

      vrEmu6522Ticks(via, TICKS_PER_BURST);

      setOrClearInterrupt(HBC56_VIA_IRQ, *vrEmu6522Int(via) == IntRequested);

      static uint8_t lastCode = 0;
      uint8_t kbdScancode = ps2kbd_read();
      static bool capsOn = false; // 4
      static bool numOn = false;  // 2
      static bool scrollOn = false; //1

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
            sleep_ms(10);
            ps2kbd_write(0xed);
            sleep_ms(10);
            ps2kbd_write((capsOn ? 0x04 : 0x00) | (scrollOn ? 0x01 : 0x00) | (numOn ? 0x02 : 0x00));
          }
        }
        lastCode = kbdScancode;
      }

      *(vrEmu6502Int(cpu)) = intReg() ? IntRequested : IntCleared;

      currentTime = delayed_by_us(currentTime, MICROSECONDS_PER_BURST);
      if (currentTime < get_absolute_time())
      {
        currentTime = get_absolute_time();
      }
      else
      {
        busy_wait_until(currentTime);
      }
    }

    vrEmu6502Destroy(cpu);
    cpu = NULL;
  }
}

void __not_in_flash_func(busWrite)(uint16_t addr, uint8_t val)
{
  if (addr < HBC56_IO_START)
  {
    ram[addr] = val;
  }
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

        default:
          // unknown device
          break;
      }
    }
  }
}

#define KB_INT_FLAG 0x02
#define KB_RDY_FLAG 0x04

uint8_t __not_in_flash_func(busRead)(uint16_t addr, bool isDbg)
{
  if (addr & 0x8000)
  {
    return pico56rom[addr & (HBC56_ROM_SIZE - 1)];
  }
  else if (addr < HBC56_IO_START)
  {
    return ram[addr];
  }
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

        default:
          return 0x00;
      }
    }
  }
}