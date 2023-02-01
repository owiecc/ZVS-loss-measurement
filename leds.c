
#include "leds.h"
#include "f28x_project.h"

void displayValue(int n)
{
    unsigned long y = 10*abs(n); // 10 is scaling factor for current control (+/-0.1A)
    unsigned long mask_ld3_0 = 0b00001111;
    unsigned long mask_ld7_4 = 0b11110000;

    GpioDataRegs.GPACLEAR.all = (mask_ld3_0 << 12) | (mask_ld7_4 << 16);
    GpioDataRegs.GPASET.all = ((y & mask_ld3_0) << 12) | ((y & mask_ld7_4) << 16);
}
