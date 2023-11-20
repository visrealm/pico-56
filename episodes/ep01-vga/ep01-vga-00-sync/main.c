/*
 * Project: pico-56 - episode 1
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "vga.h"

#include "pico/stdlib.h"

int main(void)
{
  vgaInit();

  while (1)
    tight_loop_contents();

  return 0;
}
