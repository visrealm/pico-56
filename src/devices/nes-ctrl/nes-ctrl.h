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

bool nes_begin(uint8_t clkPin, uint8_t dataPin, uint8_t latPin);

void nes_read_start(void);

void nes_read_finish(void);

uint8_t nes_get_state_1();
uint8_t nes_get_state_2();
