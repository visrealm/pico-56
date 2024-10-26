/*
 * Project: pico-56 - audio
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "audio.h"
#include "emu2149.h"

#include "pico/stdlib.h"
#include "hardware/pwm.h"

#include <stdlib.h>

#define PWM_WRAP 1024

static PSG* psg0 = NULL;
static PSG* psg1 = NULL;
static uint audioSlice = 0;
static uint8_t psg0Reg = 0;
static uint8_t psg1Reg = 0;

/*
 * create a PSG 'object'
 */
static PSG* createPSG(int psgClock, int sampleRate)
{
  PSG* psg = PSG_new(psgClock, sampleRate);
  PSG_setVolumeMode(psg, EMU2149_VOL_AY_3_8910);  // AY-3-8910 mode
  PSG_reset(psg);
  return psg;
}

/*
 * initialize the audio
 */
void audioInit(int psgClock, int sampleRate)
{
  psg0 = createPSG(psgClock, sampleRate);
  psg1 = createPSG(psgClock, sampleRate);

  // output to a PWM pair (slice) pins 20 and 21
  const uint audioPin = 20;

  gpio_set_function(audioPin, GPIO_FUNC_PWM);
  gpio_set_function(audioPin + 1, GPIO_FUNC_PWM);

  audioSlice = pwm_gpio_to_slice_num(audioPin);

  // set up the PWM
  pwm_set_clkdiv_int_frac(audioSlice, 1, 0);
  pwm_set_wrap(audioSlice, PWM_WRAP);
  pwm_set_both_levels(audioSlice, 0, 0);
  pwm_set_enabled(audioSlice, true);
}

/*
 * update the audio
 *  - called at a frequency matching the sample rate
 */
void audioUpdate()
{
  // generate audio data
  PSG_calc(psg0);
  PSG_calc(psg1);

  // mix the 3 channels from each PSG
  uint16_t leftAudio = abs((((int32_t)psg0->ch_out[0] + (int32_t)psg1->ch_out[1] + (int32_t)psg0->ch_out[2] + (int32_t)psg1->ch_out[2]))) * PWM_WRAP >> 14;
  uint16_t rightAudio = abs((((int32_t)psg1->ch_out[0] + (int32_t)psg0->ch_out[1] + (int32_t)psg0->ch_out[2] + (int32_t)psg1->ch_out[2]))) * PWM_WRAP >> 14;

  // update PWM output level
  pwm_set_both_levels(audioSlice, leftAudio, rightAudio);
}

/*
 * read the current register value from psg0
 */
uint8_t audioReadPsg0()
{
  return PSG_readReg(psg0, psg0Reg);
}

/*
 * read the current register value from psg1
 */
uint8_t audioReadPsg1()
{
  return PSG_readReg(psg1, psg1Reg);
}

/*
 * write to psg0
 *  - lsb of addr determines BC1 signal
 */
void audioWritePsg0(uint16_t addr, uint8_t val)
{
  if ((addr & 0x01) == 0)
  {
    psg0Reg = val;
  }
  else
  {
    PSG_writeReg(psg0, psg0Reg, val);
  }
}

/*
 * write to psg1
 *  - lsb of addr determines BC1 signal
 */
void audioWritePsg1(uint16_t addr, uint8_t val)
{
  if ((addr & 0x01) == 0)
  {
    psg1Reg = val;
  }
  else
  {
    PSG_writeReg(psg1, psg1Reg, val);
  }
}
