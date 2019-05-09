/* table.c - Table handling opcodes
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
 * A00103, copy a table or fill it with zeroes.
 *
 *	zargs[0] = address of table
 * 	zargs[1] = destination address or 0 for fill
 *	zargs[2] = size of table
 *
 * Note: Copying is safe even when source and destination overlap; but
 *       if zargs[1] is negative the table _must_ be copied forwards.
 *
 */
void A00103 (void)
{
    zword addr;
    zword size = zargs[2];
    zbyte value;
    int i;

    if (zargs[1] == 0)      				/* zero table */

	for (i = 0; i < size; i++)
	    A00203 ((zword) (zargs[0] + i), 0);

    else if (A00247( size ) < 0 || zargs[0] > zargs[1])	/* copy forwards */

	for (i = 0; i < ((A00247( size ) < 0) ? - A00247( size ) : size); i++) {
	    addr = zargs[0] + i;
	    LOW_BYTE (addr, value)
	    A00203 ((zword) (zargs[1] + i), value);
	}

    else						/* copy backwards */

	for (i = size - 1; i >= 0; i--) {
	    addr = zargs[0] + i;
	    LOW_BYTE (addr, value)
	    A00203 ((zword) (zargs[1] + i), value);
	}

}/* A00103 */


/*
 * A00124, store a value from a table of bytes.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry to store
 *
 */
void A00124 (void)
{
    zword addr = A00248(zargs[0] + zargs[1]);
    zbyte value;

    LOW_BYTE (addr, value)

    store (value);

}/* A00124 */


/*
 * A00125, store a value from a table of words.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry to store
 *
 */
void A00125 (void)
{
    zword z0=A00248(zargs[0]);
    zword z1=A00248(zargs[1]);
    
    zword addr = A00248(z0 + A00248(2 * z1));
    zword value;

    LOW_WORD (addr, value)

    store (A00248(value));

}/* A00125 */


/*
 * A00165, find and store the address of a target within a table.
 *
 *	zargs[0] = target value to be searched for
 *	zargs[1] = address of table
 *	zargs[2] = number of table entries to check value against
 *	zargs[3] = type of table (optional, defaults to 0x82)
 *
 * Note: The table is a word array if bit 7 of zargs[3] is set; otherwise
 *       it's a byte array. The lower bits hold the address step.
 *
 */
void A00165 (void)
{
    zword addr = zargs[1];
    int i;

    /* Supply default arguments */

    if (zargc < 4)
	zargs[3] = 0x82;

    /* Scan byte or word array */

    for (i = 0; i < zargs[2]; i++) {

	if (zargs[3] & 0x80) {	/* scan word array */

	    zword wvalue;

	    LOW_WORD (addr, wvalue)

	    if (wvalue == zargs[0])
		goto finished;

	} else {		/* scan byte array */

	    zbyte bvalue;

	    LOW_BYTE (addr, bvalue)

	    if (bvalue == zargs[0])
		goto finished;

	}

	addr += zargs[3] & 0x7f;

    }

    addr = 0;

finished:

    store (addr);
    A00202 (addr);

}/* A00165 */


/*
 * A00178, write a byte into a table of bytes.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry
 *	zargs[2] = value to be written
 *
 */
void A00178 (void)
{
    A00203 (A00248((zargs[0] + zargs[1])),
            A00248(zargs[2]));

}/* A00178 */


/*
 * A00179, write a word into a table of words.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry
 *	zargs[2] = value to be written
 *
 */
void A00179 (void)
{
    zword z0 = A00248(zargs[0]);
    zword z1 = A00248(zargs[1]);
    zword z2 = A00248(zargs[2]);

    A00204 (A00248(z0 + A00248(2*z1)), z2);

}/* A00179 */
