/*
 *  clock.h
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

#ifndef CLOCK_H
    #define CLOCK_H
    
    #include <libopencm3/stm32/rcc.h>
    #include <libopencm3/stm32/flash.h>
    
    uint8_t rcc_init_hsi_pll_64();

#endif
