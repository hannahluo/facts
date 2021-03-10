#include "tca9548a.h"
#include "nrf_log.h"

bool tca9548a_init(tca9548a_t* i2c_mux, uint8_t mux_addr, nrf_drv_twi_t* i2c) {
    NRF_LOG_INFO("initializing i2c mux");
    i2c_mux->mux_addr = mux_addr;
    i2c_mux->i2c = i2c;
    return true;
};

bool tca9548a_select(nrf_drv_twi_t* i2c, tca9548a_t* i2c_mux, uint8_t channel) {
    bool succ = i2c_write(i2c, i2c_mux->mux_addr, &channel, kRegDataLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to select i2c mux channel");
    }

    return succ;
};

bool tca9548a_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length) {
    uint8_t array[kWriteDataLength];
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        array[pos + kRegAddrLength] = *(data + pos);
    }
    bool succ = i2c_write(i2c, dev_addr, &array[kI2cMuxInit], kWriteDataLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux data");
    }

    return succ;
};

bool tca9548a_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t* reg_addr, uint8_t* data, uint8_t data_length) {
    bool succ = i2c_write(i2c, dev_addr, reg_addr, kRegDataLength);
    succ = i2c_read(i2c, dev_addr, data, kRegDataLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to read i2c mux data");
    }

    return succ;
};

bool tca9548a_deinit(tca9548a_t* i2c_mux) {
    NRF_LOG_INFO("deinitializing i2c mux");
    i2c_deinit(i2c_mux->i2c);
    // delete i2c?

    return true; // possibly not needed
};

