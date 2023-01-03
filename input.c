
#include "input.h"
#include "f28x_project.h"

button button_pressed(void)
{
    if (GpioDataRegs.GPADAT.bit.GPIO11 == 0) { return BtnOn;  }; // On
    if (GpioDataRegs.GPADAT.bit.GPIO10 == 0) { return BtnOff; }; // Off
    if (GpioDataRegs.GPADAT.bit.GPIO9 == 0) { return BtnDecr; }; // Reference decrease
    if (GpioDataRegs.GPADAT.bit.GPIO8 == 0) { return BtnIncr; }; // Reference increase
    return BtnNothing;
}
