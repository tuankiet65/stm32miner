#include "i2c.h"

static unsigned i2c_ptr = 0xffffffff;

static enum i2c_rw_status i2c_register_rw[256];
static unsigned i2c_register_size;

static unsigned char i2c_register[256];

static void (*read_callback)();
static void (*write_callback)();

void i2c_init_peripheral(unsigned char addr) {
    rcc_periph_clock_enable(RCC_GPIOA);

    // Enable I2C1 clock
    rcc_periph_clock_enable(RCC_I2C1);

    nvic_enable_irq(NVIC_I2C1_IRQ);

    // Configure PA9 and PA10 to AF4 (I2C1_SCL/I2C1_SDA)
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
    gpio_set_af(GPIOA, GPIO_AF4, GPIO9 | GPIO10);

    i2c_peripheral_disable(I2C1);
    i2c_reset(I2C1);

    i2c_enable_analog_filter(I2C1);
    i2c_enable_stretching(I2C1);
    i2c_set_speed(I2C1, i2c_speed_fm_400k, 48);

    i2c_set_own_7bit_slave_address(I2C1, addr);
    // libopencm3 for some freaky reasons doesn't provide
    // any functions to enable OAR1 address, nor the
    // i2c_set_own_7bit_slave_address does that, so we'll
    // have to do it on our own.
    // Time wasted debugging and swearing: 2 days
    I2C_OAR1(I2C1) |= I2C_OAR1_OA1EN_ENABLE;

    i2c_enable_interrupt(I2C1, I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_DDRIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE);

    i2c_peripheral_enable(I2C1);
}

void i2c_init_rw_map(const struct i2c_variable variables[], const int len) {
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

void i2c_init(unsigned char addr, const struct i2c_variable variables[], const int len) {
    i2c_init_peripheral(addr);
    i2c_init_rw_map(variables, len);
}

bool i2c_ready(uint32_t i2c) {
    return !i2c_busy(i2c);
}

void i2c_register_read_callback(void (*callback)()) {
    read_callback = callback;
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

void i2c_clear_addr_match(uint32_t i2c) {
    I2C_ICR(i2c) |= I2C_ICR_ADDRCF;
}

void i2c_clear_nack(uint32_t i2c) {
    I2C_ICR(i2c) |= I2C_ICR_NACKCF;
}

void i2c_write_txe(uint32_t i2c) {
    I2C_ISR(i2c) |= I2C_ISR_TXE;
}

void i2c1_isr() {
    if (i2c_interrupt_addr_match(I2C1)) {
        LOG(INFO, "I2C: Slave selected");
        if (i2c_is_read(I2C1)) {
            LOG(INFO, "I2C: Slave is transmitting");
        } else {
            LOG(INFO, "I2C: Slave is receiving");
            i2c_ptr = 0xffffffff;
        }
        i2c_clear_addr_match(I2C1);
        i2c_write_txe(I2C1);
        return;
    } 
    
    if (i2c_interrupt_nack(I2C1)) {
        LOG(INFO, "I2C: Slave received NACK (STOP should follow)");
        i2c_clear_nack(I2C1);
        return;
    }
    
    if (i2c_interrupt_stop(I2C1)) {
        LOG(INFO, "I2C: Slave received STOP");
        i2c_clear_stop(I2C1);
        return;
    } 
    
    if (i2c_interrupt_read_not_empty(I2C1)) {
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

        if (i2c_register_rw[i2c_ptr] != I2C_READ_WRITE) {
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
    
    if (i2c_interrupt_write_empty(I2C1)) {
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
    }

    return;
}

void i2c_dump() {
    LOG(INFO, "I2C: Data dump");
    for (unsigned i = 0; i < i2c_register_size; i += 4) {
        LOG(INFO, "I2C: 0x%02x: 0x%08x", i, i2c_register[i]);
    }
}
