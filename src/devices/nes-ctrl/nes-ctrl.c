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

#include "nes.pio.h"

static PIO pio = pio1;
static uint8_t sm = -1;

#define NES_LATCH_GPIO 26
#define NES_CLOCK_GPIO 22
#define NES1_DATA_GPIO 27
#define NES2_DATA_GPIO 28

static uint8_t nes_state_1 = 0xff;
static uint8_t nes_state_2 = 0xff;

/*
 * return NES1 state
 */
uint8_t nes_get_state_1()
{
  return nes_state_1;
}

/*
 * return NES2 state
 */
uint8_t nes_get_state_2()
{
  return nes_state_2;
}

/*
 * start dual NES PIO
 */
bool nes_begin()
{
  if (pio_can_add_program(pio, &nes_program) &&
    ((sm = pio_claim_unused_sm(pio, true)) >= 0)) {
    uint offset = pio_add_program(pio, &nes_program);
    pio_sm_config c = nes_program_get_default_config(offset);

    sm_config_set_sideset_pins(&c, NES_CLOCK_GPIO);
    sm_config_set_in_pins(&c, NES1_DATA_GPIO);
    sm_config_set_set_pins(&c, NES_LATCH_GPIO, 1);
    pio_gpio_init(pio, NES_CLOCK_GPIO);
    pio_gpio_init(pio, NES1_DATA_GPIO);
    pio_gpio_init(pio, NES2_DATA_GPIO);
    pio_gpio_init(pio, NES_LATCH_GPIO);

    gpio_pull_up(NES1_DATA_GPIO);
    gpio_pull_up(NES2_DATA_GPIO);

    pio_sm_set_pindirs_with_mask(pio, sm,
      (1 << NES_CLOCK_GPIO) | (1 << NES_LATCH_GPIO), // Outputs
      (1 << NES_CLOCK_GPIO) | (1 << NES1_DATA_GPIO) | (1 << NES2_DATA_GPIO) |
      (1 << NES_LATCH_GPIO)); // All pins
    sm_config_set_in_shift(&c, false, true, 16); // L shift, autopush @ 16 bits

    sm_config_set_clkdiv(&c, 266.0);

    // IRQ is just a 'go' flag, don't assert a system interrupt
    pio_set_irq0_source_enabled(
      pio, (enum pio_interrupt_source)(pis_interrupt0 + sm), false);

    pio_sm_clear_fifos(pio, sm);

    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    return true; // Success
  }
  return false;
}

/*
 * initiate dual NES read
 */
void nes_read_start(void)
{
  pio_interrupt_clear(pio, 0);
}

/*
 * complete dual NES read
 */
void nes_read_finish(void)
{
  uint32_t value = (sm >= 0) ? pio_sm_get_blocking(pio, sm) : 0xffff;

  uint8_t value1 = 0x00, value2 = 0x00;
  uint8_t mask = 0x01;

  for (int i = 0; i < 8; ++i)
  {
    value1 |= value & mask;
    value >>= 1;
    value2 |= value & mask;
    mask <<= 1;
  }
  nes_state_1 = value1;
  nes_state_2 = value2;
}