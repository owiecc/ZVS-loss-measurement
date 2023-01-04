
#ifndef PWM_H_
#define PWM_H_

// Defines
#define CPUFREQ 120000000
#define FSW 20000

#define PWM_PRD CPUFREQ/FSW // Switching period fCPU/fSW = 120M/20k = 6000
#define PWM_PRD_HALF CPUFREQ/FSW/2
#define PWM_PRD_QUARTER CPUFREQ/FSW/4

void updateEPWM(float, float);
void initEPWM(void);
void disablePWM(void);
void enablePWM(void);

#endif /* PWM_H_ */
