#include "imu.h"
#include "nrf_log.h"
#include "nrf_delay.h"

int8_t I2C_routine(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    bno055->bus_write = BNO055_I2C_bus_write;
    bno055->bus_read = BNO055_I2C_bus_read;
    bno055->delay_msec = BNO055_delay_msek;
    bno055->dev_addr = dev_addr;
    bno055->i2c = i2c;

    return BNO055_INIT_VALUE;
}

bool bno055_setup(struct bno055_t* bno055, nrf_drv_twi_t* i2c, uint8_t dev_addr) {
    NRF_LOG_INFO("initializing imu");
    I2C_routine(bno055, i2c, dev_addr);

    uint32_t err = bno055_init(bno055);

    uint8_t power_mode = BNO055_POWER_MODE_NORMAL;
    err += bno055_set_power_mode(power_mode);

    return (err == BNO055_SUCCESS);
};

void bno055_calib_stat() {
    uint8_t accel_calib_status = BNO055_INIT_VALUE;
    uint8_t gyro_calib_status = BNO055_INIT_VALUE;
    uint8_t mag_calib_status = BNO055_INIT_VALUE;
    uint8_t sys_calib_status = BNO055_INIT_VALUE;
    uint32_t err = BNO055_INIT_VALUE;
    err += bno055_get_accel_calib_stat(&accel_calib_status);
    err += bno055_get_mag_calib_stat(&mag_calib_status);
    err += bno055_get_gyro_calib_stat(&gyro_calib_status);
    err += bno055_get_sys_calib_stat(&sys_calib_status);

    // do something
}

bool bno055_read_raw(struct bno055_accel_t* accel_xyz, struct bno055_mag_t* mag_xyz, struct bno055_gyro_t* gyro_xyz) {
  uint32_t err = BNO055_INIT_VALUE;
  err += bno055_set_operation_mode(BNO055_OPERATION_MODE_AMG);
  err += bno055_read_accel_xyz(accel_xyz);
  err += bno055_read_mag_xyz(mag_xyz);
  err += bno055_read_gyro_xyz(gyro_xyz);
  return (err == BNO055_SUCCESS);
}

int8_t BNO055_I2C_bus_write(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int32_t BNO055_iERROR = BNO055_INIT_VALUE;
    uint8_t array[I2C_BUFFER_LEN];
    uint8_t stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;
    for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
    {
        array[stringpos + BNO055_I2C_BUS_WRITE_ARRAY_INDEX] = *(reg_data + stringpos);
    }

    NRF_LOG_INFO("bus write b4");
    BNO055_iERROR = (int8_t)i2c_write(i2c, dev_addr, &array[BNO055_INIT_VALUE], cnt);
    NRF_LOG_INFO("bus write after");

    return (int8_t)BNO055_iERROR;
}

int8_t BNO055_I2C_bus_read(nrf_drv_twi_t* i2c, uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt)
{
    int32_t BNO055_iERROR = BNO055_INIT_VALUE;
    uint8_t array[I2C_BUFFER_LEN];
    uint8_t stringpos = BNO055_INIT_VALUE;

    array[BNO055_INIT_VALUE] = reg_addr;

    BNO055_iERROR = (int8_t)i2c_read(i2c, dev_addr, &array[BNO055_INIT_VALUE], cnt);
    if (BNO055_iERROR == BNO055_SUCCESS) {
        for (stringpos = BNO055_INIT_VALUE; stringpos < cnt; stringpos++)
        {
            *(reg_data + stringpos) = array[stringpos];
        }
    }

    return (int8_t)BNO055_iERROR;
}

void BNO055_delay_msek(uint32_t ms)
{
    nrf_delay_ms(ms);
}

bool deinit(struct bno055_t* bno055) {
  uint8_t power_mode = BNO055_POWER_MODE_SUSPEND;
  uint32_t err = bno055_set_power_mode(power_mode);

  return (err == BNO055_SUCCESS);
}