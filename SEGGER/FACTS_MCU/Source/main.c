#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_timer.h"
#include "fds.h"
#include "sensorsim.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bluetooth.h"
#include "angle_calculation.h"
#include "i2c.h"
#include "bno055.h"
#include "imu.h"
#include "tca9548a.h"
#include "drv2605l.h"

#define DRV_I2C_ADDR     0x5A
#define ELSA_I2C_IMUADDR 0x29
#define ANNA_I2C_IMUADDR 0x28
#define ELSA_I2C_MUXADDR 0x71
#define ANNA_I2C_MUXADDR 0x70

#define TCA_SELECT_REG   0
#define TCA_SELECT_SIZE  1

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static  nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static struct bno055_t elsa_imu;
static struct bno055_t anna_imu;

static tca9548a_t elsa_mux; 
static drv2605l_t elsa_motor;

static tca9548a_t anna_mux;
static drv2605l_t anna_motor;

static uint8_t HAPTIC_MOTOR_CH0 = 1 << 0;
static uint8_t HAPTIC_MOTOR_CH1 = 1 << 1;
static uint8_t HAPTIC_MOTOR_CH2 = 1 << 2;

static uint8_t start_cal = 0;
static joint_axis_t calfAxis = {.x=0,.y=0,.z=0};
static joint_axis_t thighAxis = {.x=0,.y=0,.z=0};
static uint8_t calfAxisUpdated = 0;
static uint8_t thighAxisUpdated = 0;
static limits_t angleLim = {.minLimit = 0, .maxLimit = 100};

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.
    /*err_code = app_timer_create(&m_double_timer_id, APP_TIMER_MODE_REPEATED, &double_timer_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_uint_timer_id, APP_TIMER_MODE_REPEATED, &uint_timer_evt_handler);
    APP_ERROR_CHECK(err_code);*/
}

static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

void calf_joint_axis_handler(void const * data, uint8_t size)
{
    // print out data
    if(size >= sizeof(joint_axis_t)) {
        memcpy(&calfAxis, data, sizeof(joint_axis_t));
        calfAxisUpdated = 1;
    } else {
        // improper, use default
        calfAxis.x = 1;
        calfAxis.y = 0;
        calfAxis.z = 0;
        calfAxisUpdated = 1;
    }
}

void thigh_joint_axis_handler(void const * data, uint8_t size)
{
    // update
    if(size >= sizeof(joint_axis_t)) {
        joint_axis_t* jointAxis = (joint_axis_t*)(data);
        memcpy(&thighAxis, data, sizeof(joint_axis_t));
        thighAxisUpdated = 1;
    } else {
        // use default
        thighAxis.x = 1;
        thighAxis.y = 0;
        thighAxis.z = 0;
        thighAxisUpdated = 1;
    }
}

void init_cal_handler(void const * data, uint8_t size)
{
    // Whenever recv anything, start cal
    start_cal = 1;
}

// Initialize ble module
void ble_module_init()
{ 
    NRF_LOG_INFO("\r\n ***BLE Setup Start*** \r\n");
    bluetooth_init();
    register_init_cal_handler(&init_cal_handler);
    register_calf_joint_axis_handler(&calf_joint_axis_handler);
    register_thigh_joint_axis_handler(&thigh_joint_axis_handler);
    register_limits_handler(NULL);
    NRF_LOG_INFO("\r\n ***BLE Setup End*** \r\n");
}

int8_t motor_init(drv2605l_t* motor, tca9548a_t* mux, uint8_t mux_addr)
{
    if(!tca9548a_init(mux, mux_addr, &i2c_drv)) {
        NRF_LOG_ERROR("Failed to init comm to mux");
        return -1;
    }
    
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch0");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch1");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch2");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);

    return 0;
}

