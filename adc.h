
#ifndef ADC_H_
#define ADC_H_

struct ADCScaling {
    float gain;
    int offset;
};

struct ADCResult {
    float ILhi;
    float ILlo;
};

void initADC(void);
void initADCSOC(void);
float scaleADC(unsigned int, struct ADCScaling);
struct ADCResult scaleADCs(void);
struct ADCResult readADC(void);
void calibrateADC(void);

#endif /* ADC_H_ */
