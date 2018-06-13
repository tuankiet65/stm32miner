#include "i2c.h"

static volatile unsigned i2c_ptr;

static enum i2c_rw_status i2c_register_rw[256];
static unsigned i2c_register_size;

static volatile unsigned char i2c_register[256];

static void (*write_callback)() = NULL;

static const struct i2c_variable *i2c_variables;
static int i2c_variables_len;

static enum i2c_states i2c_state = I2C_ADDR_MATCH;

void i2c_init_peripheral(unsigned char addr, unsigned char mhz) {
    // Enable GPIOA clock
    RCC_AHBENR |= RCC_AHBENR_GPIOAEN;
    // Enable I2C1 clock
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Configure PA9 and PA10 to AF4 (I2C1_SCL/I2C1_SDA)
    // First, configure PA9/PA10 to alternative function mode
    GPIOA_MODER &= (~GPIO_MODE_MASK(9)) & (~GPIO_MODE_MASK(10));
    GPIOA_MODER |= (GPIO_MODE(9, GPIO_MODE_AF) | GPIO_MODE(10, GPIO_MODE_AF));
    // Configure PA9/PA10 to pull up
    GPIOA_PUPDR &= (~GPIO_PUPD_MASK(9)) & (~GPIO_PUPD_MASK(10));
    GPIOA_PUPDR |= (GPIO_PUPD(9, GPIO_PUPD_PULLUP) | GPIO_PUPD(10, GPIO_PUPD_PULLUP));
    // Configure PA9 and PA10 to AF4 (I2C1_SCL/I2C1_SDA)
    GPIOA_AFRH  &= (~GPIO_AFR_MASK(9 - 8)) & (~GPIO_AFR_MASK(10 - 8));
    GPIOA_AFRH  |= (GPIO_AFR(9 - 8, GPIO_AF4) | GPIO_AFR(10 - 8, GPIO_AF4));

    // First, disable I2C
    // After disabling I2C, all I2C related registers
    // are put back into its reset value
    I2C_CR1(I2C1) = 0x00000000;

    // Now we set the I2C timing
    // We're aiming for Fast mode (400kbps)
    unsigned char presc = mhz / 8 - 1;
    I2C_TIMINGR(I2C1) = ((10 - 1) << I2C_TIMINGR_SCLL_SHIFT)   | // SCL low period
                        (( 4 - 1) << I2C_TIMINGR_SCLH_SHIFT)   | // SCL high period
                        ((     3) << I2C_TIMINGR_SDADEL_SHIFT) | // Data hold time
                        (( 4 - 1) << I2C_TIMINGR_SCLDEL_SHIFT) | // Data setup time
                        (( presc) << I2C_TIMINGR_PRESC_SHIFT);   // Prescaler


    // Now we set and enable the slave address
    //  - 7 bit address (implied because bit = 0)
    //  - Address = addr parameter
    I2C_OAR1(I2C1)  = ((uint32_t)(addr) << 1) | I2C_OAR1_OA1EN_ENABLE;

    // On I2C_CR1, we enable:
    //  - Peripheral enable
    //  - TX interrupt
    //  - RX interrupt
    //  - Address match interrupt
    //  - NACK interrupt
    //  - STOP interrupt
    //  - Analog filter (implied because bit = 0)
    //  - SCL stretching (implied because bit = 0)
    I2C_CR1(I2C1) = I2C_CR1_PE |
                    I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_DDRIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE;

    // Enable I2C1 interrupt
    NVIC_ISER(0) |= 1 << NVIC_I2C1_IRQ;
}

void i2c_init_rw_map(const struct i2c_variable variables[], const int len) {
    i2c_variables = variables;
    i2c_variables_len = len;
    for (int i = 0; i < len; ++i) {
        for (int i2 = 0; i2 < variables[i].size; ++i2) {
            i2c_register_rw[i2c_register_size] = variables[i].rw;
            i2c_register_size++;
            if (i2c_register_size >= sizeof(i2c_register_rw)) {
                LOG(INFO, "I2C: Register size overflow, bailing");
                return;
            }
        }
    }
}

