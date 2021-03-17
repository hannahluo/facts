#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "drv2605l.h"

//typedef bool (*vibrate)(drv2605l_t*, uint8_t);

// static constexpr uint8_t kInitSuccess = 0xE0;
// static constexpr uint8_t kByteLen = 1u;
static const uint8_t kHapticMotorDefault = 0u;

// TODO: might need a setup motors function
typedef struct {
    drv2605l_t* motors;
    uint8_t num_motors;
    //vibrate haptic_motors_vibrate;
} haptic_motors_t;

bool haptic_motors_init(haptic_motors_t* motors);
bool haptic_motors_vibrate(drv2605l_t* motors, uint8_t num_motors);
bool haptic_motors_deinit(haptic_motors_t* motor_ctrl);