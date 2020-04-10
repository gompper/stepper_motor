#include <stdio.h>
#include <LPC23xx.h>                    /* LPC23xx definitions                */
//#include "../../src/linear_acceleration.h"
#include "linear_acceleration.h"

/***********************************
* PINS
************************************/

#define RESET 	BIT0	// P3.24
#define ENABLE 	BIT7	// P3.23
#define SLEEP 	BIT1	// P3.25
#define STEP 		BIT2 	// P3.26	

#define MS1 BIT2	// P4.2
#define MS2 BIT1	// P4.1
#define MS3 BIT0	// P4.0
#define DIR	BIT4    // P4.4

/***********************************
* Timer
************************************/

#define INT_TMR0	04
#define INT_TMR1	05
#define INT_TMR2	26
 
 /***********************************
* Helpers
************************************/

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define RIGHT 1
#define LEFT	0
/***********************************
* GLOBAL VARIABLES
************************************/

/* Global Variables for linear acceleration */
	double v = 0.0;
	double cycles = 0.0;
	double timerMatch = 0.0;
	double previousDelay = 2570000.0;
	int	cycles_int = 0;			

	int stepcnt = 1;

/***********************************
* FUNCTION DECLARATION
************************************/

void __irq T0ISR(void);
void __irq T1ISR(void);
void __irq T2ISR(void);

static void InitTimer0(void);
static void InitTimer1(void);
static void InitTimer2(void);

void MotorControlPinConfiguration(void);


 /***********************************
* MAIN
************************************/
int main (){

	/* set leds */
	/* function 0, clear last 16 bit*/
	PINSEL4 &= ~0x0000FFFF;
	/* Important: always use Fast IO */
	FIO2DIR  = 0x000000FF;                /* P2.0..7 defined as Outputs         */
  FIO2MASK = 0x00000000;
	
	/* config timer */
	InitTimer0();
	InitTimer1();
//	InitTimer2();
	
	//stepcnt = 1;
	
	/* config motor */
	MotorControlPinConfiguration();
	
	while(1){
		// do nothing
	}
}


 /***********************************
* FUNCTION DEFINITIONS
************************************/

/* ISR ROUTINES */

void __irq T0ISR() {
	

	VICVectAddr4 = (unsigned long)T0ISR;
	VICVectPriority4 = 0;
	VICIntEnable |= 1 << INT_TMR0;
 
	cycles = cntVal(previousDelay, stepcnt);
	cycles_int = (int)cycles;
	//timerMatch = stepPulse(cycles);
	previousDelay = cycles;

	//T0MR0				= (int)cycles;		
	T0MR0 = (int)cycles;
	T1MR0	= (int)cycles/2;
	
	FIO3SET3 = STEP; 			/* Step HIGH */
	FIO2SET |= BIT1;
	
	T0IR = 0x01;	/* Clear interrupt flag */
	VICVectAddr = 0;	/* Acknowledge Interrupt */
}

void __irq T1ISR() {
	
	
	stepcnt++;
	
	FIO3CLR3 = STEP;			/* Step LOW */
	FIO2CLR |= BIT1;

	T1IR = 0x01;
	VICVectAddr = 0;
}
 
void __irq T2ISR() {
	T2IR = 0x01;
	
	T0TCR = 0x00; 					/* stop timer 0 */
	T1TCR = 0x00; 					/* stop timer 1 */
	FIO3SET2 	|= ENABLE;		/* Diable Motor (Enable low Active) */
	FIO2SET		|= BIT3;
	
	VICVectAddr = 0;
}

/* TIMER INITIALISATIONS */
static void InitTimer0(void){
	T0TCR = 0x02;
	//T0MR0 = 0x100000;
	T0MR0 = (int)previousDelay;
	T0MCR = 0x03;
	
	VICVectAddr4 = (unsigned long)T0ISR;
	VICVectPriority4 = 0;
	VICIntEnable |= 1 << INT_TMR0;
 
	T0TCR = 0x01;
}

static void InitTimer1(void){
	T1TCR = 0x02;
	//T1MR0 = 0x2000001;
	T1MCR = 0x03;
	
	VICVectAddr5 = (unsigned long)T1ISR;
	VICVectPriority4 = 1;
	VICIntEnable |= 1 << INT_TMR1;
 
	T1TCR = 0x01;
}

static void InitTimer2(void){
	PCONP |= 0x1 << 22;			/* power timer 2 */

	T2TCR = 0x02;
	T2MR0 = 0x100000;
	T2MCR = 0x03;
	
	VICVectAddr26 = (unsigned long)T2ISR;
	VICVectPriority4 = 0;
	VICIntEnable |= 1 << INT_TMR2;
 
	T2TCR = 0x01;
}

/* MOTOR CONFIGURATIONS */

void MotorControlPinConfiguration(void){
	//printf("Gpio init ...\n"); /* debug mode - view - serial windows - Debug (printf) viewer*/
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
		
		/* Pins setzen */
		FIO3CLR2 |= ENABLE;			/* Enable ist low Active */
		FIO3SET3 |= SLEEP;			/* Sleep ist low Active */
		FIO3SET3 |= RESET;			/* Reset ist low Active */
		
		/* (DEBUG) set LED */
		FIO2CLR |= BIT3;			/* Enable ist low Active */
		FIO2SET |= BIT2;			/* Sleep ist low Active */
		FIO2SET |= BIT0;			/* Reset ist low Active */
		
		
	/* Schrittweite festlegen */
		//FIO4SET0 |= MS1 + MS2 + MS3; // Sixteenth Step 
		FIO4CLR0 |= MS1 + MS2 + MS3; // Full Step
		/* (DEBUG) set LED */
		FIO2CLR |= BIT7 | BIT6 | BIT5;			/* Enable ist low Active */
		
		
	/* Richtung festlegen */
		FIO4SET0 |= DIR; // rotate right
		//FIO4CLR0 = DIR; // rotate left
		/* (DEBUG) set LED */
		FIO2SET |= BIT4;			/* Enable ist low Active */

}
