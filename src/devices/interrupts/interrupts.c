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
#include "interrupts.h"

static uint8_t interruptRegister = 0;

void setOrClearInterrupt(int irq, bool doSet)
{
  --irq;
  interruptRegister = (interruptRegister & ~(1 << irq)) | (doSet << irq);
}


void raiseInterrupt(int irq)
{
  interruptRegister |= (1 << (irq - 1));
}

void releaseInterrupt(int irq)
{
  interruptRegister &= ~(1 << (irq - 1));
}

uint8_t intReg()
{
  return interruptRegister;
}

