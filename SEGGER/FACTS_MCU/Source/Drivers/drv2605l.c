#include "drv4605l.h"

bool init() {
    //  TODO

    // Get a read from the status register
    // Want this to Read 0xE0
    uint8_t status = drv2605l_read(STATUS_REG);
    Serial.print("Status Register 0x");
    Serial.println(status, HEX);
    return true;
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
void drv2605l_mode(uint8_t mod) {
    drv2605l_write(MODE_REG, mod);
}

// Select ERM or LRA
// Set Motor Type Using the Feedback Control Register
// Set ERM or LRA
void drv2605l_motor_select(uint8_t val) {
    drv2605l_write(FEEDBACK_REG, val);
}

// Select Library
void drv2605l_library(uint8_t lib) {
    drv2605l_write(LIB_REG, lib);
}

// Select waveform from list of waveform library effects
// data sheet page 60. This function selects the sequencer
// and the effects from the library.
void drv2605l_waveform(uint8_t seq, uint8_t wav) {
    drv2605l_write(WAVESEQ1+seq, wav);
}

// Go command
void drv2605l_go() {
    drv2605l_write(GO_REG, 1);
}

// Stop Command
void drv2605l_stop() {
    drv2605l_write(GO_REG, 0); // write 0 back to go bit to diable internal trigger
}

// Select real time playback features
void drv2605l_rtp(uint8_t val) {
    drv2605l_write(RTP_REG, val); //  Default 0x00, the mode (reg 0x01) must be set to 5
}

// Select overdrive time offset values
void drv2605l_overdrive(uint8_t drive){
    drv2605l_write(OVERDRIVE_REG, drive);
}

// Select Sustain Time offset, positive values
void drv2605l_sus_pos(uint8_t pos) {
    drv2605l_write(SUSTAINOFFSETPOS_REG, pos);
}

// Select Sustain Time offset, negative values
void drv2605l_sus_neg(uint8_t neg) {
    drv2605l_write(SUSTAINOFFSETNEG_REG, neg);
}

// Select Brake Time offset values
void drv2605l_breaktime(uint8_t brk) {
    drv2605l_write(BREAKTIME_REG, brk);
}

// Select Audio-to-Vibe control values
void drv2605l_audio2vibe(uint8_t a2v) {
    drv2605l_write(AUDIOCTRL_REG, a2v);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_min(uint8_t min) {
    drv2605l_write(AUDMINLVL_REG, min);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_max(uint8_t max) {
    drv2605l_write(AUDMAXLVL_REG, max);
}

// Select Audio-to-Vibe minimum input level values
void drv2605l_aud_min_drive(uint8_t mnd) {
    drv2605l_write(AUDMINDRIVE_REG, mnd);
}

// Select Audio-to-Vibe maximum input level values
void drv2605l_aud_max_drive(uint8_t mxd) {
    drv2605l_write(AUDMAXDRIVE_REG, mxd);
}

// Select Rated Voltage
void drv2605l_ratevolt(uint8_t rat) {
    drv2605l_write(RATEDVOLT_REG, rat);
}

// Select overdrive clamp voltage values
void drv2605l_clamp(uint8_t clp) {
    drv2605l_write(OVERDRIVECLAMP_REG, clp);
}

// Control 1 register : datasheet page 44
void drv2605l_cntrl1(uint8_t c1) {
    drv2605l_write(CONTROL1_REG, c1);
}

// Control 2 register : datasheet page 45
void drv2605l_cntrl2(uint8_t c2) {
    drv2605l_write(CONTROL2_REG, c2);
}

// Control 3 register : datasheet page 48
void drv2605l_cntrl3(uint8_t c3) {
   drv2605l_write(CONTROL3_REG, c3);
}

// Control 4 register : datasheet page 49
void drv2605l_cntrl4(uint8_t c4) {
    drv2605l_write(CONTROL4_REG, c4);
}

// Control 5 register : datasheet page 50
void drv2605l_cntrl5(uint8_t c5) {
    drv2605l_write(CONTROL5_REG, c5);
}

// Select LRA Open Loop Period Values
void drv2605l_olp(uint8_t olp) {
    drv2605l_write(OLP_REG, olp);
}

// Read the voltage monitor values
// vdd=Vbatt[7:0] x 5.6V/255
void drv2605l_vbatt(void) {
    uint8_t VBATT = drv2605l_read(VBATMONITOR_REG);
}

// Read the LRA resonance period register
// LRA Period(us) = LRA_Period[7:0] x 98.46us
void drv2605l_lra_period(void) {
    uint8_t PER = drv2605l_read(LRARESPERIOD_REG);
}

// Read function
uint8_t drv2605l_read(uint8_t reg) {
    uint8_t var ;
    // TODO
    Wire.beginTransmission(I2C_ADDR);
    Wire.write((byte)reg);
    Wire.endTransmission();
    Wire.requestFrom((byte)I2C_ADDR, (byte)1);
    var = Wire.read();

    return var;
}

// Write Function
void drv2605l_write(uint8_t where, uint8_t what) {
    // TODO
    Wire.beginTransmission(I2C_ADDR);
    Wire.write((byte)where);
    Wire.write((byte)what);
    Wire.endTransmission();
}

