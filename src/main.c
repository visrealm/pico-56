/*
 * Project: pico-56
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "bus.h"
#include "boot-menu.h"

#include "pico/stdlib.h"

int main(void)
{
  // initialize stdio over usb serial
  stdio_init_all();

  // initialize the bus (all devices)
  busInit();

  // run  the PICO-56 boot menu
  runBootMenu();

  // it's go time!
  busMainLoop();

  return 0;
}
