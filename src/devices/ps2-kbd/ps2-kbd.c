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

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include "pico/time.h"
#include "ps2-kbd.pio.h"

#include "ps2-kbd.h"
#include "interrupts.h"

#define PS2_DATA_GPIO  14
#define PS2_CLOCK_GPIO 15

#define KB_QUEUE_SIZE  16
#define KB_QUEUE_MASK  (KB_QUEUE_SIZE - 1)

char      kbQueue[KB_QUEUE_SIZE];
int       kbStart = 0;
int       kbEnd = 0;

static PIO ps2kbd_pio = pio1;
static uint8_t ps2kbd_sm = -1;

/*
 * start PS/2 keyboard PIO
 */
bool ps2kbd_begin()
{
  if (pio_can_add_program(ps2kbd_pio, &ps2kbd_program) &&
    ((ps2kbd_sm = pio_claim_unused_sm(ps2kbd_pio, true)) >= 0)) {
    uint offset = pio_add_program(ps2kbd_pio, &ps2kbd_program);
    pio_sm_config c = ps2kbd_program_get_default_config(offset);

    pio_gpio_init(ps2kbd_pio, PS2_CLOCK_GPIO);
    pio_gpio_init(ps2kbd_pio, PS2_DATA_GPIO);
    sm_config_set_in_pins(&c, PS2_DATA_GPIO);
    sm_config_set_set_pins(&c, PS2_DATA_GPIO, 2);
    sm_config_set_out_pins(&c, PS2_DATA_GPIO, 1);
    sm_config_set_jmp_pin(&c, PS2_CLOCK_GPIO);

    pio_sm_set_pindirs_with_mask(ps2kbd_pio, ps2kbd_sm,
      0, // Outputs
      (1 << PS2_CLOCK_GPIO) | (1 << PS2_DATA_GPIO)); // All pins*/

    sm_config_set_in_shift(&c, true, true, 11);  // R shift, autopush @ 11 bits
    sm_config_set_out_shift(&c, true, true, 32); // R shift, autopull @ 12 bits

    sm_config_set_clkdiv(&c, 1000.0);

    pio_sm_clear_fifos(ps2kbd_pio, ps2kbd_sm);

    pio_sm_init(ps2kbd_pio, ps2kbd_sm, offset, &c);
    pio_sm_set_enabled(ps2kbd_pio, ps2kbd_sm, true);

    return true; // Success
  }
  return false;
}

/*
 * read from PS/2 keyboard PIO FIFO queue
 */
uint8_t ps2kbd_read() {
  if (!pio_sm_is_rx_fifo_empty(ps2kbd_pio, ps2kbd_sm))
  {
    uint32_t value32 = pio_sm_get_blocking(ps2kbd_pio, ps2kbd_sm);
    value32 >>= 21;

    uint8_t value = (value32 >> 1) & 0xff;

    if (value == 0xaa)
    {
      sleep_us(350);
      ps2kbd_write(0xfa);
      return 0;
    }
    else if (value == 0xfe)
    {
      return 0;
      // resend?
    }
    else if (value == 0xfa)
    {
      return 0;
      // ack
    }
    else
    {
      return value;
    }
  }

  return 0;
}

/*
 * write to PS/2 keyboard PIO FIFO queue
 */
void ps2kbd_write(uint8_t value) {
  uint32_t value32 = value;

  uint8_t bitsSet = 0;
  for (int i = 0; i < 8; ++i)
  {
    bitsSet += ((value >> i) & 0x01);
  }

  value32 |= 0x600;
  if ((bitsSet & 0x01) == 0)
  {
    value32 |= 0x100;
  }

  pio_sm_put_blocking(ps2kbd_pio, ps2kbd_sm, (~value32));
}

/*
 * is the queue empty?
 */
bool kbdQueueEmpty()
{
  return kbEnd == kbStart;
}

/*
 * push a scancode to the queue
 */
void kbdQueuePush(uint8_t scancode)
{
  kbQueue[kbEnd++] = scancode; kbEnd &= KB_QUEUE_MASK;
  raiseInterrupt(KBD_INT);
}

/*
 * push a scancode from the queue
 */
uint8_t kbdQueuePop()
{
  uint8_t val = kbQueue[kbStart++];
  kbStart &= KB_QUEUE_MASK;
  if (kbdQueueEmpty())
    releaseInterrupt(KBD_INT);
  return val;
}
