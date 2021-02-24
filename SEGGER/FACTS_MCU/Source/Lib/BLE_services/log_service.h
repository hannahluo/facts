/*
 * Header file for service which handles calibration messaging
 */
#ifndef LOG_SERVICE_H
#define LOG_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_LOG_SERVICE_BLE_OBSERVER_PRIO   3

// Register event handler to be invoked on BLE events
#define BLE_LOG_SERVICE_DEF(_name)                          \
static ble_log_service_t _name;                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                         \
                     BLE_LOG_SERVICE_BLE_OBSERVER_PRIO,    \
                     ble_log_service_on_ble_evt, &_name) 

/* 
 *  FACTS Log Service:                    87C539C0-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Log Message     87C539C1-8E33-4070-9131-8F56AA023E45
 */


// Base UUID: 87C50000-8E33-4070-9131-8F56AA023E45
// Stored in little endian
#define BLE_UUID_LOG_SERVICE_BASE_UUID  {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x00, 0x00, 0xC5, 0x87}
#endif

// Use 12th and 13th octets to differentiate services/characteristics
#define BLE_UUID_LOG_SERVICE_UUID     (0x39C0)
#define BLE_UUID_LOG_MSG_UUID         (0x39C1)

// Log service structures
typedef struct ble_log_service_s ble_log_service_t;

typedef enum 
{
    BLE_MSG_EVT_NOTIFY_ENABLED,
    BLE_MSG_EVT_NOTIFY_DISABLED,
} ble_log_evt_t;

// Function pointer for per-characteristic event handling
typedef void (*ble_log_evt_handler_t) (void const* data, uint8_t size);

typedef struct ble_calc_char_s
{
    ble_log_evt_handler_t evtHandler;
    ble_gatts_char_handles_t charHandles;
    bool notifyEnabled;
    char* name;
} ble_log_char_t;

// Main handler for log service
typedef struct ble_log_service_s
{
    uint16_t connHandle;
    uint16_t serviceHandle;
    uint8_t uuidType;
    ble_log_char_t logMsg;
} ble_log_service_t;

typedef enum {
    char* msg;
    uint16_t size;
} log_msg_t;

// Service API
// Initialization function for log service
uint32_t ble_log_service_init(ble_log_service_t* pLogService, ble_log_evt_handler_t logMsgHandler);

// BLE event handler
void ble_log_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext);

// Log Message update function
void log_msg_characteristic_update(ble_log_service_t* pLogService, log_msg_t* msg); 

