#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c.h"

const uint8_t kChannelLength = 1u;
const uint8_t kChannelIdx = 0u;
const uint8_t kRegAddrLength = 1u;
const uint8_t kI2cMuxInit = 0u;

typedef bool (*write_data)(nrf_twi_sensor_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);
typedef bool (*read_data)(nrf_twi_sensor_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);

typedef struct {
    // channels is a bit field of active channels where a set bit = active channel
    uint8_t channels; /**< which channels to write to */
    uint8_t dev_addr; /**< i2c device address of tca9548 */
    write_data tca9548_write; /**< bus write function pointer */
    read_data tca9548_read; /**< bus write function pointer */

    nrf_twi_sensor_t* i2c;
} tca9548_t;

bool tca9548_init(const tca9548_t* i2c_mux);

// write to channels one at a time?
// data length might need to be const
bool tca9548_write_impl(nrf_twi_sensor_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels);

bool tca9548_read_impl(nrf_twi_sensor_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t data_length, uint8_t channels);

bool tca9548_deinit(const tca9548_t* i2c);
