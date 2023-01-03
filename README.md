# ZVS loss measurement controller

Controller for a full bridge converter topology. One of the transistors in the converter is the DUT. Converter is operated so that the DUT is turned-on with full ZVS and turned-on at controlled current level. Based on a TMDSCNCD280039C control card. 

## Key waveforms

There are two operating modes for the measurement: soft and fard switching. Three DUT swithing events are of interest: 

- ● turn-on
- ○ ZVS turn-on
- × turn-off

### Hard switching

In hard switching mode the turn-on and turn-off are done at the same current level. In order to achieve that an inductor with high inductance value is used. 
                                                                     
    ───●─────×─────────────────────────── iL
    
    ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ 0       
       |◀───▶|                            
         DUT                              
                                          
    ● turn-on                         
    × turn-off          

### Soft switching

In soft switching mode the turn-off is done at the same current level as in the soft switching mode. The turn-on is done at a current level that guarantees a ZVS transition for DUT. In this mode an inductor with low inductance value is used. 
                                                              
             ×──────                 ──── iL
            ╱       ╲               ╱     
    ─ ─ ─ ─╱─ ─ ─ ─ ─╲─ ─ ─ ─ ─ ─ ─╱─ ─ ─ 0
          ╱           ╲           ╱       
         ╱             ╲         ╱        
        ╱               ╲       ╱         
    ───○                 ───────          
       |◀───▶|                            
         DUT                              
                                          
    ○ ZVS turn-on                         
    × turn-off                                       

## Pinout

### PWM

| Card pin | Function | Transistor | PLECS |
|---------:|:---------|:-----------|:------| 
|       49 | PWM1A    | AH         | Q1    |
|       51 | PWM1B    | AL         | Q2    |
|       53 | PWM2B    | BH         | Q3    |
|       55 | PWM2B    | BL         | Q4    |

### Gate drivers status

| Card pin | GPIO    | Net   | I/O | Function                |
|---------:|:--------|:------|:----|:------------------------|
|       50 | GPIO-04 | READY | I   | Gate driver             |
|       52 | GPIO-05 | FAULT | I   | Desat event             |
|       54 | GPIO-06 | RESET | O   | Reset after desat event |

### ADC

| Card pin | ADC channel | Function |
|---------:|:------------|:---------|
|        9 | A0          | iL(A→B)  |

### Control buttons

| Card pin | GPIO    | Net  | Function           |
|---------:|:--------|:-----|:-------------------|
|       57 | GPIO-08 | BTN0 | Reference increase |
|       59 | GPIO-09 | BTN1 | Reference decrease |
|       61 | GPIO-10 | BTN2 | Off/Reference zero |
|       63 | GPIO-11 | BTN3 | On                 |

### Status LEDs

| Card pin | GPIO    | LED  | Value |
|---------:|:--------|:-----|:------|
|       58 | GPIO-12 | LD0  |     1 |
|       60 | GPIO-13 | LD1  |     2 |
|       62 | GPIO-14 | LD2  |     4 |
|       64 | GPIO-15 | LD3  |     8 |
|       68 | GPIO-20 | LD4  |    16 |
|       70 | GPIO-21 | LD5  |    32 |
|       72 | GPIO-22 | LD6  |    64 |
|       74 | GPIO-23 | LD7  |   128 |

## ADC and sampling

All hardware ADC channels are bandwidth limited. The exact frequency needs to be investigated. 

Sampling instance needs to be investigated. Four channels need to be sampled: Vin, Vout, Vclamp and Iout. 

Voltage inputs are isolated with an ACPL-C87B isolation amplifier. Nominal voltage for this amplifier is 2V. Voltage dividers are tuned for nominal voltage magnitudes. Maximum input voltage for the amplifier is 2.46. 

### Input voltage

Voltage divider with an isolation amplifier. R1 = 6×665k. R2 = 10k. Attenuation = 400. 800V ➞ 2.0V. 984V ➞ 2.46V (max).

### Output voltage

Voltage divider with an isolation amplifier. R1 = 6×665k. R2 = 10k. Attenuation = 400. 200V ➞ 0.5V. 920V ➞ 2.3V. 984V ➞ 2.46V (max).

### Clamp current

Current sensor with an isolation amplifier. R1 = 9×665k + 2×10k. R2 = 10k. Attenuation = 601.5. 1200V ➞ 1.995V. 1480V ➞ 2.46V (max). 

### Output current

LES XX-NP current transducer. 0A = 2.5V±0.625·I/Inom. There is a voltage divider on the output of the current transducer. R1 = 3.3k, R2 = 6.8k. Attenuation = 0.6733. ADC should see 1.68±0.4208·I/Inom volts. 

## Protection

Undervoltage and overvoltage setpoints still need to be confirmed in simulations. Output current limits need to be verified experimentally. Ideally there should also be a protection if the controllers are not steady-state within some time period. 

### Input voltage

Nominal voltage Uin = 800V. 

Overvoltage protection set at 105%·Uin = 840V. 

Undervoltage protection set at 90%·Uin = 720V. 

### Output voltage

Nominal voltage range Uout = 200-920V. 

Overvoltage protection set at Uin−10V = 190V. 

Undervoltage protection set at Uout+10V = 930V. 

### Clamp voltage

Nominal volatge Uclamp = Uin·n2/n1 = 1200V.

Overvoltage protection set at 105%·Uin = 1260V. This needs to be tested and controller needs to be tuned not to have an overshoot. 

### Output current

TBD, depends on the hardware limits. 
