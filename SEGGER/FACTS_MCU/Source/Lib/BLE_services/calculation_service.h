/*
 * Header file for service which handles calibration messaging
 */
#ifndef CALC_SERVICE_H
#define CALC_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_CALC_SERVICE_BLE_OBSERVER_PRIO   3

// Register event handler to be invoked on BLE events
#define BLE_CALC_SERVICE_DEF(_name)                         \
static ble_calc_service_t _name                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                         \
                     BLE_CALC_SERVICE_BLE_OBSERVER_PRIO,    \
                     ble_calc_service_on_ble_evt, &name) 

/* 
 *  FACTS Calibration Service: 87C539B0-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Flexion Angle     87C539B1-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Min Threshold     87C539B2-8E33-4070-9131-8F56AA023E45
 *      Characteristic 3: Max Threshold     87C539B3-8E33-4070-9131-8F56AA023E45
 *      Characteristic 4: Change Frequency  87C539B4-8E33-4070-9131-8F56AA023E45
 *      Characteristic 5: Error             87C539B5-8E33-4070-9131-8F56AA023E45
 */

// Base UUID: 87C53994-8E33-4070-9131-8F56AA023E45
// Stored in little endian
#define BLE_UUID_CALIB_SERVICE_BASE_UUID  {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x00, 0x00, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_CALC_SERVICE_UUID      (0x39B1)
#define BLE_UUID_FLEX_ANG_UUID          (0x39B2)
#define BLE_UUID_MIN_LIM_UUID           (0x39B3)
#define BLE_UUID_MAX_LIM_UUID           (0x39B4)
#define BLE_UUID_CHANGE_FREQ_UUID       (0x39B5)
#define BLE_UUID_CALC_ERR_UUID          (0x39B6)

#define NUM_CALC_SERVICE_CHARS          (5)

// Calculation service structures
typedef struct ble_calc_service_s ble_calc_service_t;

typedef enum
{
    BLE_FLEX_ANG_EVT_NOTIFY_ENABLED,
    BLE_FLEX_ANG_EVT_NOTIFY_DISABLED,
    BLE_MIN_LIM_WRITE,
    BLE_MAX_LIM_WRITE,
    BLE_CHANGE_FREQ_WRITE,
    BLE_CALC_ERR_NOTIFY_ENABLED,
    BLE_CALC_ERR_NOTIFY_DISABLED,
    NUM_BLE_CALC_EVT,
} ble_calc_evt_type_t;

typedef struct
{
    ble_calc_evt_type_t evt_type;
} ble_calc_evt_t;

// Function pointer for per-characteristic event handling
typedef void (*ble_calc_evt_handler_t) (ble_calc_service_t* p_calc_service, ble_calc_evt_t* p_evt);

// Main handler for calculation service
typedef struct ble_calc_service_s
{
    uint16_t conn_handle,
    uint16_t service_handle,
    uint8_t uuid_type,
    ble_calc_evt_handler_t evt_handler[NUM_CALC_SERVICE_CHARS];
    ble_gatts_char_handles_t char_handles[NUM_CALC_SERVICE_CHARS];
} ble_calc_service_t;

// Service API
// Initialization function for calculation service
uint32_t ble_calc_service_init(ble_calc_service_t* p_calc_service, ble_calc_evt_handler_t* app_evt_handler);

// BLE event handler
void ble_calc_service_on_ble_evt(ble_evt_t const * p_ble_evt, void* p_context);

// Flexion Angle update function
void flexion_angle_characteristic_update(ble_calc_service_t* p_calc_service, float* flexion_angle);

// Calculation error update function
void calc_error_characteristic_update(ble_calc_service_t* p_calc_service, uint8_t* err_num);

#endif