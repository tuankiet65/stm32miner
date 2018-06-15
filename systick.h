#ifndef SYSTICK_H
    #define SYSTICK_H

    #include <stdint.h>

    #include <libopencm3/cm3/scb.h>
    #include <libopencm3/cm3/nvic.h>
    #include <libopencm3/cm3/cortex.h>
    #include <libopencm3/cm3/systick.h>

    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/gpio.h>

    #include "i2c.h"
    #include "i2c_variables.h"
        
    void systick_init(uint32_t mhz);

#endif
