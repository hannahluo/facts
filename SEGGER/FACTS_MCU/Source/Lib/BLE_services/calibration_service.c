#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "calibration_service.h"

#define CCCD_WRITE_LEN      (2)

static char g_InitCalibCharName[] = "Initiate Calibration";
static char g_CalfJointAxisCharName[] = "Calf Joint Axis";
static char g_ThighJointAxisCharName[] = "Thigh Joint Axis";

static void on_connect(ble_calib_service_t* pCalibService, ble_evt_t const * pBleEvt)
{
    pCalibService->connHandle = pBleEvt->evt.gap_evt.conn_handle;
    pCalibService->initCalib.notifyEnabled = false;
    pCalibService->calfJointAxis.notifyEnabled = false;
    pCalibService->thighJointAxis.notifyEnabled = false;
}

static void on_disconnect(ble_calib_service_t* pCalibService, ble_evt_t const * pBleEvt)
{
    UNUSED_PARAMETER(pBleEvt);
    pCalibService->connHandle = BLE_CONN_HANDLE_INVALID;
    pCalibService->initCalib.notifyEnabled = false;
    pCalibService->calfJointAxis.notifyEnabled = false;
    pCalibService->thighJointAxis.notifyEnabled = false;
}

static void on_init_calib_write(ble_calib_service_t* pCalibService,  ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_calib_char_t* pChar = &pCalibService->initCalib;
    ble_calib_evt_t evt = NUM_BLE_CALIB_EVT;

    if((pEvtWrite->handle == pChar->charHandles.cccd_handle) && (pEvtWrite->len == CCCD_WRITE_LEN)) {
        // Read CCCD value
        if(ble_srv_is_notification_enabled(pEvtWrite->data)) {
            NRF_LOG_DEBUG("Notifications enabled for %s", pChar->name);
            evt = BLE_INIT_CALIB_EVT_NOTIFY_ENABLED;
            pChar->notifyEnabled = true;
        } else {
            NRF_LOG_DEBUG("Notifications disabled for %s", pChar->name);
            evt = BLE_INIT_CALIB_EVT_NOTIFY_DISABLED;
            pChar->notifyEnabled = false;
        }
    } else if ((pEvtWrite->handle == pChar->charHandles.value_handle) && (pEvtWrite->len == sizeof(init_calib_t))) {
        NRF_LOG_DEBUG("on_init_calib_write: Data=%d", *(uint8_t*)(pEvtWrite->data));
        if(*(bool*)(pEvtWrite->data)) {
            NRF_LOG_DEBUG("on_init_calib_write: Calibration mode on");
            evt = BLE_INIT_CALIB_ON;
        } else {
            NRF_LOG_DEBUG("on_init_calib_write: Calibration mode off");
            evt = BLE_INIT_CALIB_OFF;
        }

        if(pChar->evtHandler != NULL) {
            pChar->evtHandler(pCalibService, evt, pEvtWrite->data, pEvtWrite->len);
        }
    }
}

static void on_joint_axis_write(ble_calib_service_t* pCalibService, ble_calib_char_t* pChar, ble_evt_t const * pBleEvt)
{
    ble_gatts_evt_write_t const * pEvtWrite = &pBleEvt->evt.gatts_evt.params.write;
    ble_calib_evt_t evt = NUM_BLE_CALIB_EVT;

    if((pEvtWrite->handle == pChar->charHandles.value_handle) && (pEvtWrite->len == sizeof(joint_axis_t))) {
        if(pChar == &pCalibService->calfJointAxis) {
            evt = BLE_CALF_JOINT_AXIS_WRITE;
        } else {
            evt = BLE_THIGH_JOINT_AXIS_WRITE;
        }

        if(pChar->evtHandler != NULL) {
            pChar->evtHandler(pCalibService, evt, pEvtWrite->data, pEvtWrite->len);
        }
    }
}