void i2c_init(unsigned char addr, unsigned char mhz,
              const struct i2c_variable variables[], const int len) {
    i2c_init_peripheral(addr, mhz);
    i2c_init_rw_map(variables, len);
}

void i2c_register_write_callback(void (*callback)()) {
    write_callback = callback;
}

bool i2c_interrupt_addr_match(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_ADDR;
}

bool i2c_interrupt_stop(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_STOPF;
}

bool i2c_interrupt_nack(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_NACKF;
}

bool i2c_interrupt_read_not_empty(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_RXNE;
}

bool i2c_interrupt_write_empty(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_TXE;
}

bool i2c_is_read(uint32_t i2c) {
    return I2C_ISR(i2c) & I2C_ISR_DIR_READ;
}

#define i2c_clear_addr_match(i2c) (I2C_ICR(i2c) |= I2C_ICR_ADDRCF)
#define i2c_clear_nack(i2c) (I2C_ICR(i2c) |= I2C_ICR_NACKCF)
#define i2c_clear_stop(i2c) (I2C_ICR(i2c) |= I2C_ICR_STOPCF)
#define i2c_write_txe(i2c) (I2C_ISR(i2c) |= I2C_ISR_TXE)

static void memcpy_volatile(volatile void *dst, volatile const void *src, size_t len) {
    volatile unsigned char *dst_uc = dst;
    volatile const unsigned char *src_uc = src;

    for (size_t i = 0; i < len; ++i) {
        dst_uc[i] = src_uc[i];
    }
}

void i2c1_isr() {
    // Ok so I just want to rant for a little bit
    // When a master wants to read from a slave, it'll:
    //  - Generate a START condition
    //  - Address the slave, at write mode
    //  - Write the register location where it wants to read out
    //  - >>>>>> Generate a Repeated START condition <<<<<<
    //  - Address the slave, at read mode
    //  - Slave then write out data until the master sends a NACK
    // I emphasize the "Repeated START" part because STM32 doesn't freaking
    // recognize Repeated START in slave mode, thus it will not generate
    // any interurpt for this condition (there aren't any flags to enable
    // this so you know). Thus, that messes up out simple state machine
    // implementation, because we rely on STOP condition to transition 
    // i2c_state from I2C_READ to I2C_ADDR_MATCH, thus any master read 
    // operation will cause the slave to lockup.
    // So now we'll have to manually catch that situation.
    // To catch it
    //  - Address match event must occured
    //  - Last state must be I2C_READ
    //  - Master must be requesting data (aka RW bit is READ)

    if (i2c_interrupt_addr_match(I2C1) &&
            ((i2c_state == I2C_ADDR_MATCH) ||
                ((i2c_state == I2C_READ) && (i2c_is_read(I2C1)))
        )) {
        LOG(INFO, "I2C: Slave selected");
        if ((i2c_state == I2C_READ) && (i2c_is_read(I2C1))) {
            LOG(INFO, "I2C: Repeated START detected");
        }
        if (i2c_is_read(I2C1)) {
            LOG(INFO, "I2C: Slave is transmitting");
            i2c_state = I2C_WRITE;
        } else {
            LOG(INFO, "I2C: Slave is receiving");
            i2c_ptr = 0xffffffff;
            i2c_state = I2C_READ;
        }
        i2c_clear_addr_match(I2C1);
        i2c_write_txe(I2C1);
        return;
    }
    
    if (i2c_state != I2C_ADDR_MATCH && i2c_interrupt_nack(I2C1)) {
        LOG(INFO, "I2C: Slave received NACK (STOP should follow)");
        i2c_clear_nack(I2C1);
        return;
    }
    
    if (i2c_state == I2C_READ && i2c_interrupt_read_not_empty(I2C1)) {
        if (i2c_ptr == 0xffffffff) {
            i2c_ptr = i2c_get_data(I2C1);
            LOG(INFO, "I2C: Slave received address 0x%02x", i2c_ptr);
            return;
        } 
        
        if (i2c_ptr >= i2c_register_size) {
            LOG(INFO, "I2C: Write pointer overflow, ignoring");
            // TODO: does this really read RX data? (or the call gets eliminated by LTO?)
            i2c_get_data(I2C1);
            return;
        }

        if (i2c_register_rw[i2c_ptr] != I2C_RW) {
            LOG(INFO, "I2C: Writing into non-writable area (addr: 0x%02x), ignoring", i2c_ptr);
            i2c_ptr++;
            i2c_get_data(I2C1);
            return;
        }

        i2c_register[i2c_ptr] = i2c_get_data(I2C1);
        LOG(INFO, "I2C: Slave received data 0x%02x, writing into 0x%02x", i2c_register[i2c_ptr], i2c_ptr);
        i2c_ptr++;
        return;
    } 
    
    if (i2c_state == I2C_WRITE && i2c_interrupt_write_empty(I2C1)) {
        if (i2c_ptr >= i2c_register_size) {
            LOG(INFO, "I2C: Read pointer overflow, writing garbage");
            // Slave is writing out data
            // As there's no way for the slave to terminate read
            // transaction, we'll just have to send garbage
            i2c_send_data(I2C1, 0xff);
            return;
        }

        LOG(INFO, "I2C: Slave reading 0x%02x from address 0x%02x", i2c_register[i2c_ptr], i2c_ptr);
        i2c_send_data(I2C1, i2c_register[i2c_ptr]);
        i2c_ptr++;
        return;
    }

    if (i2c_state != I2C_ADDR_MATCH && i2c_interrupt_stop(I2C1)) {
        LOG(INFO, "I2C: Slave received STOP");
        i2c_clear_stop(I2C1);
        if (!i2c_is_read(I2C1)) {
            LOG(INFO, "I2C: Calling write interrupt");
            if (write_callback) write_callback();
        }
        i2c_state = I2C_ADDR_MATCH;
        return;
    }

    return;
}

