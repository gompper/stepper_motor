#include <stdio.h>
#include <LPC23xx.H>                    /* LPC23xx definitions                */
#include <stdbool.h>
#include <math.h>
#include "linear_acceleration.h"
#include "motor_helpers.h"
#include "motor_pwm.h"

/***********************************
* PINS
************************************/

#define RESET BIT0	// P3.24
#define ENABLE BIT7	// P3.23
#define SLEEP BIT1	// P3.25
#define STEP BIT2 	// P3.26	

#define MS1 BIT2	// P4.2
#define MS2 BIT1	// P4.1
#define MS3 BIT0	// P4.0
#define DIR	BIT4  // P4.4


/***********************************
* FUNCTION DECLARATIONS
************************************/

void Timer0_Init(void);
void Timer1_Init(void);
void toggle_dir(void);
void turn_left(void);
void turn_right(void);

void Interrupt_Init(void);
void GPIO_Init(void);
void GPIO_Set(void);
int drive(int a,int v_start, int v_soll, int dist, int dir);
void turn(int);

/***********************************
* IRQ
************************************/

__irq void T0_IRQHandler (void);
__irq void PWM_ISR(void);


/***********************************
* GLOBAL VARIABLES
************************************/

	bool toggle;
	int TOGGLE_STEP = STEP;
	double sinewave;
	int x=0;
	int STEP_CNT=0;
	
	int step_counter = 0;
	int direction = RIGHT;
	
/* Global Variables for linear acceleration */
	int stepcnt = 0;
	double v = 0.0;
	double cycles = 0.0;
	double timerMatch = 0.0;
	double previousDelay = 0.0;

/***********************************
* TYPEDEFS
************************************/

	typedef enum{
		Drive_Fast,
		Drive_Slow,
		Wait
	}SystemState;
	
	typedef enum{
		Accelerate,
		Drive,
		Decelerate
	}DriveState;
		
/***********************************
* MAIN
************************************/

int main(void){

	Timer0_Init();
	GPIO_Init();
	Interrupt_Init();
	
	while(1){
			if(stepcnt >= DISTANCE)
					T0TCR  = 0; // stop timer
	}
	return 0;

}

void GPIO_Init(void){
	/* Pin Function */
	/* 00 = GPIO */
		PINSEL3 &= 0x00000000;		// P1.16..31	
		PINSEL6 &= 0x00005555; 		// P3.00..15
		PINSEL7 &= 0x00000000;		// P3.16..31 <--
		PINSEL8 &= 0x55555400;		// P4.00..15 <--
		PINSEL9 &= 0x50090000;		// P4.16..31
		//PINSEL10 &= 0x00000008;		// ETM Interface
		PINSEL10 &= 0x00000000;		// ETM Interface disabled
	
	/* Pull-Up/-Down Resistors */
	/* 	00 = Pull-Up, 10 = no Resistor, 11 = Pull-Down  */
		PINMODE3 = 0x00000000;		// P1.16..31
		PINMODE7 = 0x00000000; 		// P3.16..31
		PINMODE8 = 0x00000000;		// P4.00..15
	
	/* Input / Output */
		FIO4DIR0 |= DIR + MS1 + MS2 + MS3; 	// P4.00..07
		FIO3DIR2 |= ENABLE;									// P3.16..23
		FIO3DIR3 |= SLEEP + RESET + STEP;		// P3.24..31
	
	/* Pins maskieren */
		FIO3MASK2 	= 0x00;	// P3.16..23
		FIO3MASK3 	= 0x00;	// P3.24..31
		FIO4MASK0 	= 0x00;	// P4.00..07
		
		GPIO_Set();
}

void GPIO_Set(void){
		/* Pins setzen */
		FIO3CLR2 |= ENABLE;			/* Enable ist low Active */
		FIO3SET3 |= SLEEP;			/* Sleep ist low Active */
		FIO3SET3 |= RESET;			/* Reset ist low Active */
		
	/* Schrittweite festlegen */
		//FIO4SET0 |= MS1 + MS2 + MS3; // Sixteenth Step 
		FIO4CLR0 |= MS1 + MS2 + MS3; // Full Step
		
	/* Richtung festlegen */
		FIO4SET0 |= DIR; // rotate right
		//FIO4CLR0 = DIR; // rotate left
}

void LED_Init(void) { 
  PINSEL10 = 0;                         /* Disable ETM interface, enable LEDs */
  FIO2DIR  = 0x000000FF;                /* P2.0..7 defined as Outputs         */
  FIO2MASK = 0x00000000;								/* keine Ports maskiert */
  }

