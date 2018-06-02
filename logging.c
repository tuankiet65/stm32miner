#include "logging.h"

#ifdef DEBUG

const char* LEVEL_LABEL[] = {
	"INFO   ",
	"WARNING",
	"FATAL  "
};

void log_init() {
	rcc_periph_clock_enable(RCC_USART1);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, (GPIO9 | GPIO10));
	gpio_set_af(GPIOA, GPIO_AF1, (GPIO9 | GPIO10));

	usart_disable(USART1);

	usart_set_baudrate(USART1, 912600);
	usart_set_databits(USART1, 8);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_stopbits(USART1, USART_STOPBITS_1);

	usart_enable(USART1);
}

void LOG(unsigned char level, const char msg[], ...) {
	if (level > 3) return;

	char fmt[256], result[1024];

	mini_snprintf(fmt, sizeof(fmt), "[ %s ] %s\n", LEVEL_LABEL[level], msg);

	va_list args;
	va_start(args, msg);
	int len = mini_vsnprintf(result, sizeof(result), fmt, args);
	va_end(args);

	for (int i = 0; i < len; ++i) {
		usart_send_blocking(USART1, result[i]);
	}
}

#endif