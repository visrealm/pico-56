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

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include <math.h>

int __aligned(4) sinTable[VGA_VIRTUAL_WIDTH];

static float frequency = 1.0 / 32;  // Adjust this value to change the frequency of the sine wave
static float amplitude = 75.0;     // Adjust this value to change the amplitude of the sine wave

void generateSinTable() {
  float offset = 120.0;     // Adjust this value to change the amplitude of the sine wave

  for (int x = 0; x < VGA_VIRTUAL_WIDTH; x++) {
    float angle = 2 * M_PI * frequency * x;
    sinTable[x] = offset + amplitude * sin(angle);
  }
}

static int frameCounter = 0;
static uint8_t r = 0x0f;
static uint8_t g = 0x00;
static uint8_t b = 0x00;

void interpolatedSineWave(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  if (y > 40 && y < 200)
  {
    for (int x = 1; x < VGA_VIRTUAL_WIDTH - 1; x++) {
      int value = sinTable[x];

      if (y == value) {
        pixels[x] = ((b & 0x0f) << 8) | ((g & 0x0f) << 4) | ((r & 0x0f));
      }
      else if ((sinTable[x - 1] < y && sinTable[x] > y) || (sinTable[x - 1] > y && sinTable[x] < y))
      {

        pixels[x] = ((b & 0x0c) << 8) | ((g & 0x0c) << 4) | ((r & 0x0c));
      }
      else
      {
        pixels[x] = 0x0000;  // Set pixel value to black otherwise
      }
    }
  }
  else if (y == 200)
  {
    frequency = sin(frameCounter / 3525.0);
    amplitude = sin(frameCounter / 50.0) * 75;
    generateSinTable();

    if (frameCounter % 10 == 0)
    {
      r -= 1;
      g += 1;
      b += 3;
    }
    ++frameCounter;
  }
}

int main(void)
{
  set_sys_clock_khz(252000, false);

  VgaInitParams params;
  params.scanlineFn = interpolatedSineWave;

  generateSinTable();

  vgaInit(params);

  int lastFrameCounter = 0;

  while (1)
  {
    sleep_us(100);
  }

  return 0;
}
