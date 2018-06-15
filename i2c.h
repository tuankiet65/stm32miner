/*
 *  i2c.h
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

#ifndef I2C_H
    #define I2C_H

    #include <string.h>
    #include <stdbool.h>

    #include <libopencm3/cm3/nvic.h>
    #include <libopencm3/cm3/cortex.h>
    
    #include <libopencm3/stm32/i2c.h>
    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/gpio.h>

    #include "logging.h"

    enum i2c_states {
        I2C_ADDR_MATCH,
        I2C_READ,
        I2C_WRITE
    };

    enum i2c_rw_status {
        I2C_RO,
        I2C_RW
    };
    
    struct i2c_variable {
        uint8_t id;
        uint8_t size;
        enum i2c_rw_status rw;
    };

    void i2c_init(uint8_t addr, uint8_t mhz,
                  const struct i2c_variable variables[], const uint8_t len);

    bool i2c_read(uint8_t id, volatile void *buf);
    bool i2c_write(uint8_t id, volatile const void *buf);

    void i2c_write_uint8(uint8_t id, volatile uint8_t val);
    void i2c_write_uint32(uint8_t id, volatile uint32_t val);

    void i2c_register_write_callback(void (*write_callback)());
    
    // Interrupt handler
    void i2c1_isr();

    #ifdef DEBUG
        void i2c_dump();
    #else
        #define i2c_dump() ;
    #endif
#endif
