#include "tca9548a.h"
#include "nrf_log.h"

bool tca9548a_init(const tca9548a_t* i2c_mux, const uint8_t active_channels) {
    NRF_LOG_INFO("initializing i2c mux");
    bool err = i2c_init(i2c_mux->i2c);

    if (err == false) {
        NRF_LOG_WARNING("failed to init i2c mux");
        return err;
    }

    i2c_mux->tca9548a_write = tca9548a_write_impl;
    i2c_mux->tca9548a_read = tca9548a_read_impl;
    return err;
};

bool tca9548a_write_impl(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels) {
    uint8_t channel[kChannelLength] = {channels};
    bool succ = i2c_write(i2c, dev_addr, reg_addr, &channel[kChannelIdx], kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux channel");
        return succ;
    }

    uint8_t array[kRegAddrLength + kI2cMaxBufferLength];
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        array[pos + kRegAddrLength] = *(data + pos);
    }

    succ = i2c_write(i2c, dev_addr, reg_addr, &array[kI2cMuxInit], kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux data");
        return succ;
    }

    return succ;
};

bool tca9548a_read_impl(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels) {
    uint8_t channel[kChannelLength] = {channels};
    bool succ = i2c_read(i2c, dev_addr, reg_addr, &channel[kChannelIdx], kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to read i2c mux channel");
        return succ;
    }

    uint8_t array[kI2cMaxBufferLength] = { kI2cMuxInit };
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    succ = i2c_read(i2c, dev_addr, reg_addr, &array[kI2cMuxInit], data_length);
    if (succ == false) {
        NRF_LOG_WARNING("failed to read i2c mux data");
        return succ;
    }

    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        *(data + pos) = array[pos];
    }

    return succ;

bool i2c_deinit(const tca9548a_t* i2c_mux) {
    NRF_LOG_INFO("deinitializing i2c mux");
    i2c_deinit(i2c_mux->i2c);
    // delete i2c?

    return true; // possibly not needed
};
