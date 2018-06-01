#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>

#include "clock.h"
#include "sha256.h"
#include "logging.h"

#define PORT_LED GPIOA
#define PIN_LED GPIO4

static void gpio_setup(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    gpio_mode_setup(PORT_LED, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, PIN_LED);
}

// Header of block #443888
// Hash: 0000000000000000000cdc0d2a9b33c2d4b34b4d4fa8920f074338d0dc1164dc
// Winning nonce: 0x2e597ec6
uint32_t __attribute__((section(".data"))) header[] = {
    0x20000002, // version

    0xb7a9c214, 0x56c6445c, 0x690f72d5, 0x7ad9324c, // Previous block header
    0x2d4d35a3, 0x01806a92, 0x00000000, 0x00000000, // 
    
    0xdd2c8ac2, 0xb639eb8a, 0x0679cf6b, 0x1b9b6cb2, // Merkle root
    0x0ec55d86, 0xb14392d9, 0x98a4f452, 0x87f683e8, //

    0x5855a00f, // nTime
    
    0x18038b85, // Bits aka targer
    
    0x00000000  // Starting nonce
};

uint32_t scanhash_sha256d(struct work *work, uint32_t max_nonce)
{
	uint32_t data[64];
	uint32_t hash[8];
	uint32_t midstate[8];
	uint32_t prehash[8];
	uint32_t *pdata = work->data;
	uint32_t *ptarget = work->target;
	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];
	uint32_t n = pdata[19] - 1;
	
	memcpy(data, pdata + 16, 64);
	sha256d_preextend(data);
	
	sha256_init(midstate);
	sha256_transform(midstate, pdata);
	memcpy(prehash, midstate, 32);
	sha256d_prehash(prehash, pdata + 16);
	
    int count = 0;
	do {
		data[3] = ++n;
		sha256d_ms(hash, data, midstate, prehash);
        sha256d_80_swap(hash, pdata);
        LOG(DEBUG, "nonce: 0x%08x, first byte: 0x%08x", pdata[19], hash[7]);
        pdata[19]++;
		if (unlikely(hash[7] == 0x00000000)) {
			return;
			//sha256d_80_swap(hash, pdata);
		}
        count++;
        if (count == 1000) {
            gpio_toggle(PORT_LED, PIN_LED);
            count = 0;
        }
	} while (likely(n < max_nonce));
	return 0;
}

struct work work;

int main(void)
{
    rcc_clock_setup_in_hsi_out_64mhz();
    gpio_setup();
    log_init();

    for (int i = 0; i < 20; ++i) work.data[i] = header[i];
    
    scanhash_sha256d(&work, 0xffffffff);

    return 0;
}