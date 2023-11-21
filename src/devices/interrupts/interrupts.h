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

void setOrClearInterrupt(int irq, bool doSet);

void raiseInterrupt(int irq);

void releaseInterrupt(int irq);

uint8_t intReg();
