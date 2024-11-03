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
#include "image.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include <memory.h>
#include <stdlib.h>

#define FRAMEBUFFER_STRIDE (VGA_VIRTUAL_WIDTH / 2)

uint32_t __aligned(4) dpal[256];
int frame = 0;

void generateDpal()
{
  for (int i = 0; i < 256; ++i)
  {
    dpal[i] = (birds_pal[(i & 0xf0) >> 4]) | birds_pal[(i & 0x0f)] << 16;
  }
}


void frameBufferScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  const uint8_t* src = birds + y * FRAMEBUFFER_STRIDE;
  uint32_t* dst = (uint32_t*)pixels;

  for (int x = 0; x < FRAMEBUFFER_STRIDE; ++x, ++src)
  {
    dst[x] = dpal[*src];
  }

  if (y == (VGA_VIRTUAL_HEIGHT - 1))
  {
    ++frame;
  }
}


int main(void)
{
  set_sys_clock_khz(240000, false);

  generateDpal();

  VgaInitParams params;
  params.scanlineFn = frameBufferScanline;

  vgaInit(params);

  while (1)
  {
    tight_loop_contents();
  }

  return 0;
}