static void on_write(ble_calib_service_t* pCalibService, ble_evt_t const * pBleEvt)
{
    uint16_t handle = pBleEvt->evt.gatts_evt.params.write.handle;

    if(handle == pCalibService->initCalib.charHandles.cccd_handle || handle == pCalibService->initCalib.charHandles.value_handle) {
        on_init_calib_write(pCalibService, pBleEvt);
    } else if (handle == pCalibService->calfJointAxis.charHandles.value_handle) {
        on_joint_axis_write(pCalibService, &pCalibService->calfJointAxis, pBleEvt);
    } else if (handle == pCalibService->thighJointAxis.charHandles.value_handle) {
        on_joint_axis_write(pCalibService, &pCalibService->thighJointAxis, pBleEvt);
    } else {
        NRF_LOG_DEBUG("Write to unknown handle 0x%x", handle);
    }
}

// BLE event handler
void ble_calib_service_on_ble_evt(ble_evt_t const * pBleEvt, void* pContext)
{
    ble_calib_service_t* pCalibService = (ble_calib_service_t *)pContext;
    uint16_t handle = 0;
    // check event header for event type
    switch(pBleEvt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(pCalibService, pBleEvt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(pCalibService, pBleEvt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(pCalibService, pBleEvt);
            break;
        default:
            //NRF_LOG_DEBUG("Received 0x%x event", pBleEvt->header.evt_id);
            break;            
    }
}

static uint32_t init_calib_char_add(ble_calib_service_t* pCalibService)
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
    charMd.char_props.write = 1;
    charMd.p_char_user_desc = g_InitCalibCharName;
    charMd.char_user_desc_size = sizeof(g_InitCalibCharName);
    charMd.char_user_desc_max_size = sizeof(g_InitCalibCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = &cccdMd; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalibService->uuidType;
    bleUuid.uuid = BLE_UUID_INIT_CALIB_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.write_perm);       // App can write char val

    // Set meta for characteristic value
    init_calib_t calib = false;
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(calib);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(calib);
    attrCharValue.p_value = (uint8_t*)&calib;

    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pCalibService->serviceHandle, &charMd, &attrCharValue, &pCalibService->initCalib.charHandles);
}

static uint32_t calf_joint_axis_char_add(ble_calib_service_t* pCalibService)
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
    charMd.p_char_user_desc = g_CalfJointAxisCharName;
    charMd.char_user_desc_size = sizeof(g_CalfJointAxisCharName);
    charMd.char_user_desc_max_size = sizeof(g_CalfJointAxisCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = NULL; 
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalibService->uuidType;
    bleUuid.uuid = BLE_UUID_CALF_JOINT_AXIS_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.write_perm);       // App can write char val

    // Setting meta for characterisitic value
    joint_axis_t calf = {.x=0, .y=0, .z=0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(calf);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(calf);
    attrCharValue.p_value = (uint8_t*)&calf;
    
    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pCalibService->serviceHandle, &charMd, &attrCharValue, &pCalibService->calfJointAxis.charHandles);
}

static uint32_t thigh_joint_axis_char_add(ble_calib_service_t* pCalibService)
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
    charMd.p_char_user_desc = g_ThighJointAxisCharName;
    charMd.char_user_desc_size = sizeof(g_ThighJointAxisCharName);
    charMd.char_user_desc_max_size = sizeof(g_ThighJointAxisCharName);
    charMd.p_char_pf = NULL;
    charMd.p_user_desc_md = NULL;
    charMd.p_cccd_md = NULL;
    charMd.p_sccd_md = NULL;

    // Defining uuid
    bleUuid.type = pCalibService->uuidType;
    bleUuid.uuid = BLE_UUID_THIGH_JOINT_AXIS_UUID;

    // Setting char value meta
    attrMd.vloc = BLE_GATTS_VLOC_STACK;                       // Char val lives in stack
    attrMd.rd_auth = 0;                                       // Dev doesn't need to auth reads
    attrMd.wr_auth = 0;                                       // Dev doesn't need to auth writes
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.read_perm);        // App can read char val
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attrMd.write_perm);       // App can write char val

    // Setting meta for characterisitic value
    joint_axis_t thigh = {.x=0, .y=0, .z=0};
    attrCharValue.p_uuid = &bleUuid;
    attrCharValue.p_attr_md = &attrMd;
    attrCharValue.init_len = sizeof(thigh);
    attrCharValue.init_offs = 0;
    attrCharValue.max_len = sizeof(thigh);
    attrCharValue.p_value = (uint8_t*)&thigh;
    
    // Add characteristic to SoftDevice
    return sd_ble_gatts_characteristic_add(pCalibService->serviceHandle, &charMd, &attrCharValue, &pCalibService->thighJointAxis.charHandles);
}

