#include "i2c.h"

bool i2c_init(const nrf_twi_sensor_t* i2c) {
    ret_code_t err = nrf_twi_sensor_init(i2c);

    if (err != NRF_SUCCESS) {
        // log something
        return false;
    }

    return true;
};

bool i2c_read(const nrf_twi_sensor_t* i2c, const uint8_t dev_addr, const uint8_t reg_addr, uint32_t* data, uint8_t length) {
    ret_code_t err = nrf_twi_sensor_reg_read(i2c, dev_addr, reg_addr, NULL, data, length);

    if (err != NRF_SUCCESS) {
        // log something
        return false;
    }

    return true;
};

bool i2c_write(const nrf_twi_sensor_t* i2c, const uint8_t dev_addr, const uint8_t reg_addr, uint8_t const* data, uint8_t length) {
    // data length actually needs to be buffer length - 1 (1st byte for register addr)
    ret_code_t err = nrf_twi_sensor_reg_write(i2c, dev_addr, reg_addr, data, length, false);

    if (err != NRF_SUCCESS) {
        // log something
        return false;
    }

    return true;
};

bool i2c_deinit(const nrf_twi_sensor_t* i2c) {
    return true; // possibly not needed
};
