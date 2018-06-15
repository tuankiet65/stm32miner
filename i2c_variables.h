/*
 *  i2c_variables.h
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

#ifndef I2C_VARIABLES_H
    #define I2C_VARIABLES_H

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

#endif
