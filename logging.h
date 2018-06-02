#ifndef LOGGING_H
	#define LOGGING_H
	
	#include <stdarg.h>
	#include <stdio.h>
	
	#include <libopencm3/stm32/rcc.h>
	#include <libopencm3/stm32/gpio.h>
	#include <libopencm3/stm32/usart.h>

	#include "mini_printf.h"

	#define INFO 1
	#define WARNING 2
	#define FATAL 3

	void log_init();
	void LOG(unsigned char level, const char msg[], ...);
#endif