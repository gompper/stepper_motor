#include <stdio.h>
#include <LPC23xx.H>                    /* LPC23xx definitions                */
#include "LCD.h"                      	/* Graphic LCD function prototypes    */


void LED_Init(void);
void LED_Off (unsigned int);
void LED_Out(unsigned int);
void LED_Out(unsigned int);
void Timer_Init(void);
__irq void T0_IRQHandler (void);
void Disp_Bargraph(int,int,int);

int main(void){
	int i;

	
	LED_Init();
	Timer_Init();
	

	lcd_init();
  lcd_clear();
  lcd_print("  Loading ... ");
	set_cursor (0, 1);
  for (i = 0; i < 20000000; i++);       /* Wait for initial display           */

}

void LED_Init(void) { 
  PINSEL10 = 0;                         /* Disable ETM interface, enable LEDs */
  FIO2DIR  = 0x000000FF;                /* P2.0..7 defined as Outputs         */
  FIO2MASK = 0x00000000;				/* keine Ports maskiert */
  }
  void LED_On (unsigned int num) {
  FIO2SET = (1 << num);
}

void LED_Off (unsigned int num) {
  FIO2CLR = (1 << num);
}

void LED_Out(unsigned int value) {
  FIO2CLR = 0xFF;                       /* Turn off all LEDs                  */
  FIO2SET = (value & 0xFF);             /* Turn on requested LEDs             */
}

void Timer_Init(void){
  /* Enable and setup timer interrupt, start timer                            */
  T0MR0         = 6000000;                       /* 1msec = 12000-1 at 12.0 MHz */
  T0MCR         = 3;                           /* Interrupt and Reset on MR0  */
  T0TCR         = 1;                           /* Timer0 Enable               */
  VICVectAddr4  = (unsigned long)T0_IRQHandler;/* Set Interrupt Vector        */
  VICVectCntl4  = 15;                          /* use it for Timer0 Interrupt */
  VICIntEnable  = (1  << 4);                   /* Enable Timer0 Interrupt     */
}

__irq void T0_IRQHandler (void) {
	static unsigned int counter = 1;
	static unsigned int counter2 = 1;
	if (counter <= (1 << 7)){
		counter = counter << 1;
	}
	else{
		counter = 1;
	}
	
	if (counter2 <= 78){
		counter2++;
	}
	else{
		counter2=0;
	}
	//counter2 = 20;
	Disp_Bargraph(0,1,counter2);
	LED_Out(counter);

  T0IR        = 1;                      /* Clear interrupt flag               */
  VICVectAddr = 0;                      /* Acknowledge Interrupt              */
}

/* Function for displaying bargraph on the LCD display                        */
void Disp_Bargraph(int pos_x, int pos_y, int value) {
  int i;

  set_cursor (pos_x, pos_y);
  for (i = 0; i < 16; i++)  {
    if (value > 5)  {
      lcd_putchar (0x05);
      value -= 5;
    }  else  {
      lcd_putchar (value);
      value = 0;
    }
  }
}
