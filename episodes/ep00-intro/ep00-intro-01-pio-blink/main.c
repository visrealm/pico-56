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
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"

void blinkPinForever(PIO pio, uint offset, uint pin, float freq);

int main() {

  uint offset = pio_add_program(pio0, &blink_program);

  blinkPinForever(pio0, offset, PICO_DEFAULT_LED_PIN, 4.0f);

  // here we can do whatever we want. the led will continue to blink

  return 0;
}

void blinkPinForever(PIO pio, uint offset, uint pin, float freq) {
  uint sm = pio_claim_unused_sm(pio, true);

  blink_program_init(pio, sm, offset, pin);
  pio_sm_set_enabled(pio, sm, true);

  uint delay = (uint)(clock_get_hz(clk_sys) / (2.0f * freq)) - 3.0f;

  pio_sm_put(pio, sm, delay);
}
