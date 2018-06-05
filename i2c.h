#ifndef I2C_H
    #define I2C_H

    #include <string.h>
    #include <stdbool.h>

    #include <libopencm3/stm32/i2c.h>
    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/gpio.h>
    #include <libopencm3/cm3/nvic.h>

    #include "logging.h"

    enum i2c_rw_status {
        I2C_READ,
        I2C_READ_WRITE
    };

    struct i2c_variable {
        char *id;
        int size;
        enum i2c_rw_status rw;
    };

    void i2c_init(unsigned char addr, const struct i2c_variable variables[], const int len);

    bool i2c_ready();

    bool i2c_read(char variable_id[], volatile void *buf);
    bool i2c_write(char variable_id[], volatile const void *buf);

    void i2c_register_read_callback(void (*read_callback)());
    void i2c_register_write_callback(void (*write_callback)());
    
    // Interrupt handler
    void i2c1_isr();

    void i2c_dump();
#endif
