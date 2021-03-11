#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "imu_service.h"
#include "calibration_service.h"
#include "calculation_service.h"

// Initialization functions
void bluetooth_init();

// Start advertising to other devices (called after init)
void start_advertising();

// Register handlers for different events (must be called after init)
void register_init_cal_handler(ble_calib_evt_handler_t handler);
void register_calf_joint_axis_handler(ble_calib_evt_handler_t handler);
void register_thigh_joint_axis_handler(ble_calib_evt_handler_t handler);
void register_limits_handler(ble_calc_evt_handler_t handler);

// Functions to send messages to app
void send_raw_gyro_calf(raw_gyro_t* gyroReading);
void send_raw_gyro_thigh(raw_gyro_t* gyroReading);
void send_flexion_angle(flexion_angle_t* angle);    // flexion_angle_t == double
void send_init_calib(init_calib_t cal);             // init_calib_t == bool
void send_calc_error(calc_err_t err);               // calc_err_t == enum

#endif