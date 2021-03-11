#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "imu_service.h"

#define CCCD_WRITE_LEN    (2)

static char g_RawGyroCalfCharName[] = "Raw Gyro Calf";
static char g_RawGyroThighCharName[] = "Raw Gyro Thigh";

#define PRECISION_MULTIPLIER      (4)

// Helper function to format float/double as string
static void float_to_string(char* buff, double val)
{
    int integer = (int)(val);
    int decimal = (int)((val - integer)*PRECISION_MULTIPLIER);
    if(val < 0) {
        integer *= -1;
        decimal *= -1;
        sprintf(buff, "-%d.%d", integer,decimal);
    }
    else
        sprintf(buff, "%d.%d", integer,decimal);
}

// Helper function to print raw gyro 
void print_raw_gyro_vals(char* func_name, raw_gyro_t* gyro)
{
    char x[20];
    char y[20];
    char z[20];

    float_to_string(x, gyro->x);
    float_to_string(y, gyro->y);
    float_to_string(z, gyro->z);

    NRF_LOG_DEBUG("%s: Gyro(%s,%s,%s)", func_name, x, y, z);
}

// Handle GAP connection event
static void on_connect(ble_imu_service_t* pImuService, ble_evt_t const * pBleEvt)
{
    pImuService->connHandle = pBleEvt->evt.gap_evt.conn_handle;
    pImuService->rawGyroThigh.notifyEnabled = false;
    pImuService->rawGyroCalf.notifyEnabled = false;
}

// Handle GAP disconnect event
static void on_disconnect(ble_imu_service_t* pImuService, ble_evt_t const * pBleEvt)
{
    UNUSED_PARAMETER(pBleEvt);
    pImuService->connHandle = BLE_CONN_HANDLE_INVALID;
    pImuService->rawGyroThigh.notifyEnabled = false;
    pImuService->rawGyroCalf.notifyEnabled = false;
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
            if(pChar == &pImuService->rawGyroCalf) {
                evt = BLE_RAW_GYRO_CALF_EVT_NOTIFY_ENABLED;
            } else {
                evt = BLE_RAW_GYRO_THIGH_EVT_NOTIFY_ENABLED;
            }
            pChar->notifyEnabled = true;
        } else {
            NRF_LOG_DEBUG("Notifications DISABLED for %s", pChar->name);
            if(pChar == &pImuService->rawGyroCalf) {
                evt = BLE_RAW_GYRO_CALF_EVT_NOTIFY_DISABLED;
            } else {
                evt = BLE_RAW_GYRO_THIGH_EVT_NOTIFY_DISABLED;
            }
            pChar->notifyEnabled = false;
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
            if(handle == pImuService->rawGyroCalf.charHandles.cccd_handle) {
                on_write(pImuService, &pImuService->rawGyroCalf, pBleEvt);
            } else if (handle == pImuService->rawGyroThigh.charHandles.cccd_handle) {
                on_write(pImuService, &pImuService->rawGyroThigh, pBleEvt);
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
static uint32_t raw_gyro_calf_char_add(ble_imu_service_t* pImuService)
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
    charMd.p_char_user_desc = g_RawGyroCalfCharName;
    charMd.char_user_desc_size = sizeof(g_RawGyroCalfCharName);
    charMd.char_user_desc_max_size = sizeof(g_RawGyroCalfCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pImuService->uuidType;
    bleUuid.uuid = BLE_UUID_RAW_GYRO_CALF_UUID;
    NRF_LOG_INFO("raw_gyro_calf_char_add: Adding 0x%x", bleUuid.uuid);

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App cannot write char val

    // Setting characteristic value settings
    raw_gyro_t initGyro = {.x=0.0,.y=0.0,.z=0.0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(initGyro);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(initGyro);
    attrCharValue.p_value = (uint8_t*)&initGyro;

    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pImuService->serviceHandle, &charMd, &attrCharValue, &pImuService->rawGyroCalf.charHandles);
}

// Function to add accel characterisitic to BLE stack
static uint32_t raw_gyro_thigh_char_add(ble_imu_service_t* pImuService)
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
    charMd.p_char_user_desc = g_RawGyroThighCharName;
    charMd.char_user_desc_size = sizeof(g_RawGyroThighCharName);
    charMd.char_user_desc_max_size = sizeof(g_RawGyroThighCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pImuService->uuidType;
    bleUuid.uuid = BLE_UUID_RAW_GYRO_THIGH_UUID;
    NRF_LOG_INFO("raw_accel_thigh_char_add: Added 0x%x", bleUuid.uuid);

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App cannot write char val

    // Setting characteristic value settings
    raw_gyro_t initGyro = {.x=0.0,.y=0.0,.z=0.0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(initGyro);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(initGyro);
    attrCharValue.p_value = (uint8_t*)&initGyro;

    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pImuService->serviceHandle, &charMd, &attrCharValue, &pImuService->rawGyroThigh.charHandles);
}

// Function for initializing service
uint32_t ble_imu_service_init(ble_imu_service_t* pImuService, ble_imu_evt_handler_t gyroCalfEvtHandler, ble_imu_evt_handler_t gyroThighEvtHandler)
{
    uint32_t errCode;
    ble_uuid_t bleUuid;
    
    pImuService->connHandle = BLE_CONN_HANDLE_INVALID;
    pImuService->rawGyroCalf.evtHandler = gyroCalfEvtHandler;
    pImuService->rawGyroThigh.evtHandler = gyroThighEvtHandler;
    pImuService->rawGyroCalf.name = g_RawGyroCalfCharName;
    pImuService->rawGyroThigh.name = g_RawGyroCalfCharName;

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
    errCode = raw_gyro_calf_char_add(pImuService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add raw gyro calf characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    // Add accel characteristic to service
    errCode = raw_gyro_thigh_char_add(pImuService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add raw gyro thigh characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    return NRF_SUCCESS;
}

// Function to update gyro characteristic value
void raw_gyro_calf_characteristic_update(ble_imu_service_t* pImuService, raw_gyro_t* gyroVal)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if(pImuService->connHandle != BLE_CONN_HANDLE_INVALID) {
        // Update val
        gattsVal.len = sizeof(*gyroVal);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)gyroVal;

        errCode = sd_ble_gatts_value_set(pImuService->connHandle, pImuService->rawGyroCalf.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pImuService->rawGyroCalf.notifyEnabled) {
            print_raw_gyro_vals("raw_gyro_calf_characteristic_update", gyroVal);
            uint16_t len = sizeof(*gyroVal);
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pImuService->rawGyroCalf.charHandles.value_handle;
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
void raw_gyro_thigh_characteristic_update(ble_imu_service_t* pImuService, raw_gyro_t* gyroVal)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if(pImuService->connHandle != BLE_CONN_HANDLE_INVALID) {
        // Update val
        gattsVal.len = sizeof(*gyroVal);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)gyroVal;

        errCode = sd_ble_gatts_value_set(pImuService->connHandle, pImuService->rawGyroThigh.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pImuService->rawGyroThigh.notifyEnabled) {
            print_raw_gyro_vals("raw_gyro_thigh_characteristic_update", gyroVal);
            
            uint16_t len = sizeof(*gyroVal);
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pImuService->rawGyroThigh.charHandles.value_handle;
            hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = 0;
            hvxParams.p_len = &len;
            hvxParams.p_data = (uint8_t*)gyroVal;

            errCode = sd_ble_gatts_hvx(pImuService->connHandle, &hvxParams);
            APP_ERROR_CHECK(errCode);
        }
    }
}