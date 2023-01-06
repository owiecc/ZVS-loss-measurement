
#include "pwm.h"
#include "f28x_project.h"

// # Structure of the PWM modulator
//
// - EPwm1 = half-bridge A (Q1+Q2)
// - EPwm2 = half-bridge A (Q3+Q4)
//
// ## Phase delays between individual leg modulators
//
// EPwm1 --- (+0) EPwm2 (PWM generation)
//   |
//   └-- (+π/4) EPwm3 (ADC triggering)

void updateEPWM(float m1, float m2)
{
    // TODO: clamp m1&m2 values
    EPwm1Regs.CMPA.bit.CMPA = PWM_PRD_HALF*m1;
    EPwm2Regs.CMPA.bit.CMPA = PWM_PRD_HALF*m2;
}

void initEPWM(void)
{
    EALLOW;
    // Configure state of PWM signals after trip
    EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_LO;          // EPWM1A low on trip
    EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_HI;          // EPWM1B high on trip
    EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_LO;
    EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_HI;

    // Force trip
    disablePWM();

    // PWM1 configuration
    EPwm1Regs.TBPRD = PWM_PRD_HALF;                 // Set period to 20kHz
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;         // Master module
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;          // Shadow register
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;              // TBCLK = SYSCLK
    EPwm1Regs.EPWMSYNCOUTEN.bit.ZEROEN = 1;         // Sync out on CTR = 0
    EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;              // High on counter up
    EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Low on counter down         //
    EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;       // Active high, complementary
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;               // Channel A controls channel B
    EPwm1Regs.DBFED.bit.DBFED = 20;                 // 200ns
    EPwm1Regs.DBRED.bit.DBRED = 20;                 // 200ns

    // PWM2 configuration
    EPwm2Regs.TBPRD = PWM_PRD_HALF;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;              // zero phase delay
    EPwm2Regs.TBPHS.bit.TBPHS = 0;
    EPwm2Regs.EPWMSYNCINSEL.bit.SEL = 1;            // Sync to PWM1
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;
    EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBFED.bit.DBFED = 20;
    EPwm2Regs.DBRED.bit.DBRED = 20;

    // PWM3 configuration
    EPwm3Regs.TBPRD = PWM_PRD_HALF;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm3Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm3Regs.TBPHS.bit.TBPHS = PWM_PRD_QUARTER;    // π/4 phase delay
    EPwm3Regs.EPWMSYNCINSEL.bit.SEL = 1;            // Sync to PWM1

    EPwm3Regs.ETSEL.bit.SOCAEN = 1;                 // Enable SOC on A group
    EPwm3Regs.ETSEL.bit.SOCASEL = 1;                // Select SOC on CTR = 0
    EPwm3Regs.ETPS.bit.SOCAPRD = 1;                 // Generate pulse on 1st event

    EPwm3Regs.ETSEL.bit.SOCBEN = 1;                 // Enable SOC on B group
    EPwm3Regs.ETSEL.bit.SOCBSEL = 2;                // Select SOC on CTR = PRD
    EPwm3Regs.ETPS.bit.SOCBPRD = 1;                 // Generate pulse on 1st event
    EDIS;
}

void disablePWM(void) {
    EALLOW;
    // Force trip
    EPwm1Regs.TZFRC.bit.OST = 1;
    EPwm2Regs.TZFRC.bit.OST = 1;
    EPwm3Regs.TZFRC.bit.OST = 1;

    // Disable counters
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_FREEZE; 
    EDIS;

    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;  // disable gate drivers
}

void enablePWM(void) {
    EALLOW;
    // Set counter value
    EPwm1Regs.TBCTR = 0;
    EPwm2Regs.TBCTR = 0;
    EPwm3Regs.TBCTR = 0;

    // Set duty cycle to 100% on high-side transistor
    EPwm1Regs.CMPA.bit.CMPA = PWM_PRD_HALF;
    EPwm2Regs.CMPA.bit.CMPA = 0; 
    EPwm3Regs.CMPA.bit.CMPA = 0; 

    // Counter up/down mode
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 
    EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 

    // Clear trip
    EPwm1Regs.TZCLR.bit.OST = 1;
    EPwm2Regs.TZCLR.bit.OST = 1;
    EPwm3Regs.TZCLR.bit.OST = 1;
    EDIS;

    GpioDataRegs.GPASET.bit.GPIO6 = 1;  // enable gate drivers
}
