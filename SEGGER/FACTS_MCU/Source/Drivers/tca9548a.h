#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"

static const uint8_t kChannelLength = 1u;
static const uint8_t kChannelIdx = 0u;
static const uint8_t kRegAddrLength = 1u;
static const uint8_t kI2cMuxInit = 0u;
static const uint8_t kI2cMaxBufferLength = 8u;

//typedef bool (*write_data)(nrf_drv_twi_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);
//typedef bool (*read_data)(nrf_drv_twi_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);

typedef struct {
    // channels is a bit field of active channels where a set bit = active channel
    // uint8_t channel; /**< which channels to write to */
    uint8_t dev_addr; /**< i2c device address of tca9548a */
    //write_data tca9548a_write; /**< bus write function pointer */
    //read_data tca9548a_read; /**< bus write function pointer */

    nrf_drv_twi_t* i2c;
} tca9548a_t;

bool tca9548a_init(tca9548a_t* i2c_mux, uint8_t dev_addr, nrf_drv_twi_t* i2c);

// write to channels one at a time?
// data length might need to be const
// might need a bigger device address
bool tca9548a_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t* reg_addr, uint8_t* data, uint8_t data_length, uint8_t channel);

bool tca9548a_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t* reg_addr, uint8_t* data, uint8_t data_length, uint8_t channel);

bool tca9548a_deinit(tca9548a_t* i2c_mux);
