/*
 * Project: pico-56 - boot menu input
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "input.h"

#include "ps2-kbd.h"
#include "nes-ctrl.h"

#include "pico/stdlib.h"

 /*
  * get input for boot menu (accepts keyboard or nes)
  */
BootMenuInput currentInput()
{
  static uint8_t lastScancode = 0;

  BootMenuInput input = BMI_NONE;

  if (!kbdQueueEmpty())
  {
    uint8_t scancode = kbdQueuePop();
    if (lastScancode != 0xf0)
    {
      switch (scancode)
      {
        case 0x72: input = BMI_DOWN; break;
        case 0x75: input = BMI_UP; break;
        case 0x6b: input = BMI_LEFT; break;
        case 0x74: input = BMI_RIGHT; break;
        case 0x7d: input = BMI_PGUP; break;
        case 0x7a: input = BMI_PGDOWN; break;
        case 0x29:
        case 0x5a: input = BMI_SELECT; break;
      }
    }
    lastScancode = scancode;
  }

  uint8_t nes1state = nes_get_state_1();

  if ((nes1state & 0x04) == 0)
  {
    input = BMI_DOWN;
  }
  else if ((nes1state & 0x08) == 0)
  {
    input = BMI_UP;
  }
  if ((nes1state & 0x01) == 0)
  {
    input = BMI_RIGHT;
  }
  else if ((nes1state & 0x02) == 0)
  {
    input = BMI_LEFT;
  }
  else if ((nes1state & 0xf0) != 0xf0) // any non d-pad button
  {
    input = BMI_SELECT;
  }

  return input;
}