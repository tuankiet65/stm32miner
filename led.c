/*
 *  led.c
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

#include "led.h"

void led_init() {
    // Enable clock to IO port A
    switch (PORT_LED) {
        case GPIOA:
            RCC_AHBENR |= RCC_AHBENR_GPIOAEN;
            break;
        case GPIOB:
            RCC_AHBENR |= RCC_AHBENR_GPIOBEN;
            break;
        case GPIOF:
            RCC_AHBENR |= RCC_AHBENR_GPIOFEN;
            break;
    };

    uint8_t pin_index = __builtin_ctz(PIN_LED);
    
    // Set LED light pin to output
    GPIO_MODER(PORT_LED) &= (~GPIO_MODE_MASK(pin_index));
    GPIO_MODER(PORT_LED) |= GPIO_MODE(pin_index, GPIO_MODE_OUTPUT);

    // Set LED light pin output speed to 100MHz
    // (heh, we're only going to turn it on or off every 1s)
    GPIO_OSPEEDR(PORT_LED) &= (~GPIO_OSPEED_MASK(pin_index));
    GPIO_OSPEEDR(PORT_LED) |= GPIO_OSPEED(pin_index, GPIO_OSPEED_100MHZ);

    // Clear the pullup/pulldown as we don't want the output
    // LED to be pulled up or down
    GPIO_PUPDR(PORT_LED) &= (~GPIO_PUPD_MASK(pin_index));
}

void led_toggle() {
    uint32_t led_odr = GPIO_ODR(PORT_LED);
    GPIO_BSRR(PORT_LED) = ((led_odr & PIN_LED) << 16) | ((~led_odr) & PIN_LED);
}

void led_on() {
    GPIO_BSRR(PORT_LED) = PIN_LED;
}

void led_off() {
    GPIO_BSRR(PORT_LED) = PIN_LED << 16;
}
