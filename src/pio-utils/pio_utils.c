/*
 * Project: pico-56 - pio utilities
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */

#include "pio_utils.h"

#define BITS_PER_ITER 4
#define ITERATIONS (32 / BITS_PER_ITER)
#define BIT_MASK ((1 << BITS_PER_ITER) - 1)

#if BITS_PER_ITER * ITERATIONS != 32
#  error "BITS_PER_ITER must be a multiple of 32"
#endif

 /*
  * set a pio state machine x register
  */
void pio_set_x(PIO pio, uint sm, uint32_t x)
{
  // shift in 4 bits at a time (8 times)
  // we can only shift in up to 5 bits at a time
  // but doing 4 as it's easier (factor of 32)
  for (int i = 0; i < ITERATIONS; ++i, x >>= BITS_PER_ITER)
  {
    // set y to lowest 4 bits of value
    pio_sm_exec(pio, sm, pio_encode_set(pio_x, x & BIT_MASK));

    // shift the 4 bits into isr
    pio_sm_exec(pio, sm, pio_encode_in(pio_x, BITS_PER_ITER));
  }

  // copy isr into y register
  pio_sm_exec(pio, sm, pio_encode_mov(pio_x, pio_isr));
}


/*
 * set a pio state machine y register
 */
void pio_set_y(PIO pio, uint sm, uint32_t y)
{
  // shift in 4 bits at a time (8 times)
  // we can only shift in up to 5 bits at a time
  // but doing 4 as it's easier (factor of 32)
  for (int i = 0; i < ITERATIONS; ++i, y >>= BITS_PER_ITER)
  {
    // set y to lowest 4 bits of value
    pio_sm_exec(pio, sm, pio_encode_set(pio_y, y & BIT_MASK));

    // shift the 4 bits into isr
    pio_sm_exec(pio, sm, pio_encode_in(pio_y, BITS_PER_ITER));
  }

  // copy isr into y register
  pio_sm_exec(pio, sm, pio_encode_mov(pio_y, pio_isr));
}
