/*
 * Header file for service which handles passing of raw IMU data to phone app
 */
#ifndef IMU_SERVICE_H
#define IMU_SERVICE_H

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

//#define RAW_ACCEL_NEEDED

#define BLE_IMU_SERVICE_BLE_OBSERVER_PRIO   3

// Register IMU event handler to be invoked on BLE events
#define BLE_IMU_SERVICE_DEF(_name)                        \
static ble_imu_service_t _name                            \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                       \
                     BLE_IMU_SERVICE_BLE_OBSERVER_PRIO,   \
                     ble_imu_service_on_ble_evt, &name)

/* 
 *  FACTS IMU Service: 87C539A0-8E33-4070-9131-8F56AA023E45
 *      Characteristic 1: Gyro X  87C539A1-8E33-4070-9131-8F56AA023E45
 *      Characteristic 2: Gyro Y  87C539A2-8E33-4070-9131-8F56AA023E45
 *      Characteristic 3: Gyro Z  87C539A3-8E33-4070-9131-8F56AA023E45
 *      Characteristic 4: Accel X 87C539A4-8E33-4070-9131-8F56AA023E45 (ensure RAW_ACCEL_NEEDED defined)
 *      Characteristic 5: Accel Y 87C539A5-8E33-4070-9131-8F56AA023E45 (ensure RAW_ACCEL_NEEDED defined)
 *      Characteristic 6: Accel Z 87C539A6-8E33-4070-9131-8F56AA023E45 (ensure RAW_ACCEL_NEEDED defined)
 */

// Base UUID: 87C50000-8E33-4070-9131-8F56AA023E45
// Stored little endian
#define BLE_UUID_IMU_SERVICE_BASE_UUID   {0x45, 0x3E, 0x02, 0xAA, 0x56, 0x8F, 0x31, 0x91, 0x70, 0x40, 0x33, 0x8E, 0x94, 0x39, 0xC5, 0x87}

// 12th & 13th octets of service & characteristics
#define BLE_UUID_IMU_SERVICE_UUID                 (0x39A0)
#define BLE_UUID_GYRO_X_UUID                      (0x39A1)
#define BLE_UUID_GYRO_Y_UUID                      (0x39A2)
#define BLE_UUID_GYRO_Z_UUID                      (0x39A3)
#ifdef RAW_ACCEL_NEEDED
  #define BLE_UUID_ACCEL_X_UUID                   (0x39A4)
  #define BLE_UUID_ACCEL_Y_UUID                   (0x39A5)
  #define BLE_UUID_ACCEL_Z_UUID                   (0x39A6)
#endif

#ifdef RAW_ACCEL_NEEDED
  #define NUM_IMU_CHARS                           (6)
#else
  #define NUM_IMU_CHARS                           (3)
#endif

// For access of char_handles member of ble_imu_service_t handler
typedef enum
{
    BLE_GYRO_X_CHAR_IDX,
    BLE_GYRO_Y_CHAR_IDX,
    BLE_GYRO_Z_CHAR_IDX,
#ifdef RAW_ACCEL_NEEDED
    BLE_ACCEL_X_CHAR_IDX,
    BLE_ACCEL_Y_CHAR_IDX,
    BLE_ACCEL_Z_CHAR_IDX,
#endif
} ble_imu_char_handle_idx_t;

// IMU Service structures
typedef struct ble_imu_service_s ble_imu_service_t;

typedef enum
{
    BLE_GYRO_X_EVT_NOTIFY_ENABLED,
    BLE_GYRO_X_EVT_NOTIFY_DISABLED,
    BLE_GYRO_Y_EVT_NOTIFY_ENABLED,
    BLE_GYRO_Y_EVT_NOTIFY_DISABLED,
    BLE_GYRO_Z_EVT_NOTIFY_ENABLED,
    BLE_GYRO_Z_EVT_NOTIFY_DISABLED,
#ifdef RAW_ACCEL_NEEDED
    BLE_ACCEL_X_EVT_NOTIFY_ENABLED,
    BLE_ACCEL_X_EVT_NOTIFY_DISABLED,
    BLE_ACCEL_Y_EVT_NOTIFY_ENABLED,
    BLE_ACCEL_Y_EVT_NOTIFY_DISABLED,
    BLE_ACCEL_Z_EVT_NOTIFY_ENABLED,
    BLE_ACCEL_Z_EVT_NOTIFY_DISABLED,
#endif
    NUM_BLE_IMU_EVT_TYPES,
} ble_imu_evt_type_t;

typedef struct
{
    ble_imu_evt_type_t evt_type;
} ble_imu_evt_t;

// Function pointer for per-characteristic event handling
typedef void (*ble_imu_evt_handler_t) (ble_imu_service_t* p_imu_service, ble_imu_evt_t* p_evt);

// Main IMU service handle
typedef struct ble_imu_service_s
{
    uint16_t conn_handle;
    uint16_t service_handle;
    uint8_t uuid_type;
    ble_imu_evt_handler_t evt_handler[NUM_IMU_CHARS];
    ble_gatts_char_handles_t char_handles[NUM_IMU_CHARS];
} ble_imu_service_t;

// Service API
// Initialization function
uint32_t ble_imu_service_init(ble_imu_service_t* p_imu_service, ble_imu_evt_handler_t* app_evt_handler);

// BLE event handler
void ble_imu_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

// Gyro X update function
void gyro_x_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_x_val);

// Gyro Y update function
void gyro_y_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_y_val);

// Gyro Z update function
void gyro_z_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_z_val);

#ifdef RAW_ACCEL_NEEDED
  // Accel X update function
  void accel_x_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_x_val);

  // Accel Y update function
  void accel_y_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_y_val);

  // Accel Z update function
  void accel_z_characteristic_update(ble_imu_service_t* p_imu_service, float* gyro_z_val);
#endif

#endif