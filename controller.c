
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"
#include "leds.h"

#define PI_ILhi_Ki 50.0f
#define PI_ILlo_Ki 0.05f

static struct piController PI_ILhi = {0, 0, 0, 0, 0}; // ILhi controller
static struct piController PI_ILlo = {0, 0, 0, 0, 0}; // ILlo controller

static float refILhi = 0.0f; // value overwritten in main.c
static float refILlo = -5.0f;

void initPIConttrollers(void)
{
    PI_ILhi = initPI(PI_ILhi_Ki/FSW, 2*PI_ILhi_Ki/FSW, 0.5, -1, 0.02, 0.48, -0.4);
    PI_ILlo = initPI(PI_ILlo_Ki/FSW, 2*PI_ILlo_Ki/FSW, 0.5, -1, 0.00, 0.4, -0.4);
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

    //static unsigned int ncycles = 0;

    //ncycles++;
    //if (ncycles >= HARD_CYCLE_LIMIT) // trip the converter
    //{
    //    disablePWM();
    //    ncycles = 0;
    //}

    float errILhi = refILhi - meas.ILhi; // TODO fix measured values
    float errILlo = refILlo - meas.ILlo;

    float u1 = updatePI(&PI_ILhi, errILhi);
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
