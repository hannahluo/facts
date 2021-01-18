#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "imu_service.h"

#define CCCD_WRITE_LEN    (2)

static char g_RawGyroCharName[] = "Raw Gyro";
static char g_RawAccelCharName[] = "Raw Accel";

// Handle GAP connection event
static void on_connect(ble_imu_service_t* pImuService, ble_evt_t const * pBleEvt)
{
    pImuService->connHandle = pBleEvt->evt.gap_evt.conn_handle;
    pImuService->rawAccel.notifyEnabled = false;
    pImuService->rawGyro.notifyEnabled = false;
}

// Handle GAP disconnect event
static void on_disconnect(ble_imu_service_t* pImuService, ble_evt_t const * pBleEvt)
{
    UNUSED_PARAMETER(pBleEvt);
    pImuService->connHandle = BLE_CONN_HANDLE_INVALID;
    pImuService->rawAccel.notifyEnabled = false;
    pImuService->rawGyro.notifyEnabled = false;
}

// Handle a GATTS write event for char
static void on_write(ble_imu_service_t* pImuService, ble_imu_char_t* pChar, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;

    // Handle write to CCCD
    if((pEvtWrite->handle == pChar->charHandles.cccd_handle) && (pEvtWrite->len == CCCD_WRITE_LEN)) {
        ble_imu_evt_t evt;
        // Read CCCD value
        if(ble_srv_is_notification_enabled(pEvtWrite->data)) {
            NRF_LOG_DEBUG("Notifications ENABLED for %s", pChar->name);
            evt = BLE_RAW_GYRO_EVT_NOTIFY_ENABLED;
            pChar->notifyEnabled = true;
        } else {
            NRF_LOG_DEBUG("Notifications DISABLED for %s", pChar->name);
            evt = BLE_RAW_ACCEL_EVT_NOTIFY_ENABLED;
            pChar->notifyEnabled = false;
        }
        // Pass to characteristic event handler if present
        if(pChar->evtHandler != NULL) {
            pChar->evtHandler(pImuService, evt);
        }
    }

}

// BLE event handler
void ble_imu_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext)
{
    ble_imu_service_t* pImuService = (ble_imu_service_t *) pContext;
    uint16_t handle = 0;
    // check event header for event type
    switch(pBleEvt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(pImuService, pBleEvt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(pImuService, pBleEvt);
            break;
        case BLE_GATTS_EVT_WRITE:
            handle = pBleEvt->evt.gatts_evt.params.write.handle;
            // Determine which characteristic was written to
            if(handle == pImuService->rawGyro.charHandles.cccd_handle) {
                on_write(pImuService, &pImuService->rawGyro, pBleEvt);
            } else if (handle == pImuService->rawAccel.charHandles.cccd_handle) {
                on_write(pImuService, &pImuService->rawAccel, pBleEvt);
            } else {
                NRF_LOG_DEBUG("Write to unknown handle 0x%x", handle);
                return;
            }
            break;
        default:
            //NRF_LOG_DEBUG("Received 0x%x event", pBleEvt->header.evt_id);
            break;            
    }
}

// Function to add gyro characteristic to BLE stack
static uint32_t raw_gyro_char_add(ble_imu_service_t* pImuService)
{
    ble_gatts_char_md_t charMd;
    ble_gatts_attr_md_t cccdMd;
    ble_gatts_attr_t attrCharValue;
    ble_uuid_t bleUuid;
    ble_gatts_attr_md_t attrMd;

    memset(&charMd, 0, sizeof(charMd));
    memset(&cccdMd, 0, sizeof(cccdMd));
    memset(&attrMd, 0, sizeof(attrMd));
    memset(&attrCharValue, 0, sizeof(attrCharValue));

    // Settings for CCCD
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.read_perm);        // App can read CCCD
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.write_perm);       // App can write CCCD
    cccdMd.vloc = BLE_GATTS_VLOC_STACK;                       // set CCCD attribute to live in stack

    // Set meta for characteristic value
    charMd.char_props.read = 1;
    charMd.char_props.notify = 1;
    charMd.p_char_user_desc = g_RawGyroCharName;
    charMd.char_user_desc_size = sizeof(g_RawGyroCharName);
    charMd.char_user_desc_max_size = sizeof(g_RawGyroCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pImuService->uuidType;
    bleUuid.uuid = BLE_UUID_RAW_GYRO_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App cannot write char val

    // Setting characteristic value settings
    raw_gyro_t initGyro = {.gyroX=0.0,.gyroY=0.0,.gyroZ=0.0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(initGyro);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(initGyro);
    attrCharValue.p_value = (uint8_t*)&initGyro;

    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pImuService->serviceHandle, &charMd, &attrCharValue, &pImuService->rawGyro.charHandles);
}

