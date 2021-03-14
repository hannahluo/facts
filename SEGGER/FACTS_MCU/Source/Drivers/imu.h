/*
#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bno055.h"

static const uint8_t BNO055_I2C_BUS_WRITE_ARRAY_INDEX = 1u;
static const uint8_t I2C_BUFFER_LEN = 8u;
static const uint8_t I2C0 = 5u;
static const uint8_t BNO055_ID = 0xA0;

void I2C_routine(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_setup(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr);

void bno055_set_external_xtal(bool use_xtal);

void bno055_get_calibration_status();

bool bno055_read_raw(struct bno055_accel_t* accel_xyz, struct bno055_mag_t* mag_xyz, struct bno055_gyro_t* gyro_xyz);

int8_t BNO055_I2C_bus_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

int8_t BNO055_I2C_bus_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

void BNO055_delay_msek(uint32_t msek);

bool deinit(struct bno055_t* bno055);
*/
