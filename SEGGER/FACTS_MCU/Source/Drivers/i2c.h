#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "nrf_twi_sensor.h"
#include "nrf_drv_twi.h"

static const uint8_t kSCLPin = 27u;
static const uint8_t kSDAPin = 26u;

bool i2c_init(const nrf_drv_twi_t* i2c);

bool i2c_read(const nrf_drv_twi_t* i2c, const uint8_t dev_addr, uint8_t* data, uint8_t length);

// first byte in data needs to be register address;
bool i2c_write(const nrf_drv_twi_t* i2c, const uint8_t dev_addr, uint8_t const* data, uint8_t length);

bool i2c_deinit(nrf_drv_twi_t* i2c);
