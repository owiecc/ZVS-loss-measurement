
#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "adc.h"

enum converter_states {StateInitDSP,StateStandby,StateStartup,StateOn,StateShutdown,StateTrip};

void initPIConttrollers(void);

void setControllerILRef(float);
void adjControllerILRef(float);

__interrupt void adcA1ISR(void);

#endif /* CONTROLLER_H_ */
