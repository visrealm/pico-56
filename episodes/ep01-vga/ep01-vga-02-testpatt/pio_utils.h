/*
 * Project: pico-56 - pio utilities
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#ifndef _PICO56_PIO_UTILS_H
#define _PICO56_PIO_UTILS_H

#include "hardware/pio.h"

void pio_set_y(PIO pio, uint sm, uint32_t y);

#endif