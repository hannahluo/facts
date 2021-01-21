/*
 * Header file for service which handles calibration messaging
 */
#ifndef CALIB_SERVICE_H
#define CALIB_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_CALIB_SERVICE_BLE_OBSERVER_PRIO   3

// Register event handler to be invoked on BLE events
#define BLE_CALIB_SERVICE_DEF(_name)                        \
static ble_calib_service_t _name;                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                         \
                     BLE_CALIB_SERVICE_BLE_OBSERVER_PRIO,   \
                     ble_calib_service_on_ble_evt, &_name) 
/* 
 *  FACTS Calibration Service: 87C53994-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Initiate Calibration  87C53995-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Calf Joint Axis       87C53996-8E33-4070-9131-8F56AA023E45
 *      Characteristic 3: Thigh Joint Axis      87C53997-8E33-4070-9131-8F56AA023E45
 */

// Base UUID: 87C53994-8E33-4070-9131-8F56AA023E45
// Stored in little endian
#define BLE_UUID_CALIB_SERVICE_BASE_UUID  {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x00, 0x00, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_CALIB_SERVICE_UUID       (0x3994)
#define BLE_UUID_INIT_CALIB_UUID          (0x3995)
#define BLE_UUID_CALF_JOINT_AXIS_UUID     (0x3996)
#define BLE_UUID_THIGH_JOINT_AXIS_UUID    (0x3997)

#define NUM_CALIB_SERVICE_CHARS                   (3)

// Calibration service structures
typedef struct ble_calib_service_s ble_calib_service_t;

typedef enum
{
    BLE_INIT_CALIB_ON,
    BLE_INIT_CALIB_OFF,
    BLE_INIT_CALIB_EVT_NOTIFY_ENABLED,
    BLE_INIT_CALIB_EVT_NOTIFY_DISABLED,
    BLE_CALF_JOINT_AXIS_WRITE,
    BLE_THIGH_JOINT_AXIS_WRITE,
    NUM_BLE_CALIB_EVT,
} ble_calib_evt_t;

// Function pointer for per-characteristic event handling
typedef void(*ble_calib_evt_handler_t) (ble_calib_service_t* p_calib_service, ble_calib_evt_t evt, void const * data, uint8_t size);

typedef struct ble_calib_char_s
{
    ble_calib_evt_handler_t evtHandler;
    ble_gatts_char_handles_t charHandles;
    bool notifyEnabled;
    char* name;
} ble_calib_char_t;

// Main handler for calibration service
typedef struct ble_calib_service_s
{
    uint16_t connHandle;
    uint16_t serviceHandle;
    uint8_t uuidType;
    ble_calib_char_t initCalib;
    ble_calib_char_t calfJointAxis;
    ble_calib_char_t thighJointAxis;
} ble_calib_service_t;

// Joint axis data
typedef struct {
    double x;
    double y;
    double z;
} joint_axis_t;

// Init calib data
typedef bool init_calib_t;

// Service API
// Initialization function for calibration service
uint32_t ble_calib_service_init(ble_calib_service_t* pCalibService, ble_calib_evt_handler_t initCalHandler, 
                                ble_calib_evt_handler_t calfJointAxisHandler, ble_calib_evt_handler_t thighJointAxisHandler);

// BLE event handler
void ble_calib_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext);

// Init Calib update function
void init_calib_characteristic_update(ble_calib_service_t* pCalibService, init_calib_t* initCalibValue);

#endif