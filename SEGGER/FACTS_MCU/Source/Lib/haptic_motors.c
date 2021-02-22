#include "haptic_motors.h"
#include "nrf_log.h"

bool haptic_motors_init(const haptic_motors_t* motor_ctrl) {
    NRF_LOG_INFO("initializing haptic motors");
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motor_ctrl->num_motors; ++i) {
        err = drv2605l_init(motor_ctrl->motors);

        if (err == false) {
            NRF_LOG_WARNING("haptic motor init error");
            return err;
        }
    }

    //motor_ctrl->haptic_motors_vibrate = haptic_motors_vibrate_impl;

    return err;
}

bool haptic_motors_vibrate(drv2605l_t* motors, uint8_t num_motors) {
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motors->num_motors; ++i) {
        // do something

        if (err == false) {
            NRF_LOG_WARNING("haptic motor vibrate error");
            return err;
        }
    }

    return err;
}

bool haptic_motors_deinit(haptic_motors_t* motor_ctrl) {
    NRF_LOG_INFO("deinitializing haptic motors");
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motor_ctrl->num_motors; ++i) {
        err = drv2605l_deinit(motor_ctrl->motors);
    }

    return true; // possibly not needed
}