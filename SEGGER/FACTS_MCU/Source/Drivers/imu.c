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

    /* Make sure we have the right device */
    uint8_t id = BNO055_INIT_VALUE;
    err += bno055_read_chip_id(&id);
    if (err == BNO055_SUCCESS && id != BNO055_ID) {
        nrf_delay_ms(1000); // hold on for boot
        err += bno055_read_chip_id(&id);
        if (id != BNO055_ID) {
            return false; // still not? ok bail
        }
    }

    /* Switch to config mode (just in case since this is the default) */
    err += bno055_set_operation_mode(BNO055_OPERATION_MODE_CONFIG);

    /* Reset */
    err += bno055_set_sys_rst(BNO055_SYS_RST_MSK);
    /* Delay incrased to 30ms due to power issues https://tinyurl.com/y375z699 */
    nrf_delay_ms(30);
    while (bno055_read_chip_id(&id) != BNO055_ID) {
        nrf_delay_ms(10);
    }
    nrf_delay_ms(50);

    /* Set to normal power mode */
    err += bno055_set_power_mode(BNO055_POWER_MODE_NORMAL);
    nrf_delay_ms(10);

    err += bno055_write_page_id(BNO055_PAGE_ZERO);

    uint8_t trigger_addy = BNO055_SYS_TRIGGER_ADDR;
    uint8_t clear_reg = BNO055_INIT_VALUE;
    err += BNO055_I2C_bus_write(i2c, dev_addr, trigger_addy, &clear_reg, 1);
    nrf_delay_ms(10);
   
    /* Set the requested operating mode (see section 3.3) */
    //bno055_set_operation_mode(mode);
    //delay(20);

    return (err == BNO055_SUCCESS);
};

void bno055_set_external_xtal(bool use_xtal) {
    uint8_t op_mode = BNO055_INIT_VALUE;
    uint32_t err = bno055_get_operation_mode(&op_mode);

    /* Switch to config mode (just in case since this is the default) */
    err += bno055_set_operation_mode(BNO055_OPERATION_MODE_CONFIG);
    nrf_delay_ms(25);
    err += bno055_write_page_id(BNO055_PAGE_ZERO);
    if (use_xtal) {
        err += bno055_set_clk_src(BNO055_CLK_SRC_MSK);
    } else {
        err += bno055_set_clk_src(BNO055_INIT_VALUE);
    }
    nrf_delay_ms(10);
    /* Set the requested operating mode (see section 3.3) */
    err += bno055_set_operation_mode(op_mode);
    nrf_delay_ms(20);
}

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