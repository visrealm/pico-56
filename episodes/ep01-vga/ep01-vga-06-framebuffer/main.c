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

#include <memory.h>
#include <stdlib.h>

uint16_t __aligned(4) frameBuffer[VGA_VIRTUAL_WIDTH * VGA_VIRTUAL_HEIGHT];
int frame = 0;

void frameBufferScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  memcpy(pixels, frameBuffer + y * VGA_VIRTUAL_WIDTH, VGA_VIRTUAL_WIDTH * sizeof(uint16_t));

  if (y == (VGA_VIRTUAL_HEIGHT - 1))
  {
    ++frame;
  }
}

void setPixel(uint16_t x, uint16_t y, uint16_t c)
{
  x %= VGA_VIRTUAL_WIDTH;
  y %= VGA_VIRTUAL_HEIGHT;
  frameBuffer[y * VGA_VIRTUAL_WIDTH + x] = c;
}


void hline(uint16_t x, uint16_t y, uint16_t w, uint16_t c)
{
  uint16_t maxX = x + w;
  if (y >= VGA_VIRTUAL_HEIGHT) return;
  if (maxX >= VGA_VIRTUAL_WIDTH) maxX = VGA_VIRTUAL_WIDTH - 1;
  uint16_t* p = frameBuffer + y * VGA_VIRTUAL_WIDTH + x;
  while (x++ <= maxX)
  {
    *p = c;
    ++p;
  }
}

void vline(uint16_t x, uint16_t y, uint16_t h, uint16_t c)
{
  uint16_t maxY = y + h;
  if (x >= VGA_VIRTUAL_WIDTH) return;
  if (maxY >= VGA_VIRTUAL_HEIGHT) maxY = VGA_VIRTUAL_HEIGHT - 1;
  uint16_t* p = frameBuffer + y * VGA_VIRTUAL_WIDTH + x;
  while (y++ <= maxY)
  {
    *p = c;
    p += VGA_VIRTUAL_WIDTH;
  }
}



void rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c)
{
  hline(x, y, w, c);
  hline(x, y + h, w, c);
  vline(x, y, h, c);
  vline(x + w, y, h, c);
}

void filledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c)
{
  uint16_t maxY = y + h;
  if (maxY >= VGA_VIRTUAL_HEIGHT) maxY = VGA_VIRTUAL_HEIGHT;

  while (y++ < maxY)
  {
    hline(x, y, w, c);
  }
}



int main(void)
{
  set_sys_clock_khz(252000, false);

  VgaInitParams params;
  params.scanlineFn = frameBufferScanline;

  vgaInit(params);

  while (1)
  {
    int x = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);
    int y = rand() % (int)(VGA_VIRTUAL_HEIGHT * 0.75);
    int w = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);
    int h = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);

    filledRect(x, y, w, h, rand() & 0x0fff);
    rect(x, y, w, h, 0x0fff);
  }

  return 0;
}
