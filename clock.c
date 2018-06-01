#include "clock.h"

void rcc_clock_setup_in_hsi_out_64mhz(void) {
    rcc_osc_on(RCC_HSI);
    rcc_wait_for_osc_ready(RCC_HSI);
    rcc_set_sysclk_source(RCC_HSI);

    rcc_set_hpre(RCC_CFGR_HPRE_NODIV);
    rcc_set_ppre(RCC_CFGR_PPRE_NODIV);

    flash_prefetch_enable();
    flash_set_ws(FLASH_ACR_LATENCY_024_048MHZ);

    /* 8MHz * 16 / 2 = 64MHz */
    rcc_set_pll_multiplication_factor(RCC_CFGR_PLLMUL_MUL16);
    rcc_set_pll_source(RCC_CFGR_PLLSRC_HSI_CLK_DIV2);

    rcc_osc_on(RCC_PLL);
    rcc_wait_for_osc_ready(RCC_PLL);
    rcc_set_sysclk_source(RCC_PLL);

    rcc_apb1_frequency = 8000000 * 16 / 2;
    rcc_ahb_frequency = rcc_apb1_frequency;
}