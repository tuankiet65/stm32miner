/*
 *  main.c
 *  Copyright (C) 2018 Ho Tuan Kiet
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>

#include "clock.h"
#include "sha256.h"
#include "logging.h"

#define PORT_LED GPIOA
#define PIN_LED GPIO4

static void gpio_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

// Header of block #443888
// Hash: 0000000000000000000cdc0d2a9b33c2d4b34b4d4fa8920f074338d0dc1164dc
// Winning nonce: 0x2e597ec6
static const uint32_t header[32] = {
    0x02000020, // Version
    
    0x14c2a9b7, 0x5c44c656, 0xd5720f69, 0x4c32d97a, // Previous block hash
    0xa3354d2d, 0x926a8001, 0x00000000, 0x00000000, //
    
    0xc28a2cdd, 0x8aeb39b6, 0x6bcf7906, 0xb26c9b1b, // Merkle root
    0x865dc50e, 0xd99243b1, 0x52f4a498, 0xe883f687, //
    
    0x0fa05558, // nTime
    
    0x858b0318, // Bits

    0xc67e0000, // starting nonce
    
    0x80000000, 0x00000000, 0x00000000, 0x00000000, //
    0x00000000, 0x00000000, 0x00000000, 0x00000000, // SHA-256 padding
    0x00000000, 0x00000000, 0x00000000, 0x00000280  //
};

int main() {
    rcc_clock_setup_in_hsi_out_64mhz();
    gpio_setup();
    log_init();

    uint32_t result;
    scanhash_sha256d(header, &result);

    return 0;
}
