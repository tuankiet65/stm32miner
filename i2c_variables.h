#ifndef I2C_VARIABLES_H
    #define I2C_VARIABLES_H

    enum i2c_variable_ids {
        version = 0,
        state,
        hashrate,
        current_job_id,
        winning_nonce,

        new_job_id,
        new_header,
        execute_job,
        force_calibrate
    };

    #define STATE_READY 0x01
    #define STATE_WORKING 0x02
    #define STATE_FOUND 0x03
    #define STATE_NOT_FOUND 0x04
    #define STATE_ERROR 0xff

    struct i2c_variable __attribute__((weak)) i2c_variables[] = {
        { .id = version        , .size = sizeof(char[8])     , .rw = I2C_RO },
        { .id = state          , .size = sizeof(uint8_t)     , .rw = I2C_RO },
        { .id = hashrate       , .size = sizeof(uint32_t)    , .rw = I2C_RO },
        { .id = current_job_id , .size = sizeof(uint8_t)     , .rw = I2C_RO },
        { .id = winning_nonce  , .size = sizeof(uint32_t)    , .rw = I2C_RO },

        { .id = new_job_id     , .size = sizeof(uint8_t)     , .rw = I2C_RW },
        { .id = new_header     , .size = sizeof(uint32_t[20]), .rw = I2C_RW },
        { .id = execute_job    , .size = sizeof(uint8_t)     , .rw = I2C_RW },
        { .id = force_calibrate, .size = sizeof(uint8_t)     , .rw = I2C_RW },
    };

#endif
