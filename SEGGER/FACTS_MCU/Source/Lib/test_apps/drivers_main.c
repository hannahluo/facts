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
// #include "bno055.h"
#include "imu.h"
#include "tca9548a.h"
#include "drv2605l.h"
//#include "haptic_motors.h"

#define DRV_I2C_ADDR     0x5A
#define ELSA_I2C_IMUADDR 0x29
#define ANNA_I2C_IMUADDR 0x28
#define ELSA_I2C_MUXADDR 0x71
#define ANNA_I2C_MUXADDR 0x70

#define TCA_SELECT_REG   0
#define TCA_SELECT_SIZE  1

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static struct bno055_t elsa_imu;
static struct bno055_t anna_imu;

static tca9548a_t elsa_mux; 
static drv2605l_t elsa_motor;

static tca9548a_t anna_mux;
static drv2605l_t anna_motor;

static const uint8_t HAPTIC_MOTOR_CH0 = 1 << 0;
static const uint8_t HAPTIC_MOTOR_CH1 = 1 << 1;
static const uint8_t HAPTIC_MOTOR_CH2 = 1 << 2;

#define CLK_REG_BASE              (0x40000000)
#define CLK_REG_OFFSET            (0x418)
#define CLK_REG_HF_OFFSET         (0x40c)

#define IMU_ELSA_RESET_PIN        (14)
#define IMU_ANNA_RESET_PIN        (11)
#define MUX_ELSA_RESET_PIN        (16)

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    uint32_t clk_val = 0;
    volatile uint32_t* clk_reg_ptr = (uint32_t*)(CLK_REG_BASE+CLK_REG_OFFSET);
    clk_val = *clk_reg_ptr;
    NRF_LOG_INFO("Clk Reg val 0x%x", clk_val);
    volatile uint32_t* hf_clk_reg_ptr = (uint32_t*)(CLK_REG_BASE+CLK_REG_HF_OFFSET);
    clk_val = *hf_clk_reg_ptr;
    NRF_LOG_INFO("Hf clk reg val 0x%x", clk_val);

    // Reset imu
    nrf_gpio_cfg(IMU_ELSA_RESET_PIN,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(IMU_ELSA_RESET_PIN);
    nrf_gpio_cfg(MUX_ELSA_RESET_PIN,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(MUX_ELSA_RESET_PIN);
    //nrf_delay_ms(1000);
    //nrf_gpio_pin_set(IMU_ELSA_RESET_PIN);

    /*nrf_gpio_cfg(IMU_ANNA_RESET_PIN,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(IMU_ANNA_RESET_PIN);
    nrf_delay_ms(1000);
    nrf_gpio_pin_set(IMU_ANNA_RESET_PIN);*/

    // add error handling lol
    NRF_LOG_INFO("\r\nStart.");
    NRF_LOG_FLUSH();

    NRF_LOG_INFO("\r\nInitializing");
    NRF_LOG_FLUSH();
    if(!i2c_init(&i2c_drv)) {
        NRF_LOG_ERROR("Failed to init i2c");
        return -1;
    }
    while (nrf_drv_twi_is_busy(&i2c_drv)) {};
    nrf_delay_ms(1000);
    //bno055_setup(&elsa_imu, &i2c_drv, ELSA_I2C_IMUADDR);
    //bno055_setup(&anna_imu, &i2c_drv, ANNA_I2C_IMUADDR);
    //nrf_delay_ms(100);

    tca9548a_init(&elsa_mux, 0x71, &i2c_drv);

    NRF_LOG_INFO("\r\nMotor Setup");
    NRF_LOG_FLUSH();
    tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
    drv2605l_init(&elsa_motor, DRV_I2C_ADDR, &elsa_mux);
    drv2605l_mode(&elsa_motor, 0);
    drv2605l_library(&elsa_motor, 6);
    drv2605l_waveform(&elsa_motor, 0, 47);
    drv2605l_waveform(&elsa_motor, 1, 0);
    tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
    drv2605l_init(&elsa_motor, DRV_I2C_ADDR, &elsa_mux);
    drv2605l_mode(&elsa_motor, 0);
    drv2605l_library(&elsa_motor, 6);
    drv2605l_waveform(&elsa_motor, 0, 47);
    drv2605l_waveform(&elsa_motor, 1, 0);
    tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE);
    drv2605l_init(&elsa_motor, DRV_I2C_ADDR, &elsa_mux);
    drv2605l_mode(&elsa_motor, 0);
    drv2605l_library(&elsa_motor, 6);
    drv2605l_waveform(&elsa_motor, 0, 47);
    drv2605l_waveform(&elsa_motor, 1, 0);

    NRF_LOG_INFO("Entering Loop");
    NRF_LOG_FLUSH();
    while (true)
    {
        NRF_LOG_INFO("Motor Run");
        NRF_LOG_FLUSH();
        
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
        drv2605l_go(&elsa_motor);
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
        drv2605l_go(&elsa_motor);
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE);
        drv2605l_go(&elsa_motor);
        nrf_delay_ms(5000);
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH0, TCA_SELECT_SIZE);
        drv2605l_stop(&elsa_motor);
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH1, TCA_SELECT_SIZE);
        drv2605l_stop(&elsa_motor);
        tca9548a_write(&i2c_drv, 0x71, TCA_SELECT_REG, &HAPTIC_MOTOR_CH2, TCA_SELECT_SIZE);
        drv2605l_stop(&elsa_motor);
        
        nrf_delay_ms(500);
    }
}

/** @} */