/*
 * Project: pico-56 - boot menu input
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#pragma once

typedef enum
{
  BMI_NONE,
  BMI_UP,
  BMI_DOWN,
  BMI_LEFT,
  BMI_RIGHT,
  BMI_PGUP,
  BMI_PGDOWN,
  BMI_SELECT,
} BootMenuInput;

BootMenuInput currentInput();