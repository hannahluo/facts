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
#include "bno055.h"
#include "imu.h"
#include "angle_calculation.h"

#include <math.h>

#define TEST_PIN_SCL      (27)
#define TEST_PIN_SDA      (26)
#define IMU_RESET_PIN     (14)
#define MUX_RESET_PIN     (16)

static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

int main()
{
    log_init();

    // nrf_gpio_pin_pull_get(TEST_PIN_SCL);
    // nrf_gpio_pin_pull_get(TEST_PIN_SDA);

    nrf_gpio_cfg(IMU_RESET_PIN,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(IMU_RESET_PIN);
    nrf_gpio_cfg(MUX_RESET_PIN,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(MUX_RESET_PIN);

    nrf_gpio_cfg(TEST_PIN_SCL,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(TEST_PIN_SDA,NRF_GPIO_PIN_DIR_OUTPUT,NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_pin_clear(TEST_PIN_SCL);
    nrf_gpio_pin_clear(TEST_PIN_SDA);

    //nrf_delay_ms(1000);
    //nrf_gpio_pin_set(IMU_RESET_PIN);

    while(1)
    {
        nrf_delay_ms(1000);
        nrf_gpio_pin_toggle(TEST_PIN_SCL);
        nrf_gpio_pin_toggle(TEST_PIN_SDA);
    }

}