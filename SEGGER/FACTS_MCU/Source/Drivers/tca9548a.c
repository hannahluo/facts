#include "tca9548a.h"
#include "nrf_log.h"

bool tca9548a_init(tca9548a_t* i2c_mux, uint8_t dev_addr, nrf_drv_twi_t* i2c) {
    NRF_LOG_INFO("initializing i2c mux");
    i2c_mux->dev_addr = dev_addr;
    i2c_mux->i2c = i2c;
    // bool err = i2c_init(i2c_mux->i2c);

    // if (err == false) {
    //     NRF_LOG_WARNING("failed to init i2c mux");
    //     return err;
    // }

    return true;
};

bool tca9548a_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channel) {
    //uint8_t ch[kChannelLength];
    //memset(ch, channel, sizeof(uint8_t)); 
    /*bool succ = i2c_write(i2c, 0x70, &channel, kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux channel");
        return succ;
    }*/

    uint8_t array[kWriteDataLength];
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        array[pos + kRegAddrLength] = *(data + pos);
    }
    bool succ = i2c_write(i2c, dev_addr, &array[kI2cMuxInit], kWriteDataLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux data");
        return succ;
    }

    return succ;
};

bool tca9548a_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t* reg_addr, uint8_t* data, uint8_t data_length, uint8_t channel) {
    // uint8_t ch[kChannelLength];
    // memset(ch, channel, sizeof(uint8_t));  
    /*bool succ = i2c_write(i2c, 0x70, &channel, kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to write i2c mux channel");
        return succ;
    } */

    /*uint8_t array[kI2cMaxBufferLength];
    uint8_t pos = kI2cMuxInit;
    array[kI2cMuxInit] = reg_addr;
    bool succ = i2c_read(i2c, dev_addr, &array[kI2cMuxInit], data_length);
    if (succ == false) {
        NRF_LOG_WARNING("failed to read i2c mux data");
        return succ;
    }

    for (pos = kI2cMuxInit; pos < data_length; ++pos) {
        *(data + pos) = array[pos];
    }*/

    bool succ = i2c_write(i2c, dev_addr, reg_addr, kChannelLength);
    succ = i2c_read(i2c, dev_addr, data, kChannelLength);
    if (succ == false) {
        NRF_LOG_WARNING("failed to read i2c mux data");
        return succ;
    }

    return succ;
};

bool tca9548a_deinit(tca9548a_t* i2c_mux) {
    NRF_LOG_INFO("deinitializing i2c mux");
    i2c_deinit(i2c_mux->i2c);
    // delete i2c?

    return true; // possibly not needed
};

