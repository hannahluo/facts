#include "drv2605l.h"
#include "nrf_log.h"

bool drv2605l_init(drv2605l_t* motor, uint8_t channel_number, tca9548a_t* i2c_mux) {
    NRF_LOG_INFO("initializing haptic motor driver");
    motor->channel_number = channel_number;
    motor->i2c_mux = i2c_mux;
    //bool err = tca9548a_init(motor->i2c_mux);

    //if (err == false) {
    //    NRF_LOG_WARNING("failed to init haptic motor driver");
    //    return err;
    //}

    // assign any fct ptrs

    // Want this to Read 0xE0
    uint8_t data = kByteData;
    bool err = drv2605l_read(motor, STATUS_REG, &data);
    if (err == false) {
        NRF_LOG_WARNING("failed to read haptic motor driver");
    }

    if (data != kInitSuccess) {
        NRF_LOG_WARNING("haptic motor driver status error");
        err = false;
    }
    NRF_LOG_INFO("initialized haptic motor driver: %d", data);

    return err;
}

bool drv2605l_deinit(drv2605l_t* motor) {
    NRF_LOG_INFO("deinitializing haptic motor driver");
    tca9548a_deinit(motor->i2c_mux);
    // delete i2x mux?

    return true; // possibly not needed
}

// Select Mode
// Write 0x00 to get out of standby and use internal trigger (using GO command)
// Write 0x01 to get out of standby + use External Trigger (edge triggered)
// Write 0x02 to get out of standby + External Trigger (level triggered)
// Write 0x03 to get out of standby + PWM input and analog output
// Write 0x04 to use Audio to Vibe
// Write 0x05 to use real time playback
// Write 0x06 to perform a diagnostic - result stored in diagnostic bit in register 0x00
// Write 0x07 to run auto calibration
void drv2605l_mode(drv2605l_t* motor, uint8_t mod) {
    drv2605l_write(motor, MODE_REG, mod);
}

// Select ERM or LRA
// Set Motor Type Using the Feedback Control Register
// Set ERM or LRA
void drv2605l_motor_select(drv2605l_t* motor, uint8_t val) {
    drv2605l_write(motor, FEEDBACK_REG, val);
}

// Select Library
void drv2605l_library(drv2605l_t* motor, uint8_t lib) {
    drv2605l_write(motor, LIB_REG, lib);
}

// Select waveform from list of waveform library effects
// data sheet page 60. This function selects the sequencer
// and the effects from the library.
void drv2605l_waveform(drv2605l_t* motor, uint8_t seq, uint8_t wav) {
    drv2605l_write(motor, WAVESEQ1+seq, wav);
}

// Go command
bool drv2605l_go(drv2605l_t* motor) {
    return drv2605l_write(motor, GO_REG, 1);
}

// Stop Command
void drv2605l_stop(drv2605l_t* motor) {
    drv2605l_write(motor, GO_REG, 0); // write 0 back to go bit to diable internal trigger
}

// Select real time playback features
void drv2605l_rtp(drv2605l_t* motor, uint8_t val) {
    drv2605l_write(motor, RTP_REG, val); //  Default 0x00, the mode (reg 0x01) must be set to 5
}

// Select overdrive time offset values
void drv2605l_overdrive(drv2605l_t* motor, uint8_t drive){
    drv2605l_write(motor, OVERDRIVE_REG, drive);
}

// Select Sustain Time offset, positive values
void drv2605l_sus_pos(drv2605l_t* motor, uint8_t pos) {
    drv2605l_write(motor, SUSTAINOFFSETPOS_REG, pos);
}

// Select Sustain Time offset, negative values
void drv2605l_sus_neg(drv2605l_t* motor, uint8_t neg) {
    drv2605l_write(motor, SUSTAINOFFSETNEG_REG, neg);
}

// Select Brake Time offset values
void drv2605l_breaktime(drv2605l_t* motor, uint8_t brk) {
    drv2605l_write(motor, BREAKTIME_REG, brk);
}

