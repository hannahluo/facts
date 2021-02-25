#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "log_service.h"

#define CCCD_WRITE_LEN      (2)

static char g_LogMsgCharName[] = "Log Message";

static void on_connect(ble_log_service_t* pLogService, ble_evt_t const * pBleEvt)
{
    pLogService->connHandle = pBleEvt->evt.gap_evt.conn_handle;
    pLogService->logMsg.notifyEnabled = false;
}

static void on_disconnect(ble_log_service_t* pLogService, ble_evt_t const * pBleEvt)
{
    pLogService->connHandle = BLE_CONN_HANDLE_INVALID;
    pLogService->logMsg.notifyEnabled = false;
}

static void on_log_msg_write(ble_log_service_t* pLogService, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_log_char_t* pChar = &pLogService->logMsg;
    ble_log_evt_t evt = NUM_BLE_LOG_EVT;

    if((pEvtWrite->handle == pChar->charHandles.cccd_handle) && (pEvtWrite->len == CCCD_WRITE_LEN)) {
        // Read CCCD value
        if(ble_srv_is_notification_enabled(pEvtWrite->data)) {
            NRF_LOG_DEBUG("Notifications enabled for %s", pChar->name);
            pChar->notifyEnabled = true;
            evt = BLE_MSG_EVT_NOTIFY_ENABLED;
        } else {
            NRF_LOG_DEBUG("Notifications disabled for %s", pChar->name);
            pChar->notifyEnabled = false;
            evt = BLE_MSG_EVT_NOTIFY_DISABLED;
        }
    }
}

void ble_log_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext)
{
    ble_log_service_t* pLogService = (ble_log_service_t *)pContext;

    switch(pBleEvt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(pLogService, pBleEvt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(pLogService, pBleEvt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_log_msg_write(pLogService, pBleEvt);
            break;
        default:
            //NRF_LOG_DEBUG("Received 0x%x event", pBleEvt->header.evt_id);
            break;
    }
}

static uint32_t log_msg_char_add(ble_log_service_t* pLogService)
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
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.read_perm);      // App can read to cccd
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccdMd.write_perm);     // App can write to cccd
    cccdMd.vloc = BLE_GATTS_VLOC_STACK;

    // Set meta for characteristic value
    charMd.char_props.read = 1;
    charMd.char_props.notify = 1;
    charMd.p_char_user_desc = g_LogMsgCharName;
    charMd.char_user_desc_size = sizeof(g_LogMsgCharName);
    charMd.char_user_desc_max_size = sizeof(g_LogMsgCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd;
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pLogService->uuidType;
    bleUuid.uuid = BLE_UUID_LOG_MSG_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;     // Char value lives in stack
    attrMd.rd_auth = 0;
    attrMd.wr_auth = 0;
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attrMd.write_perm);  // App can write char val

    log_msg_t msg = {.msg = "", .size=1};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = msg.size;
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = LOG_MSG_MAX_LEN;
    attrCharValue.p_value = msg.msg;

    // Add to SD
    return sd_ble_gatts_characteristic_add(pLogService->serviceHandle, &charMd, &attrCharValue, &pLogService->logMsg.charHandles);
}

uint32_t ble_log_service_init(ble_log_service_t* pLogService, ble_log_evt_handler_t logMsgHandler)
{
    uint32_t errCode;
    ble_uuid_t bleUuid;

    pLogService->connHandle = BLE_CONN_HANDLE_INVALID;
    pLogService->logMsg.evtHandler = logMsgHandler;
    pLogService->logMsg.name = g_LogMsgCharName;

    ble_uuid128_t baseUuid = {BLE_UUID_LOG_SERVICE_BASE_UUID};
    errCode = sd_ble_uuid_vs_add(&baseUuid, &pLogService->uuidType);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add vendor-specific UUID for log service with err=0x%x", errCode);
        return errCode;
    }

    bleUuid.type = pLogService->uuidType;
    bleUuid.uuid = BLE_UUID_LOG_SERVICE_UUID;

    // Register with SoftDevice
    errCode = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &bleUuid, &pLogService->serviceHandle);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add log service to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    errCode = log_msg_char_add(pLogService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add log message characteristic to GATTS module with err=0x%x");
        return errCode;
    }

    return NRF_SUCCESS;
}

// Function to update log message
void log_msg_characteristic_update(ble_log_service_t* pLogService, log_msg_t* msg)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if((pLogService->connHandle != BLE_CONN_HANDLE_INVALID)) {
        // Update val
        gattsVal.len = msg->size;
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)msg->msg;

        errCode = sd_ble_gatts_value_set(pLogService->connHandle, pLogService->logMsg.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pLogService->logMsg.notifyEnabled) {
            uint16_t len = msg->size;
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pLogService->logMsg.charHandles.value_handle;
            hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = 0;
            hvxParams.p_len = &len;
            hvxParams.p_data = (uint8_t*)msg->msg;

            errCode = sd_ble_gatts_hvx(pLogService->connHandle, &hvxParams);
            APP_ERROR_CHECK(errCode);
        }
    }
}