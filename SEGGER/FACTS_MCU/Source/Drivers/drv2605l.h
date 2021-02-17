#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "tca9548.h"

// adapted from https://github.com/sparkfun/SparkFun_Haptic_Motor_Driver_Arduino_Library

//I2C address
#define I2C_ADDR 0x5A

//The Status Register (0x00): The Device ID is bits 7-5. For DRV2605L it should be 7 or 111.
//bits 4 and 2 are reserved. Bit 3 is the diagnostic result. You want to see 0.
//bit 1 is the over temp flag, you want this to be 0
//bit 0 is  over current flag, you want this to be zero.
// Ideally the register will read 0xE0.
#define STATUS_REG 0x00

//The Mode Register (0x01):
//Default 010000000 -- Need to get it out of Standby
//Set to 0000 0000=0x00 to use Internal Trigger
//Set to 0000 0001=0x01 to use External Trigger (edge mode)(like a switch on the IN pin)
//Set to 0000 0010=0x02 to use External Trigger (level mode)
//Set to 0000 0011=0x03 to use PWM input and analog output
//Set to 0000 0100=0x04 to use Audio to Vibe
//Set to 0000 0101=0x05 to use Real-Time Playback
//Set to 0000 0110=0x06 to perform a diagnostic test - result stored in Diagnostic bit in register 0x00
//Set to 0000 0111 =0x07 to run auto calibration
#define MODE_REG 0x01

//The Feedback Control Register (0x1A)
//bit 7: 0 for ERM, 1 for LRA -- Default is 0
//Bits 6-4 control brake factor
//bits 3-2 control the Loop gain
//bit 1-0 control the BEMF gain
#define FEEDBACK_REG 0x1A

//The Real-Time Playback Register (0x02)
//There are 6 ERM libraries.
#define RTP_REG 0x02

//The Library Selection Register (0x03)
//See table 1 in Data Sheet for
#define LIB_REG 0x03

//The waveform Sequencer Register (0X04 to 0x0B)
#define WAVESEQ1 0x04 //Bit 7: set this include a wait time between playback
#define WAVESEQ2 0x05
#define WAVESEQ3 0x06
#define WAVESEQ4 0x07
#define WAVESEQ5 0x08
#define WAVESEQ6 0x09
#define WAVESEQ7 0x0A
#define WAVESEQ8 0x0B

//The Go register (0x0C)
//Set to 0000 0001=0x01 to set the go bit
#define GO_REG 0x0C

//The Overdrive Time Offset Register (0x0D)
//Only useful in open loop mode
#define OVERDRIVE_REG 0x0D

//The Sustain Time Offset, Positive Register (0x0E)
#define SUSTAINOFFSETPOS_REG 0x0E

//The Sustain Time Offset, Negative Register (0x0F)
#define SUSTAINOFFSETNEG_REG 0x0F

//The Break Time Offset Register (0x10)
#define BREAKTIME_REG 0x10

//The Audio to Vibe control Register (0x11)
#define AUDIOCTRL_REG 0x11

//The Audio to vibe minimum input level Register (0x12)
#define AUDMINLVL_REG 0x12

//The Audio to Vibe maximum input level Register (0x13)
#define AUDMAXLVL_REG 0x13

// Audio to Vibe minimum output Drive Register (0x14)
#define AUDMINDRIVE_REG 0x14

//Audio to Vibe maximum output Drive Register (0x15)
#define AUDMAXDRIVE_REG 0X15

//The rated Voltage Register (0x16)
#define RATEDVOLT_REG 0x16

//The Overdive clamp Voltage (0x17)
#define OVERDRIVECLAMP_REG 0x17

//The Auto-Calibration Compensation - Result Register (0x18)
#define COMPRESULT_REG 0x18

//The Auto-Calibration Back-EMF Result Register (0x19)
#define BACKEMF_REG 0x19

//The Control1 Register (0x1B)
//For AC coupling analog inputs and
//Controlling Drive time
#define CONTROL1_REG 0x1B

//The Control2 Register (0x1C)
//See Data Sheet page 45
#define CONTROL2_REG 0x1C

//The COntrol3 Register (0x1D)
//See data sheet page 48
#define CONTROL3_REG 0x1D

