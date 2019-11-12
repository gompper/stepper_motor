#include <stdio.h>
#include <LPC23xx.H>                    /* LPC23xx definitions                */
//#include "LCD.h"                      	/* Graphic LCD function prototypes    */
#include <stdbool.h>

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80


#define RESET BIT0
#define ENABLE BIT7
#define SLEEP BIT1
#define STEP BIT2		

void LED_Init(void);

void Timer_Init(void);
__irq void T0_IRQHandler (void);


	bool toggle;
	int TOGGLE_STEP = STEP;


int main(void){
	int i;

	
	LED_Init();
	Timer_Init();
	
		/* Motor GPIO */
	PINMODE7 = 0x00000000; // pull up resistor
	PINSEL7 &= 0x00000000;		/* clear P3.23..26 */
	
	FIO3DIR2 |= ENABLE;
	FIO3MASK2 	= 0x00000000;
	FIO3SET2 &= ~ENABLE;
	
	FIO3DIR3 	|= SLEEP + RESET + STEP;
	FIO3MASK3 	= 0x00000000;
	FIO3SET3 |= SLEEP;
	FIO3SET3 |= RESET;
	
	
  for (i = 0; i < 20000000; i++);       /* Wait for initial display           */
	while(1);
}

void LED_Init(void) { 
  PINSEL10 = 0;                         /* Disable ETM interface, enable LEDs */
  FIO2DIR  = 0x000000FF;                /* P2.0..7 defined as Outputs         */
  FIO2MASK = 0x00000000;				/* keine Ports maskiert */
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
				/* GPIO */
	//FIO3CLR3 = BIT0;
	//FIO3CLR3 |= STEP;
	//TOGGLE_STEP ^= STEP;
	toggle ^= true;
	if (toggle){
		FIO3SET3 |= STEP;
	} else {
		FIO3CLR3 = STEP;
	}

	FIO2CLR = 0xFF;                       /* Turn off all LEDs                  */
  FIO2SET = (0x01 & 0xFF);             /* Turn on requested LEDs             */
	

  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
	

}


