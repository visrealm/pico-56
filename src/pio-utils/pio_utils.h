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

#pragma once

#include "hardware/pio.h"

void pio_set_xy(PIO pio, uint sm, uint32_t val, enum pio_src_dest dest);
#define pio_set_x(pio, sm, x) pio_set_xy(pio, sm, x, pio_x)
#define pio_set_y(pio, sm, y) pio_set_xy(pio, sm, y, pio_y)
