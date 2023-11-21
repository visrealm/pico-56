/*
 * Project: pico-56 - NES controller
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

bool nes_begin();

void nes_read_start(void);
void nes_read_finish(void);

uint8_t nes_get_state_1();
uint8_t nes_get_state_2();
