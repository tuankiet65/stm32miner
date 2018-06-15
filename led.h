/*
 *  led.h
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