// Select Audio-to-Vibe control values
void drv2605l_audio2vibe(drv2605l_t* motor, uint8_t a2v) {
    drv2605l_write(motor, AUDIOCTRL_REG, a2v);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_min(drv2605l_t* motor, uint8_t min) {
    drv2605l_write(motor, AUDMINLVL_REG, min);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_max(drv2605l_t* motor, uint8_t max) {
    drv2605l_write(motor, AUDMAXLVL_REG, max);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_min_drive(drv2605l_t* motor, uint8_t mnd) {
    drv2605l_write(motor, AUDMINDRIVE_REG, mnd);
}

// Select Audio-to-Vibe maximum input level values
void drv2605l_aud_max_drive(drv2605l_t* motor, uint8_t mxd) {
    drv2605l_write(motor, AUDMAXDRIVE_REG, mxd);
}

// Select Rated Voltage
void drv2605l_ratevolt(drv2605l_t* motor, uint8_t rat) {
    drv2605l_write(motor, RATEDVOLT_REG, rat);
}

// Select overdrive clamp voltage values
void drv2605l_clamp(drv2605l_t* motor, uint8_t clp) {
    drv2605l_write(motor, OVERDRIVECLAMP_REG, clp);
}

// Control 1 register : datasheet page 44
void drv2605l_cntrl1(drv2605l_t* motor, uint8_t c1) {
    drv2605l_write(motor, CONTROL1_REG, c1);
}

// Control 2 register : datasheet page 45
void drv2605l_cntrl2(drv2605l_t* motor, uint8_t c2) {
    drv2605l_write(motor, CONTROL2_REG, c2);
}

// Control 3 register : datasheet page 48
void drv2605l_cntrl3(drv2605l_t* motor, uint8_t c3) {
   drv2605l_write(motor, CONTROL3_REG, c3);
}

// Control 4 register : datasheet page 49
void drv2605l_cntrl4(drv2605l_t* motor, uint8_t c4) {
    drv2605l_write(motor, CONTROL4_REG, c4);
}

// Control 5 register : datasheet page 50
void drv2605l_cntrl5(drv2605l_t* motor, uint8_t c5) {
    drv2605l_write(motor, CONTROL5_REG, c5);
}

// Select LRA Open Loop Period Values
void drv2605l_olp(drv2605l_t* motor, uint8_t olp) {
    drv2605l_write(motor, OLP_REG, olp);
}

// Read the voltage monitor values
// vdd=Vbatt[7:0] x 5.6V/255
uint8_t drv2605l_vbatt(drv2605l_t* motor) {
    uint8_t data = kByteData;
    drv2605l_read(motor, VBATMONITOR_REG, &data);
    return data;
}

// Read the LRA resonance period register
// LRA Period(us) = LRA_Period[7:0] x 98.46us
uint8_t drv2605l_lra_period(drv2605l_t* motor) {
    uint8_t data = kByteData;
    drv2605l_read(motor, LRARESPERIOD_REG, &data);
    return data;
}

// Read function
bool drv2605l_read(drv2605l_t* motor, uint8_t reg_addr, uint8_t* data) {
    uint8_t array[kByteLen];
    memset(array, kByteData, sizeof(uint8_t));
    bool succ = tca9548a_read(motor->i2c_mux->i2c, motor->i2c_mux->dev_addr, reg_addr,
        &array[kByteData], kByteLen, motor->channel_number);

    if (succ) {
        *data = array[0];
    } else {
        NRF_LOG_WARNING("haptic motor driver read error");
    }

    return succ;
}

// Write function
bool drv2605l_write(drv2605l_t* motor, uint8_t reg_addr, uint8_t data) {
    uint8_t array[kByteLen];
    memset(array, data, sizeof(uint8_t));
    bool succ = tca9548a_write(motor->i2c_mux->i2c, motor->i2c_mux->dev_addr, reg_addr,
        &array[kByteData], kByteLen, motor->channel_number);

    if (succ == false) {
        NRF_LOG_WARNING("haptic motor driver write error");
    }

    return succ;
}

