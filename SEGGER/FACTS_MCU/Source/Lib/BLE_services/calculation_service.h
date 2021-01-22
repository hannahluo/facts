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
static ble_calc_service_t _name;                            \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                         \
                     BLE_CALC_SERVICE_BLE_OBSERVER_PRIO,    \
                     ble_calc_service_on_ble_evt, &_name) 

/* 
 *  FACTS Calibration Service: 87C539B0-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Flexion Angle     87C539B1-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Min Limit         87C539B2-8E33-4070-9131-8F56AA023E45
 *      Characteristic 3: Max Limit         87C539B3-8E33-4070-9131-8F56AA023E45
 *      Characteristic 4: Calc Error        87C539B4-8E33-4070-9131-8F56AA023E45
 */

// Base UUID: 87C53994-8E33-4070-9131-8F56AA023E45
// Stored in little endian
#define BLE_UUID_CALC_SERVICE_BASE_UUID  {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x00, 0x00, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_CALC_SERVICE_UUID      (0x39B0)
#define BLE_UUID_FLEXION_ANGLE_UUID     (0x39B1)
#define BLE_UUID_LIMITS_UUID            (0x39B2)
#define BLE_UUID_CALC_ERR_UUID          (0x39B3)

// Calculation service structures
typedef struct ble_calc_service_s ble_calc_service_t;

typedef enum
{
    BLE_FLEXION_ANGLE_EVT_NOTIFY_ENABLED,
    BLE_FLEXION_ANGLE_EVT_NOTIFY_DISABLED,
    BLE_LIMITS_WRITE,
    BLE_CALC_ERR_NOTIFY_ENABLED,
    BLE_CALC_ERR_NOTIFY_DISABLED,
    NUM_BLE_CALC_EVT,
} ble_calc_evt_t;

// Function pointer for per-characteristic event handling
typedef void (*ble_calc_evt_handler_t) (void const * data, uint8_t size);

typedef struct ble_calc_char_s
{
    ble_calc_evt_handler_t evtHandler;
    ble_gatts_char_handles_t charHandles;
    bool notifyEnabled;
    char* name;
} ble_calc_char_t;

// Main handler for calculation service
typedef struct ble_calc_service_s
{
    uint16_t connHandle;
    uint16_t serviceHandle;
    uint8_t uuidType;
    ble_calc_char_t flexionAngle;
    ble_calc_char_t limits;
    ble_calc_char_t calcErr;
} ble_calc_service_t;

// Data structs
typedef double flexion_angle_t;

typedef struct {
    double minLimit;
    double maxLimit;
} limits_t;

typedef enum  {
    CALC_SERVICE_NO_ERROR,
    NUM_CALC_SERVICE_ERRORS,
} calc_err_t;

// Service API
// Initialization function for calculation service
uint32_t ble_calc_service_init(ble_calc_service_t* pCalcService, ble_calc_evt_handler_t flexionAngleHandler,
                                ble_calc_evt_handler_t limitsHandler, ble_calc_evt_handler_t calcErrHandler);

// BLE event handler
void ble_calc_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext);

// Flexion Angle update function
void flexion_angle_characteristic_update(ble_calc_service_t* pCalcService, flexion_angle_t* angle);

// Calculation error update function
void calc_error_characteristic_update(ble_calc_service_t* pCalcService, calc_err_t* error);

// Printing limits to log (debug level)
void print_limits(char* func_name, limits_t* limits);

#endif