// Function to add accel characterisitic to BLE stack
static uint32_t raw_accel_char_add(ble_imu_service_t* pImuService)
{
    ble_gatts_char_md_t charMd;
    ble_gatts_attr_md_t cccdMd;
    ble_gatts_attr_t attrCharValue;
    ble_uuid_t bleUuid;
    ble_gatts_attr_md_t attrMd;

    memset(&charMd, 0, sizeof(charMd));
    memset(&cccdMd, 0, sizeof(cccdMd));
    memset(&attrMd, 0, sizeof(attrMd));
    memset(&attrCharValue, 0, sizeof(attrCharValue));

    // Set up CCCD meta
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.read_perm);        // App can read CCCD
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.write_perm);       // App can write CCCD
    cccdMd.vloc = BLE_GATTS_VLOC_STACK;                       // set CCCD attribute to live in stack

    // Set meta for characteristic value
    charMd.char_props.read = 1;
    charMd.char_props.notify = 1;
    charMd.p_char_user_desc = g_RawAccelCharName;
    charMd.char_user_desc_size = sizeof(g_RawAccelCharName);
    charMd.char_user_desc_max_size = sizeof(g_RawAccelCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pImuService->uuidType;
    bleUuid.uuid = BLE_UUID_RAW_GYRO_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App cannot write char val

    // Setting characteristic value settings
    raw_accel_t initAccel = {.accelX=0.0,.accelY=0.0,.accelZ=0.0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(initAccel);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(initAccel);
    attrCharValue.p_value = (uint8_t*)&initAccel;

    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pImuService->serviceHandle, &charMd, &attrCharValue, &pImuService->rawAccel.charHandles);
}

// Function for initializing service
uint32_t ble_imu_service_init(ble_imu_service_t* pImuService, ble_imu_evt_handler_t gyroEvtHandler, ble_imu_evt_handler_t accelEvtHandler)
{
    uint32_t errCode;
    ble_uuid_t bleUuid;
    
    pImuService->connHandle = BLE_CONN_HANDLE_INVALID;
    pImuService->rawGyro.evtHandler = gyroEvtHandler;
    pImuService->rawAccel.evtHandler = accelEvtHandler;
    pImuService->rawGyro.name = g_RawGyroCharName;
    pImuService->rawAccel.name = g_RawAccelCharName;

    ble_uuid128_t baseUuid = {BLE_UUID_IMU_SERVICE_BASE_UUID};
    errCode = sd_ble_uuid_vs_add(&baseUuid, &pImuService->uuidType);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add vendor-specific UUID for IMU service with err=0x%x", errCode);
        return errCode;
    }

    bleUuid.type = pImuService->uuidType;
    bleUuid.uuid = BLE_UUID_IMU_SERVICE_UUID;

    // Register service with SoftDevice
    errCode = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &bleUuid, &pImuService->serviceHandle);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add IMU service to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    // Add gyro characteristic to service
    errCode = raw_gyro_char_add(pImuService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add raw gyro characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    // Add accel characteristic to service
    errCode = raw_accel_char_add(pImuService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add raw accel characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    return NRF_SUCCESS;
}

// Function to update gyro characteristic value
void raw_gyro_characteristic_update(ble_imu_service_t* pImuService, raw_gyro_t* gyroVal)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if(pImuService->connHandle != BLE_CONN_HANDLE_INVALID) {
        // Update val
        gattsVal.len = sizeof(*gyroVal);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)gyroVal;

        errCode = sd_ble_gatts_value_set(pImuService->connHandle, pImuService->rawGyro.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pImuService->rawGyro.notifyEnabled) {
           NRF_LOG_DEBUG("Sending raw gyro updated (%f,%f,%f)", gyroVal->gyroX, gyroVal->gyroY, gyroVal->gyroZ);
           uint16_t len = sizeof(*gyroVal);
           ble_gatts_hvx_params_t hvxParams;
           memset(&hvxParams, 0, sizeof(hvxParams));

           hvxParams.handle = pImuService->rawGyro.charHandles.value_handle;
           hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
           hvxParams.offset = 0;
           hvxParams.p_len = &len;
           hvxParams.p_data = (uint8_t*)gyroVal;

           errCode = sd_ble_gatts_hvx(pImuService->connHandle, &hvxParams);
           APP_ERROR_CHECK(errCode);
        }
    }
}

// Function to update gyro characteristic value
void raw_accel_characteristic_update(ble_imu_service_t* pImuService, raw_accel_t* accelVal)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if(pImuService->connHandle != BLE_CONN_HANDLE_INVALID) {
        // Update val
        gattsVal.len = sizeof(*accelVal);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)accelVal;

        errCode = sd_ble_gatts_value_set(pImuService->connHandle, pImuService->rawAccel.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pImuService->rawAccel.notifyEnabled) {
           NRF_LOG_DEBUG("Sending raw accel updated (%f,%f,%f)", accelVal->accelX, accelVal->accelY, accelVal->accelZ);
           uint16_t len = sizeof(*accelVal);
           ble_gatts_hvx_params_t hvxParams;
           memset(&hvxParams, 0, sizeof(hvxParams));

           hvxParams.handle = pImuService->rawAccel.charHandles.value_handle;
           hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
           hvxParams.offset = 0;
           hvxParams.p_len = &len;
           hvxParams.p_data = (uint8_t*)accelVal;

           errCode = sd_ble_gatts_hvx(pImuService->connHandle, &hvxParams);
           APP_ERROR_CHECK(errCode);
        }
    }
}