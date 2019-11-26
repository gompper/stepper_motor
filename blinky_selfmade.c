#include <stdio.h>
#include <LPC23xx.H>                    /* LPC23xx definitions                */
#include <stdbool.h>
#include <math.h>

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

#define RESET BIT0	// P3.24
#define ENABLE BIT7	// P3.23
#define SLEEP BIT1	// P3.25
#define STEP BIT2 	// P3.26	
#define MS1 BIT2	// P4.2
#define MS2 BIT1	// P4.1
#define MS3 BIT0	// P4.0

#define PI 3.14159265

void Timer_Init(void);
__irq void T0_IRQHandler (void);

	bool toggle;
	int TOGGLE_STEP = STEP;
	double sinewave;
	int x=0;

int main(void){
	
	Timer_Init();
	
		/* Motor GPIO */
	PINMODE7 = 0x00000000; 		/* pull up resistor */
	PINSEL7 &= 0x00000000;		/* clear P3.23..26 */
	
	FIO4DIR0 |= MS1 + MS2 + MS3;
	FIO4MASK0 = 0x00000000;
	FIO4SET0 |= MS1 + MS2 + MS3; 
	
	FIO3DIR2 |= ENABLE;
	FIO3MASK2 	= 0x00000000;
	FIO3SET2 &= ~ENABLE;			/* Enable ist low Active */
	
	FIO3DIR3 	|= SLEEP + RESET + STEP;
	FIO3MASK3 	= 0x00000000;
	FIO3SET3 |= SLEEP;				/* Sleep ist low Active */
	FIO3SET3 |= RESET;				/* Reset ist low Active */
	
	while(1);
}

void LED_Init(void) { 
  PINSEL10 = 0;                         /* Disable ETM interface, enable LEDs */
  FIO2DIR  = 0x000000FF;                /* P2.0..7 defined as Outputs         */
  FIO2MASK = 0x00000000;								/* keine Ports maskiert */
  }

void Timer_Init(void){
  /* Enable and setup timer interrupt, start timer                            */
	T0MR0         = 24000;                       /* 1msec = 12000-1 at 12.0 MHz */
  T0MCR         = 3;                           /* Interrupt and Reset on MR0  */
  T0TCR         = 1;                           /* Timer0 Enable               */
  VICVectAddr4  = (unsigned long)T0_IRQHandler;/* Set Interrupt Vector        */
  VICVectCntl4  = 15;                          /* use it for Timer0 Interrupt */
  VICIntEnable  = (1  << 4);                   /* Enable Timer0 Interrupt     */
}

__irq void T0_IRQHandler (void) {

	if (x<200)
		x++;
	else
		x=0;
	
	sinewave = 24000*sin(((double)x/100)*PI)+48000; /* erzeugt ein sinusf�rmiges Signal mit einem positiven Offset*/
	//T0MR0 				= (int)sinewave;									/* Sinussignal wird in das Timer0 Register geschrieben */
	toggle ^= true;
	
	if (toggle){
		FIO3SET3 |= STEP; /* Step On */
	} else {
		FIO3CLR3 = STEP;	/* Step Off */
	}


  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}


