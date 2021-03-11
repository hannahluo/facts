#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bno055.h"

#define BNO055_I2C_BUS_WRITE_ARRAY_INDEX ((uint8_t)1)
#define I2C_BUFFER_LEN 8
#define I2C0           5

int8_t I2C_routine(nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_setup(bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr);

void bno055_calibrate();

bool bno055_read_raw(bno055_accel_t* accel_xyz, bno055_mag_t* mag_xyz, bno055_gyro_t* gyro_xyz);

int8_t BNO055_I2C_bus_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

int8_t BNO055_I2C_bus_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

void BNO055_delay_msek(uint32_t msek);

bool deinit(bno055_t* bno055);


