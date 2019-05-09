/* redirect.c - Output redirection to Z-machine memory
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

#define MAX_NESTING 16

extern zword A00267 (zword);

static int depth = -1;

static struct {
    zword xsize;
    zword table;
    zword width;
    zword total;
} redirect[MAX_NESTING];


/*
 * A00273
 *
 * Begin output redirection to the memory of the Z-machine.
 *
 */
void A00273 (zword table, zword xsize, bool buffering)
{
    if (++depth < MAX_NESTING) {

	if (!buffering)
	    xsize = 0xffff;
	else {
	    if (A00247( xsize ) >= 0)
		xsize = A00267 (xsize);
	    else
		xsize = -xsize;
	}

	A00204 (table, 0);

	redirect[depth].table = table;
	redirect[depth].width = 0;
	redirect[depth].total = 0;
	redirect[depth].xsize = xsize;

	A00081 = TRUE;

   } else A00188 (ERR_STR3_NESTING);

}/* A00273 */


/*
 * A00276
 *
 * Redirect a newline to the memory of the Z-machine.
 *
 */
void A00276 (void)
{
    zword size;
    zword addr;

    redirect[depth].total += redirect[depth].width;
    redirect[depth].width = 0;

    addr = redirect[depth].table;

    LOW_WORD (addr, size)
    addr += 2;

    if (redirect[depth].xsize != 0xffff) {

	redirect[depth].table = addr + size;
	size = 0;

    } else A00203 ((zword) (addr + (size++)), 13);

    A00204 (redirect[depth].table, size);

}/* A00276 */


/*
 * A00275
 *
 * Redirect a string of characters to the memory of the Z-machine.
 *
 */
void A00275 (const zchar *s)
{
    zword size;
    zword addr;
    zchar c;

    if (A00035 == V6) {

	int width = A00240 (s);

	if (redirect[depth].xsize != 0xffff)

	    if (redirect[depth].width + width > redirect[depth].xsize) {

		if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
		    width = A00240 (++s);

		A00276 ();

	    }

	redirect[depth].width += width;

    }

    addr = redirect[depth].table;

    LOW_WORD (addr, size)
    addr += 2;

    while ((c = *s++) != 0)
	A00203 ((zword) (addr + (size++)), A00193 (c));

    A00204 (redirect[depth].table, size);

}/* A00275 */


/*
 * A00274
 *
 * End of output redirection.
 *
 */
void A00274 (void)
{
    if (depth >= 0) {

	if (redirect[depth].xsize != 0xffff)
	    A00276 ();

	if (A00035 == V6) {

	    A00062 = (redirect[depth].xsize != 0xffff) ?
		redirect[depth].total : redirect[depth].width;

	    SET_WORD (H_LINE_WIDTH, A00062)

	}

	if (depth == 0)
	    A00081 = FALSE;

	depth--;

    }

}/* A00274 */
