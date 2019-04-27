/*
 * variable.c
 *
 * Variable and stack related opcodes
 *
 */

#include "frotz.h"

/*
 * z_dec, decrement a variable.
 *
 * 	zargs[0] = variable to decrement
 *
 */

void z_dec (void)
{
    zword value;

    if (zargs[0] == 0)
	(*sp)--;
    else if (zargs[0] < 16)
	(*(fp - zargs[0]))--;
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value--;
	SET_WORD (addr, value)
    }

}/* z_dec */

/*
 * A00099, decrement a variable and A00193 if now less than value.
 *
 * 	zargs[0] = variable to decrement
 * 	zargs[1] = value to check variable against
 *
 */

void A00099 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = --(*sp);
    else if (zargs[0] < 16)
	value = --(*(fp - zargs[0]));
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value--;
	SET_WORD (addr, value)
    }

    A00193 ((short) value < (short) zargs[1]);

}/* A00099 */

/*
 * z_inc, increment a variable.
 *
 * 	zargs[0] = variable to increment
 *
 */

void z_inc (void)
{
    zword value;

    if (zargs[0] == 0)
	(*sp)++;
    else if (zargs[0] < 16)
	(*(fp - zargs[0]))++;
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value++;
	SET_WORD (addr, value)
    }

}/* z_inc */

/*
 * A00114, increment a variable and A00193 if now greater than value.
 *
 * 	zargs[0] = variable to increment
 * 	zargs[1] = value to check variable against
 *
 */

void A00114 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = ++(*sp);
    else if (zargs[0] < 16)
	value = ++(*(fp - zargs[0]));
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value++;
	SET_WORD (addr, value)
    }

    A00193 ((short) value > (short) zargs[1]);

}/* A00114 */

/*
 * A00118, store the value of a variable.
 *
 *	zargs[0] = variable to store
 *
 */

void A00118 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = *sp;
    else if (zargs[0] < 16)
	value = *(fp - zargs[0]);
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
    }

    store (value);

}/* A00118 */

/*
 * z_pop, pop a value off the game stack and discard it.
 *
 *	no zargs used
 *
 */

void z_pop (void)
{

    sp++;

}/* z_pop */

/*
 * A00130, pop n values off the game or user stack and discard them.
 *
 *	zargs[0] = number of values to discard
 *	zargs[1] = address of user stack (optional)
 *
 */

void A00130 (void)
{

    if (zargc == 2) {		/* it's a user stack */

	zword size;
	zword addr = zargs[1];

	LOW_WORD (addr, size)

	size += zargs[0];
	A00195 (addr, size);

    } else sp += zargs[0];	/* it's the game stack */

}/* A00130 */

/*
 * A00141, pop a value off...
 *
 * a) ...the game or a user stack and store it (V6)
 *
 *	zargs[0] = address of user stack (optional)
 *
 * b) ...the game stack and write it to a variable (other than V6)
 *
 *	zargs[0] = variable to write value to
 *
 */

void A00141 (void)
{
    zword value;

    if (A00025 != V6) {	/* not a V6 game, pop stack and write */

	value = *sp++;

	if (zargs[0] == 0)
	    *sp = value;
	else if (zargs[0] < 16)
	    *(fp - zargs[0]) = value;
	else {
	    zword addr = A00032 + 2 * (zargs[0] - 16);
	    SET_WORD (addr, value)
	}

    } else {			/* it's V6, but is there a user stack? */

	if (zargc == 1) {	/* it's a user stack */

	    zword size;
	    zword addr = zargs[0];

	    LOW_WORD (addr, size)

	    size++;
	    A00195 (addr, size);

	    addr += 2 * size;
	    LOW_WORD (addr, value)

	} else value = *sp++;	/* it's the game stack */

	store (value);

    }

}/* A00141 */

/*
 * A00142, push a value onto the game stack.
 *
 *	zargs[0] = value to push onto the stack
 *
 */

void A00142 (void)
{

    *--sp = zargs[0];

}/* A00142 */

/*
 * A00143, push a value onto a user stack then A00193 if successful.
 *
 *	zargs[0] = value to push onto the stack
 *	zargs[1] = address of user stack
 *
 */

void A00143 (void)
{
    zword size;
    zword addr = zargs[1];

    LOW_WORD (addr, size)

    if (size != 0) {

	A00195 ((zword) (addr + 2 * size), zargs[0]);

	size--;
	A00195 (addr, size);

    }

    A00193 (size);

}/* A00143 */

/*
 * A00172, write a value to a variable.
 *
 * 	zargs[0] = variable to be written to
 *      zargs[1] = value to write
 *
 */

void A00172 (void)
{
    zword value = zargs[1];

    if (zargs[0] == 0)
	*sp = value;
    else if (zargs[0] < 16)
	*(fp - zargs[0]) = value;
    else {
	zword addr = A00032 + 2 * (zargs[0] - 16);
	SET_WORD (addr, value)
    }

}/* A00172 */
