
#ifndef INPUT_H_
#define INPUT_H_

typedef enum parameter {Iout, Vclamp} adj_param;
typedef enum button {BtnNothing, BtnOn, BtnOff, BtnDecr, BtnIncr} button;

button button_pressed(void);

#endif /* INPUT_H_ */
