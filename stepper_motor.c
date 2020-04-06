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
#define DIR	BIT4  // P4.4

#define PI 3.14159265

#define RIGHT 1
#define LEFT	0

void Timer_Init(void);
void toggle_dir(void);
void turn_left(void);
void turn_right(void);
void PWM_Init(void);
void PWM_Set(int);
void Interrupt_Init(void);
void GPIO_Init(void);
void GPIO_Set(void);
int drive(int a,int v_start, int v_max, int dist, int dir);
void turn(int);

__irq void T0_IRQHandler (void);
__irq void PWM_ISR(void);

	bool toggle;
	int TOGGLE_STEP = STEP;
	double sinewave;
	int x=0;
	int STEP_CNT=0;
	
	int step_counter = 0;
	int direction = RIGHT;

	typedef enum{
		Drive_Fast,
		Drive_Slow,
		Wait
	}SystemState;
	
int main(void){
	SystemState state = Wait;
	
	//Timer_Init();
	GPIO_Init();
	GPIO_Set();
	PWM_Init();
	Interrupt_Init();
	
	while(1){
		switch(state){
			case Drive_Fast:
				turn_left();
				PWM_Set(20000);
				while(step_counter > 1){
				}
				state = Drive_Slow;
				break;
			case Drive_Slow:
				turn_right();
				PWM_Set(200000);
				while(step_counter <= 50){
				}
				state = Drive_Fast;
				break;
			case Wait:
				drive(10,10,20,10,RIGHT);
				
				while(1);
				break;
			default:
				break;
		}
	}
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

void Timer_Init(void){
  /* Enable and setup timer interrupt, start timer                            */
	T0MR0         = 22000;                       /* 1msec = 12000-1 at 12.0 MHz */
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
	//sinewave = 24000*sin(((double)x/100)*PI)+48000; /* erzeugt ein sinusförmiges Signal mit einem positiven Offset*/
	//T0MR0 				= (int)sinewave;									/* Sinussignal wird in das Timer0 Register geschrieben */
	toggle ^= true;
	if (toggle){
		FIO3SET3 = STEP; /* Step On */
	} else {
		FIO3CLR3 = STEP;	/* Step Off */
	}
	STEP_CNT++;
	if(STEP_CNT%1000){
		toggle_dir();
	}
  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}

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

/* Interrupt Service Routine */
__irq void PWM_ISR(void)
{	
	if ( PWM1IR & 0x0001 )	/* If interrupt due to match channel 0 */
	{
		if (direction == RIGHT){
			step_counter++;
		}else if (direction == LEFT){
			step_counter--;
		}
		PWM1IR = 0x0001;	/* Clear flag for PWM match channel 0 interrupt */
	}	
	VICVectAddr = 0x00000000;
}

/* Interrupt initialization */
void Interrupt_Init(void)
{
	VICVectAddr8 = (unsigned) PWM_ISR; 	/* PWM ISR Address */
	VICVectCntl8 = (VICVectCntl0 | 8); 	/* Enable PWM IRQ slot */
	VICIntEnable = 1 << 8;							/* Enable PWM Interrupt */
	VICIntSelect = VICIntSelect | 0x00000000; /* PWM configured as IRQ */
}

int drive(int a,int v_start, int v_max, int dist, int dir){
	int start = step_counter;
	turn(dir);
	
	PWM_Set(50000);
	while( (step_counter - start) < dist ){
	
	}
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
