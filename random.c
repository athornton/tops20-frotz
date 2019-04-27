/*
 * random.c
 *
 * Z-machine random number generator
 *
 */

#include "frotz.h"

static long A = 1;

static interval = 0;
static counter = 0;

/*
 * A00016
 *
 * Set the seed value for the random number generator.
 *
 */

void A00016 (int value)
{

    if (value == 0) {		/* ask interface for seed value */
	A = A00211 ();
	interval = 0;
    } else if (value < 1000) {	/* special seed value */
	counter = 0;
	interval = value;
    } else {			/* standard seed value */
	A = value;
	interval = 0;
    }

}/* A00016 */

/*
 * A00147, store a random number or set the random number seed.
 *
 *	zargs[0] = range (positive) or seed value (negative)
 *
 */

void A00147 ()
{

    if ((short) zargs[0] <= 0) {	/* set random seed */

	A00016 (- (short) zargs[0]);
	store (0);

    } else {				/* generate random number */

	zword result;

	if (interval != 0) {		/* ...in special mode */
	    result = counter++;
	    if (counter == interval) counter = 0;
	} else {			/* ...in standard mode */
	    A = 0x015a4e35L * A + 1;
	    result = (A >> 16) & 0x7fff;
	}

	store ((zword) (result % zargs[0] + 1));

    }

}/* A00147 */
