/*
 * Header file for service which handles calibration messaging
 */
#ifndef IMU_SERVICE_H
#define IMU_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_CALIB_SERVICE_BLE_OBSERVER_PRIO   3

// Register event handler to be invoked on BLE events
#define BLE_CALIB_SERVICE_DEF(_name)                        \
static ble_calib_service_t _name                            \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                       \
                     BLE_CALIB_SERVICE_BLE_OBSERVER_PRIO,   \
                     ble_calib_service_on_ble_evt, &name) 

/* 
 *  FACTS Calibration Service: 87C53994-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Initiate Calibration  87C53995-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Calf Joint Axis X     87C53996-8E33-4070-9131-8F56AA023E45
 *      Characteristic 3: Calf Joint Axis Y     87C53997-8E33-4070-9131-8F56AA023E45
 *      Characteristic 4: Calf Joint Axis Z     87C53998-8E33-4070-9131-8F56AA023E45
 *      Characteristic 5: Thigh Joint Axis X    87C53999-8E33-4070-9131-8F56AA023E45
 *      Characteristic 6: Thigh Joint Axis Y    87C5399A-8E33-4070-9131-8F56AA023E45
 *      Characteristic 7: Thigh Joint Axis Z    87C5399B-8E33-4070-9131-8F56AA023E45
 */

// Base UUID: 87C53994-8E33-4070-9131-8F56AA023E45
 // Stored in little endian
#define BLE_UUID_CALIB_SERVICE_BASE_UUID  {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x00, 0x00, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_CALIB_SERVICE_UUID       (0x3994)
#define BLE_UUID_INIT_CALIB_UUID          (0x3995)
#define BLE_UUID_CALF_X_UUID              (0x3996)
#define BLE_UUID_CALF_Y_UUID              (0x3997)
#define BLE_UUID_CALF_Z_UUID              (0x3998)
#define BLE_UUID_THIGH_X_UUID             (0x3999)
#define BLE_UUID_THIGH_Y_UUID             (0x399A)
#define BLE_UUID_THIGH_Z_UUID             (0x399B)

#define NUM_CALIB_CHARS                   (7)

// To access char_handles member of ble_imu_service_t handler
typedef enum
{
    BLE_INIT_CALIB_HANDLES_IDX,
    BLE_CALF_X_HANDLES_IDX,
    BLE_CALF_Y_HANDLES_IDX,
    BLE_CALF_Z_HANDLES_IDX,
    BLE_THIGH_X_HANDLES_IDX,
    BLE_THIGH_Y_HANDLES_IDX,
    BLE_THIGH_Z_HANDLES_IDX,
} ble_calib_char_handle_idx_t;

// Calibration service structures
typedef struct ble_calib_service_s ble_calib_service_t;

typedef enum
{
    BLE_INIT_CALIB_WRITE,
    BLE_INIT_CALIB_EVT_NOTIFY_ENABLED,
    BLE_INIT_CALIB_EVT_NOTIFY_DISABLED,
    BLE_CALF_X_WRITE,
    BLE_CALF_Y_WRITE,
    BLE_CALF_Z_WRITE,
    BLE_THIGH_X_WRITE,
    BLE_THIGH_Y_WRITE,
    BLE_THIGH_Z_WRITE,
    NUM_BLE_CALIB_EVT,
} ble_calib_evt_type_t;

typedef struct 
{
    ble_calib_evt_type_t evt_type;
} ble_calib_evt_t;

// Function pointer for per-characteristic event handling
typedef void(*ble_calib_evt_handler_t) (ble_calib_service_t* p_calib_service, ble_calib_evt_t* p_evt);

// Main handler for calibration service
typedef struct ble_calib_service_s
{
    uint16_t conn_handle;
    uint16_t service_handle;
    uint8_t uuid_type;
    ble_calib_evt_handler_t evt_handler[NUM_CALIB_CHARS];
    ble_gatts_char_handles_t char_handles[NUM_CALIB_CHARS];
} ble_calib_service_t;

// Service API
// Initialization function for calibration service
uint32_t ble_calib_service_init(ble_calib_service_t* p_calib_service, ble_calib_evt_handler_t* app_evt_handler);

// BLE event handler
void ble_calib_service_on_ble_evt(ble_evt_t const * p_ble_evt, void* p_context);

// Init Calib update function
void init_calib_characteristic_update(ble_calib_service_t* p_calib_service, uint8_t* init_calib_val);

