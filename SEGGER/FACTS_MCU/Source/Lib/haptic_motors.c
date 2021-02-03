#include "haptic_motors.h"

bool haptic_motors_init(const haptic_motors_t* motor_ctrl) {
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motor_ctrl->num_motors; ++i) {
        err = drv2506l_init(motor_ctrl->motors);

        if (err == false) {
            // log something
            return err;
        }
    }

    motor_ctrl->haptic_motors_vibrate = haptic_motors_vibrate_impl;

    return err;
}

bool haptic_motors_vibrate_impl(bool haptic_motors_vibrate_impl(drv2605l_t* motors, uint8_t num_motors) {
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motor_ctrl->num_motors; ++i) {
        // do something

        if (err == false) {
            // log something
            return err;
        }
    }

    return err;
}

bool drv2605l_deinit(const haptic_motors_t* motor_ctrl) {
    uint8_t i = kHapticMotorDefault;
    bool err = false;
    for (i = kHapticMotorDefault; i < motor_ctrl->num_motors; ++i) {
        err = drv2506l_deinit(motor_ctrl->motors);
    }

    return true; // possibly not needed
}