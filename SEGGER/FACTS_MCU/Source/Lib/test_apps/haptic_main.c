#include <stdio.h>

#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c.h"
#include "tca9548a.h"
#include "drv2605l.h"

#define DRV_I2C_ADDR     0x5A
#define ELSA_I2C_MUXADDR 0x71
#define ANNA_I2C_MUXADDR 0x70

#define TCA_SELECT_REG   0
#define TCA_SELECT_SIZE  1

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static tca9548a_t elsa_mux; 
static drv2605l_t elsa_motor;

static tca9548a_t anna_mux;
static drv2605l_t anna_motor;

static uint8_t HAPTIC_MOTOR_CH0 = 1 << 0;
static uint8_t HAPTIC_MOTOR_CH1 = 1 << 1;
static uint8_t HAPTIC_MOTOR_CH2 = 1 << 2;

// Initialize I2C
int8_t i2c_module_init()
{
    NRF_LOG_INFO("\r\n***I2C Setup Start***\r\n");
    if(!i2c_init(&i2c_drv)) {
        NRF_LOG_ERROR("Failed to init i2c");
        return -1;
    }
    while (nrf_drv_twi_is_busy(&i2c_drv)) {};
    nrf_delay_ms(1000);
    NRF_LOG_INFO("\r\n***I2C Setup End***\r\n");
    
    return 0;
}

/**
 * @brief Function for main application entry.
 */

int8_t motor_init(drv2605l_t* motor, tca9548a_t* mux, uint8_t mux_addr)
{
    if(!tca9548a_init(mux, mux_addr, &i2c_drv)) {
        NRF_LOG_ERROR("Failed to init comm to mux");
        return -1;
    }
    
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch0");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch1");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for init");
        return -1;
    }
    if(!drv2605l_init(motor, DRV_I2C_ADDR, mux)) {
        NRF_LOG_ERROR("Failed to init mux ch2");
        return -1;
    }
    drv2605l_mode(motor, 0);
    drv2605l_library(motor, 6);
    drv2605l_waveform(motor, 0, 47);
    drv2605l_waveform(motor, 1, 0);

    return 0;
}

// Initialize haptic
int8_t haptic_module_init()
{
    NRF_LOG_INFO("\r\n ***Motor Setup Start*** \r\n");
    if(motor_init(&elsa_motor, &elsa_mux, ELSA_I2C_MUXADDR) < 0) {
        NRF_LOG_ERROR("Failed to initialize elsa(thigh) haptics");
        return -1;
    }
    if(motor_init(&anna_motor, &anna_mux, ANNA_I2C_MUXADDR) < 0) {
        NRF_LOG_ERROR("Failed to initialize anna(calf) haptics");
        return -1;
    }
    NRF_LOG_INFO("\r\n ***Motor Setup End*** \r\n");

    return 0;
}

int8_t turn_on_motors(drv2605l_t* motor, uint8_t mux_addr)
{
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for go");
        return -1;
    }
    drv2605l_go(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for go");
        return -1;
    }
    drv2605l_go(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for go");
        return -1;
    }
    drv2605l_go(motor);

    return 0;
}

int8_t turn_off_motors(drv2605l_t* motor, uint8_t mux_addr)
{
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch0 for stop");
        return -1;
    }
    drv2605l_stop(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch1 for stop");
        return -1;
    }
    drv2605l_stop(motor);
    if(!tca9548a_write(&i2c_drv, mux_addr, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE)) {
        NRF_LOG_ERROR("Failed to set motor channel to mux ch2 for stop");
        return -1;
    }
    drv2605l_stop(motor);

    return 0;
}

void haptic_module_deinit()
{
    drv2605l_deinit(&elsa_motor);
    drv2605l_deinit(&anna_motor);
    NRF_LOG_FLUSH();
}

int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    // add error handling
    NRF_LOG_INFO("\r\nInitializing...");
    if(i2c_module_init() < 0) {
        NRF_LOG_ERROR("Failed to initialize i2c module");
        return -1;
    }

    if(haptic_module_init() < 0) {
        NRF_LOG_ERROR("Failed to initialize haptic module");
        haptic_module_deinit();
        return -1;
    }
    NRF_LOG_INFO("Initialized");

    NRF_LOG_INFO("The boys are buzzing");
    NRF_LOG_FLUSH();
    uint8_t cnt = 0;
    while (true)
    {
        cnt++;
        if(cnt % 2 == 0) {
            NRF_LOG_INFO("Motor Run");
            NRF_LOG_FLUSH();
            if(turn_on_motors(&elsa_motor, ELSA_I2C_MUXADDR) < 0) {
                NRF_LOG_ERROR("Failed to turn on elsa motor");
                break;
            }
            if(turn_on_motors(&anna_motor, ANNA_I2C_MUXADDR) < 0) {
                NRF_LOG_ERROR("Failed to turn on anna motor");
                break;
            }
        } else {
            NRF_LOG_INFO("Motor Off");
            NRF_LOG_FLUSH();
            if(turn_off_motors(&elsa_motor, ELSA_I2C_MUXADDR) < 0) {
                NRF_LOG_ERROR("Failed to turn off elsa motor");
                break;
            }
            if(turn_off_motors(&anna_motor, ANNA_I2C_MUXADDR) < 0) {
                NRF_LOG_ERROR("Failed to turn off anna motor");
                break;
            }
        }
        
        nrf_delay_ms(3000);
    }

    NRF_LOG_INFO("Haptic Motor Testing Done");
    haptic_module_deinit();
}

/** @} */