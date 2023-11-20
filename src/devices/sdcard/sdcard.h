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

#pragma once

#include "ff.h"
#include "sd_card.h"    

#ifdef __cplusplus
extern "C" {
#endif

  size_t sd_get_num();

  sd_card_t* sd_get_by_num(size_t num);

#ifdef __cplusplus
}
#endif