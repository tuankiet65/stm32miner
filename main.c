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

#include <libopencm3/cm3/cortex.h>

#include "i2c.h"
#include "led.h"
#include "clock.h"
#include "sha256.h"
#include "logging.h"
#include "address.h"
#include "systick.h"
#include "i2c_variables.h"

#ifndef GIT_VERSION
    #define GIT_VERSION "unknown"
#endif

uint32_t header[32];

volatile uint8_t new_data;
volatile uint32_t *nonce_ptr, last_nonce;
volatile uint32_t calculated_hashrate;
volatile uint8_t counter;

void sys_tick_handler() {
    if (!nonce_ptr) return;

    counter++;
    if (counter == 20) {
        calculated_hashrate = (*nonce_ptr) - last_nonce;
        last_nonce = (*nonce_ptr);
        i2c_write(hashrate, &calculated_hashrate);
        LOG("Hashrate: %d hash/s", calculated_hashrate);
        counter = 0;
    }

    led_toggle();
}

void write_callback() {
    i2c_read(execute_job, &new_data);
}

int main() {
    uint8_t clockrate = rcc_init_hsi_pll_64();
    log_init();
    systick_init(clockrate, 50);
    led_init();

    i2c_init(get_address(), clockrate, i2c_variables, sizeof(i2c_variables) / sizeof(struct i2c_variable));
    i2c_register_write_callback(write_callback);
    i2c_write(version, GIT_VERSION);
    i2c_write_uint8(state, STATE_READY);

    LOG("stm32miner, commit "GIT_VERSION);
    LOG("Ready, waiting for job");

    while (1) {
        led_on();
        nonce_ptr = NULL;

        if (!new_data) continue;
        
        // Critical section because I2C is interrupt-driven
        // Thus, to access I2C data it's necessary to disable
        // interrupt, or race condition will happen

        CM_ATOMIC_BLOCK() {
            i2c_write_uint8(execute_job, 0);
            i2c_write_uint8(current_job_id, 0);
            i2c_write_uint32(hashrate, 0);
            i2c_write_uint32(winning_nonce, 0);

            LOG("New job");
            new_data = 0;

            i2c_read(new_header, header);
            // SHA-256 padding
            // Its actual content is {
            //    0x80000000, 0x00000000, 0x00000000, 0x00000000,
            //    0x00000000, 0x00000000, 0x00000000, 0x00000000,
            //    0x00000000, 0x00000000, 0x00000000, 0x00000280 
            // } but we're short on program space
            header[20] = 0x80000000;
            header[31] = 0x00000280;
            
            uint8_t job_id = 0;
            i2c_read(new_job_id, &job_id);
            i2c_write(current_job_id, &job_id);
            
            last_nonce = header[19];

            i2c_write_uint8(state, STATE_WORKING);
            i2c_dump();
        }

        uint32_t result;
        if (scanhash_sha256d(header, &result, &new_data, &nonce_ptr)) {
            i2c_write_uint8(state, STATE_FOUND);
            i2c_write(winning_nonce, &result);
        } else {
            i2c_write_uint8(state, STATE_NOT_FOUND);
        }
    }
    
    return 0;
}
