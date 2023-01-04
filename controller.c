
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"
#include "leds.h"

#define n1 14
#define n2 21
#define N ((float)n1/(float)n2)
#define Ninv ((float)n2/(float)n1)
#define PI_Vc_Ki 0.05f
#define PI_Io_Ki 0.10f
#define SOFT_CYCLE_LIMIT 50000 // controllers regulate to initial state
#define HARD_CYCLE_LIMIT 60000 // PWM off
#define AUX_SUPPLY_MIN 195

static struct piController PI_ILhi = {0, 0, 0, 0, 0}; // ILhi controller
static struct piController PI_ILlo = {0, 0, 0, 0, 0}; // ILlo controller

static float refILhi = 0.0f; // value overwritten in main.c
static float refILlo = -5.0f;

void initPIConttrollers(void)
{
    PI_ILhi = initPI(PI_Vc_Ki/FSW, 2*PI_Vc_Ki/FSW, 0.5, -1, 0.99, 1.0, 0.2);
    PI_ILlo = initPI(PI_Io_Ki/FSW, 2*PI_Io_Ki/FSW, 0.5, -1, 0.00, 0.25, -0.25);
}

void setControllerILRef(float x) {
    refILhi = x;
    displayValue((int)(refILhi));
}

void adjControllerILRef(float x) {
    setControllerILRef(refILhi + x);
}

// adcA1ISR - ADC A Interrupt 1 ISR
// Runs with every switching cycle, runs the control law for the converter
__interrupt void adcA1ISR(void)
{
    struct ADCResult meas = scaleADCs();

    static unsigned int ncycles = 0;
    ncycles++;

    //
    if (meas.Vin < AUX_SUPPLY_MIN) { ncycles = SOFT_CYCLE_LIMIT; }

    float refVclamp = meas.Vin*Ninv; // track Vin with Vclamp

    if (ncycles >= SOFT_CYCLE_LIMIT) // set default references
    {
        refVclamp = 0.0f;
        setControllerILRef(0.0f);
    }

    if (ncycles >= HARD_CYCLE_LIMIT) // trip the converter
    {
        disablePWM();
        ncycles = 0;
    }
    float errILhi = refILhi - meas.Iout; // TODO fix measured values
    float errILlo = refILlo - meas.Iout;

    float u1 = updatePI(&PI_ILhi, errILhi);
    float u2 = updatePI(&PI_ILlo, errILlo);

    updateModulator(1.0f - u1 - u2, u2);

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
