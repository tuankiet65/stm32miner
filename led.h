#ifndef LED_H
    #define LED_H

    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/gpio.h>
    
    #ifndef PORT_LED
        #define PORT_LED GPIOA
    #endif
    #ifndef PIN_LED
        #define PIN_LED  GPIO4
    #endif

    void led_init();
    void led_toggle();
    void led_on();
    void led_off();
#endif