// Initialize haptic
int8_t haptic_module_init()
{
    NRF_LOG_INFO("\r\n ***Motor Setup Start*** \r\n");
    if(motor_init(&elsa_motor, &elsa_mux, ELSA_I2C_MUXADDR) < 0) {
        NRF_LOG_ERROR("Failed to initialize elsa(thigh) haptics");
        return -1;
    }
    if(motor_init(&anna_motor, &anna_mux, ANNA_I2C_MUXADDR) < 0) {
        NRF_LOG_ERROR("Failed to initialize anna(calf) haptics");
        return -1;
    }
    NRF_LOG_INFO("\r\n ***Motor Setup End*** \r\n");

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

// Calibration
void calibrate_imu()
{
    bno055_get_calibration_status(&i2c_drv, ELSA_I2C_IMUADDR);
    bno055_get_calibration_status(&i2c_drv, ANNA_I2C_IMUADDR);
} 

// Calibration
int8_t calibrate_facts()
{
    struct bno055_gyro_double_t bno055GyroThigh = {.x=0,.y=0,.z=0};
    struct bno055_gyro_double_t bno055GyroCalf = {.x=0,.y=0,.z=0};

    raw_gyro_t gyroCalf = {.x = 0,.y=0,.z=0};
    raw_gyro_t gyroThigh = {.x=0,.y=0,.z=0};

    // set rps units
    if(!bno055_set_gyr_unit(BNO055_GYRO_UNIT_RPS, &i2c_drv, ELSA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to set elsa gyro IMU units");
        return -1;
    }

    if(!bno055_set_gyr_unit(BNO055_GYRO_UNIT_RPS, &i2c_drv, ANNA_I2C_IMUADDR)) {
        NRF_LOG_ERROR("Failed to set anna gyro IMU units");
        return -1;
    }

    // Wait for app to start FACTS cal sequence
    while(start_cal){};

    // Wait for joint axis to be recvd
    while(!calfAxisUpdated || !thighAxisUpdated) {
        // Read
        if(!bno055_convert_double_gyr_xyz_rps(&bno055GyroThigh, &i2c_drv, ELSA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read raw gyro from elsa(thigh)");
            return -1;
        }
        if(!bno055_convert_double_gyr_xyz_rps(&bno055GyroCalf, &i2c_drv, ANNA_I2C_IMUADDR)) {
            NRF_LOG_ERROR("Failed to read raw gyro from anna(calf)");
            return -1;
        }
        
        gyroCalf.x = bno055GyroCalf.x;
        gyroCalf.y = bno055GyroCalf.y;
        gyroCalf.z = bno055GyroCalf.z;
        gyroThigh.x = bno055GyroThigh.x;
        gyroThigh.y = bno055GyroThigh.y;
        gyroThigh.z = bno055GyroThigh.z;

        // Write to BLE module
        send_raw_gyro_calf(&gyroCalf);
        send_raw_gyro_thigh(&gyroThigh);

        // Wait for bus to clear
        nrf_delay_ms(100);
    }

    return 0;
}

void conv_quat_double(struct bno055_quaternion_t* bno055Quat, quat_t* quat)
{
    const double scale = (1.0 / (1 << 14));
    quat->w = (double)bno055Quat->w * scale;
    quat->x = (double)bno055Quat->x * scale;
    quat->y = (double)bno055Quat->y * scale;
    quat->z = (double)bno055Quat->z * scale;
}

int8_t turn_on_motors(drv2605l_t* motor, uint8_t mux_addr)
{
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for go");
        return -1;
    }
    drv2605l_go(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for go");
        return -1;
    }
    drv2605l_go(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for go");
        return -1;
    }
    drv2605l_go(motor);

    return 0;
}

int8_t turn_off_motors(drv2605l_t* motor, uint8_t mux_addr)
{
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for stop");
        return -1;
    }
    drv2605l_stop(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for stop");
        return -1;
    }
    drv2605l_stop(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for stop");
        return -1;
    }
    drv2605l_stop(motor);

    return 0;
}

// Calculate
int8_t get_facts()
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
        conv_quat_double(&bno055QuatThigh, &quatThigh);
        
        // Calc Shit
        angle = calculate_angle(&quatCalf, &quatThigh);

        // Send to BLE
        send_flexion_angle(&angle);

        // Check against limits
        if((angle < angleLim.minLimit || angle > angleLim.maxLimit)) {
            if(!motorsRunning) {
                // Turn on
                motorsRunning = 1;
                if(turn_on_motors(&elsa_motor, ELSA_I2C_MUXADDR) < 0) {
                    NRF_LOG_ERROR("Failed to turn on elsa motor");
                    return -1;
                }
                if(turn_on_motors(&anna_motor, ANNA_I2C_MUXADDR) < 0) {
                    NRF_LOG_ERROR("Failed to turn off elsa motor");
                    return -1;
                }
            }
        } else {
            if(motorsRunning) {
                // Turn off
                motorsRunning = 0;
                if(turn_off_motors(&elsa_motor, ELSA_I2C_MUXADDR) < 0) {
                    NRF_LOG_ERROR("Failed to turn off elsa motor");
                    return -1;
                }
                if(turn_off_motors(&anna_motor, ANNA_I2C_MUXADDR) < 0) {
                    NRF_LOG_ERROR("Failed to turn off anna motor");
                    return -1;
                }
            }
        }
    }

    return 0;
}

void cleanup()
{
    // IMUs
    deinit(&elsa_imu);
    deinit(&anna_imu);

    // motor
    drv2605l_deinit(&elsa_motor);
    drv2605l_deinit(&anna_motor);

    // mux
    tca9548a_deinit(&elsa_mux);
    tca9548a_deinit(&anna_mux);

    // i2c
    i2c_deinit(&i2c_drv);

    // Push all logs out
    NRF_LOG_FLUSH();
}

#define FICR_BASE_ADDR         (0x10000000)
#define DEVICE_ID_0_OFFSET     (0x060)
#define DEVICE_ID_1_OFFSET     (0x064)

int main()
{
    log_init();
    uint32_t device_id[2] = {0};
    uint32_t* device_id_ptr = (uint32_t*)(FICR_BASE_ADDR+DEVICE_ID_0_OFFSET);
    device_id[0] = device_id_ptr[0];
    device_id[1] = device_id_ptr[1];
    NRF_LOG_INFO("Starting device id 0x%x%x", device_id[0], device_id[1]);

    // Initialize.
    timers_init();
    power_management_init();
    ble_module_init();
    if(i2c_module_init() < 0) {
        NRF_LOG_ERROR("Failed to init i2c");
        cleanup();
        return -1;
    }
    if(imu_module_init() < 0) {
        NRF_LOG_ERROR("Failed to init imu");
        cleanup();
        return -1;
    }
    if(haptic_module_init() < 0) {
        NRF_LOG_ERROR("Failed to init haptics");
        cleanup();
        return -1;
    }

    // Calibration
    calibrate_imu();
    if(calibrate_facts() < 0) {
        NRF_LOG_ERROR("Failed to calibrate FACTS properly. Shutting down");
        cleanup();
        return -1;
    }
    
    // Calculation
    if(get_facts() < 0) {
        NRF_LOG_ERROR("Failed to get FACTS");
        cleanup();
        return -1;
    }

    NRF_LOG_INFO("***Shutting Down Facts***");
}
