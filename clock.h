#ifndef CLOCK_H
    #define CLOCK_H
    
    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/flash.h>
    
    void rcc_clock_setup_in_hsi_out_64mhz(void);

#endif