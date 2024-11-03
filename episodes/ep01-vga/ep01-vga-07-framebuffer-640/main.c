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

#define FRAMEBUFFER_STRIDE (VGA_VIRTUAL_WIDTH / 2)

uint8_t __aligned(4) frameBuffer[VGA_VIRTUAL_HEIGHT * FRAMEBUFFER_STRIDE];
uint16_t __aligned(4) palette[16];
uint32_t __aligned(4) dpal[256];
int frame = 0;

void generateDpal()
{
  for (int i = 0; i < 256; ++i)
  {
    dpal[i] = (palette[(i & 0xf0) >> 4]) | palette[(i & 0x0f)] << 16;
  }
}


void frameBufferScanline(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH])
{
  uint8_t* src = frameBuffer + y * FRAMEBUFFER_STRIDE;
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

void setPixel(uint16_t x, uint16_t y, uint8_t c)
{
  int index = y * FRAMEBUFFER_STRIDE + x / 2;
  if (x & 1)
  {
    frameBuffer[index] = (frameBuffer[index] & 0xf0) | c;
  }
  else
  {
    frameBuffer[index] = (frameBuffer[index] & 0x0f) | (c << 4);
  }
}

void hline(uint16_t x, uint16_t y, uint16_t w, uint8_t c)
{
  uint16_t maxX = x + w;
  if (y >= VGA_VIRTUAL_HEIGHT) return;
  if (maxX >= VGA_VIRTUAL_WIDTH) maxX = VGA_VIRTUAL_WIDTH - 1;
  uint8_t* p = frameBuffer + y * FRAMEBUFFER_STRIDE + x / 2;
  uint8_t dc = (c << 4) | c;
  if (x & 1)
  {
    *p = (*p & 0xf0) | c;
    ++x;
    ++p;
  }

  int oneMore = 0;
  if ((maxX & 1) == 0)
  {
    --maxX;
    oneMore = 1;
  }

  while (x <= maxX)
  {
    *p = dc;
    ++p;
    x += 2;
  }

  if (oneMore)
  {
    *p = (*p & 0x0f) | (c << 4);
  }
}

void vline(uint16_t x, uint16_t y, uint16_t h, uint8_t c)
{
  uint16_t maxY = y + h;
  if (x >= VGA_VIRTUAL_WIDTH) return;
  if (maxY >= VGA_VIRTUAL_HEIGHT) maxY = VGA_VIRTUAL_HEIGHT - 1;
  uint8_t* p = frameBuffer + y * FRAMEBUFFER_STRIDE + x / 2;
  while (y++ <= maxY)
  {
    if (x & 1)
    {
      *p = (*p & 0xf0) | c;
    }
    else
    {
      *p = (*p & 0x0f) | (c << 4);
    }
    p += FRAMEBUFFER_STRIDE;
  }
}

void rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t c)
{
  hline(x, y, w, c);
  hline(x, y + h, w, c);
  vline(x, y, h, c);
  vline(x + w, y, h, c);
}

void filledRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t c)
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

  palette[0] = 0x0000;
  palette[1] = 0x0777;
  palette[2] = 0x0f00;
  palette[3] = 0x0ff0;
  palette[4] = 0x00f0;
  palette[5] = 0x00ff;
  palette[6] = 0x000f;
  palette[7] = 0x0f0f;
  palette[8] = 0x0800;
  palette[9] = 0x0880;
  palette[10] = 0x0080;
  palette[11] = 0x0088;
  palette[12] = 0x0008;
  palette[13] = 0x0808;
  palette[14] = 0x0aaa;
  palette[15] = 0x0ffff;
  generateDpal();


  VgaInitParams params;
  params.scanlineFn = frameBufferScanline;

  vgaInit(params);

  int lastFrame = 0;
  while (1)
  {
    if (lastFrame != frame)
    {
      for (int i = 0; i < 15; ++i)
      {
        palette[i] += 1;
      }
      generateDpal();
    }

    int rand1 = rand(), rand2 = rand();

    int x = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);
    int y = rand() % (int)(VGA_VIRTUAL_HEIGHT * 0.75);
    int w = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);
    int h = rand() % (int)(VGA_VIRTUAL_WIDTH * 0.75);

    filledRect(x, y, w, h, rand() & 0x0f);
    rect(x, y, w, h, 0x0f);

    lastFrame = frame;
  }

  return 0;
}
