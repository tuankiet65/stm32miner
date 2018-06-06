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

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>

#include "i2c.h"
#include "clock.h"
#include "sha256.h"
#include "logging.h"

#ifndef GIT_VERSION
    #define GIT_VERSION "unknown"
#endif

#define PORT_LED GPIOA
#define PIN_LED GPIO4

static void gpio_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

// // Header of block #443888
// // Hash: 0000000000000000000cdc0d2a9b33c2d4b34b4d4fa8920f074338d0dc1164dc
// // Winning nonce: 0x2e597ec6
// static uint32_t header[32] = {
//     0x02000020, // Version
    
//     0x14c2a9b7, 0x5c44c656, 0xd5720f69, 0x4c32d97a, // Previous block hash
//     0xa3354d2d, 0x926a8001, 0x00000000, 0x00000000, //
    
//     0xc28a2cdd, 0x8aeb39b6, 0x6bcf7906, 0xb26c9b1b, // Merkle root
//     0x865dc50e, 0xd99243b1, 0x52f4a498, 0xe883f687, //
    
//     0x0fa05558, // nTime
    
//     0x858b0318, // Bits

//     0x00000000, // starting nonce
    
//     0x80000000, 0x00000000, 0x00000000, 0x00000000, //
//     0x00000000, 0x00000000, 0x00000000, 0x00000000, // SHA-256 padding
//     0x00000000, 0x00000000, 0x00000000, 0x00000280  //
// };

uint32_t header[40];
const uint32_t sha256_padding[20] = {
    0x80000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000280 
};

const struct i2c_variable i2c_variables[] = {
    { .id = "new_job_id"     , .size = sizeof(unsigned char), .rw = I2C_READ_WRITE },
    { .id = "new_header"     , .size = sizeof(uint32_t[20]) , .rw = I2C_READ_WRITE },
    { .id = "execute_job"    , .size = sizeof(unsigned char), .rw = I2C_READ_WRITE },
    { .id = "force_calibrate", .size = sizeof(unsigned char), .rw = I2C_READ_WRITE },

    { .id = "version"        , .size = sizeof(char[8])      , .rw = I2C_READ },
    { .id = "hashrate"       , .size = sizeof(uint32_t)     , .rw = I2C_READ },
    { .id = "current_job_id" , .size = sizeof(unsigned char), .rw = I2C_READ },
    { .id = "finished"       , .size = sizeof(unsigned char), .rw = I2C_READ },
    { .id = "winning_nonce"  , .size = sizeof(uint32_t)     , .rw = I2C_READ },
};

volatile unsigned char new_data = 0;

const unsigned char ZERO = 0;
const unsigned char ONE = 1;

void write_callback() {
    i2c_read("execute_job", &new_data);
}

int main() {
    rcc_clock_setup_in_hsi_out_64mhz();
    gpio_setup();
    log_init();

    i2c_init(0x69, i2c_variables, sizeof(i2c_variables) / sizeof(struct i2c_variable));
    i2c_write("version", GIT_VERSION);
    i2c_register_write_callback(write_callback);

    LOG(INFO, "Ready, waiting for job");

    while (1) {
        // Critical section because I2C is interrupt-driven
        // Thus, to access I2C data it's necessary to disable
        // interrupt, or race condition will happen
        if (new_data) {
            CM_ATOMIC_BLOCK() {
                LOG(INFO, "New job");
                new_data = 0;
                i2c_write("execute_job", &ZERO);

                i2c_read("new_header", header);
                memcpy(header + 20, sha256_padding, sizeof(sha256_padding));

                unsigned char new_job_id = 0;
                i2c_read("new_job_id", &new_job_id);
                i2c_write("current_job_id", &new_job_id);

                i2c_write("finished", &ZERO);
                i2c_dump();
            }

            uint32_t result;
            if (scanhash_sha256d(header, &result, &new_data)) {
                i2c_write("finished", &ONE);
                i2c_write("winning_nonce", &result);
            }
        }
    }

    return 0;
}
