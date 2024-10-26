/*
 * Project: pico-56 - PS/2 keyboard
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

char processAsciiToPs2(char c);

extern uint8_t ascii2Ps2[128];
