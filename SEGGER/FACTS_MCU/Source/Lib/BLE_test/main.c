/**
 * Copyright (c) 2015 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup tw_sensor_example main.c
 * @{
 * @ingroup nrf_twi_example
 * @brief TWI Sensor Example main file.
 *
 * This file contains the source code for a sample application using TWI.
 *
 */

#include <stdio.h>

#include "boards.h"
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
//#include "haptic_motors.h"
#include "bno055.h"

#define ELSA_I2C_IMUADDR 0x29
#define ANNA_I2C_IMUADDR 0x28
#define ELSA_I2C_MUXADDR 0x71
#define ANNA_I2C_MUXADDR 0x70
#define HAPTIC_MOTOR_CH0 0
#define HAPTIC_MOTOR_CH1 1
#define HAPTIC_MOTOR_CH2 2

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* TWI instance. */
static const nrf_drv_twi_t i2c_drv = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static struct bno055_t elsa_imu = { .dev_addr = ELSA_I2C_IMUADDR, .i2c = &i2c_drv };
static struct bno055_t anna_imu = { .dev_addr = ANNA_I2C_IMUADDR, .i2c = &i2c_drv };

static tca9548a_t elsa_mux;  // = { .dev_addr = ELSA_I2C_MUXADDR, .i2c = &i2c_drv };
static drv2605l_t elsa_m1; // = { .channel_number = HAPTIC_MOTOR_CH1, .i2c_mux = &elsa_mux };
//static struct drv2605l_t elsa_m2 = { .channel_number = HAPTIC_MOTOR_CH2, .i2c_mux = &elsa_mux };
//static struct drv2605l_t elsa_m3 = { .channel_number = HAPTIC_MOTOR_CH3, .i2c_mux = &elsa_mux };

static tca9548a_t anna_mux;  // = { .dev_addr = ANNA_I2C_MUXADDR, .i2c = &i2c_drv };
static drv2605l_t anna_m1; // = { .channel_number = HAPTIC_MOTOR_CH1, .i2c_mux = &anna_mux };
//static struct drv2605l_t anna_m2 = { .channel_number = HAPTIC_MOTOR_CH2, .i2c_mux = &anna_mux };
//static struct drv2605l_t anna_m3 = { .channel_number = HAPTIC_MOTOR_CH3, .i2c_mux = &anna_mux };

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    // add error handling lol
    NRF_LOG_INFO("\r\nStart.");
    NRF_LOG_FLUSH();

    NRF_LOG_INFO("\r\nInitializing");
    NRF_LOG_FLUSH();
    i2c_init(&i2c_drv);
    while (nrf_drv_twi_is_busy(&i2c_drv)) {};
    nrf_delay_ms(1000);
    bno055_init(&elsa_imu);
    bno055_init(&anna_imu);

    /*tca9548a_init(&elsa_mux, 0x5A, &i2c_drv);
    // tca9548a_init(&anna_mux, ANNA_I2C_MUXADDR, &i2c_drv);

    NRF_LOG_INFO("\r\nMotor Setup");
    NRF_LOG_FLUSH();
    drv2605l_init(&elsa_m1, HAPTIC_MOTOR_CH0, &elsa_mux);

    uint8_t md = kByteData;
    drv2605l_read(&elsa_m1, MODE_REG, &md);
    NRF_LOG_INFO("MODE 1: %d", md);
    drv2605l_write(&elsa_m1, MODE_REG, 0);
    drv2605l_read(&elsa_m1, MODE_REG, &md);
    NRF_LOG_INFO("MODE 2: %d", md);

    drv2605l_library(&elsa_m1, 6);
    // drv2605l_init(&anna_m1, HAPTIC_MOTOR_CH0, &anna_mux);
    // drv2605l_library(&anna_m1, 6); */

    struct bno055_accel_t elsa_data;
    struct bno055_accel_t anna_data;

    NRF_LOG_INFO("Entering Loop");
    NRF_LOG_FLUSH();
    while (true)
    {
        /*drv2605l_waveform(&elsa_m1, 0, 47);
        drv2605l_waveform(&elsa_m1, 1, 0);

        NRF_LOG_INFO("Motor Run");
        NRF_LOG_FLUSH();
        drv2605l_go(&elsa_m1);
        nrf_delay_ms(5000);
        drv2605l_stop(&elsa_m1);*/

        bool res1 = bno055_read_accel_xyz(&elsa_data);
        bool res2 = bno055_read_accel_xyz(&anna_data);
        NRF_LOG_INFO("elsa accel x: %d y: %d z: %d", elsa_data.x, elsa_data.y, elsa_data.z);
        NRF_LOG_INFO("anna accel x: %d y: %d z: %d", anna_data.x, anna_data.y, anna_data.z);

        nrf_delay_ms(5000);
    }
}

/** @} */
