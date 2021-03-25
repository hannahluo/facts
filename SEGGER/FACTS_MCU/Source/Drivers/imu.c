#include "imu.h"
#include "nrf_log.h"
#include "nrf_delay.h"

void I2C_routine(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    bno055->bus_write = BNO055_I2C_bus_write;
    bno055->bus_read = BNO055_I2C_bus_read;
    bno055->delay_msec = BNO055_delay_msek;
    bno055->dev_addr = dev_addr;
    bno055->i2c = i2c;
}

bool bno055_setup(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    NRF_LOG_INFO("initializing imu");
    I2C_routine(bno055, i2c, dev_addr);

    uint32_t err = bno055_init(bno055);

    // Make sure we have the right device
    uint8_t id = BNO055_INIT_VALUE;
    err += BNO055_I2C_bus_read(i2c, dev_addr, BNO055_CHIP_ID_REG, &id, 1);
    if (err == BNO055_SUCCESS && id != BNO055_ID) {
        nrf_delay_ms(1000); // hold on for boot
        // err += bno055_read_chip_id(&id);
        err += BNO055_I2C_bus_read(i2c, dev_addr, BNO055_CHIP_ID_REG, &id, 1);
        if (id != BNO055_ID) {
            return false; // still not? ok bail
        }
    }

    // Set to normal power mode // MAYBE JUST WRITE REG?
    uint8_t pwr_mode = BNO055_POWER_MODE_NORMAL; 
    err += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_POWER_MODE_REG, &pwr_mode, 1);

    uint8_t p_zero = BNO055_PAGE_ZERO;
    err += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_PAGE_ID_REG, &p_zero, 1);

    uint8_t use_xtal = BNO055_CLK_SRC_MSK; // not necessarily needed
    err += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_CLK_SRC_REG, &use_xtal, 1);

    // Set the requested operating mode (see section 3.3)
    uint8_t op_mode = BNO055_OPERATION_MODE_NDOF;
    err += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    return (err == BNO055_SUCCESS);
};

void bno055_get_calibration_status(nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    uint8_t calib_status = BNO055_INIT_VALUE;
    uint32_t err = BNO055_INIT_VALUE;

    uint8_t accel_calib_status = BNO055_INIT_VALUE;
    uint8_t gyro_calib_status = BNO055_INIT_VALUE;
    uint8_t mag_calib_status = BNO055_INIT_VALUE;
    uint8_t sys_calib_status = BNO055_INIT_VALUE;

    BNO055_I2C_bus_read(i2c, dev_addr, BNO055_CALIB_STAT_ADDR, &calib_status, 1);
    sys_calib_status= (calib_status>> 6) & 0x03;
    gyro_calib_status = (calib_status >> 4) & 0x03;
    accel_calib_status = (calib_status >> 2) & 0x03;
    mag_calib_status = calib_status & 0x03;

    NRF_LOG_INFO("BNO055 CALIB STAT: acc %d, mag %d, gyr %d, sys %d", 
      accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status);
}

int8_t bno055_get_syscal_status(nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    uint8_t calib_status = BNO055_INIT_VALUE;
    uint32_t err = BNO055_INIT_VALUE;
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;

    uint8_t accel_calib_status = BNO055_INIT_VALUE;
    uint8_t gyro_calib_status = BNO055_INIT_VALUE;
    uint8_t mag_calib_status = BNO055_INIT_VALUE;
    uint8_t sys_calib_status = BNO055_INIT_VALUE;

    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr, BNO055_CALIB_STAT_ADDR, &calib_status, 1);
    if(com_rslt != 0) {
        return -1;
    }
    sys_calib_status= (calib_status>> 6) & 0x03;
    gyro_calib_status = (calib_status >> 4) & 0x03;
    accel_calib_status = (calib_status >> 2) & 0x03;
    mag_calib_status = calib_status & 0x03;

    NRF_LOG_INFO("BNO055 CALIB STAT: acc %d, mag %d, gyr %d, sys %d", 
      accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status);

    return sys_calib_status;
}

