
#include "init.h"
#include "f28x_project.h"
#include "f28003x_device.h"
#include "controller.h"
#include "pwm.h"
#include "adc.h"

void initDSP(void)
{
    InitSysCtrl(); // Init device clock and peripherals
    InitGpio(); // Init GPIO

    EALLOW;
    // PWM: GPIO0-GPIO3
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1; // ePWM
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1; // ePWM
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1; // ePWM
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1; // ePWM

    // Gate driver status
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 0;  // READY input
    GpioCtrlRegs.GPADIR.bit.GPIO5 = 0;  // DESAT input
    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;  // RESET output

    // LEDs
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0;  // LD0
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 0;  // LD1
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO13 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;  // LD2
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO14 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;  // LD3
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO15 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO20 = 0;  // LD4
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO20 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0;  // LD5
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO22 = 0;  // LD6
    GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO22 = 1;  // output

    GpioCtrlRegs.GPAPUD.bit.GPIO23 = 0;  // LD7
    GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0; // GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO23 = 1;  // output

    // Button inputs
    GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = 1; // Qualification period = SYSCLKOUT/2
    GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;     // input
    GpioCtrlRegs.GPAPUD.bit.GPIO8 = 0;     // enable pull-up
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 2;   // 6 samples
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 0;     // input
    GpioCtrlRegs.GPAPUD.bit.GPIO9 = 0;     // enable pull-up
    GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 2;   // 6 samples
    GpioCtrlRegs.GPADIR.bit.GPIO10 = 0;    // input
    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;    // enable pull-up
    GpioCtrlRegs.GPAQSEL1.bit.GPIO10 = 2;  // 6 samples
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 0;    // input
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;    // enable pull-up
    GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = 2;  // 6 samples
    EDIS;

    DINT; // Disable global interrupt INTM

    InitPieCtrl(); // Init PIE control registers. All PIE interrupts disabled. All flags cleared.

    IER = 0x0000; // Disable individual CPU interrupts
    IFR = 0x0000; // Clear individual CPU interrupt flags

    InitPieVectTable(); // Init the PIE vector table with pointers to ISRs

    // Map ISR functions
    EALLOW;
    PieVectTable.ADCA1_INT = &adcA1ISR; // Function for ADCA interrupt 1
    EDIS;

    initADC(); // Configure the ADC and power it up
    initEPWM(); // Configure the ePWM
    initADCSOC(); // Setup the ADC for ePWM triggered conversions on channel 1

    // Enable global Interrupts and higher priority real-time debug events:
    IER |= M_INT1;  // Enable group 1 interrupts

    EINT;           // Enable global interrupt INTM
    ERTM;           // Enable global real time interrupt DBGM


    PieCtrlRegs.PIEIER1.bit.INTx1 = 1; // Enable PIE interrupt

    // Sync ePWM
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}
