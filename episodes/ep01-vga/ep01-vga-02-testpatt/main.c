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

int addColor(int r, int g, int b, int y, int count, uint16_t* pixels)
{
  r += (y - 120) * 2.125;
  g += (y - 120) * 2.125;
  b += (y - 120) * 2.125;
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;

  for (int i = 0; i < count; ++i)
  {
    pixels[i] = ((b & 0xf0) << 4) | ((g & 0xf0)) | ((r & 0xf0) >> 4);
  }

  return count;
}

int addColorMild(int r, int g, int b, int y, int count, uint16_t* pixels)
{
  r -= (y - 120) * 1.0625;
  g -= (y - 120) * 1.0625;
  b -= (y - 120) * 1.0625;
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;

  for (int i = 0; i < count; ++i)
  {
    pixels[i] = ((b & 0xf0) << 4) | ((g & 0xf0)) | ((r & 0xf0) >> 4);
  }

  return count;
}


void testPatternFunc(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  static int offset = 0;

  int x = 1;
  int r = 0, g = 0, b = 0;

  const uint16_t borderColor = 0x0f00;

  if (y == 0 || y == VGA_VIRTUAL_HEIGHT - 1)
  {
    for (int i = 0; i < VGA_VIRTUAL_WIDTH; ++i)
    {
      pixels[i] = borderColor;
    }
    return;
  }

  int pixelsPerBand = 11;

  x += addColorMild(127, 127, 127, y, 54, pixels + x);
  x += addColor(255, 0, 0, y, pixelsPerBand, pixels + x);
  x += addColor(255, 63, 0, y, pixelsPerBand, pixels + x);
  x += addColor(255, 127, 0, y, pixelsPerBand, pixels + x);
  x += addColor(255, 192, 0, y, pixelsPerBand, pixels + x);
  x += addColor(255, 255, 0, y, pixelsPerBand, pixels + x);
  x += addColor(192, 255, 0, y, pixelsPerBand, pixels + x);
  x += addColor(127, 255, 0, y, pixelsPerBand, pixels + x);
  x += addColor(63, 255, 0, y, pixelsPerBand, pixels + x);
  x += addColor(0, 255, 0, y, pixelsPerBand, pixels + x);
  x += addColor(0, 255, 63, y, pixelsPerBand, pixels + x);
  x += addColor(0, 255, 127, y, pixelsPerBand, pixels + x);
  x += addColor(0, 255, 192, y, pixelsPerBand, pixels + x);
  x += addColor(0, 255, 255, y, pixelsPerBand, pixels + x);
  x += addColor(0, 192, 255, y, pixelsPerBand, pixels + x);
  x += addColor(0, 127, 255, y, pixelsPerBand, pixels + x);
  x += addColor(0, 63, 255, y, pixelsPerBand, pixels + x);
  x += addColor(0, 0, 255, y, pixelsPerBand, pixels + x);
  x += addColor(63, 0, 255, y, pixelsPerBand, pixels + x);
  x += addColor(127, 0, 255, y, pixelsPerBand, pixels + x);
  x += addColor(192, 0, 255, y, pixelsPerBand, pixels + x);
  x += addColor(255, 0, 255, y, pixelsPerBand, pixels + x);
  x += addColor(255, 0, 192, y, pixelsPerBand, pixels + x);
  x += addColor(255, 0, 127, y, pixelsPerBand, pixels + x);
  x += addColor(255, 0, 63, y, pixelsPerBand, pixels + x);
}

int main(void)
{
  set_sys_clock_khz(126000, false);

  VgaInitParams params;
  params.scanlineFn = testPatternFunc;

  vgaInit(params);

  while (1)
    tight_loop_contents();

  return 0;
}