bool bno055_remap_setup(u8 remap_axis, u8 remap_x_sign, u8 remap_y_sign, u8 remap_z_sign,
 nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    switch (remap_axis) {
        case BNO055_REMAP_X_Y:
        case BNO055_REMAP_Y_Z:
        case BNO055_REMAP_Z_X:
        case BNO055_REMAP_X_Y_Z_TYPE0:
        case BNO055_REMAP_X_Y_Z_TYPE1:
        case BNO055_DEFAULT_AXIS:
            com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                            BNO055_REMAP_AXIS_VALUE_REG,
                                            &data_u8r,
                                            BNO055_GEN_READ_WRITE_LENGTH);
            if (com_rslt == BNO055_SUCCESS) {
                data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_REMAP_AXIS_VALUE, remap_axis);
                com_rslt += BNO055_I2C_bus_write(i2c, dev_addr,
                                                  BNO055_REMAP_AXIS_VALUE_REG,
                                                  &data_u8r,
                                                  BNO055_GEN_READ_WRITE_LENGTH);
            }
            break;
        default:
            /* Write the default axis remap value */
            com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                            BNO055_REMAP_AXIS_VALUE_REG,
                                            &data_u8r,
                                            BNO055_GEN_READ_WRITE_LENGTH);
            if (com_rslt == BNO055_SUCCESS) {
                data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_REMAP_AXIS_VALUE, BNO055_DEFAULT_AXIS);
                com_rslt += BNO055_I2C_bus_write(i2c, dev_addr,
                                                BNO055_REMAP_AXIS_VALUE_REG,
                                                &data_u8r,
                                                BNO055_GEN_READ_WRITE_LENGTH);
            }
            break;
    }

    /* Write the value of x-axis remap */
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_AXIS_MAP_SIGN_ADDR,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (com_rslt == BNO055_SUCCESS) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_REMAP_X_SIGN, remap_x_sign);
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_REMAP_Y_SIGN, remap_y_sign);
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_REMAP_Z_SIGN, remap_z_sign);
        com_rslt += BNO055_I2C_bus_write(i2c, dev_addr,
                                        BNO055_AXIS_MAP_SIGN_ADDR,
                                        &data_u8r,
                                        BNO055_GEN_READ_WRITE_LENGTH);
    }

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == 0) ? true : false;
}

bool bno055_set_acc_unit(uint8_t accel_unit, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    /* Write the accel unit */
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_ACCEL_UNIT_REG,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (com_rslt == BNO055_SUCCESS) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_ACCEL_UNIT, accel_unit);
        com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, 
                                          BNO055_ACCEL_UNIT_REG,
                                          &data_u8r,
                                          BNO055_GEN_READ_WRITE_LENGTH);
    }

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == 0) ? true : false;
}

bool bno055_accel_setup(uint8_t accel_unit, uint8_t accel_range, uint8_t accel_bw, nrf_drv_twi_t* i2c,
 uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_ACCEL_CONFIG_ADDR,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (accel_range < BNO055_ACCEL_RANGE) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_ACCEL_RANGE, accel_range);
    } else {
        com_rslt = BNO055_OUT_OF_RANGE;
    }

    if (accel_bw < BNO055_ACCEL_GYRO_BW_RANGE) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_ACCEL_BW, accel_bw);
    } else {
        com_rslt = BNO055_OUT_OF_RANGE;
    }

    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, 
                            BNO055_ACCEL_CONFIG_ADDR,
                            &data_u8r,
                            BNO055_GEN_READ_WRITE_LENGTH);

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == 0) ? true : false;
}

bool bno055_set_gyr_unit(uint8_t gyro_unit, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    /* Write the gyro unit */
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_GYRO_UNIT_REG,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (com_rslt == BNO055_SUCCESS) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_GYRO_UNIT, gyro_unit);
        com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, 
                                          BNO055_GYRO_UNIT_REG,
                                          &data_u8r,
                                          BNO055_GEN_READ_WRITE_LENGTH);
    }

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == 0) ? true : false;
}

