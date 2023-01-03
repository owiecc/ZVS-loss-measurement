
#include "f28x_project.h"
#include "controller.h"
#include "init.h"
#include "pwm.h"
#include "adc.h"
#include "leds.h"
#include "input.h"

// Globals
static enum converter_states converter_state = StateInitDSP;

// Functions
void adjust_reference(enum button);

// Main
void main(void)
{
    while(1)
    {
        enum button button = button_pressed();

        switch (converter_state)
        {
        case StateInitDSP:
        {
            initDSP(); // Configure GPIO, ADC, PWM
            initPIConttrollers(); // Initialize PI controllers
            setControllerILRef(0.0);
            calibrateADC();
            converter_state = StateStandby;
            break;
        }
        case StateStandby:
        {
            adjust_reference(button);
            converter_state = (button == BtnOn) ? StateStartup : StateStandby;
            break;
        }
        case StateStartup:
        {
            initPIConttrollers();
            enablePWM();
            converter_state = StateOn;
            break;
        }
        case StateOn:
        {
            adjust_reference(button);
            converter_state = (button == BtnOff) ? StateShutdown : converter_state;
            break;
        }
        case StateShutdown:
        {
            relayOff();
            disablePWM();
            converter_state = StateStandby;
            break;
        }
        case StateTrip:
        {
            // Clear trip condition only if trip clear button is pressed and the converter is within SOA
            if (button == BtnOff)
            {
                converter_state = StateStandby;
            }
            break;
        }
        default:
        {
            break;
        }
        }
        DELAY_US(50000); // 50ms
    }
}

void adjust_reference(enum button button_pressed)
{
    static enum parameter param = Iout;

    // react to button only on press, not on hold
    static enum button button_prev = BtnNothing;
    int isPressed = (button_pressed != button_prev) && button_pressed != BtnNothing;
    enum button button = isPressed ? button_pressed : BtnNothing;

    // indicate adjustment mode; blink on button press
    if (param == Vclamp && !isPressed) { ledOn(LEDVclampAdjust); } else { ledOff(LEDVclampAdjust); }
    if (param == Iout && !isPressed) { ledOn(LEDIoutAdjust); } else { ledOff(LEDIoutAdjust); }

    if (param == Iout && button == BtnIncr) { adjControllerILRef(+1.0); }
    if (param == Iout && button == BtnDecr) { adjControllerILRef(-1.0); }

    button_prev = button_pressed;
    // TODO display reference parameter and Iout, Vclamp reference values values

    return;
}
