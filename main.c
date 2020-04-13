/***********************************************************************************
* ARM Programmierung WiSe 19/20 
* Project Stepper Motor
* 
* Date					version			User			comment
************************************************************************************
* 10-04-20				2.0				Urs				Merge and fixed Timers
************************************************************************************
* 12-04-20 				2.1				Gina			Insert header to keep track of latest main 	
************************************************************************************
*
*****************************/
#include <stdio.h>
#include <math.h>
#include <LPC23xx.h>                    /* LPC23xx definitions                */
#include "linear_acceleration.h"

/***********************************
* PINS
************************************/

#define RESET 	BIT0	// P3.24
#define ENABLE 	BIT7	// P3.23
#define SLEEP 	BIT1	// P3.25
#define STEP 		BIT2 	// P3.26	

/* Microstep Selection Bits */
#define MS1 BIT2	// P4.2
#define MS2 BIT1	// P4.1
#define MS3 BIT0	// P4.0

/* Direction */
#define DIR	BIT4	// P4.4

/***********************************
* Timer
************************************/

#define INT_TMR0	04
#define INT_TMR1	05
 
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
	static double cycles = 0.0;
	
	static int stepcnt = 0;			// Beschleungigungsschritt
	static int stepcnt_tot = 0;	// Relative Position zum letzten Wendepunkt
	static int position = 0;		// Absolute Position
	
	static int vmax_reached = 0;	// Relative Position bei der die maximale Geschw. erreicht wurde.
	
	static int acc = 1;
	static double peakCycles, breakCycles, total;
	int breakSteps;
	long totalaccCycles = 2565239; 	//first delay
	int accSteps = 1;					/* how many steps we need/have for acceleration and breaking */

/***********************************
* FUNCTION DECLARATION
************************************/

void __irq T0ISR(void);
void __irq T1ISR(void);

static void InitTimer0(void);
static void InitTimer1(void);

void MotorControlPinConfiguration(void);

void toggle_dir(void);
void turn(int dir);
void turn_left(void);
void turn_right(void);


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
	
	/* config motor */
	MotorControlPinConfiguration();

	while(1){
		if(stepcnt_tot < (DISTANCE/2)){	// 1. Hälfte der Strecke
			if (cycles < MAXSPEED_DELAY && cycles != 0){	// Abfrage ob die max. Geschw. erreicht wurde
				acc = 0;	// Wurde die max. Geschw. erreicht wird mit konstanter Geschw. weitergefahren
			}
			else{
				acc = 1;	// Beschleunigen bis max. Geschw. oder 2. Hälfte der Strecke erreicht
			}
		}else if(stepcnt_tot < DISTANCE){	// 2. Hälfte der Strecke
			if(acc == 1){ // Abfrage ob noch beschleunigt wird
				acc = -1;	// Negative Beschleunigung bis zum Stillstand
			}else{
				if(DISTANCE - stepcnt_tot < vmax_reached){ // Abfrage ob schon abgebremst werden muss
					acc = -1; // Negative Beschleunigung bis zum Stillstand
				}else{
					acc = 0;	// Weiterfahren mit konstanter Geschw.
				}
			}
		}else if(stepcnt_tot == DISTANCE){ // Abfrage ob das Ziel erreicht wurde
			vmax_reached = 0;	
		}
	}	
}


 /***********************************
* FUNCTION DEFINITIONS
************************************/

/* ISR ROUTINES */

void __irq T0ISR() {
	T0TCR = 0x02; // Counter Reset
	
	if(stepcnt_tot < DISTANCE){
		stepcnt_tot++;
	}else{
		stepcnt_tot = 1;
	}
	
	/* Beschleunigungsschritt updaten */
	if (acc == 1) { stepcnt++; }
	if (acc == -1 && stepcnt != 0) { stepcnt --;}
	
	/* Relative Position speichern, falls maximale Geschwindigkeit erreicht wurde. */
	if (acc == 0 && vmax_reached == 0) {vmax_reached = stepcnt_tot;} 
	
	if ((FIO4PIN0 & DIR) > 0){position++;} else {position--;} // Aktuelle Richtung auslesen und Position updaten
	
	cycles = cntVal(T0MR0, stepcnt, acc);	// Neues Delay berechnen
		
	cycles = (stepcnt==0)?FIRSTDELAY:cycles; // Erstes Delay ist fix
	
	/* Timer Match Register updaten */
	T0MR0 = (int)cycles;
	T1MR0	= (int)cycles/2;
	
	/* Einen Schritt gehen */
	FIO3SET3 = STEP; 			/* Step HIGH */
	FIO2SET |= BIT1;
	
	/* Richtung umkehren bei erreichen des Ziels */
	if(stepcnt_tot == DISTANCE) toggle_dir();
	
	T0IR = 0x01;			/* Clear interrupt flag */
	VICVectAddr = 0;	/* Acknowledge Interrupt */
	
	T0TCR = 0x01;	// enable Timer0;
	T1TCR = 0x01; // enable Timer1;
}

void __irq T1ISR() {
	T1TCR = 0x02; // Counter Reset
	
	FIO3CLR3 = STEP;			/* Step LOW */
	FIO2CLR |= BIT1;

	T1IR = 0x01;			/* Clear interrupt flag */
	VICVectAddr = 0;	/* Acknowledge Interrupt */
}

/* TIMER INITIALIZATIONS */
static void InitTimer0(void){
	T0TCR = 0x02;
	T0MR0 = FIRSTDELAY;
	
	T0MCR = 0x03; // interrupt on MR0; reset on MR0; 
	
	VICVectAddr4 = (unsigned long)T0ISR;
	VICVectPriority4 = 0;
	VICIntEnable |= 1 << INT_TMR0;
 
	T0TCR = 0x01;
}

static void InitTimer1(void){
	T1TCR = 0x02;
	T1MR0 = T0MR0/2;
	
	T1MCR = 0x03; // interrupt on MR0; reset on MR0; 
	
	VICVectAddr5 = (unsigned long)T1ISR;
	VICVectPriority4 = 1;
	VICIntEnable |= 1 << INT_TMR1;
 
	T1TCR = 0x01;
}

/* MOTOR CONFIGURATIONS */
void MotorControlPinConfiguration(void){
	/* Pin Function */
	/* 00 = GPIO */
		PINSEL3 &= 0x00000000;		// P1.16..31	
		PINSEL6 &= 0x00005555; 		// P3.00..15
		PINSEL7 &= 0x00000000;		// P3.16..31 
		PINSEL8 &= 0x55555400;		// P4.00..15 
		PINSEL9 &= 0x50090000;		// P4.16..31
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
		turn(DIRECTION);
}

/* Richtung toggeln */
void toggle_dir(){
	int reg_dir = FIO4PIN0 & DIR; 
	if(reg_dir > 0){
		FIO4CLR0 = DIR;
	}
	else{
		FIO4SET0 = DIR;
	}
}

/* Richtung vorgeben */
void turn(int dir){
	switch(dir){
		case RIGHT:
			turn_right();
		
			/* DEBUG */
			FIO2SET |= BIT4;
		
			break;
		case LEFT:
			turn_left();
			
			/* DEBUG */
			FIO2CLR |= BIT4;
		
			break;
		default:
			break;
	}
}

/* Rechts herum drehen */
void turn_right(){
	FIO4SET0 = DIR;
}

/* Links herum drehen */
void turn_left(){
	FIO4CLR0 = DIR;
}
