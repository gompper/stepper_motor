#include <stdio.h>
#include <math.h>

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
#define DISTANCE	100

/********************************************
 * make no changes beyond this line
 * ******************************************/

#define FREQ		12000000	// timer frequency in Hz
#define	ANGLE		2.0*M_PI/SPR	// alpha in rad
#define OMEGA		2.0*M_PI/FREQ	// omega in rad
#define T		1.0/FREQ
/********************************************
 *	functions
 * ******************************************/

/* Counter value
 *
 * We approximate the calculation of the counter value with
 * the taylor series in order to avoid calculating two square roots:
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
double firstDelayC0(){
	printf("debug - first delay c0");
	return (1.0/T * sqrt(2.0*ANGLE/OMEGA)) * 0.676;
}
double cntVal(double cntValPrevious, int n){
	return (cntValPrevious = 0.0) ? firstDelayC0() : cntValPrevious - (2.0*cntValPrevious)/(4.0*n+1);
}


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
double stepPulse(double cntVal, int t){
	return cntVal*t;
}


int main()
{

	printf("#define	ANGLE		2.0*M_PI/SPR ====> %5.f\n", ANGLE);
	printf("#define OMEGA		2.0*M_PI/FREQ ===> %5.f\n", OMEGA);
	printf("#define T		1.0/FREQ  =======> %5.f\n\n\n", T);

	int c0;
	int stepcnt;
	int steps;
	double timerdelay = 0.0;
	double previousDelay = 0.0;
	stepcnt = 0;
	while(stepcnt < DISTANCE){
		timerdelay = cntVal(previousDelay, stepcnt);
		printf("Timer Match Register [n %d]:\t%5.f\n", stepcnt, timerdelay);
		previousDelay = timerdelay;
		stepcnt++;
	}

	return 0;
}
