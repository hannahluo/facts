#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "calculation_service.h"

#define CCCD_WRITE_LEN        (2)

static char g_FlexionAngleCharName[] = "Flexion Angle";
static char g_LimitsCharName[] = "Limits";
static char g_CalcErrCharName[] = "Calculation Error";

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

void print_limits(char* func_name, limits_t* limits)
{
    char minLimit[20];
    char maxLimit[20];
    float_to_string(minLimit, limits->minLimit);
    float_to_string(maxLimit, limits->maxLimit);

    NRF_LOG_DEBUG("%s: min_limit=%s max_limit=%s", func_name, minLimit, maxLimit);
}

static void on_connect(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    pCalcService->connHandle = pBleEvt->evt.gap_evt.conn_handle;
    pCalcService->flexionAngle.notifyEnabled = false;
    pCalcService->limits.notifyEnabled = false;
    pCalcService->calcErr.notifyEnabled = false;
}

static void on_disconnect(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    UNUSED_PARAMETER(pBleEvt);
    pCalcService->connHandle = BLE_CONN_HANDLE_INVALID;
    pCalcService->flexionAngle.notifyEnabled = false;
    pCalcService->limits.notifyEnabled = false;
    pCalcService->calcErr.notifyEnabled = false;
}

static void on_flexion_angle_write(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_calc_char_t* pChar = &pCalcService->flexionAngle;
    ble_calc_evt_t evt = NUM_BLE_CALC_EVT;

    if((pEvtWrite->handle == pChar->charHandles.cccd_handle) && (pEvtWrite->len == CCCD_WRITE_LEN)) {
        // Read CCCD value
        if(ble_srv_is_notification_enabled(pEvtWrite->data)) {
            NRF_LOG_DEBUG("Notifications enabled for %s", pChar->name);
            evt = BLE_FLEXION_ANGLE_EVT_NOTIFY_ENABLED;
            pChar->notifyEnabled = true;
        } else {
            NRF_LOG_DEBUG("Notifications disabled for %s", pChar->name);
            evt = BLE_FLEXION_ANGLE_EVT_NOTIFY_DISABLED;
            pChar->notifyEnabled = false;
        }
    }
}

static void on_calc_err_write(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_calc_char_t* pChar = &pCalcService->calcErr;
    ble_calc_evt_t evt = NUM_BLE_CALC_EVT;

    if((pEvtWrite->handle == pChar->charHandles.cccd_handle) && (pEvtWrite->len == CCCD_WRITE_LEN)) {
        // Read CCCD value
        if(ble_srv_is_notification_enabled(pEvtWrite->data)) {
            NRF_LOG_DEBUG("Notifications enabled for %s", pChar->name);
            evt = BLE_CALC_ERR_NOTIFY_ENABLED;
            pChar->notifyEnabled = true;
        } else {
            NRF_LOG_DEBUG("Notifications disabled for %s", pChar->name);
            evt = BLE_CALC_ERR_NOTIFY_DISABLED;
            pChar->notifyEnabled = false;
        }
    }
}

static void on_limits_write(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_calc_char_t* pChar = &pCalcService->limits;
    ble_calc_evt_t evt = NUM_BLE_CALC_EVT;

    if((pEvtWrite->handle == pChar->charHandles.value_handle) && (pEvtWrite->len == sizeof(limits_t))) {
        evt = BLE_LIMITS_WRITE;

        if(pChar->evtHandler != NULL) {
            pChar->evtHandler(pEvtWrite->data, pEvtWrite->len);
        }
    }
}

void on_write(ble_calc_service_t* pCalcService, ble_evt_t const * pBleEvt)
{
    uint16_t handle = pBleEvt->evt.gatts_evt.params.write.handle;
    if(handle == pCalcService->flexionAngle.charHandles.cccd_handle) {
        on_flexion_angle_write(pCalcService, pBleEvt);
    } else if (handle == pCalcService->calcErr.charHandles.cccd_handle) {
        on_calc_err_write(pCalcService, pBleEvt);
    } else if (handle == pCalcService->limits.charHandles.value_handle) {
        on_limits_write(pCalcService, pBleEvt);
    } else {
        NRF_LOG_DEBUG("Write to unknown handle 0x%x", handle);
    }
}

