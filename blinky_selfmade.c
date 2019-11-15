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

#define RESET BIT0
#define ENABLE BIT7
#define SLEEP BIT1
#define STEP BIT2
#define MS1 BIT2
#define MS2 BIT1
#define MS3 BIT0

#define PI 3.14159265

void Timer_Init(void);
void GPIO_Init(void);
void set_ms(int); // Set Microstepping 1: Full Step, 2: Half Step, 3: Quarter Step, 4: Eigth Step, 5: Sixteenth Step
__irq void T0_IRQHandler (void);
__irq void T1_IRQHandler (void);

	bool toggle;
	int state = 1;

int main(void){

	Timer_Init();
	GPIO_Init();

	while(1){
		switch(state){
			case 1:set_ms(1);
			break;
			case 2:set_ms(2);
			break;
			case 3:set_ms(3);
			break;
			case 4:set_ms(4);
			break;
			case 5:set_ms(5);
			break;
			default:break;
		}
	}
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

	T1MR0         = 12000000;                       /* 1msec = 12000-1 at 12.0 MHz */
	T1MCR         = 3;                           /* Interrupt and Reset on MR0  */
	T1TCR         = 1;                           /* Timer1 Enable               */
	VICIntEnable  = (1  << 5);                   /* Enable Timer1 Interrupt     */
	VICVectAddr4  = (unsigned long)T1_IRQHandler;/* Set Interrupt Vector        */
	VICVectCntl4  = 15;                          /* use it for Timer0 Interrupt */
}

void GPIO_Init(){
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
}
__irq void T0_IRQHandler (void) {
	toggle ^= true;
	if (toggle){
		FIO3SET3 |= STEP; /* Step On */
	} else {
		FIO3CLR3 = STEP;	/* Step Off */
	}
  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}

__irq void T1_IRQHandler (void){
	if(state<6)state++;
	else state=1;
}
void set_ms(int i){
	switch(i){
		case 1: FIO4CLR0 |= MS1 + MS2 + MS3;
		break;
		case 2: FIO4SET0 |= MS1;
		FIO4CLR0 |= MS2 + MS3;
		break;
		case 3: FIO4SET0 |= MS2;
		FIO4CLR0 |= MS1 + MS3;
		break;
		case 4: FIO4SET0 |= MS1 + MS2;
		FIO4CLR0 |= MS3;
		break;
		case 5: FIO4SET0 |= MS1 + MS2 + MS3;
		break;
		default: break;
	}
}