/**************************
* 	Timer 0
**************************/
	
void Timer0_Init(void){
	/* timer match register */
	T0MR0         = 0;		                       /* 1msec = 12000-1 at 12.0 MHz */
  T0MCR         = 3;                           /* Interrupt and Reset on MR0  */
  T0TCR         = 1;                           /* Timer0 Enable               */
  /* Timer 0 IRQ */
	VICVectAddr4  = (unsigned long)T0_IRQHandler;/* Set Interrupt Vector        */
  VICVectCntl4  = 15;                          /* use it for Timer0 Interrupt */
  VICIntEnable  = (1  << 4);                   /* Enable Timer0 Interrupt     */
}

/**************************
* 	Timer 1
**************************/
	
void Timer1_Init(void){
	/* timer match register */
	T1MR0         = 0;		                       /* 1msec = 12000-1 at 12.0 MHz */
  T1MCR         = 3;                           /* Interrupt and Reset on MR0  */
  T1TCR         = 1;                           /* Timer0 Enable               */
  /* Timer 1 IRQ */
	VICVectAddr  = (unsigned long)T0_IRQHandler;/* Set Interrupt Vector        */
  VICVectCntl5  = 14;                          /* use it for Timer1 Interrupt */
  VICIntEnable  = (1  << 5);                   /* Enable Timer0 Interrupt     */
}


/***************************
*	IRQ
****************************/

/*
*	TIMER 0 ISR
* Runs when Timer0 Match Register Value ist met.
* At this point we need to send a pulse to the 
* motor and find a new timer value.
*/
__irq void T0_IRQHandler (void) {
	
	cycles = cntVal(previousDelay, stepcnt);
	timerMatch = stepPulse(cycles);
	//v = velocity(cycles); // I dont think we get to use this
	previousDelay = cycles;
	stepcnt++;
	T0MR0				= timerMatch;
	T1MR0				= (int)timerMatch/2;
	T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
	
}

/*
*	TIMER 1 ISR
* Runs when Timer1 Match Register Value ist met.
* Timer 1 should always interrupt at half the period
* of Timer0
*/
__irq void T1_IRQHandler (void) {
	
	stepcnt++;
	/* TODO : HERE HERE HERE HERE HERE HERE HERE HERE HERE  */
	/* TODO : HERE HERE HERE HERE HERE HERE HERE HERE HERE  */
	/* TODO : HERE HERE HERE HERE HERE HERE HERE HERE HERE  */
	/* TODO : HERE HERE HERE HERE HERE HERE HERE HERE HERE  */
	/* TODO : HERE HERE HERE HERE HERE HERE HERE HERE HERE  */

	/* TODO : TOGGLE SOME PIN? WHICH ONE CONTROLS MOTOR? */
	
	
	T1IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
	
}

/***************************
* Motor Controls
****************************/

void toggle_dir(){
	int reg_dir = FIO4PIN0 & DIR; 
	if(reg_dir > 0){
		FIO4CLR0 = DIR;
	}
	else{
		FIO4SET0 = DIR;
	}
}

void turn_right(){
	FIO4SET0 = DIR;
	direction = RIGHT;
}

void turn_left(){
	FIO4CLR0 = DIR;
	direction = LEFT;
}

int drive(int a, int v_start, int v_soll, int dist, int dir){
	DriveState dstate = Accelerate;
	
	int start = step_counter;
	int s_beschl = (pow(v_soll,2) - pow(v_start,2)) / (2 * a); /* Beschleunigungsweg */
	int s_drive = dist - s_beschl;
	
	turn(dir);
	
	switch(dstate){
		case Accelerate:
			PWM_Set(v_start);
			while(step_counter - start < s_beschl){
				if(step_counter % a){
					PWM_Set(v_start - a);
				}
			}
			dstate = Drive;
		case Drive:
			while(step_counter - start < s_drive){
				
			}
			dstate = Decelerate;			
		case Decelerate:
			
		default: break;
	}
	
	PWM_Set(20000); /* 2000000 ~1.25 s --> ~1.777 MHz*/
	//while( (step_counter - start) < dist ){	}
		
	PWM1TCR = 0x02; // Reset and disable counter for PWM
	return 0;
}

void turn(int dir){
	switch(dir){
		case RIGHT:
			turn_right();
			break;
		case LEFT:
			turn_left();
			break;
		default:
			break;
	}
}
