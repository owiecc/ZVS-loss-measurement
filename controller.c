
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"
#include "leds.h"

#define PI_ILhi_Ki 2.0f
#define PI_ILlo_Ki 0.01f
#define HARD_CYCLE_LIMIT 20000 // PWM off
#define D_LIMIT 0.48f

static struct piController PI_DeltaIL = {0, 0, 0, 0, 0}; // ILhi controller
static struct piController PI_ILlo = {0, 0, 0, 0, 0}; // ILlo controller

static float refILhi = 2.0f;
static float refILlo = 0.0f; // value overwritten in main.c

void initPIConttrollers(void)
{
    PI_DeltaIL = initPI(PI_ILhi_Ki/FSW, 2*PI_ILhi_Ki/FSW, 0.5, -1, 0.02, D_LIMIT, 0);
    PI_ILlo = initPI(PI_ILlo_Ki/FSW, 2*PI_ILlo_Ki/FSW, 0.5, -1, 0.00, D_LIMIT, -D_LIMIT);
}

void setControllerILRef(float x) {
    refILlo = x;
    displayValue((int)(10*refILlo));
}

void adjControllerILRef(float x) {
    setControllerILRef(refILlo + x);
}

// adcA1ISR - ADC A Interrupt 1 ISR
// Runs with every switching cycle, runs the control law for the converter
__interrupt void adcA1ISR(void)
{
    struct ADCResult meas = scaleADCs();

    static unsigned int ncycles = 0;

    //ncycles++;
    //if (ncycles >= HARD_CYCLE_LIMIT) // trip the converter
    //{
    //    disablePWM();
    //    ncycles = 0;
    //}

    //meas.ILhi = 0.0f;
    //meas.ILlo = 0.0f;

    float errDeltaIL = (refILhi - refILlo) - (meas.ILhi - meas.ILlo);
    float errILlo = refILlo - meas.ILlo;

    float u1 = updatePI(&PI_DeltaIL, errDeltaIL);
    float u2 = updatePI(&PI_ILlo, errILlo);

    updateEPWM(1.0f - u1 - u2, u1);

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
