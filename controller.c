
#include "PI_controller.h"
#include "controller.h"
#include "f28x_project.h"
#include "pwm.h"

#define n1 14
#define n2 21
#define N ((float)n1/(float)n2)
#define Ninv ((float)n2/(float)n1)
#define FSW 40000
#define PI_Vc_Ki 0.05f
#define PI_Io_Ki 0.10f
#define CYCLE_LIMIT 60000

static struct piController PI_Vc = {0, 0, 0, 0, 0}; // Vclamp controller
static struct piController PI_Io = {0, 0, 0, 0, 0}; // Iout controller

static float refIo = 0.0f; // value overwritten in main.c
static float refDeltaVclamp = 0.0f;

static enum trip_status * tripFeedback;

struct OPLimitsConverter SOA = {
    .Vin =    (struct OPLimits) {.tripHi =  840, .startupHi =  820, .startupLo =  780, .tripLo = 720 },
    .Vout =   (struct OPLimits) {.tripHi =  930, .startupHi =  925, .startupLo =  195, .tripLo = 190 },
    .Vclamp = (struct OPLimits) {.tripHi = 1260, .startupHi = 1230, .startupLo = 1170, .tripLo = 1140},
    .Iout =   (struct OPLimits) {.tripHi =    2, .startupHi =  0.1, .startupLo = -0.1, .tripLo = 0   }
};

inline int isInSOAOn(float val, struct OPLimits lims)
{
    return val < lims.tripHi && val > lims.tripLo;
}

inline int isInSOAStartup(float val, struct OPLimits lims)
{
    return val < lims.startupHi && val > lims.startupLo;
}

enum trip_status isInSOA(struct ADCResult sensors, enum converter_states cs)
{
    switch (cs) {
    case StateOn:
        if (!isInSOAOn(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAOn(sensors.Vclamp, SOA.Vclamp)) return TripSOAVclamp;
        if (!isInSOAOn(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAOn(sensors.Vin, SOA.Vin)) return TripSOAVin;
        break;
    case StateStandby:
        if (!isInSOAStartup(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAStartup(sensors.Vclamp, SOA.Vout)) return TripSOAVclamp; // Vclamp is charged to Vout
        if (!isInSOAStartup(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAStartup(sensors.Vin, SOA.Vin)) return TripSOAVin;
        break;
    default:
        if (!isInSOAStartup(sensors.Iout, SOA.Iout)) return TripOC;
        if (!isInSOAStartup(sensors.Vclamp, SOA.Vclamp)) return TripSOAVclamp;
        if (!isInSOAStartup(sensors.Vout, SOA.Vout)) return TripSOAVout;
        if (!isInSOAStartup(sensors.Vin, SOA.Vin)) return TripSOAVin;
    }
    return NoTrip;
}

void initTripFeedback(enum trip_status *x)
{
    tripFeedback = x;
}

void initPIConttrollers(void)
{
    PI_Vc = initPI(PI_Vc_Ki/FSW, 2*PI_Vc_Ki/FSW, 0.5, -1, 0.99, 1.0, 0.2);
    PI_Io = initPI(PI_Io_Ki/FSW, 2*PI_Io_Ki/FSW, 0.5, -1, 0.00, 0.25, -0.25);
}

void setControllerVclampRef(float x) { refDeltaVclamp = x; }
void adjControllerVclampRef(float x) { refDeltaVclamp += x; }

void setControllerIoutRef(float x) { refIo = x; }
void adjControllerIoutRef(float x) { refIo += x; }

// adcA1ISR - ADC A Interrupt 1 ISR
// Runs with every switching cycle, runs the control law for the converter
__interrupt void adcA1ISR(void)
{
    struct ADCResult meas = scaleADCs();

    static unsigned int ncycles = CYCLE_LIMIT;

    if (ncycles-- == 0) // trip
    {
        disablePWM();
        *tripFeedback = TripOC;//isInSOA(meas, StateOn);
        ncycles = CYCLE_LIMIT;
    }
    else // normal operation
    {
        float errVclamp = meas.Vin*Ninv + refDeltaVclamp - meas.Vclamp;
        float errIout = refIo - meas.Iout;

        float d = updatePI(&PI_Vc, -errVclamp);
        float p = updatePI(&PI_Io, errIout);

        updateModulator(d, p);
    }

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Clear the interrupt flag

    // Check if overflow has occurred
    if(1 == AdcaRegs.ADCINTOVF.bit.ADCINT1)
    {
        AdcaRegs.ADCINTOVFCLR.bit.ADCINT1 = 1; //clear INT1 overflow flag
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    }

    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Acknowledge the interrupt
}
