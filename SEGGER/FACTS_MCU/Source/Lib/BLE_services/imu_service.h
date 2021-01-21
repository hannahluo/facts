#ifndef IMU_SERVICE_H
#define IMU_SERVICE_H

/*
 * Header file for service which handles passing of raw IMU data to phone app
 */

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#define RAW_ACCEL_NEEDED

#define BLE_IMU_SERVICE_BLE_OBSERVER_PRIO   3

// Register IMU event handler to be invoked on BLE events
#define BLE_IMU_SERVICE_DEF(_name)                        \
static ble_imu_service_t _name;                           \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                       \
                     BLE_IMU_SERVICE_BLE_OBSERVER_PRIO,   \
                     ble_imu_service_on_ble_evt, &_name)

/* 
 *  FACTS IMU Service: 87C539A0-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Raw Gyro    87C539A1-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Raw Accel   87C539A2-8E33-4070-9131-8F56AA023E45 (ensure RAW_ACCEL_NEEDED defined)
 */

// Base UUID: 87C50000-8E33-4070-9131-8F56AA023E45
// Stored little endian
#define BLE_UUID_IMU_SERVICE_BASE_UUID   {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0xA0, 0x39, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_IMU_SERVICE_UUID                 (0x39A0)
#define BLE_UUID_RAW_GYRO_UUID                    (0x39A1)
#ifdef RAW_ACCEL_NEEDED
 #define BLE_UUID_RAW_ACCEL_UUID                  (0x39A2)
#endif

// IMU Service structures
typedef struct ble_imu_service_s ble_imu_service_t;

typedef enum
{
    BLE_RAW_GYRO_EVT_NOTIFY_ENABLED,
    BLE_RAW_GYRO_EVT_NOTIFY_DISABLED,
#ifdef RAW_ACCEL_NEEDED
    BLE_RAW_ACCEL_EVT_NOTIFY_ENABLED,
    BLE_RAW_ACCEL_EVT_NOTIFY_DISABLED,
#endif
    NUM_BLE_IMU_EVT_TYPES,
} ble_imu_evt_t;

// Function pointer for per-characteristic event handling
typedef void (*ble_imu_evt_handler_t) (ble_imu_service_t* pImuService, ble_imu_evt_t evt, void const * data, uint8_t size);

typedef struct ble_imu_char_s
{
    ble_imu_evt_handler_t evtHandler;
    ble_gatts_char_handles_t charHandles;
    bool notifyEnabled;
    char* name;
} ble_imu_char_t;

// Main IMU service handle
typedef struct ble_imu_service_s
{
    uint16_t connHandle;
    uint16_t serviceHandle;
    uint8_t uuidType;
    ble_imu_char_t rawGyro;
    ble_imu_char_t rawAccel;
} ble_imu_service_t;

// Gyro data
typedef struct {
    double gyroX;
    double gyroY;
    double gyroZ;
} raw_gyro_t;

// Accel data 
typedef struct {
    double accelX;
    double accelY;
    double accelZ;
} raw_accel_t;

// Service API
// Initialization function
uint32_t ble_imu_service_init(ble_imu_service_t* pImuService, ble_imu_evt_handler_t gyroEvtHandler, ble_imu_evt_handler_t accelEvtHandler);

// BLE event handler
void ble_imu_service_on_ble_evt(ble_evt_t const * pBleEvt, void * pContext);

// Gyro X update function
void raw_gyro_characteristic_update(ble_imu_service_t* pImuService, raw_gyro_t* gyroVal);

#ifdef RAW_ACCEL_NEEDED
  // Accel X update function
  void raw_accel_characteristic_update(ble_imu_service_t* pImuService, raw_accel_t* accelVal);
#endif

#endif