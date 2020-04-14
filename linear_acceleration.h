#ifndef LIN_ACCELL_H
#define LIN_ACCELL_H

/********************************************
 * APPLY CHANGS TO MOTOR CONFIGURATION HERE!
 * TODO: Replace defines with user input via
 * webserver
 * *****************************************/
 
/***********************************
* USER INPUT
************************************/

/* How many steps do you want to move? */
#define DISTANCE	200	// [steps]

/* What is the desired maximum speed? */
#define MAXSPEED	200 // [steps/s]

/* What acceleration do you want to drive the motor with? */
#define OMEGA_ACCEL	10.0		// Motor Acceleration in steps/s^2

/* Turn right or left? */
#define DIRECTION	1	// 1 = RIGHT, 0 = LEFT
 
/* How many steps does your motor make per round? */
#define SPR	200		// steps per round of your motor


/********************************************/

#define	PI				3.1415926
#define	FREQ			12000000						// timer frequency in Hz
#define	ANGLE			2.0*PI/SPR					// alpha in rad
#define	OMEGA			ANGLE*OMEGA_ACCEL	// omega in rad/s^2
#define MAXSPEED_DELAY FREQ*ANGLE*200/(2*PI*MAXSPEED)
#define	FIRSTDELAY (int)((FREQ * sqrt(2.0*ANGLE/(OMEGA))) * 0.676)

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

double cntVal(double cntValPrevious, int n, int acc);
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

#endif
