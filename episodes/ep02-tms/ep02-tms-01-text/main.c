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

#include "vrEmuTms9918Util.h"

#include "pico/stdlib.h"
#include "hardware/clocks.h"

#include <stdlib.h>

extern const uint8_t tmsFont[];
extern size_t tmsFontBytes;

VrEmuTms9918* tms = NULL;

#define HMARGIN ((VGA_VIRTUAL_WIDTH - TMS9918_PIXELS_X) / 2)
#define VMARGIN ((VGA_VIRTUAL_HEIGHT - TMS9918_PIXELS_Y) / 2)

uint16_t __aligned(4) tmsPal[16];
uint8_t __aligned(4) tmsScanlineBuffer[TMS9918_PIXELS_X];

uint16_t colorFromRgb(uint16_t r, uint16_t g, uint16_t b)
{
  return ((uint16_t)(r / 16.0f) & 0x0f) | (((uint16_t)(g / 16.0f) & 0x0f) << 4) | (((uint16_t)(b / 16.0f) & 0x0f) << 8);
}

static void tmsScanline(uint16_t y, VgaParams* params, uint16_t* pixels)
{
  const uint32_t vBorder = (params->vVirtualPixels - TMS9918_PIXELS_Y) / 2;
  const uint32_t hBorder = (params->hVirtualPixels - TMS9918_PIXELS_X) / 2;

  uint16_t bg = tmsPal[vrEmuTms9918RegValue(tms, TMS_REG_FG_BG_COLOR) & 0x0f];

  if (y < vBorder || y >= (vBorder + TMS9918_PIXELS_Y))
  {
    for (int x = 0; x < params->hVirtualPixels; ++x)
    {
      pixels[x] = bg;
    }
    return;
  }

  y -= vBorder;

  for (int x = 0; x < hBorder; ++x)
  {
    pixels[x] = bg;
  }

  vrEmuTms9918ScanLine(tms, y, tmsScanlineBuffer);

  int tmsX = 0;
  for (int x = hBorder; x < hBorder + TMS9918_PIXELS_X; ++x, ++tmsX)
  {
    pixels[x] = tmsPal[tmsScanlineBuffer[tmsX]];
  }

  for (int x = hBorder + TMS9918_PIXELS_X; x < params->hVirtualPixels; ++x)
  {
    pixels[x] = bg;
  }
}


int main(void)
{
  set_sys_clock_khz(252000, false);

  tms = vrEmuTms9918New();

  for (int c = 0; c < 16; ++c)
  {
    uint32_t rgba8 = vrEmuTms9918Palette[c];
    tmsPal[c] = colorFromRgb((rgba8 & 0xff000000) >> 24, (rgba8 & 0xff0000) >> 16, (rgba8 & 0xff00) >> 8);
  }

  vrEmuTms9918InitialiseGfxI(tms);
  vrEmuTms9918SetAddressWrite(tms, TMS_DEFAULT_VRAM_COLOR_ADDRESS);
  for (int i = 0; i < 32; ++i)
  {
    vrEmuTms9918WriteData(tms, vrEmuTms9918FgBgColor(i & 0x0f, TMS_WHITE));
  }

  //  vrEmuTms9918WriteByteRpt(tms, vrEmuTms9918FgBgColor(TMS_BLACK, TMS_WHITE), 32);
  vrEmuTms9918SetAddressWrite(tms, TMS_DEFAULT_VRAM_PATT_ADDRESS + 32 * 8);
  vrEmuTms9918WriteBytes(tms, tmsFont, tmsFontBytes);

  vrEmuTms9918SetAddressWrite(tms, TMS_DEFAULT_VRAM_NAME_ADDRESS);
  vrEmuTms9918WriteString(tms, "Hello, vrEmuTms9918!");

  VgaInitParams params = { 0 };
  params.params = vgaGetParams(VGA_640_480_60HZ, 2);
  params.scanlineFn = tmsScanline;
  params.endOfFrameFn = NULL;

  vgaInit(params);

  int c = 32;

  while (1)
  {
    tight_loop_contents();
  }

  vrEmuTms9918Destroy(tms);

  return 0;
}
