/*
 * Project: pico-56 - episode 0
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "pico/stdlib.h"

int main(void)
{
  // initialise the pin (standard io)
  gpio_init(PICO_DEFAULT_LED_PIN);

  // set as output
  gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

  while (1)
  {
    // on
    gpio_put(PICO_DEFAULT_LED_PIN, true);
    sleep_ms(500);

    // off
    gpio_put(PICO_DEFAULT_LED_PIN, false);
    sleep_ms(500);
  }

  return 0;
}
