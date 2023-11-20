/*
 * Project: pico-56 - vga
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

typedef struct
{
  uint32_t displayPixels;
  uint32_t frontPorchPixels;
  uint32_t syncPixels;
  uint32_t backPorchPixels;
  uint32_t totalPixels;
  float freqHz;
  bool syncHigh;
} VgaSyncParams;

typedef struct
{
  uint32_t pixelClockKHz;
  VgaSyncParams hSyncParams;
  VgaSyncParams vSyncParams;
  uint32_t hPixelScale;
  uint32_t vPixelScale;
  uint32_t hVirtualPixels;
  uint32_t vVirtualPixels;
  float pioDivider;
  float pioFreqKHz;
  float pioClocksPerPixel;
  float pioClocksPerScaledPixel;
} VgaParams;

typedef enum
{
  VGA_640_480_60HZ,
  VGA_640_400_70HZ,
  VGA_800_600_60HZ,
  VGA_1024_768_60HZ,
  VGA_1280_1024_60HZ,
} VgaMode;

typedef void (*vgaScanlineRgbFn)(uint16_t y, VgaParams* params, uint16_t* pixels);
typedef void (*vgaEndOfFrameFn)(uint64_t frameNumber);
typedef void (*vgaEndOfScanlineFn)();

extern VgaParams vgaGetParams(VgaMode mode, int pixelScale);

extern uint32_t vgaMinimumPioClockKHz(VgaParams* params);

typedef struct
{
  VgaParams params;
  vgaScanlineRgbFn scanlineFn;
  vgaEndOfFrameFn endOfFrameFn;
  vgaEndOfScanlineFn endOfScanlineFn;
} VgaInitParams;


void vgaInit(VgaInitParams params);

VgaInitParams vgaCurrentParams();