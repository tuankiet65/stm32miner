#ifndef SYSTICK_H
    #define SYSTICK_H

    #include <stdint.h>

    #include <libopencm3/cm3/scb.h>
    #include <libopencm3/cm3/systick.h>

    #include "i2c.h"
    
    void systick_init(uint32_t mhz, uint32_t ms);

#endif
