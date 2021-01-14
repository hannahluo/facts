#ifndef DEBUG_SERVICE_H
#define DEBUG_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define BLE_DEBUG_SERVICE_BLE_OBSERVER_PRIO   3

// Register an event handler which is invoked upon BLE event
#define BLE_DEBUG_SERVICE_DEF(_name)                                      \
static ble_debug_service_t _name;                                         \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                       \
                     BLE_DEBUG_SERVICE_BLE_OBSERVER_PRIO,                 \
                     ble_debug_service_on_ble_evt, &_name)

/*
 * FACTS BLE Profile
 * Debug Service:     87C53994-8E33-4070-9131-8F56AA023E45
 *    Characteristic 1: Timer 1 characteristic
 */

// Base UUID: 87C50000-8E33-4070-9131-8F56AA023E45
// store in little endian
#define BLE_UUID_DEBUG_SERVICE_BASE_UUID   {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x94, 0x39, 0xC5, 0x87}

// Service/characteristic UUIDs (12th and 13th octets)
#define BLE_UUID_DEBUG_SERVICE_UUID              0x3994
#define BLE_UUID_TIMER_1_CHAR_UUID               0x3995

// Debug Service structures
typedef struct ble_debug_service_s  ble_debug_service_t;

typedef enum
{
    BLE_TIMER_1_ELAPSED_EVT_NOTIFY_ENABLED,
    BLE_TIMER_1_ELAPSED_EVT_NOTIFY_DISABLED,
    NUM_BLE_DEBUG_EVT_TYPES,
}   ble_debug_evt_type_t;

typedef struct
{
    ble_debug_evt_type_t evt_type;
} ble_debug_evt_t;

// BLE event handler function pointer
typedef void (*ble_debug_evt_handler_t) (ble_debug_service_t* p_debug_service, ble_debug_evt_t* p_evt);

// Main handler for debug service
typedef struct ble_debug_service_s
{
    uint16_t conn_handle;                                   // handle to the connection obj
    uint16_t service_handle;                                // handle to the service
    uint8_t uuid_type;                                      // defines if BT SIG-defined, or custom UUUID
    ble_debug_evt_handler_t evt_handler;                    // BLE event handler associated with the service
    ble_gatts_char_handles_t timer_1_elapsed_char_handles;  // has handles to the char val, user desc, cccd, and sccd attributes
} ble_debug_service_t;

// Service API
// Initialization function for debug service
uint32_t ble_debug_service_init(ble_debug_service_t* p_debug_service, ble_debug_evt_handler_t app_evt_handler);

// BLE event handler for debug service to pass to SoftDevice
void ble_debug_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

// Timer 1 characteristic update function
void timer_1_characteristic_update(ble_debug_service_t * p_debug_service, uint8_t * timer_action);

#endif