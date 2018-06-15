#include "systick.h"

void systick_init(uint32_t mhz) {
    // Enable clock to IO port A
    RCC_AHBENR |= RCC_AHBENR_GPIOAEN;

    // Set LED light pin to output
    GPIOA_MODER &= (~GPIO_MODE_MASK(4));
    GPIOA_MODER |= GPIO_MODE(4, GPIO_MODE_OUTPUT);

    // Set LED light pin output speed to 2MHz
    // (heh, we're only going to turn it on or off every 1s)
    GPIOA_OSPEEDR &= (~GPIO_OSPEED_MASK(4));
    GPIOA_OSPEEDR |= GPIO_OSPEED(4, GPIO_OSPEED_2MHZ);

    // Enable SysTick, triggering every 1s, clock source = (HCLK / 8)
    // Small rant: PM0215, pg. 86.
    //  * CLKSOURCE = 0: External reference clock = (HCLK / 8)
    //  * CLKSOURCE = 1: Processor clock = (HCLK)
    // Why we don't check for maximum reload value here:
    // Let's just say that the maximum possible clock is 96MHz
    // (maximum stable clock, with 8MHz HSE + 12x PLL)
    // This means that the SysTick clock runs at 12MHz, smaller
    // then the maximum reload value (0x00ffffff == 16777215)
    // So we don't really need to check whether the clockrate
    // is valid or not. PM0215, pg.87
    //STK_RVR = (64000000 / 10) - 1;
    STK_RVR = ((mhz * 1000000) >> 3) - 1;
    STK_CVR = 0;
    STK_CSR = STK_CSR_ENABLE | STK_CSR_TICKINT | STK_CSR_CLKSOURCE_EXT;

    // Configure SysTick exception priority to the lowest possible
    // (because this SysTick isn't doing anything important, so best
    //  to prioritize other interrupts over this)
    // Small rant: **Device interrupt priority** are configured using
    // the NVIC registers, while **Exception handler priority** are configured
    // using the System Control Block registers. PM0215, pg.77
    // However the note doesn't even inform that fact to the reader,
    // I only figured it out from reading CMSIS's NVIC_SetPriority.
    // Yep, the function name is that misleading
    // Also the SysTick handler is an exception handler i.e. you don't need
    // to enable it, it's always on
    // Also when writing 0xff, the first 5 bits is ignored i.e
    // the value 192 is understood
    // SCB_SHPR(SCB_SHPR_PRI_15_SYSTICK / 4) => SCB_SHPR3
    // ((SCB_SHPR_PRI_15_SYSTICK % 4) << 3)  => LSB position of PRI_15 in SCB_SHPR3
    SCB_SHPR(SCB_SHPR_PRI_15_SYSTICK / 4) |= 0xff << ((SCB_SHPR_PRI_15_SYSTICK % 4) << 3);
}
