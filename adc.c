
#include "adc.h"
#include "f28x_project.h"

#define Imeas_nom 25.0f
#define Imeas_sens 0.625f
#define Vref_ext 3.3f

const int N_AVG_IOUT_CAL = 128;

struct ADCScaling ADCcal = { .gain = Vref_ext/(4046)*Imeas_nom/Imeas_sens, .offset = 4096/2 }; // LEM LKSR 6-NP; calibrated parameters

void calibrateADC(void)
{
    DELAY_US(1000000); // 1.0s

    unsigned long int IoutOffset = 0;
    for (int i = 0; i < N_AVG_IOUT_CAL; i++)
    {
        DELAY_US(320000/N_AVG_IOUT_CAL); // total process should take multiple of 20ms (works in 50/60Hz grid)
        readADC();
        IoutOffset += AdcaResultRegs.ADCRESULT3; // Iout ADC value
    }

    ADCcal.offset = (int)(IoutOffset/N_AVG_IOUT_CAL);
}

struct ADCResult readADC(void)
{
    // Do not trigger ADCINT1 if PWMs are disabled; force trigger the ADC conversion
    if (EPwm1Regs.TBCTL.bit.CTRMODE == TB_FREEZE)
    {
        EALLOW;
        AdcaRegs.ADCINTSEL1N2.bit.INT1E = 0;   // Disable ADCINT1

        AdcaRegs.ADCSOCFRC1.all = 0x000F; // Force SOC0 to SOC3
        DELAY_US(1);

        AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable ADCINT1
        AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure ADCINT1 flag is cleared
        EDIS;
    }

    struct ADCResult adcOut;
    adcOut = scaleADCs();

    return adcOut;
}

inline struct ADCResult scaleADCs(void)
{
    struct ADCResult adcOut;
    adcOut.ILlo = scaleADC(AdcaResultRegs.ADCRESULT0, ADCcal);
    adcOut.ILhi = scaleADC(AdcaResultRegs.ADCRESULT1, ADCcal);

    return adcOut;
}

inline float scaleADC(unsigned int ADCResult, struct ADCScaling coeffADC)
{
    return ((int)ADCResult - coeffADC.offset)*coeffADC.gain;
}

// initADC - Function to configure and power up ADCA.
void initADC(void)
{
    SetVREF(ADC_ADCA, ADC_EXTERNAL, ADC_VREF3P3); // Setup external VREF

    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; // Set ADCCLK divider to /4
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; // Power up the ADC and then delay for 1 ms
    EDIS;

    DELAY_US(1000);
}

// initADCSOC - Function to configure ADCA's SOCs to be triggered by ePWM3
void initADCSOC(void)
{
    // Select the channels to convert and the end of conversion flag
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;     // SOC0 will convert pin A0 = iL
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 0x09;   // Trigger on ePWM3 SOCA

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 0;     // SOC1 will convert pin A0 = iL
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 24;     // Sample window is 25 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 0x0A;   // Trigger on ePWM3 SOCB

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 1; // End of SOC1 will set ADCINT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   // Enable ADCINT1
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; // Make sure ADCINT1 flag is cleared
    EDIS;
}
