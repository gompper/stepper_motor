#ifndef MOTO_PWM_H
#define MOTO_PWM_H
#include <LPC23xx.H>                    /* LPC23xx definitions                */
#include "motor_helpers.h"

void PWM_Init(void);
void PWM_Set(int);

#endif
