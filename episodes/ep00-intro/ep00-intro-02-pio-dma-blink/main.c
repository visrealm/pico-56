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
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "dma-blink.pio.h"
#include <math.h>
#include <stdlib.h>


#define LED_BUFFER_SIZE 256
uint32_t __aligned(LED_BUFFER_SIZE * 4) ledBuffer[LED_BUFFER_SIZE];

void blinkPinDma(PIO pio, uint offset, uint pin, uint32_t buffer[LED_BUFFER_SIZE]);
void ledPwmBufferSine(uint32_t buffer[LED_BUFFER_SIZE]);
void ledPwmBufferTriangle(uint32_t buffer[LED_BUFFER_SIZE]);
void ledPwmBufferSawtooth(uint32_t buffer[LED_BUFFER_SIZE]);
void ledPwmBufferRandom(uint32_t buffer[LED_BUFFER_SIZE]);

int main() {

  uint offset = pio_add_program(pio0, &blink_program);

  ledPwmBufferSine(ledBuffer);

  blinkPinDma(pio0, offset, PICO_DEFAULT_LED_PIN, ledBuffer);

  while (1)
  {
    sleep_ms(10000);
    ledPwmBufferTriangle(ledBuffer);
    sleep_ms(10000);
    ledPwmBufferSawtooth(ledBuffer);
    sleep_ms(10000);
    ledPwmBufferRandom(ledBuffer);
    sleep_ms(10000);
    ledPwmBufferSine(ledBuffer);
  }

  return 0;
}

void ledPwmBufferSine(uint32_t buffer[LED_BUFFER_SIZE]) {
  for (int i = 0; i < LED_BUFFER_SIZE; i += 2)
  {
    buffer[i] = (sin(i / (double)LED_BUFFER_SIZE * M_PI * 2.0) + 1.0) * 1000000;
    buffer[i + 1] = 2000000 - buffer[i];

    buffer[i] += 1;
    buffer[i + 1] += 1;
  }
}

void ledPwmBufferTriangle(uint32_t buffer[LED_BUFFER_SIZE]) {
  for (int i = 0; i < LED_BUFFER_SIZE; i += 2)
  {
    buffer[i] = fabs((i - (LED_BUFFER_SIZE / 2)) / ((double)LED_BUFFER_SIZE / 2.0)) * 2000000;
    buffer[i + 1] = 2000000 - buffer[i];

    buffer[i] += 1;
    buffer[i + 1] += 1;
  }
}

void ledPwmBufferSawtooth(uint32_t buffer[LED_BUFFER_SIZE]) {
  for (int i = 0; i < LED_BUFFER_SIZE; i += 2)
  {
    buffer[i] = i / (double)LED_BUFFER_SIZE * 1000000;
    buffer[i + 1] = 1000000 - buffer[i];

    buffer[i] += 1;
    buffer[i + 1] += 1;
  }
}


void ledPwmBufferRandom(uint32_t buffer[LED_BUFFER_SIZE]) {
  for (int i = 0; i < LED_BUFFER_SIZE; i += 2)
  {
    buffer[i] = (rand() / (double)RAND_MAX) * 1000000;
    buffer[i + 1] = 1000000 - buffer[i];

    buffer[i] += 1;
    buffer[i + 1] += 1;
  }
}

void blinkPinDma(PIO pio, uint offset, uint pin, uint32_t buffer[LED_BUFFER_SIZE]) {

  uint sm = pio_claim_unused_sm(pio, true);

  blink_program_init(pio, sm, offset, pin);

  int dmaChan = dma_claim_unused_channel(true);
  dma_channel_config dmaChanConfig = dma_channel_get_default_config(dmaChan);
  channel_config_set_transfer_data_size(&dmaChanConfig, DMA_SIZE_32);            // 32 bits at a time
  channel_config_set_read_increment(&dmaChanConfig, true);                       // increment read
  channel_config_set_write_increment(&dmaChanConfig, false);                     // don't increment write 
  channel_config_set_dreq(&dmaChanConfig, pio_get_dreq(pio, sm, true)); // transfer when there's space in fifo
  channel_config_set_ring(&dmaChanConfig, false, 10);

  // setup the dma channel and set it going
  dma_channel_configure(dmaChan, &dmaChanConfig, &pio->txf[sm], buffer, ~0, true);

  pio_sm_set_enabled(pio, sm, true);
}