void ble_calc_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext)
{
    ble_calc_service_t* pCalcService = (ble_calc_service_t *) pContext;
    uint16_t handle = 0;
    
    switch(pBleEvt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(pCalcService, pBleEvt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(pCalcService, pBleEvt);
            break;
        case BLE_GATTS_EVT_WRITE:
            handle = pBleEvt->evt.gatts_evt.params.write.handle;
            on_write(pCalcService, pBleEvt);
            break;
        default:
            //NRF_LOG_DEBUG("Received 0x%x event", pBleEvt->header.evt_id);
            break;
    }
}

static uint32_t flexion_angle_char_add(ble_calc_service_t* pCalcService)
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
    charMd.p_char_user_desc = g_FlexionAngleCharName;
    charMd.char_user_desc_size = sizeof(g_FlexionAngleCharName);
    charMd.char_user_desc_max_size = sizeof(g_FlexionAngleCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalcService->uuidType;
    bleUuid.uuid = BLE_UUID_FLEXION_ANGLE_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App can write char val

    // Set meta for characteristic value
    flexion_angle_t angle = 0.0f;
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(angle);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(angle);
    attrCharValue.p_value = (uint8_t*)&angle;

    // Add characteristic to Soft Device
    return sd_ble_gatts_characteristic_add(pCalcService->serviceHandle, &charMd, &attrCharValue, &pCalcService->flexionAngle.charHandles);
}

static uint32_t calc_err_char_add(ble_calc_service_t* pCalcService)
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
    charMd.p_char_user_desc = g_CalcErrCharName;
    charMd.char_user_desc_size = sizeof(g_CalcErrCharName);
    charMd.char_user_desc_max_size = sizeof(g_CalcErrCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalcService->uuidType;
    bleUuid.uuid = BLE_UUID_CALC_ERR_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App cannot write char val

    // Set meta for characteristic value
    calc_err_t err = CALC_SERVICE_NO_ERROR;
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(err);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(err);
    attrCharValue.p_value = (uint8_t*)&err;

    // Add characteristic to Soft Device
    return sd_ble_gatts_characteristic_add(pCalcService->serviceHandle, &charMd, &attrCharValue, &pCalcService->calcErr.charHandles);
}

static uint32_t limits_char_add(ble_calc_service_t* pCalcService)
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

    // Set meta for characteristic value
    charMd.char_props.read = 1;
    charMd.char_props.write = 1;
    charMd.p_char_user_desc = g_LimitsCharName;
    charMd.char_user_desc_size = sizeof(g_LimitsCharName);
    charMd.char_user_desc_max_size = sizeof(g_LimitsCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = NULL; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalcService->uuidType;
    bleUuid.uuid = BLE_UUID_LIMITS_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.write_perm);       // App can write char val

    // Set meta for characteristic value
    limits_t limits = {.minLimit = 0, .maxLimit = 0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(limits);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(limits);
    attrCharValue.p_value = (uint8_t*)&limits;

    // Add characteristic to Soft Device
    return sd_ble_gatts_characteristic_add(pCalcService->serviceHandle, &charMd, &attrCharValue, &pCalcService->limits.charHandles);
}

uint32_t ble_calc_service_init(ble_calc_service_t* pCalcService, ble_calc_evt_handler_t flexionAngleHandler,
                                ble_calc_evt_handler_t limitsHandler, ble_calc_evt_handler_t calcErrHandler)
{
    uint32_t errCode;
    ble_uuid_t bleUuid;

    pCalcService->connHandle = BLE_CONN_HANDLE_INVALID;
    pCalcService->flexionAngle.evtHandler = flexionAngleHandler;
    pCalcService->limits.evtHandler = limitsHandler;
    pCalcService->calcErr.evtHandler = calcErrHandler;
    pCalcService->flexionAngle.name = g_FlexionAngleCharName;
    pCalcService->limits.name = g_LimitsCharName;
    pCalcService->calcErr.name = g_CalcErrCharName;

    ble_uuid128_t baseUuid = {BLE_UUID_CALC_SERVICE_BASE_UUID};
    errCode = sd_ble_uuid_vs_add(&baseUuid, &pCalcService->uuidType);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add vendor-scpecific UUID for Calculation servic with err=0x%x", errCode);
        return errCode;
    }

    bleUuid.type = pCalcService->uuidType;
    bleUuid.uuid = BLE_UUID_CALC_SERVICE_UUID;

    // Register with SoftDevice
    errCode = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &bleUuid, &pCalcService->serviceHandle);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add Calculation service to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    errCode = flexion_angle_char_add(pCalcService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add flexion angle characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }
    
    errCode = limits_char_add(pCalcService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add limits characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    errCode = calc_err_char_add(pCalcService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add flexion angle characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    return NRF_SUCCESS;
}

// Function to update flexion angle
void flexion_angle_characteristic_update(ble_calc_service_t* pCalcService, flexion_angle_t* angle)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if((pCalcService->connHandle != BLE_CONN_HANDLE_INVALID)) {
        // Update val
        gattsVal.len = sizeof(*angle);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)angle;

        errCode = sd_ble_gatts_value_set(pCalcService->connHandle, pCalcService->flexionAngle.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pCalcService->flexionAngle.notifyEnabled) {
            NRF_LOG_DEBUG("Updating flexion angle with " NRF_LOG_FLOAT_MARKER "\r\n", NRF_LOG_FLOAT(*angle));
            uint16_t len = sizeof(*angle);
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pCalcService->flexionAngle.charHandles.value_handle;
            hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = 0;
            hvxParams.p_len = &len;
            hvxParams.p_data = (uint8_t*)(angle);

            errCode = sd_ble_gatts_hvx(pCalcService->connHandle, &hvxParams);
            APP_ERROR_CHECK(errCode);
        }
    }
}

// Function to update calculation error
void calc_error_characteristic_update(ble_calc_service_t* pCalcService, calc_err_t* error)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if((pCalcService->connHandle != BLE_CONN_HANDLE_INVALID)) {
        // Update val
        gattsVal.len = sizeof(*error);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)error;

        errCode = sd_ble_gatts_value_set(pCalcService->connHandle, pCalcService->calcErr.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pCalcService->calcErr.notifyEnabled) {
            NRF_LOG_DEBUG("Updating calc error with %u", *error);
            uint16_t len = sizeof(*error);
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pCalcService->calcErr.charHandles.value_handle;
            hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = 0;
            hvxParams.p_len = &len;
            hvxParams.p_data = (uint8_t*)(error);

            errCode = sd_ble_gatts_hvx(pCalcService->connHandle, &hvxParams);
            APP_ERROR_CHECK(errCode);
        }
    }
}