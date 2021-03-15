#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "bno055.h"

//DEFAULT MODE NDOF, CHANGE TO AMG TO READ NON FUSION DATA
static const uint8_t BNO055_I2C_BUS_WRITE_ARRAY_INDEX = 1u;
static const uint8_t I2C_BUFFER_LEN = 8u;
static const uint8_t I2C0 = 5u;
static const uint8_t BNO055_ID = 0xA0;

void I2C_routine(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_setup(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr);

void bno055_get_calibration_status(nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_remap_setup(u8 remap_axis, u8 remap_x_sign, u8 remap_y_sign, u8 remap_z_sign,
 nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_set_acc_unit(uint8_t accel_unit, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_accel_setup(uint8_t accel_unit, uint8_t accel_range, uint8_t accel_bw, nrf_drv_twi_t* i2c,
 uint8_t dev_addr);

bool bno055_set_gyr_unit(uint8_t gyro_unit, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_gyro_setup(uint8_t gyro_range, uint8_t gyro_bw, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_read_accel(struct bno055_accel_t *accel, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_read_gyro(struct bno055_gyro_t *gyro, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_read_quat(struct bno055_quaternion_t *quaternion, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_set_data_out_format(uint8_t data_output_format, nrf_drv_twi_t* i2c, uint8_t dev_addr);

bool bno055_convert_double_acc_xyz_msq(struct bno055_accel_double_t *accel_xyz, nrf_drv_twi_t* i2c,
 uint8_t dev_addr);

bool bno055_convert_double_gyr_xyz_rps(struct bno055_gyro_double_t *gyro_xyz, nrf_drv_twi_t* i2c,
 uint8_t dev_addr);

int8_t BNO055_I2C_bus_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

int8_t BNO055_I2C_bus_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);

void BNO055_delay_msek(uint32_t msek);

bool deinit(struct bno055_t* bno055);
