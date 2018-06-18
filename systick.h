/*
 *  systick.h
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

#ifndef SYSTICK_H
    #define SYSTICK_H

    #include <stdint.h>

    #include <libopencm3/cm3/scb.h>
    #include <libopencm3/cm3/systick.h>
    
    void systick_init(uint32_t mhz, uint32_t ms);

#endif