bool bno055_gyro_setup(uint8_t gyro_range, uint8_t gyro_bw, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_GYRO_CONFIG_ADDR,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (gyro_range < BNO055_GYRO_RANGE) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_GYRO_RANGE, gyro_range);
    } else {
        com_rslt = BNO055_OUT_OF_RANGE;
    }

    if (gyro_bw < BNO055_ACCEL_GYRO_BW_RANGE) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_GYRO_BW, gyro_bw);
    } else {
        com_rslt = BNO055_OUT_OF_RANGE;
    }

    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, 
                            BNO055_GYRO_CONFIG_ADDR,
                            &data_u8r,
                            BNO055_GEN_READ_WRITE_LENGTH);

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_read_accel(struct bno055_accel_t *accel, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8[BNO055_ACCEL_XYZ_DATA_SIZE] = {
        BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE
    };
 
    /* Read the six bytes data of accel xyz*/
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_ACCEL_DATA_X_LSB_VALUEX_REG,
                                    data_u8,
                                    BNO055_ACCEL_XYZ_DATA_SIZE);

    /* Data X*/
    data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB],
                                                                BNO055_ACCEL_DATA_X_LSB_VALUEX);
    data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB],
                                                                BNO055_ACCEL_DATA_X_MSB_VALUEX);
    accel->x =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB]));

    /* Data Y*/
    data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB],
                                                                BNO055_ACCEL_DATA_Y_LSB_VALUEY);
    data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB],
                                                                BNO055_ACCEL_DATA_Y_MSB_VALUEY);
    accel->y =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB]));

    /* Data Z*/
    data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB],
                                                                BNO055_ACCEL_DATA_Z_LSB_VALUEZ);
    data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB],
                                                                BNO055_ACCEL_DATA_Z_MSB_VALUEZ);
    accel->z =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB]));

    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_read_gyro(struct bno055_gyro_t *gyro, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8[BNO055_GYRO_XYZ_DATA_SIZE] = {
        BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE
    };

    /* Read the six bytes data of gyro xyz*/
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_GYRO_DATA_X_LSB_VALUEX_REG,
                                    data_u8,
                                    BNO055_GYRO_XYZ_DATA_SIZE);

    /* Data x*/
    data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB],
                                                                BNO055_GYRO_DATA_X_LSB_VALUEX);
    data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB],
                                                                BNO055_GYRO_DATA_X_MSB_VALUEX);
    gyro->x =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_X_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_X_LSB]));

    /* Data y*/
    data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB],
                                                                BNO055_GYRO_DATA_Y_LSB_VALUEY);
    data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB],
                                                                BNO055_GYRO_DATA_Y_MSB_VALUEY);
    gyro->y =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_Y_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_Y_LSB]));

    /* Data z*/
    data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB],
                                                                BNO055_GYRO_DATA_Z_LSB_VALUEZ);
    data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB] = BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB],
                                                                BNO055_GYRO_DATA_Z_MSB_VALUEZ);
    gyro->z =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_XYZ_Z_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_XYZ_Z_LSB]));

    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_read_quat(struct bno055_quaternion_t *quaternion, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8[BNO055_QUATERNION_WXYZ_DATA_SIZE] = {
        BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE,
        BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE
    };

    /* Read the eight byte value
     * of quaternion wxyz data*/
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_QUATERNION_DATA_W_LSB_VALUEW_REG,
                                    data_u8,
                                    BNO055_QUATERNION_WXYZ_DATA_SIZE);

    /* Data W*/
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_LSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_LSB],
                            BNO055_QUATERNION_DATA_W_LSB_VALUEW);
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_MSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_MSB],
                            BNO055_QUATERNION_DATA_W_MSB_VALUEW);
    quaternion->w =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_W_LSB]));

    /* Data X*/
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_LSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_LSB],
                            BNO055_QUATERNION_DATA_X_LSB_VALUEX);
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_MSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_MSB],
                            BNO055_QUATERNION_DATA_X_MSB_VALUEX);
    quaternion->x =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_X_LSB]));

    /* Data Y*/
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_LSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_LSB],
                            BNO055_QUATERNION_DATA_Y_LSB_VALUEY);
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_MSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_MSB],
                            BNO055_QUATERNION_DATA_Y_MSB_VALUEY);
    quaternion->y =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Y_LSB]));

    /* Data Z*/
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_LSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_LSB],
                            BNO055_QUATERNION_DATA_Z_LSB_VALUEZ);
    data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_MSB] =
        BNO055_GET_BITSLICE(data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_MSB],
                            BNO055_QUATERNION_DATA_Z_MSB_VALUEZ);
    quaternion->z =
        (s16)((((s32)((s8)data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_MSB])) << BNO055_SHIFT_EIGHT_BITS) |
              (data_u8[BNO055_SENSOR_DATA_QUATERNION_WXYZ_Z_LSB]));

    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_set_data_out_format(uint8_t data_output_format, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_ERROR;
    uint8_t data_u8r = BNO055_INIT_VALUE;

    uint8_t op_mode = BNO055_OPERATION_MODE_CONFIG;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);

    /* Write the data output format */
    com_rslt = BNO055_I2C_bus_read(i2c, dev_addr,
                                    BNO055_DATA_OUTPUT_FORMAT_REG,
                                    &data_u8r,
                                    BNO055_GEN_READ_WRITE_LENGTH);
    if (com_rslt == BNO055_SUCCESS) {
        data_u8r = BNO055_SET_BITSLICE(data_u8r, BNO055_DATA_OUTPUT_FORMAT, data_output_format);
        com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, 
                                          BNO055_DATA_OUTPUT_FORMAT_REG,
                                          &data_u8r,
                                          BNO055_GEN_READ_WRITE_LENGTH);
    }

    op_mode = BNO055_OPERATION_MODE_NDOF;
    com_rslt += BNO055_I2C_bus_write(i2c, dev_addr, BNO055_OPERATION_MODE_REG, &op_mode, 1);
    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_convert_double_acc_xyz_msq(struct bno055_accel_double_t *accel_xyz, nrf_drv_twi_t* i2c,
 uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_SUCCESS;
    struct bno055_accel_t reg_accel_xyz = { BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE };

    bool succ = bno055_set_acc_unit(BNO055_ACCEL_UNIT_MSQ, i2c, dev_addr);
    if (succ) {
        /* Read the accel raw xyz data*/
        succ = bno055_read_accel(&reg_accel_xyz, i2c, dev_addr);
        if (succ) {
            /* Convert raw xyz to m/s2*/
            accel_xyz->x = (double)(reg_accel_xyz.x / BNO055_ACCEL_DIV_MSQ);
            accel_xyz->y = (double)(reg_accel_xyz.y / BNO055_ACCEL_DIV_MSQ);
            accel_xyz->z = (double)(reg_accel_xyz.z / BNO055_ACCEL_DIV_MSQ);
        }
        else
        {
            com_rslt = BNO055_ERROR;
        }
    } else {
        com_rslt = BNO055_ERROR;
    }

    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

bool bno055_convert_double_gyr_xyz_rps(struct bno055_gyro_double_t *gyro_xyz, nrf_drv_twi_t* i2c,
 uint8_t dev_addr) {
    BNO055_RETURN_FUNCTION_TYPE com_rslt = BNO055_SUCCESS;
    struct bno055_gyro_t reg_gyro_xyz = { BNO055_INIT_VALUE, BNO055_INIT_VALUE, BNO055_INIT_VALUE };

    bool succ = bno055_set_gyr_unit(BNO055_GYRO_UNIT_RPS, i2c, dev_addr);
    if (succ) {
        /* Read gyro raw x data */
        succ = bno055_read_gyro(&reg_gyro_xyz, i2c, dev_addr);
        if (succ) {
            /* Convert the raw gyro xyz to rps*/
            gyro_xyz->x = (double)(reg_gyro_xyz.x / BNO055_GYRO_DIV_RPS);
            gyro_xyz->y = (double)(reg_gyro_xyz.y / BNO055_GYRO_DIV_RPS);
            gyro_xyz->z = (double)(reg_gyro_xyz.z / BNO055_GYRO_DIV_RPS);
        } else {
            com_rslt = BNO055_ERROR;
        }
    } else {
            com_rslt = BNO055_ERROR;
    }

    return (com_rslt == BNO055_SUCCESS) ? true : false;
}

int8_t BNO055_I2C_bus_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    uint8_t array[2];
    uint8_t stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;
    for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
    {
        array[stringpos + BNO055_I2C_BUS_WRITE_ARRAY_INDEX] = *(reg_data + stringpos);
    }

    bool succ = i2c_write(i2c, dev_addr, &array[BNO055_INIT_VALUE], 2);
    return (succ == true) ? BNO055_SUCCESS : BNO055_ERROR;
}

int8_t BNO055_I2C_bus_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    uint8_t array[I2C_BUFFER_LEN];
    uint8_t stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;
    bool succ = i2c_write(i2c, dev_addr, &array[BNO055_INIT_VALUE], 1);
    i2c_read(i2c, dev_addr, &array[BNO055_INIT_VALUE], cnt);
    if (succ) {
        for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
        {
            *(reg_data + stringpos) = array[stringpos];
        }
    }

    return (succ == true) ? BNO055_SUCCESS : BNO055_ERROR;
}

void BNO055_delay_msek(uint32_t ms) {
    nrf_delay_ms(ms);
}

bool deinit(struct bno055_t* bno055) {
  uint8_t power_mode = BNO055_POWER_MODE_SUSPEND;
  uint32_t err = bno055_set_power_mode(power_mode);

  return (err == BNO055_SUCCESS);
}