//The Control4 Register (0x1E)
//See Data sheet page 49
#define CONTROL4_REG 0x1E

//The Control5 Register (0x1F)
//See Data Sheet page 50
#define CONTROL5_REG 0X1F

//The LRA Open Loop Period Register (0x20)
//This register sets the period to be used for driving an LRA when
//Open Loop mode is selected: see data sheet page 50.
#define OLP_REG 0x20

//The V(Batt) Voltage Monitor Register (0x21)
//This bit provides a real-time reading of the supply voltage
//at the VDD pin. The Device must be actively sending a waveform to take
//reading Vdd=Vbatt[7:0]*5.6V/255
#define VBATMONITOR_REG 0x21

//The LRA Resonance-Period Register
//This bit reports the measurement of the LRA resonance period
#define LRARESPERIOD_REG 0x22

typedef bool (*vibrate)(nrf_drv_twi_t*, uint8_t, uint8_t, uint8_t*, uint8_t, uint8_t);

static constexpr uint8_t kInitSuccess = 0xE0;
static constexpr uint8_t kByteLen = 1u;
static constexpr uint8_t kByteData = 0u;

typedef struct {
    // channels is a bit field of active channels where a set bit = active channel
    uint8_t channel_number; /**< which channels to write to */
    uint8_t dev_addr = I2C_ADDR; /**< i2c device address of drv2605l */

    tca9548_t* i2c_mux; /**< bus write function pointer */
} drv2605l_t;

// TODO: might need a setup motors function
typedef struct {
    drv2605l_t* motors;
    uint8_t num_motors;
    vibrate haptic_motors_vibrate;
} haptic_motors_t;

bool haptic_motors_init(const haptic_motors_t* motors);
bool haptic_motors_vibrate_impl(drv2605l_t* motors, uint8_t num_motors);

bool drv2605l_init(const drv2605l_t* motor);
bool drv2605l_write(drv2605l_t* motor, uint8_t reg_addr, uint8_t data);
bool drv2605l_read(drv2605l_t* motor, uint8_t reg_addr, uint8_t* data);
bool drv2605l_deinit(const drv2605l_t* motor);

// figure out which ones we need
void drv2605l_mode(drv2605l_t* motor, uint8_t mode);
void drv2605l_motor_select(drv2605l_t* motor, uint8_t val);
void drv2605l_rtp(drv2605l_t* motor, uint8_t val);
void drv2605l_library(drv2605l_t* motor, uint8_t val);
void drv2605l_waveform(drv2605l_t* motor, uint8_t seq, uint8_t wav);
void drv2605l_go(drv2605l_t* motor);
void drv2605l_stop(drv2605l_t* motor);
void drv2605l_overdrive(drv2605l_t* motor, uint8_t drive);
void drv2605l_sus_pos(drv2605l_t* motor, uint8_t pos);
void drv2605l_sus_neg(drv2605l_t* motor, uint8_t neg);
void drv2605l_breaktime(drv2605l_t* motor, uint8_t brk);
void drv2605l_audio2vibe(drv2605l_t* motor, uint8_t a2v);
void drv2605l_aud_min(drv2605l_t* motor, uint8_t min);
void drv2605l_aud_max(drv2605l_t* motor, uint8_t max);
void drv2605l_aud_min_drive(drv2605l_t* motor, uint8_t mnd);
void drv2605l_aud_max_drive(drv2605l_t* motor, uint8_t mxd);
void drv2605l_ratevolt(drv2605l_t* motor, uint8_t rat);
void drv2605l_clamp(drv2605l_t* motor, uint8_t clp);
void drv2605l_cntrl1(drv2605l_t* motor, uint8_t c1);
void drv2605l_cntrl2(drv2605l_t* motor, uint8_t c2);
void drv2605l_cntrl3(drv2605l_t* motor, uint8_t c3);
void drv2605l_cntrl4(drv2605l_t* motor, uint8_t c4);
void drv2605l_cntrl5(drv2605l_t* motor, uint8_t c5);
void drv2605l_olp(drv2605l_t* motor, uint8_t olp);
void drv2605l_vbatt(drv2605l_t* motor);
void drv2605l_lra_period(drv2605l_t* motor);
