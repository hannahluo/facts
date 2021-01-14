#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_error.h"
#include "ble_srv_common.h"
#include "debug_service.h"

static const uint8_t Timer1CharName[] = "Timer 1 Elapsed";
static bool timer_notifications_enabled = false;

// Handling a GAP connection event
static void on_connect(ble_debug_service_t * p_debug_service, ble_evt_t const * p_ble_evt)
{
    // store connection handle from GAP into our debug_service structure
    p_debug_service-> conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
    // set the notifications_enabled flag to off
    timer_notifications_enabled = false;
}

// Handling a GAP disconnection event
static void on_disconnect(ble_debug_service_t * p_debug_service, ble_evt_t const * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_debug_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    timer_notifications_enabled = false;
}

// Handling a characteristic write event from GATT client
// This is mainly to allow for writing to the CCCD (for enable/disable notifications)
static void on_write(ble_debug_service_t * p_debug_service, ble_evt_t const * p_ble_evt)
{
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    // Check that the write event was to CCCD attribute and of correct length (2 bytes)
    if((p_evt_write->handle == p_debug_service->timer_1_elapsed_char_handles.cccd_handle) && (p_evt_write->len == 2)) {
        ble_debug_evt_t evt;
        if(ble_srv_is_notification_enabled(p_evt_write->data)) {
            NRF_LOG_INFO("Notifications ENABLED for timer 1 elapsed");
            evt.evt_type = BLE_TIMER_1_ELAPSED_EVT_NOTIFY_ENABLED;
            timer_notifications_enabled = true;
        } else {
            NRF_LOG_INFO("Notifications DISABLED for timer 1 elapsed");
            evt.evt_type = BLE_TIMER_1_ELAPSED_EVT_NOTIFY_DISABLED;
        }
        // Pass to service event handler (in this case is NULL)
        if(p_debug_service->evt_handler != NULL) {
            p_debug_service->evt_handler(p_debug_service, &evt);
        }
    }
}

// Function to handle addition of timer characteristic to the BLE stack
static uint32_t timer_1_elapsed_char_add(ble_debug_service_t * p_debug_service)
{
    ble_gatts_char_md_t char_md;      // GATT characteristic metadata (e.g. defines the characteristic declaration)
    ble_gatts_attr_md_t cccd_md;      // Attribute metadata for cccd (e.g. permissions)
    ble_gatts_attr_t attr_char_value; // GATT attribute (uuid, attribute metadata, val etc.)
    ble_uuid_t ble_uuid;              // UUID (can be 16-but or 128 bit)
    ble_gatts_attr_md_t attr_md;      // Attribute metadata for characteristic value (e.g. permissions)
    uint8_t init_value;               // initial value of the characteristic

    memset(&char_md, 0, sizeof(char_md));
    memset(&cccd_md, 0, sizeof(cccd_md));
    memset(&attr_md, 0, sizeof(attr_md));
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    // Set permissions on CCCD and characteristic value (no protection, open link)
    // e.g. anybody can read the CCCD attribute
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);

    // Set Security Mode 1 Level 2 (enc, no auth) for cccd write permissions
    // Encryption required to write to CCCD attribute
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);

    // Set characteristic value attribute to have no write permissions
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    // Set characteristic value attribute to have open read permissions
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);

    // CCCD settings -- sets the location of the CCCD attr value in the stack memory
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;  

    // Set characteristic meta
    // setting read/notify client access
    char_md.char_props.read = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc = Timer1CharName;                   // setting the description of characteristic
    char_md.char_user_desc_size = sizeof(Timer1CharName);
    char_md.char_user_desc_max_size = sizeof(Timer1CharName);
    char_md.p_char_pf = NULL;                                   // setting the characteristic presentation format (set to NULL for N/A)
    char_md.p_user_desc_md = NULL;                              // setting the attribute meta for User Description descriptor (NULL for default)
    char_md.p_cccd_md = &cccd_md;                               // setting the attribute meta for CCCD  (NULL for default)
    char_md.p_sccd_md = NULL;                                   // setting the attribute meta for SCCD (NULL for default)
    
    // Defining UUID for Timer 1 characteristic 
    ble_uuid.type = p_debug_service->uuid_type;                 // setting UUID to match p_debug_service (either Bluetooth SIG-defined, or custom)
    ble_uuid.uuid = BLE_UUID_TIMER_1_CHAR_UUID;                 // UUID (either 16-bit UUID or octets 12-13 of 128-bit UUID)

    // Setting the characteristic value meta
    attr_md.vloc = BLE_GATTS_VLOC_STACK;                        // characteristic value to be located in stack memory
    attr_md.rd_auth = 0;                                        // read authorization (by the server) not required when client tries to read
    attr_md.wr_auth = 0;                                        // write authorization (by the server) not required when server tries to read
    attr_md.vlen = 0;                                           // variable length of the value

    // Setting the characteristic value settings
    attr_char_value.p_uuid = &ble_uuid;                         // set pointer to attr uuid
    attr_char_value.p_attr_md = &attr_md;                       // set pointer to attr meta
    attr_char_value.init_len = sizeof(init_value);              // set initial length of attribute value
    attr_char_value.init_offs = 0;                              // set the initial offset to use when initializing the attribute
    attr_char_value.max_len = sizeof(init_value);               // set max length of attribute value
    attr_char_value.p_value = &init_value;                      // set the initial value

    // Add the characteristic (declaration, value, and descriptor(s) to the attribute table)
    return sd_ble_gatts_characteristic_add(p_debug_service->service_handle, &char_md, &attr_char_value, &p_debug_service->timer_1_elapsed_char_handles);
}

