#include "address.h"

uint8_t get_address() {
    // Bit config:
    // PA7 PA6 PA5 PA1 PA0
    // MSB    <--->    LSB
    RCC_AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA_MODER &= (~GPIO_MODE_MASK(0)) & 
                   (~GPIO_MODE_MASK(1)) & 
                   (~GPIO_MODE_MASK(5)) & 
                   (~GPIO_MODE_MASK(6)) & 
                   (~GPIO_MODE_MASK(7));
    GPIOA_MODER |= GPIO_MODE(0, GPIO_MODE_INPUT) |
                   GPIO_MODE(1, GPIO_MODE_INPUT) |
                   GPIO_MODE(5, GPIO_MODE_INPUT) |
                   GPIO_MODE(6, GPIO_MODE_INPUT) |
                   GPIO_MODE(7, GPIO_MODE_INPUT);
    GPIOA_PUPDR &= (~GPIO_PUPD_MASK(0)) & 
                   (~GPIO_PUPD_MASK(1)) & 
                   (~GPIO_PUPD_MASK(5)) & 
                   (~GPIO_PUPD_MASK(6)) & 
                   (~GPIO_PUPD_MASK(7));
    GPIOA_PUPDR |= GPIO_PUPD(0, GPIO_PUPD_PULLDOWN) |
                   GPIO_PUPD(1, GPIO_PUPD_PULLDOWN) |
                   GPIO_PUPD(5, GPIO_PUPD_PULLDOWN) |
                   GPIO_PUPD(6, GPIO_PUPD_PULLDOWN) |
                   GPIO_PUPD(7, GPIO_PUPD_PULLDOWN);

    uint32_t gpioa = GPIOA_IDR;

    return (
       ((gpioa & (GPIO_IDR(5) | GPIO_IDR(6) | GPIO_IDR(7))) >> 3) |
        (gpioa & (GPIO_IDR(0) | GPIO_IDR(1)))
    );
}
