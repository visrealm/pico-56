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
#include <stdbool.h>

#define KBD_INT 2

bool ps2kbd_begin(uint8_t clkPin, uint8_t dataPin);
uint8_t ps2kbd_read();
void ps2kbd_write(uint8_t value);

extern bool isShifted;
extern bool isControlled;

extern bool kbdQueueEmpty();

extern void kbdQueuePush(uint8_t scancode);

extern uint8_t kbdQueuePop();

extern char processAsciiToPs2(char c);

extern uint8_t ascii2Ps2[128];