// Initialization function for debug service
uint32_t ble_debug_service_init(ble_debug_service_t* p_debug_service, ble_debug_evt_handler_t app_evt_handler)
{
    uint32_t err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_debug_service->conn_handle = BLE_CONN_HANDLE_INVALID;
    if(app_evt_handler != NULL) {
        // If callback fn specified, 
        // This callback is invoked every time client writes to the timer 1 characteristic
        p_debug_service->evt_handler = app_evt_handler;
    }

    // Add service UUID
    ble_uuid128_t base_uuid = {BLE_UUID_DEBUG_SERVICE_BASE_UUID};
    err_code = sd_ble_uuid_vs_add(&base_uuid, &p_debug_service->uuid_type); // adds vendor-specific base UUID to BLE stack table
    if(err_code != NRF_SUCCESS) {
        return err_code;
    }

    // Set up UUID for service (base+service-specific)
    ble_uuid.type = p_debug_service->uuid_type;
    ble_uuid.uuid = BLE_UUID_DEBUG_SERVICE_UUID;

    // Set up and add the service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_debug_service->service_handle);
    if(err_code != NRF_SUCCESS) {
        return err_code;
    }

    // Add the characteristics to service
    err_code = timer_1_elapsed_char_add(p_debug_service);
    if(err_code != NRF_SUCCESS) {
        return err_code;
    }
    
    return NRF_SUCCESS;
}

// BLE event handler for debug service to pass to SoftDevice
void ble_debug_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_debug_service_t * p_debug_service = (ble_debug_service_t *) p_context;

    switch(p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_debug_service, p_ble_evt);
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_debug_service, p_ble_evt);
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_debug_service, p_ble_evt);
            break;
        default:
            // No implementation
            break; 
    }
}

// Timer 1 characteristic update function
void timer_1_characteristic_update(ble_debug_service_t * p_debug_service, uint8_t * timer_action)
{
    uint32_t err_code = NRF_SUCCESS;

    // holds the new characteristic value
    ble_gatts_value_t gatts_value;
    memset(&gatts_value,0, sizeof(gatts_value));

    if(p_debug_service->conn_handle != BLE_CONN_HANDLE_INVALID) {
        gatts_value.len = sizeof(uint8_t);
        gatts_value.offset = 0;
        gatts_value.p_value = timer_action;

        // Update characteristic value
        err_code = sd_ble_gatts_value_set(p_debug_service->conn_handle, p_debug_service->timer_1_elapsed_char_handles.value_handle, &gatts_value);
        APP_ERROR_CHECK(err_code);

        if(timer_notifications_enabled) {
            NRF_LOG_INFO("Sending notification for timer 1 press/release");
            uint16_t len = sizeof(uint8_t);
            ble_gatts_hvx_params_t hvx_params;    // structure for notifications/indications
            memset(&hvx_params, 0, sizeof(hvx_params));
            
            hvx_params.handle = p_debug_service->timer_1_elapsed_char_handles.value_handle;
            hvx_params.type = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = 0;
            hvx_params.p_len = &len;
            hvx_params.p_data = (uint8_t*)timer_action;

            err_code = sd_ble_gatts_hvx(p_debug_service->conn_handle, &hvx_params);
            APP_ERROR_CHECK(err_code);
        }
    }
}

