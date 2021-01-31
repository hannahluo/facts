#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "nrf_twi_sensor.h"

bool i2c_init(const nrf_twi_sensor_t* i2c);

bool i2c_read(const nrf_twi_sensor_t* i2c, const uint8_t dev_addr, const uint8_t reg_addr, uint8_t* data, uint8_t length);

// first byte in data needs to be register address;
bool i2c_write(const nrf_twi_sensor_t* i2c, const uint8_t dev_addr, const uint8_t reg_addr, uint8_t const* data, uint8_t length);

bool i2c_deinit(const nrf_twi_sensor_t* i2c);
