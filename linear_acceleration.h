#ifndef LIN_ACCELL_H
#define LIN_ACCELL_H

/********************************************
 * APPLY CHANGS TO MOTOR CONFIGURATION HERE!
 * TODO: Replace defines with user input via
 * webserver
 * *****************************************/
/* How many steps does your motor make per round? */
#define SPR		200		// steps per round of your motor

/* What speed do you want to drive the motor with? */
#define OMEGA_ACCEL	10		// Motor Speed

/* How many steps do you want to move? */
#define DISTANCE 20

/********************************************
 * make no changes beyond this line
 * ******************************************/

#define				PI				3.1415926
#define				FREQ			12000000	// timer frequency in Hz
const double	ANGLE	=		(2.0*PI)/SPR;	// alpha in rad
const double	OMEGA	=		(2.0*PI)/FREQ;	// omega in rad

/********************************************
 *	functions
 * ******************************************/

/* Counter value
 *
 * We approximate the calculation of the counter value with
 * B
 *  the taylor series in order to avoid calculating two square roots:
 *
 *	c[0] = 1/t * sqrt(2*a/omega)
 *	c[n] = c[0] * (sqrt(n+1) - sqrt(n))
 *
 * The counter value at time n, using tylor series approximation for the
 * inter-step delay is given by: 
 *
 * 	c[n] = c[n-1] - (2*c[n-1])/(4*n+1)
 *
 * This calculation is much faster than the double square root, but
 * introduces an error of 0.44 at n = 1. A way to compensate for this 
 * error is by multiplying c[0] with 0.676.
 */
double firstDelayC0(void);
double cntVal(double cntValPrevious, int n);
/* Step Pulse
 * ^           ^           ^
 * |           |           |
 * |           |           |
 * |___________|______...__|
 * t0          t1          tn
 * { dt = c0*t }
 *
 * A counter generates these pulses, running at frequency ft (Hz).
 * The delay dt programmed by the counter is:
 * 	dt = c*t = c/ft (s)
 */
int stepPulse(double cntVal);
double velocity(double cycles);


/*
* Timer Controler 
*/
void TimerController(void);

#endif
