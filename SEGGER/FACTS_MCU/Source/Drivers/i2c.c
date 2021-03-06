#include "i2c.h"
#include "nrf_log.h"

// /* TWI instance. */
// static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

// // if we want interrupts
// void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
// {
//     switch (p_event->type)
//     {
//         case NRF_DRV_TWI_EVT_DONE:
//             if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
//             {
//                 data_handler(m_sample);
//             }
//             m_xfer_done = true;
//             break;
//         default:
//             break;
//     }
// }

// FIX
bool i2c_init(const nrf_drv_twi_t* i2c) {
    NRF_LOG_INFO("initializing i2c");
    // ret_code_t err = nrf_twi_sensor_init(i2c);

    const nrf_drv_twi_config_t twi_i2c_config = {
       .scl                = kSCLPin,
       .sda                = kSDAPin,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    ret_code_t err = nrf_drv_twi_init(i2c, &twi_i2c_config, NULL, NULL);
    if (err != NRF_SUCCESS) {
        NRF_LOG_WARNING("failed to initialize i2c");
        return false;
    }

    nrf_drv_twi_enable(i2c);

    return true;
};

bool i2c_read(const nrf_drv_twi_t* i2c, const uint8_t dev_addr, uint8_t* data, uint8_t length) {
    //ret_code_t err = nrf_drv_twi_rx(i2c, dev_addr, reg_addr, data, length);
    ret_code_t err = nrf_drv_twi_rx(i2c, dev_addr, data, length);
    NRF_LOG_INFO("reading: %d, on device: %d", *data, dev_addr);

    if (err != NRF_SUCCESS) {
        NRF_LOG_WARNING("failed to read i2c: %d, dev addr: %d", err, dev_addr);
        return false;
    }

    return true;
};

bool i2c_write(const nrf_drv_twi_t* i2c, const uint8_t dev_addr, uint8_t const* data, uint8_t length) {
    // data length actually needs to be buffer length - 1 (1st byte for register addr)
    // ret_code_t err = nrf_drv_twi_tx(i2c, dev_addr, reg_addr, data, length);
    ret_code_t err = nrf_drv_twi_tx(i2c, dev_addr, data, length, false);
    // NRF_LOG_INFO("writing: %d, on device: %d", *data, dev_addr);

    if (err != NRF_SUCCESS) {
        // NRF_LOG_WARNING("failed to write i2c: %d, dev addr: %d", err, dev_addr);
        return false;
    }

    return true;
};

bool i2c_deinit(nrf_drv_twi_t* i2c) {
    NRF_LOG_INFO("deinitializing i2c");
    return true; // possibly not needed
};




// /**
//  * @brief Function for reading data from temperature sensor.
//  */
// static void read_sensor_data()
// {
//     m_xfer_done = false;

//     /* Read 1 byte from the specified address - skip 3 bits dedicated for fractional part of temperature. */
//     ret_code_t err_code = nrf_drv_twi_rx(&m_twi, LM75B_ADDR, &m_sample, sizeof(m_sample));
//     APP_ERROR_CHECK(err_code);
// }