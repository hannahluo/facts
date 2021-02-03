#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "haptic_motors.h"

typedef bool (*vibrate)(nrf_twi_sensor_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);

// static constexpr uint8_t kInitSuccess = 0xE0;
// static constexpr uint8_t kByteLen = 1u;
static constexpr uint8_t kHapticMotorDefault = 0u;

// TODO: might need a setup motors function
typedef struct {
    drv2605l_t* motors;
    uint8_t num_motors;
    vibrate haptic_motors_vibrate;
} haptic_motors_t;

bool haptic_motors_init(const haptic_motors_t* motors);
bool haptic_motors_vibrate_impl(drv2605l_t* motors, uint8_t num_motors);
bool haptic_motors_deinit(const haptic_motors_t* motor_ctrl);