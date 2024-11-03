/*
 * Project: pico-56 - sdcard
 *
 * Copyright (c) 2023 Troy Schrapel
 *
 * This code is licensed under the MIT license
 *
 * https://github.com/visrealm/pico-56
 *
 */


#include "sdcard.h"

static spi_t spi = {
    .hw_inst = spi0,
    .sck_gpio = 18,
    .mosi_gpio = 19,
    .miso_gpio = 16,
    .baud_rate = 125 * 1000 * 1000 / 4,
//    .DMA_IRQ_num = DMA_IRQ_1,
    //.use_exclusive_DMA_IRQ_handler = 1
};

/* SPI Interface */
static sd_spi_if_t spi_if = {
    .spi = &spi,  // Pointer to the SPI driving this card
    .ss_gpio = 17      // The SPI slave select GPIO for this SD card
};

/* Configuration of the SD Card socket object */
static sd_card_t sd_card = {
  /* "pcName" is the FatFs "logical drive" identifier.
  (See http://elm-chan.org/fsw/ff/doc/filename.html#vol) */
  .type = SD_IF_SPI,
  .spi_if_p = &spi_if,  // Pointer to the SPI interface driving this card
  .use_card_detect = false
};

size_t sd_get_num()
{
  return 1;
}

sd_card_t* sd_get_by_num(size_t num) {
  if (0 == num)
  {
    return &sd_card;
  }
  else
  {
    return NULL;
  }
}