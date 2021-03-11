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
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bluetooth.h"
#include "i2c.h"
#include "tca9548a.h"
#include "drv2605l.h"

APP_TIMER_DEF(m_double_timer_id);
APP_TIMER_DEF(m_uint_timer_id);

#define TIMER_TIMEOUT_TICKS             APP_TIMER_TICKS(1000)

#define DRV_I2C_ADDR     0x5A

#define TCA_SELECT_REG   0
#define TCA_SELECT_SIZE  1

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static tca9548a_t elsa_mux; 
static drv2605l_t elsa_motor;

static const uint8_t HAPTIC_MOTOR_CH0 = 1 << 0;
static const uint8_t HAPTIC_MOTOR_CH1 = 1 << 1;

static char string1[] = "String1!";
static char string2[] = "String2!";
static char* a_string[] = {string1, string2};

void vibrate() {
    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
    drv2605l_go(&elsa_motor);
    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
    drv2605l_go(&elsa_motor);

    nrf_delay_ms(5000);

    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
    drv2605l_stop(&elsa_motor);
    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
    drv2605l_stop(&elsa_motor);
}

// Timer timeout event handler
void double_timer_evt_handler(void* p_context)
{
    // To-do
    static double cnt = 8;
    cnt += 0.01; // Hannah Luo this is where the angle gets updated
    raw_gyro_t gyro_calf_fake_data = {.x=cnt, .y=cnt+1, .z=cnt+2};
    //send_raw_gyro_calf(&gyro_calf_fake_data);

    raw_gyro_t gyro_thigh_fake_data = {.x=cnt+2, .y=cnt+1, .z=cnt};
    //send_raw_gyro_thigh(&gyro_thigh_fake_data);

    flexion_angle_t angle = cnt;
    send_flexion_angle(&angle); // Hannah Luo this is where we send it over BT

    if (angle > 90.0) vibrate();
}
void uint_timer_evt_handler(void* p_context)
{
    // To-do
    static uint8_t cnt = 0;
    ++cnt;
    
    init_calib_t val;
    if(cnt%2==0) {
      val = true;
    } else {
      val = false;
    }
    send_init_calib(val);

    if(cnt > NUM_CALC_SERVICE_ERRORS) {
        cnt = 0;
    }
    calc_err_t err = cnt;
    //send_calc_error(err);
}

static void application_timers_start(void)
{
    /* YOUR_JOB: Start your timers. below is an example of how to start a timer.
       ret_code_t err_code;
       err_code = app_timer_start(m_app_timer_id, TIMER_INTERVAL, NULL);
       APP_ERROR_CHECK(err_code); */
    // Jack Zhu
    ret_code_t err_code;
    err_code = app_timer_start(m_double_timer_id, TIMER_TIMEOUT_TICKS, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(m_uint_timer_id, TIMER_TIMEOUT_TICKS, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    // Initialize timer module.
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    // Create timers.

    // Jack Zhu
    err_code = app_timer_create(&m_double_timer_id, APP_TIMER_MODE_REPEATED, &double_timer_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_uint_timer_id, APP_TIMER_MODE_REPEATED, &uint_timer_evt_handler);
    APP_ERROR_CHECK(err_code);

    APP_ERROR_CHECK(err_code);
}

void calf_joint_axis_handler(void const * data, uint8_t size)
{
    // print out data
    joint_axis_t* jointAxis = (joint_axis_t*)(data);
}

void thigh_joint_axis_handler(void const * data, uint8_t size)
{
    // print out data
    joint_axis_t* jointAxis = (joint_axis_t*)(data);
}

void init_cal_handler(void const * data, uint8_t size)
{
    bool isCal = *(bool*)(data);
    if(isCal) {
        NRF_LOG_DEBUG("init_cal_handler: init calibration on");
    } else {
        NRF_LOG_DEBUG("init_cal_handler: init calibration off");
    }
}

void limits_handler(void const* data, uint8_t size)
{
    limits_t* limits = (limits_t*)data;
}


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
    
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

#define FICR_ADDR       (0x10000000)
#define DEVICE_ID_0     (0x060)
#define DEVICE_ID_1     (0x064)

int main(void)
{
    bool erase_bonds=false;
    log_init();

    uint32_t device_id[2] = {0};
    uint32_t* device_id_ptr = (uint32_t*)(FICR_ADDR+DEVICE_ID_0);
    device_id[0] = device_id_ptr[0];
    device_id[1] = device_id_ptr[1];
    NRF_LOG_INFO("Starting device id 0x%x%x", device_id[0], device_id[1]);

    // Initialize.
    timers_init();
    power_management_init();

    // BLE
    bluetooth_init();
    register_init_cal_handler(&init_cal_handler);
    register_calf_joint_axis_handler(&calf_joint_axis_handler);
    register_thigh_joint_axis_handler(&thigh_joint_axis_handler);
    register_limits_handler(&limits_handler);

    // I2C
    i2c_init(&i2c_drv);
    while (nrf_drv_twi_is_busy(&i2c_drv)) {};
    nrf_delay_ms(1000);

    tca9548a_init(&elsa_mux, 0x70, &i2c_drv);

    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
    drv2605l_init(&elsa_motor, DRV_I2C_ADDR, &elsa_mux);
    drv2605l_mode(&elsa_motor, 0);
    drv2605l_library(&elsa_motor, 6);
    drv2605l_waveform(&elsa_motor, 0, 47);
    drv2605l_waveform(&elsa_motor, 1, 0);

    tca9548a_write(&i2c_drv, 0x70, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
    drv2605l_init(&elsa_motor, DRV_I2C_ADDR, &elsa_mux);
    drv2605l_mode(&elsa_motor, 0);
    drv2605l_library(&elsa_motor, 6);
    drv2605l_waveform(&elsa_motor, 0, 47);
    drv2605l_waveform(&elsa_motor, 1, 0);

    // Start execution.
    NRF_LOG_INFO("FACTS started");
    application_timers_start();
    start_advertising();

    // Enter main loop.
    for (;;)
    {
        idle_state_handle();
    }
}
