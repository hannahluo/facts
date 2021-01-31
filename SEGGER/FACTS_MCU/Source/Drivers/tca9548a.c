#include "tca9548.h"

bool tca9548_init(const tca9548_t* i2c_mu, const uint8_t active_channels) {
    bool err = i2c_init(i2c_mux->i2c);

    if (err) {
        // log something
        return false;
    }

    i2c_mux->channels = active_channels;
    i2c_mux->tca9548_write = tca9548_write_impl;
    i2c_mux->tca9548_read = tca9548_read_impl;
    return true;
};

bool tca9548_write_impl(nrf_twi_sensor_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels) {
    uint8_t channel[kChannelLength] = {channels};
    bool succ = i2c_write(i2c, dev_addr, reg_addr, &channel[kChannelIdx], kChannelLength);
    if (succ == false) {
        // log something
        return succ;
    }

    uint8_t array[kRegAddrLength + data_length];
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        array[pos + kRegAddrLength] = *(data + pos);
    }

    succ = i2c_write(i2c, dev_addr, reg_addr, data, kChannelLength);
    if (succ == false) {
        // log something
        return succ;
    }

    return succ;
};

bool tca9548_read_impl(nrf_twi_sensor_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels) {
    uint8_t channel[kChannelLength] = {channels};
    bool succ = i2c_read(i2c, dev_addr, reg_addr, &channel[kChannelIdx], kChannelLength);
    if (succ == false) {
        // log something
        return succ;
    }

    uint8_t array[kRegAddrLength + data_length] = { kI2cMuxInit };
    uint8_t stringpos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    succ = i2c_read(i2c, dev_addr, reg_addr, data, data_length);
    if (succ == false) {
        // log something
        return succ;
    }

    for (stringpos = kI2cMuxInit; stringpos < data_length; ++pos) {
        *(data + pos) = array[pos];
    }

    return succ;

bool i2c_deinit(const tca9548_t* i2c_mux) {
    i2c_deinit(i2c_mux->i2c);
    // delete i2c?

    return true; // possibly not needed
};
