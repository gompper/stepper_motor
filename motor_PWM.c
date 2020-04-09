#include "motor_pwm.h"


/* PWM Initialization */
void PWM_Init(){
	PWM1TCR = 0x02; // Reset and disable counter for PWM
	
	PWM1PR	= 0x1D; // Prescale value for 1usec, Pclk=30MHz
	PWM1MR0 = 200000;
	PWM1MR3 = 1000;
	//PWM1MCR = 0x2; PWM ohne interrupt
	PWM1MCR = 0x3; // MR0 Interrupt und MR0 Reset 
	PWM1LER = 0x9;
	PWM1PCR = 0x800; // PWM3 output enabled
	PCONP = BIT1 + BIT6 ; 	// PCPWM1 Power/Clock Control Bit
	PCLKSEL0 = 0x00001000; 	// Set PWM Clock to CCLK/1
	PINSEL7 |= 0x00300000;	// P3.16..31 <-- P3.26 PWM Function PWM1.3 --> PWM Channel 3 --> Single Edge PWM: Set by Match 0, Reset by Match 3
	//PINMODE = ; // Evtl. Pull-Up / Pull-Down Widerstand an den PWM Ausgang
	// MR0 mind. 200, MR1 = 1, MR2 = 100 bei Prozessorgeschw. von 72 MHz; ca. 72 Takte ~ 1 us
	//PWM1CTCR = 0; kp ob das gebraucht wird
	
	//PWM1TCR = 0x9; // Counter Enable + PWM Enable
}

void PWM_Set(int T){
	PWM1TCR = 0x02; // Reset and disable counter for PWM
	PWM1MR0 = T; 		// Periodendauer
	PWM1TCR = 0x9; 	// Counter Enable + PWM Enable
}
