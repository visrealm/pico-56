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

#ifndef _PICO56_VGA_H
#define _PICO56_VGA_H

#include <inttypes.h>

#define VGA_VIRTUAL_WIDTH  640
#define VGA_VIRTUAL_HEIGHT 480

typedef void (*vgaScanlineFn)(uint16_t y, uint16_t pixels[VGA_VIRTUAL_WIDTH]);

typedef struct
{
  vgaScanlineFn scanlineFn;
} VgaInitParams;


void vgaInit(VgaInitParams params);

#endif