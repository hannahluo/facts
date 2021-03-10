#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"

static const uint8_t kRegAddrLength = 1u;
static const uint8_t kI2cMuxInit = 0u;
static const uint8_t kRegDataLength = 1u;
static const uint8_t kWriteDataLength = kRegAddrLength + kRegDataLength;

typedef struct {
    // channels is a bit field of active channels where a set bit = active channel
    // uint8_t channel; /**< which channels to write to */
    uint8_t mux_addr; /**< i2c device address of tca9548a */

    nrf_drv_twi_t* i2c;
} tca9548a_t;

bool tca9548a_init(tca9548a_t* i2c_mux, uint8_t mux_addr, nrf_drv_twi_t* i2c);

bool tca9548a_select(nrf_drv_twi_t* i2c, tca9548a_t* i2c_mux, uint8_t channel);

bool tca9548a_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length);

bool tca9548a_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t* reg_addr, uint8_t* data, uint8_t data_length);

bool tca9548a_deinit(tca9548a_t* i2c_mux);