#ifdef DEBUG
    void i2c_dump() {
        const char h2d[16] = "0123456789abcdef";
        int ptr = 0;
        LOG(INFO, "I2C: Data dump");
        for (int i = 0; i < i2c_variables_len; ++i) {
            LOG(INFO, "ID: 0x%02x:", i2c_variables[i].id);
            
            char hexdump[512];
            memset(hexdump, 0, sizeof(hexdump));

            for (int i2 = 0; i2 < i2c_variables[i].size; i2++, ptr++) {
                hexdump[3*(i2%16)]   = h2d[i2c_register[ptr] / 16];
                hexdump[3*(i2%16)+1] = h2d[i2c_register[ptr] % 16];
                hexdump[3*(i2%16)+2] = ' ';

                if (i2 % 16 == 15) {
                    LOG(INFO, "    %s", hexdump);
                    memset(hexdump, 0, sizeof(hexdump));
                }
            }
            
            if (hexdump[0] != '\0') {
                LOG(INFO, "    %s", hexdump);
            }
        }
    }
#endif

static bool i2c_find_variable_ptr(unsigned char id, unsigned char *ptr, unsigned char *size) {
    unsigned char curr = 0;
    for (int i = 0; i < i2c_variables_len; ++i) {
        if (i2c_variables[i].id == id) {
            *ptr = curr;
            *size = i2c_variables[i].size;
            return true;
        } else {
            curr += i2c_variables[i].size;
        }
    }

    return false;
}

bool i2c_read(unsigned char id, volatile void *buf) {
    unsigned char ptr, size;
    if (!i2c_find_variable_ptr(id, &ptr, &size)) {
        return false;
    }
    memcpy_volatile(buf, i2c_register + ptr, size);
    return true;
}

bool i2c_write(unsigned char id, volatile const void *buf) {
    unsigned char ptr, size;
    if (!i2c_find_variable_ptr(id, &ptr, &size)) {
        return false;
    }
    memcpy_volatile(i2c_register + ptr, buf, size);
    return true;
}
