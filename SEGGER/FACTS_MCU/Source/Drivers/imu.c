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

// SET UP A DEV ADDY
void bno055_get_calibration_status() {
    uint8_t accel_calib_status = BNO055_INIT_VALUE;
    uint8_t gyro_calib_status = BNO055_INIT_VALUE;
    uint8_t mag_calib_status = BNO055_INIT_VALUE;
    uint8_t sys_calib_status = BNO055_INIT_VALUE;
    uint32_t err = BNO055_INIT_VALUE;
    err += bno055_get_accel_calib_stat(&accel_calib_status);
    err += bno055_get_mag_calib_stat(&mag_calib_status);
    err += bno055_get_gyro_calib_stat(&gyro_calib_status);
    err += bno055_get_sys_calib_stat(&sys_calib_status);

    NRF_LOG_INFO("BNO055 CALIB STAT: acc %d, mag %d, gyr %d, sys %d", 
      accel_calib_status, mag_calib_status, gyro_calib_status, sys_calib_status);
}

bool bno055_read_raw(struct bno055_accel_t* accel_xyz, struct bno055_mag_t* mag_xyz, struct bno055_gyro_t* gyro_xyz) {
    uint32_t err = BNO055_INIT_VALUE;
    err += bno055_read_accel_xyz(accel_xyz);
    err += bno055_read_mag_xyz(mag_xyz);
    err += bno055_read_gyro_xyz(gyro_xyz);
    return (err == BNO055_SUCCESS);
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

