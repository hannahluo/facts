#include <stdio.h>

#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c.h"
// #include "bno055.h"
#include "imu.h"
#include "tca9548a.h"
#include "drv2605l.h"
//#include "haptic_motors.h"

// Test defines
#define GYRO_TESTING
#define QUAT_TESTING

#define DRV_I2C_ADDR     0x5A
#define ELSA_I2C_IMUADDR 0x29
#define ANNA_I2C_IMUADDR 0x28

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static struct bno055_t elsa_imu;
static struct bno055_t anna_imu;

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

// Initialize I2C
int8_t i2c_module_init()
{
    NRF_LOG_INFO("\r\n***I2C Setup Start***\r\n");
    if(!i2c_init(&i2c_drv)) {
        NRF_LOG_ERROR("Failed to init i2c");
        return -1;
    }
    while (nrf_drv_twi_is_busy(&i2c_drv)) {};
    nrf_delay_ms(1000);
    NRF_LOG_INFO("\r\n***I2C Setup End***\r\n");
    
    return 0;
}

int8_t imu_module_init()
{
    NRF_LOG_INFO("\r\n***IMU Setup Start***\r\n");
    if(!bno055_setup(&elsa_imu, &i2c_drv, ELSA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to init elsa imu");
        return -1;
    }
    if(!bno055_setup(&anna_imu, &i2c_drv, ANNA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to init anna imu");
        return -1;
    }
    nrf_delay_ms(100);

    // remap?

    NRF_LOG_INFO("\r\n***IMU Setup End***\r\n");

    return 0;
}

void cleanup()
{
    // IMUs
    deinit(&elsa_imu);
    deinit(&anna_imu);

    // i2c
    i2c_deinit(&i2c_drv);

    // Push all logs out
    NRF_LOG_FLUSH();
}

// testing gyro
int8_t test_gyro()
{
    // set rps units
    if(!bno055_set_gyr_unit(BNO055_GYRO_UNIT_RPS, &i2c_drv, ELSA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to set elsa gyro IMU units");
        return -1;
    }

    if(!bno055_set_gyr_unit(BNO055_GYRO_UNIT_RPS, &i2c_drv, ANNA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to set anna gyro IMU units");
        return -1;
    }

    // Continually read and print gyro values
    struct bno055_gyro_double_t gyroCalf = {.x=0,.y=0,.z=0};
    struct bno055_gyro_double_t gyroThigh = {.x=0,.y=0,.z=0};
    while(1) {
        if(!bno055_convert_double_gyr_xyz_rps(&gyroCalf, &i2c_drv, ANNA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read anna(calf) gyro");
            return -1;
        }
        NRF_LOG_INFO("Anna(Calf) Gyro Readings: ");
        NRF_LOG_INFO("X: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(gyroCalf.x));
        NRF_LOG_INFO("Y: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(gyroCalf.y));
        NRF_LOG_INFO("Z: " NRF_LOG_FLOAT_MARKER " \r\n", NRF_LOG_FLOAT(gyroCalf.z));

        if(!bno055_convert_double_gyr_xyz_rps(&gyroThigh, &i2c_drv, ELSA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read elsa(thigh) gyro");
            return -1;
        }

        NRF_LOG_INFO("Elsa(Thigh) Gyro Readings: ");
        NRF_LOG_INFO("X: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(gyroThigh.x));
        NRF_LOG_INFO("Y: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(gyroThigh.y));
        NRF_LOG_INFO("Z: " NRF_LOG_FLOAT_MARKER " \r\n", NRF_LOG_FLOAT(gyroThigh.z));
    }

    return 0;
}

// testing quat
int8_t test_quat()
{
    vector_t thighAxisVec = {.x=thighAxis.x, .y=thighAxis.y, .z=thighAxis.z};
    vector_t calfAxisVec = {.x=calfAxis.x, .y=calfAxis.y, .z=calfAxis.z};
    struct bno055_quaternion_t bno055QuatCalf = {.w=0, .x=0, .y=0, .z=0};
    struct bno055_quaternion_t bno055QuatThigh = {.w=0, .x=0, .y=0, .z=0};
    quat_t quatCalf = {.w=0,.x=0,.y=0,.z=0};
    quat_t quatThigh = {.w=0,.x=0,.y=0,.z=0};
    uint8_t motorsRunning = 0;

    update_joint_axes(&calfAxisVec, &thighAxisVec);
    double angle = 0;
    while(1) {
        // Read quaternion
        if(!bno055_read_quat(&bno055QuatCalf, &i2c_drv, ANNA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read quaternion from anna(calf)");
            return -1;
        }
        if(!bno055_read_quat(&bno055QuatThigh, &i2c_drv, ELSA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read quaternion from elsa(thigh)");
            return -1;
        }

        // Convert to double
        conv_quat_double(&bno055QuatCalf, &quatCalf);
        NRF_LOG_INFO("Anna(Calf) Quat Readings: ");
        NRF_LOG_INFO("W: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatCalf.w));
        NRF_LOG_INFO("X: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatCalf.x));
        NRF_LOG_INFO("Y: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatCalf.y));
        NRF_LOG_INFO("Z: " NRF_LOG_FLOAT_MARKER " \r\n", NRF_LOG_FLOAT(quatCalf.z));

        conv_quat_double(&bno055QuatThigh, &quatThigh);
        NRF_LOG_INFO("Elsa(Thigh) Quat Readings: ");
        NRF_LOG_INFO("W: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatThigh.w));
        NRF_LOG_INFO("X: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatThigh.x));
        NRF_LOG_INFO("Y: " NRF_LOG_FLOAT_MARKER " ", NRF_LOG_FLOAT(quatThigh.y));
        NRF_LOG_INFO("Z: " NRF_LOG_FLOAT_MARKER " \r\n", NRF_LOG_FLOAT(quatThigh.z));
        
        // Calc Shit
        angle = calculate_angle(&quatCalf, &quatThigh);
        NRF_LOG_INFO("Angle Estimation: " NRF_LOG_FLOAT_MARKER " \r\n", NRF_LOG_FLOAT(angle));
    }
}

int main()
{
    log_init();
#if defined(GYRO_TESTING) && defined(QUAT_TESTING)
    NRF_LOG_ERROR("Gyro and quat testing enabled");
    return -1;
#endif

#if !defined(GYRO_TESTING) && !defined(QUAT_TESTING)
    NRF_LOG_ERROR("No testing enabled");
    return -1;
#endif

#ifdef GYRO_TESTING
    NRF_LOG_INFO("Gyro testing enabled");
#endif
#ifdef QUAT_TESTING
    NRF_LOG_INFO("Quat testing enabled");
#endif

    if(i2c_module_init() < 0) {
        NRF_LOG_ERROR("Failed to init i2c");
        NRF_LOG_FLUSH();
        i2c_deinit(&i2c_drv);
        return -1;
    }
    if(imu_module_init() < 0) {
        NRF_LOG_ERROR("Failed to init imu");
        cleanup();
        return -1;
    }

    NRF_LOG_INFO("Reading calibration");
    bno055_get_calibration_status(&i2c_drv, ELSA_I2C_IMUADDR);
    bno055_get_calibration_status(&i2c_drv, ANNA_I2C_IMUADDR);
    NRF_LOG_INFO("Done");

#ifdef GYRO_TESTING
    test_gyro();
#endif
#ifdef QUAT_TESTING
    test_quat();
#endif

    NRF_LOG_INFO("IMU Testing Done");
    cleanup();

}