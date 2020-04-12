#include <math.h>
#include "linear_acceleration.h"

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
//double firstDelayC0(){
	//return (FREQ * sqrt((2.0*ANGLE)/OMEGA)) * 0.676;
	//return 0x272477;
//}
/* acc = acceleration must be 1 (positive) or -1 (negative) */
double cntVal(double cntValPrevious, int n, int acc){
	return cntValPrevious - acc*((2.0*cntValPrevious)/(4.0*n+1));
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
int stepPulse(double cntVal){
	return (int)cntVal/FREQ;
}

double velocity(double cycles){
	return ANGLE/(cycles/FREQ);
}
