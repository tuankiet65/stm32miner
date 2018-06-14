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

#include "i2c.h"
#include "clock.h"
#include "sha256.h"
#include "logging.h"
#include "address.h"

#ifndef GIT_VERSION
    #define GIT_VERSION "unknown"
#endif

uint32_t header[40];

enum i2c_variable_ids {
    version = 0,
    state,
    hashrate,
    current_job_id,
    winning_nonce,

    new_job_id,
    new_header,
    execute_job,
    force_calibrate
};

#define STATE_READY 0x01
#define STATE_WORKING 0x02
#define STATE_FOUND 0x03
#define STATE_NOT_FOUND 0x04
#define STATE_ERROR 0xff

struct i2c_variable i2c_variables[] = {
    { .id = version        , .size = sizeof(char[8])     , .rw = I2C_RO },
    { .id = state          , .size = sizeof(uint8_t)     , .rw = I2C_RO },
    { .id = hashrate       , .size = sizeof(uint32_t)    , .rw = I2C_RO },
    { .id = current_job_id , .size = sizeof(uint8_t)     , .rw = I2C_RO },
    { .id = winning_nonce  , .size = sizeof(uint32_t)    , .rw = I2C_RO },

    { .id = new_job_id     , .size = sizeof(uint8_t)     , .rw = I2C_RW },
    { .id = new_header     , .size = sizeof(uint32_t[20]), .rw = I2C_RW },
    { .id = execute_job    , .size = sizeof(uint8_t)     , .rw = I2C_RW },
    { .id = force_calibrate, .size = sizeof(uint8_t)     , .rw = I2C_RW },
};

volatile uint8_t new_data = 0;

void write_callback() {
    i2c_read(execute_job, &new_data);
}

int main() {
    rcc_clock_setup_in_hsi_out_64mhz();
    log_init();
    
    i2c_init(get_address(), 64, i2c_variables, sizeof(i2c_variables) / sizeof(struct i2c_variable));
    i2c_register_write_callback(write_callback);
    i2c_write(version, GIT_VERSION);
    i2c_write_uint8(state, STATE_READY);

    LOG(INFO, "stm32miner, commit "GIT_VERSION);
    LOG(INFO, "Ready, waiting for job");

    while (1) {
        // Critical section because I2C is interrupt-driven
        // Thus, to access I2C data it's necessary to disable
        // interrupt, or race condition will happen
        if (new_data) {
            CM_ATOMIC_BLOCK() {
                i2c_write_uint8(execute_job, 0);
                i2c_write_uint8(current_job_id, 0);
                i2c_write_uint32(hashrate, 0);
                i2c_write_uint32(winning_nonce, 0);

                LOG(INFO, "New job");
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

                uint8_t new_job_id = 0;
                i2c_read(new_job_id, &new_job_id);
                i2c_write(current_job_id, &new_job_id);
                
                i2c_write_uint8(state, STATE_WORKING);
                i2c_dump();
            }

            uint32_t result;
            if (scanhash_sha256d(header, &result, &new_data)) {
                i2c_write_uint8(state, STATE_FOUND);
                i2c_write(winning_nonce, &result);
            } else {
                i2c_write_uint8(state, STATE_NOT_FOUND);
            }
        }
    }

    return 0;
}