uint32_t ble_calib_service_init(ble_calib_service_t* pCalibService, ble_calib_evt_handler_t initCalHandler, 
                                ble_calib_evt_handler_t calfJointAxisHandler, ble_calib_evt_handler_t thighJointAxisHandler)
{
    uint32_t errCode;
    ble_uuid_t bleUuid;

    pCalibService->connHandle = BLE_CONN_HANDLE_INVALID;
    pCalibService->initCalib.evtHandler = initCalHandler;
    pCalibService->calfJointAxis.evtHandler = calfJointAxisHandler;
    pCalibService->thighJointAxis.evtHandler = thighJointAxisHandler;
    pCalibService->initCalib.name = g_InitCalibCharName;
    pCalibService->calfJointAxis.name = g_CalfJointAxisCharName;
    pCalibService->thighJointAxis.name = g_ThighJointAxisCharName;

    ble_uuid128_t baseUuid = {BLE_UUID_CALIB_SERVICE_BASE_UUID};
    errCode = sd_ble_uuid_vs_add(&baseUuid, &pCalibService->uuidType);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add vendor-specific UUID for Calibration service with err=0x%x", errCode);
        return errCode;
    }

    bleUuid.type = pCalibService->uuidType;
    bleUuid.uuid = BLE_UUID_CALIB_SERVICE_UUID;

    // Register service with SoftDevice
    errCode = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &bleUuid, &pCalibService->serviceHandle);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add Calibration service to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    errCode = init_calib_char_add(pCalibService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add initiate calibration characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    // Add calf joint axis characteristic to service
    errCode = calf_joint_axis_char_add(pCalibService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add calf joint axis characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    // Add thigh joint axis characteristic to service
    errCode = thigh_joint_axis_char_add(pCalibService);
    if(errCode != NRF_SUCCESS) {
        NRF_LOG_ERROR("Failed to add thigh joint axis characteristic to GATTS module with err=0x%x", errCode);
        return errCode;
    }

    return NRF_SUCCESS;
}

// Function to update init cal characteristic value
void init_calib_characteristic_update(ble_calib_service_t* pCalibService, init_calib_t* initCalibValue)
{
    uint32_t errCode = NRF_SUCCESS;
    ble_gatts_value_t gattsVal;
    memset(&gattsVal, 0, sizeof(gattsVal));

    if(pCalibService->connHandle != BLE_CONN_HANDLE_INVALID) {
        // Update val 
        gattsVal.len = sizeof(*initCalibValue);
        gattsVal.offset = 0;
        gattsVal.p_value = (uint8_t*)initCalibValue;

        errCode = sd_ble_gatts_value_set(pCalibService->connHandle, pCalibService->initCalib.charHandles.value_handle, &gattsVal);
        APP_ERROR_CHECK(errCode);

        if(pCalibService->initCalib.notifyEnabled) {
            NRF_LOG_DEBUG("Updating init cal with %d", (uint8_t)*initCalibValue);
            uint16_t len = sizeof(*initCalibValue);
            ble_gatts_hvx_params_t hvxParams;
            memset(&hvxParams, 0, sizeof(hvxParams));

            hvxParams.handle = pCalibService->initCalib.charHandles.value_handle;
            hvxParams.type = BLE_GATT_HVX_NOTIFICATION;
            hvxParams.offset = 0;
            hvxParams.p_len = &len;
            hvxParams.p_data = (uint8_t*)(initCalibValue);

            errCode = sd_ble_gatts_hvx(pCalibService->connHandle, &hvxParams);
            APP_ERROR_CHECK(errCode);
        }
    }
}