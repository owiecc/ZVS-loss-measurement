
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

static struct piController PI_Vc = {0, 0, 0, 0, 0}; // Vclamp controller
static struct piController PI_Io = {0, 0, 0, 0, 0}; // Iout controller

static float refIo = 0.0f; // value overwritten in main.c
static float refDeltaVclamp = 0.0f;

struct OPConverter SOA = {
    .Vin =    (struct Range) {.lo = 190.0f, .hi =  840.0f},
    .Vout =   (struct Range) {.lo =  90.0f, .hi =  925.0f},
    .Vclamp = (struct Range) {.lo =  90.0f, .hi = 1260.0f},
    .Iout =   (struct Range) {.lo =  -6.0f, .hi =    6.0f}
};

inline int inRange(float x, struct Range r)
{
    return (x<r.hi && x>r.lo) ? 1 : 0;
}

void initPIConttrollers(void)
{
    PI_Vc = initPI(PI_Vc_Ki/FSW, 2*PI_Vc_Ki/FSW, 0.5, -1, 0.99, 1.0, 0.2);
    PI_Io = initPI(PI_Io_Ki/FSW, 2*PI_Io_Ki/FSW, 0.5, -1, 0.00, 0.25, -0.25);
}

void setControllerDeltaVclampRef(float x) { refDeltaVclamp = x; }
void adjControllerDeltaVclampRef(float x) { refDeltaVclamp += x; }

void setControllerIoutRef(float x) { refIo = x; }
void adjControllerIoutRef(float x) { refIo += x; }

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
        setControllerDeltaVclampRef(0.0f);
        setControllerIoutRef(0.0f);
    }

    if (ncycles >= HARD_CYCLE_LIMIT) // trip the converter
    {
        disablePWM();
        ncycles = 0;
    }

    float errVclamp = refVclamp + refDeltaVclamp - meas.Vclamp;
    float errIout = refIo - meas.Iout;

    float d = updatePI(&PI_Vc, -errVclamp);
    float p = updatePI(&PI_Io, errIout);

    updateModulator(d, p);

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
