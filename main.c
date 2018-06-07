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
    // Enable clock to IO port A
    RCC_AHBENR |= RCC_AHBENR_GPIOAEN;

    // Set LED light pin to output
    GPIOA_MODER |= GPIO_MODE(4, GPIO_MODE_OUTPUT);
    // Set LED light pin output speed to 100MHz
    GPIOA_OSPEEDR |= GPIO_OSPEED(4, GPIO_OSPEED_100MHZ);
}

uint32_t header[40];

enum i2c_variable_ids {
    new_job_id,
    new_header,
    execute_job,
    force_calibrate,

    version,
    hashrate,
    current_job_id,
    finished,
    winning_nonce
};

struct i2c_variable i2c_variables[] = {
    { .id = new_job_id     , .size = sizeof(unsigned char), .rw = I2C_RW },
    { .id = new_header     , .size = sizeof(uint32_t[20]) , .rw = I2C_RW },
    { .id = execute_job    , .size = sizeof(unsigned char), .rw = I2C_RW },
    { .id = force_calibrate, .size = sizeof(unsigned char), .rw = I2C_RW },

    { .id = version        , .size = sizeof(char[8])      , .rw = I2C_RO },
    { .id = hashrate       , .size = sizeof(uint32_t)     , .rw = I2C_RO },
    { .id = current_job_id , .size = sizeof(unsigned char), .rw = I2C_RO },
    { .id = finished       , .size = sizeof(unsigned char), .rw = I2C_RO },
    { .id = winning_nonce  , .size = sizeof(uint32_t)     , .rw = I2C_RO },
};

volatile unsigned char new_data = 0;

unsigned char ZERO = 0;
unsigned char ONE = 1;

void write_callback() {
    i2c_read(execute_job, &new_data);
}

int main() {
    rcc_clock_setup_in_hsi_out_64mhz();
    gpio_setup();
    log_init();

    i2c_init(0x69, 64, i2c_variables, sizeof(i2c_variables) / sizeof(struct i2c_variable));
    //i2c_write(version, GIT_VERSION);
    i2c_register_write_callback(write_callback);

    LOG(INFO, "stm32miner, commit "GIT_VERSION);
    LOG(INFO, "Ready, waiting for job");

    while (1) {
        // Critical section because I2C is interrupt-driven
        // Thus, to access I2C data it's necessary to disable
        // interrupt, or race condition will happen
        if (new_data) {
            CM_ATOMIC_BLOCK() {
                LOG(INFO, "New job");
                new_data = 0;
                i2c_write(execute_job, &ZERO);

                i2c_read(new_header, header);
                // SHA-256 padding
                // Its actual content is {
                //    0x80000000, 0x00000000, 0x00000000, 0x00000000,
                //    0x00000000, 0x00000000, 0x00000000, 0x00000000,
                //    0x00000000, 0x00000000, 0x00000000, 0x00000280 
                // } but we're short on program space
                header[20] = 0x80000000;
                header[31] = 0x00000280;

                unsigned char new_job_id = 0;
                i2c_read(new_job_id, &new_job_id);
                i2c_write(current_job_id, &new_job_id);

                i2c_write(finished, &ZERO);
                i2c_dump();
            }

            gpio_set(PORT_LED, PIN_LED);
            uint32_t result;
            if (scanhash_sha256d(header, &result, &new_data)) {
                i2c_write(finished, &ONE);
                i2c_write(winning_nonce, &result);
            }
            gpio_clear(PORT_LED, PIN_LED);
        }
    }

    return 0;
}
