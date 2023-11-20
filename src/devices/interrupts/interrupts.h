/*
 * Project: pico-56 - interrupt handler
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

extern void setOrClearInterrupt(int irq, bool doSet);

extern void raiseInterrupt(int irq);

extern void releaseInterrupt(int irq);

extern uint8_t intReg();
