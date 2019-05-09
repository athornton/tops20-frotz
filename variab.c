/* variable.c - Variable and stack related opcodes
 *	Copyright (c) 1995-1997 Stefan Jokisch
 *
 * This file is part of Frotz.
 *
 * Frotz is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Frotz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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
	zword addr = A00042 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value--;
	SET_WORD (addr, value)
    }

}/* z_dec */


/*
 * A00104, decrement a variable and A00202 if now less than value.
 *
 * 	zargs[0] = variable to decrement
 * 	zargs[1] = value to check variable against
 *
 */
void A00104 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = --(*sp);
    else if (zargs[0] < 16)
	value = --(*(fp - zargs[0]));
    else {
	zword addr = A00042 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value--;
	SET_WORD (addr, value)
    }

    A00202 (A00247( value ) < A00247( zargs[1]) );

}/* A00104 */


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
	zword addr = A00042 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value++;
	SET_WORD (addr, value)
    }

}/* z_inc */


/*
 * A00119, increment a variable and A00202 if now greater than value.
 *
 * 	zargs[0] = variable to increment
 * 	zargs[1] = value to check variable against
 *
 */
void A00119 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = ++(*sp);
    else if (zargs[0] < 16)
	value = ++(*(fp - zargs[0]));
    else {
	zword addr = A00042 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
	value++;
	SET_WORD (addr, value)
    }

    A00202 (A00247( value ) > A00247( zargs[1]) );

}/* A00119 */


/*
 * A00123, store the value of a variable.
 *
 *	zargs[0] = variable to store
 *
 */
void A00123 (void)
{
    zword value;

    if (zargs[0] == 0)
	value = *sp;
    else if (zargs[0] < 16)
	value = *(fp - zargs[0]);
    else {
	zword addr = A00042 + 2 * (zargs[0] - 16);
	LOW_WORD (addr, value)
    }

    store (value);

}/* A00123 */


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
 * A00135, pop n values off the game or user stack and discard them.
 *
 *	zargs[0] = number of values to discard
 *	zargs[1] = address of user stack (optional)
 *
 */
void A00135 (void)
{
    if (zargc == 2) {		/* it's a user stack */

	zword size;
	zword addr = zargs[1];

	LOW_WORD (addr, size)

	size += zargs[0];
	A00204 (addr, size);

    } else sp += zargs[0];	/* it's the game stack */

}/* A00135 */


/*
 * A00146, pop a value off...
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
void A00146 (void)
{
    zword value;

    if (A00035 != V6) {	/* not a V6 game, pop stack and write */

	value = *sp++;

	if (zargs[0] == 0)
	    *sp = value;
	else if (zargs[0] < 16)
	    *(fp - zargs[0]) = value;
	else {
	    zword addr = A00042 + 2 * (zargs[0] - 16);
	    SET_WORD (addr, value)
	}

    } else {			/* it's V6, but is there a user stack? */

	if (zargc == 1) {	/* it's a user stack */

	    zword size;
	    zword addr = zargs[0];

	    LOW_WORD (addr, size)

	    size++;
	    A00204 (addr, size);

	    addr += 2 * size;
	    LOW_WORD (addr, value)

	} else value = *sp++;	/* it's the game stack */

	store (value);

    }

}/* A00146 */


/*
 * A00147, push a value onto the game stack.
 *
 *	zargs[0] = value to push onto the stack
 *
 */
void A00147 (void)
{
    *--sp = zargs[0];

}/* A00147 */


/*
 * A00148, push a value onto a user stack then A00202 if successful.
 *
 *	zargs[0] = value to push onto the stack
 *	zargs[1] = address of user stack
 *
 */
void A00148 (void)
{
    zword size;
    zword addr = zargs[1];

    LOW_WORD (addr, size)

    if (size != 0) {

	A00204 ((zword) (addr + 2 * size), zargs[0]);

	size--;
	A00204 (addr, size);

    }

    A00202 (size);

}/* A00148 */


/*
 * A00177, write a value to a variable.
 *
 * 	zargs[0] = variable to be written to
 *      zargs[1] = value to write
 *
 */
void A00177 (void)
{
    zword value = zargs[1];

    if (zargs[0] == 0)
	*sp = value;
    else if (zargs[0] < 16)
	*(fp - zargs[0]) = value;
    else {
	zword addr = A00042 + 2 * (zargs[0] - 16);
	SET_WORD (addr, value)
    }

}/* A00177 */
