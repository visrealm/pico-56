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

void rgbScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  uint16_t c = 0x0f00;
  if (y == 0 || y == VGA_VIRTUAL_HEIGHT - 1) c = 0x0fff;
  else if (y < 160) c = 0x000f;
  else if (y < 320) c = 0x00f0;
  for (int x = 1; x < VGA_VIRTUAL_WIDTH - 1; ++x)
  {
    pixels[x] = c;
  }
  pixels[0] = 0x0fff;
  pixels[VGA_VIRTUAL_WIDTH - 1] = 0x0fff;
}

int main(void)
{
  set_sys_clock_khz(126000, false);

  VgaInitParams params;
  params.scanlineFn = rgbScanline;

  vgaInit(params);

  while (1)
    tight_loop_contents();

  return 0;
}
