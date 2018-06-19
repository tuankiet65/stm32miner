/*
 *  clock.c
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

#include "clock.h"

uint8_t rcc_init_hsi_pll_64() {
    /* Procedure: 
     *  - Enable and set HSI as system clock
     *  - Enable flash prefetch and set flash wait state to 1
     *  - Configure PLL clock source and multiplication factor
     *  - Enable and set PLL as system clock
     */

    // Turn HSI clock on
    RCC_CR |= RCC_CR_HSION;
    // Wait for HSI to be ready
    while (!(RCC_CR & RCC_CR_HSIRDY));
    // Set HSI as SYSCLK
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_HSI;

    // Enable flash prefetch and set wait state to 1
    // (because we're running at 64MHz)
    FLASH_ACR = (FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1WS);

    // Now we set PLL parameters
    // First we set multiplication factor to 16
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_PLLMUL) | RCC_CFGR_PLLMUL_MUL16;
    // Then we set PLL source to (HSI / 2)
    RCC_CFGR |= RCC_CFGR_PLLSRC_HSI_CLK_DIV2;
    // PLL clock rate is: HSI 8MHz /2 * 16 = 64MHz
    // Granted, this is much higher than the maximum allowed
    // clock rate of the STM32F030F4P6 (48MHz), but I have yet to
    // see any kind of instabilities, so let it be like that

    // Now we turn PLL on
    RCC_CR |= RCC_CR_PLLON;
    // Wait for PLL to be ready
    while (!(RCC_CR & RCC_CR_PLLRDY));
    // Then we set PLL as SYSCLK
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW) | RCC_CFGR_SW_PLL;

    // Set HCLK and PCLK divisor to 1 (aka SYSCLK and APB clock == PLL clock)
    RCC_CFGR = (RCC_CFGR & (~RCC_CFGR_HPRE)) | RCC_CFGR_HPRE_NODIV;
    RCC_CFGR = (RCC_CFGR & (~RCC_CFGR_PPRE)) | RCC_CFGR_PPRE_NODIV;

    return 64;
}
