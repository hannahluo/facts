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

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "bluetooth.h"

APP_TIMER_DEF(m_double_timer_id);
APP_TIMER_DEF(m_uint_timer_id);

#define TIMER_TIMEOUT_TICKS             APP_TIMER_TICKS(1000)

static char string1[] = "String1!";
static char string2[] = "String2!";
static char* a_string[] = {string1, string2};

// Timer timeout event handler
void double_timer_evt_handler(void* p_context)
{
    // To-do
    static double cnt = 8;
    cnt += 0.01;
    raw_gyro_t gyro_fake_data = {.x=cnt, .y=cnt+1, .z=cnt+2};
    send_raw_gyro(&gyro_fake_data);

    raw_accel_t accel_fake_data = {.x=cnt+2, .y=cnt+1, .z=cnt};
    send_raw_accel(&accel_fake_data);

    flexion_angle_t angle = cnt;
    send_flexion_angle(&angle);
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
    send_calc_error(err);
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



int main(void)
{
    bool erase_bonds=false;

    // Initialize.
    log_init();
    timers_init();
    power_management_init();

    // BLE
    bluetooth_init();
    register_init_cal_handler(&init_cal_handler);
    register_calf_joint_axis_handler(&calf_joint_axis_handler);
    register_thigh_joint_axis_handler(&thigh_joint_axis_handler);
    register_limits_handler(&limits_handler);

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