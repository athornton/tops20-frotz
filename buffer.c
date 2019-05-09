/* buffer.c - Text buffering and word wrapping
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

#include <string.h>
#include "frotz.h"

extern void A00000 (zchar);
extern void A00001 (const zchar *);
extern void A00002 (void);

static zchar buffer[TEXT_BUFFER_SIZE];
static int bufpos = 0;

static zchar prev_c = 0;

/*
 * A00194
 *
 * Copy the contents of the text buffer to the output streams.
 *
 */
void A00194 (void)
{
    static bool locked = FALSE;

    /* Make sure we stop when A00194 is called from A00194.
       Note that this is difficult to avoid as we might print a newline
       during A00194, which might cause a newline interrupt, that
       might execute any arbitrary opcode, which might flush the buffer. */

    if (locked || bufpos == 0)
	return;

    /* Send the buffer to the output streams */

    buffer[bufpos] = 0;


    locked = TRUE;

    A00001 (buffer);

    locked = FALSE;

    /* Reset the buffer */

    bufpos = 0;
    prev_c = 0;

}/* A00194 */

/*
 * A00196
 *
 * High level output function.
 *
 */
void A00196 (zchar c)
{
    static bool flag = FALSE;
    A00093 = TRUE;

    if (A00084 || A00081 || A00092) {

	if (!flag) {

	    /* Characters 0 and ZC_RETURN are special cases */

	    if (c == ZC_RETURN)
		{ A00195 (); return; }
	    if (c == 0)
		return;

	    /* Flush the buffer before a whitespace or after a hyphen */

	    if (c == ' ' || c == ZC_INDENT || c == ZC_GAP || (prev_c == '-' && c != '-'))
		A00194 ();

	    /* Set the flag if this is part one of a style or font change */

	    if (c == ZC_NEW_FONT || c == ZC_NEW_STYLE)
		flag = TRUE;

	    /* Remember the current character code */

	    prev_c = c;

	} else flag = FALSE;

	/* Insert the character into the buffer */

	buffer[bufpos++] = c;

	if (bufpos == TEXT_BUFFER_SIZE)
	    A00188 (ERR_TEXT_BUF_OVF);

    } else A00000 (c);

}/* A00196 */


/*
 * A00195
 *
 * High level newline function.
 *
 */
void A00195 (void)
{
    A00194 (); A00002 (); A00093 = FALSE;

}/* A00195 */


/*
 * A00189
 *
 * Initialize buffer variables.
 *
 */
void A00189(void)
{
    memset(buffer, 0, sizeof (zchar) * TEXT_BUFFER_SIZE);
    bufpos = 0;
    prev_c = 0;
}
