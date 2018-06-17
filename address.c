/*
 *  address.c
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

#include "address.h"

uint8_t get_address() {
    // Bit config:
    // PA7 PA6 PA5 PA4 PA1 PA0
    // MSB     <----->     LSB
    RCC_AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA_MODER &= (~GPIO_MODE_MASK(0)) & 
                   (~GPIO_MODE_MASK(1)) & 
                   (~GPIO_MODE_MASK(4)) &
                   (~GPIO_MODE_MASK(5)) & 
                   (~GPIO_MODE_MASK(6)) & 
                   (~GPIO_MODE_MASK(7));
    GPIOA_MODER |= GPIO_MODE(0, GPIO_MODE_INPUT) |
                   GPIO_MODE(1, GPIO_MODE_INPUT) |
                   GPIO_MODE(4, GPIO_MODE_INPUT) |
                   GPIO_MODE(5, GPIO_MODE_INPUT) |
                   GPIO_MODE(6, GPIO_MODE_INPUT) |
                   GPIO_MODE(7, GPIO_MODE_INPUT);
    GPIOA_PUPDR &= (~GPIO_PUPD_MASK(0)) & 
                   (~GPIO_PUPD_MASK(1)) & 
                   (~GPIO_PUPD_MASK(4)) & 
                   (~GPIO_PUPD_MASK(5)) & 
                   (~GPIO_PUPD_MASK(6)) & 
                   (~GPIO_PUPD_MASK(7));
    GPIOA_PUPDR |= GPIO_PUPD(0, GPIO_PUPD_PULLUP) |
                   GPIO_PUPD(1, GPIO_PUPD_PULLUP) |
                   GPIO_PUPD(4, GPIO_PUPD_PULLUP) |
                   GPIO_PUPD(5, GPIO_PUPD_PULLUP) |
                   GPIO_PUPD(6, GPIO_PUPD_PULLUP) |
                   GPIO_PUPD(7, GPIO_PUPD_PULLUP);

    uint32_t gpioa = GPIOA_IDR;
    uint8_t address = (
       ((gpioa & (GPIO7 | GPIO6 | GPIO5 | GPIO4)) >> 2) |
       ((gpioa & (                GPIO1 | GPIO0))     )
    );

    LOG("get_address: GPIOA is 0x%04x", gpioa);
    LOG("get_address: PA7 is %d", (gpioa & GPIO7) ? (1) : (0));
    LOG("get_address: PA6 is %d", (gpioa & GPIO6) ? (1) : (0));
    LOG("get_address: PA5 is %d", (gpioa & GPIO5) ? (1) : (0));
    LOG("get_address: PA4 is %d", (gpioa & GPIO4) ? (1) : (0));
    LOG("get_address: PA1 is %d", (gpioa & GPIO1) ? (1) : (0));
    LOG("get_address: PA0 is %d", (gpioa & GPIO0) ? (1) : (0));
    LOG("get_address: address is 0x%02x", address);
    
    return address;
}
