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
#include "vga-modes.h"
#include "slides.h"

#include "pico/stdlib.h"

#include <memory.h>
#include <stdlib.h>

const uint16_t* current = lorikeets;
const uint16_t* last = lorikeets;

void slideshowScanline(uint16_t y, VgaParams* params, uint16_t* pixels)
{
  static int frame = 0;
  static int subframe = 0;
  static int transition = 0;

  if (y == 0 && subframe == 0)
  {
    last = current;

    if (frame < 200) current = lorikeets;
    else if (frame < 400) current = hbc56;
    else if (frame < 600) current = toucan;
    else if (frame < 800) current = townhouses;
    else current = swirls;
    ++transition;
  }

  if (transition & 0x01)
  {
    if (subframe >= 20 || y <= (subframe * 12))
    {
      memcpy(pixels, current + (y * params->hVirtualPixels), params->hVirtualPixels * sizeof(uint16_t));
    }
    else
    {
      memcpy(pixels, last + (y * params->hVirtualPixels), params->hVirtualPixels * sizeof(uint16_t));
    }
  }
  else
  {
    int offset = subframe < 20 ? (320 - subframe * 16) : 0;

    memcpy(pixels + offset, current + (y * params->hVirtualPixels), (params->hVirtualPixels - offset) * sizeof(uint16_t));
    memcpy(pixels, last + (y * (params->hVirtualPixels)) - offset, offset * sizeof(uint16_t));
  }

  if (y == params->vVirtualPixels - 1)
  {
    ++frame;
    frame %= 1000;
    subframe = frame % 200;
  }
}


int main(void)
{
  set_sys_clock_khz(252000, false);

  VgaInitParams params = { 0 };
  params.params = vgaGetParams(VGA_640_480_60HZ, 2);
  params.scanlineFn = slideshowScanline;

  vgaInit(params);

  while (1)
  {
    tight_loop_contents();
  }

  return 0;
}
