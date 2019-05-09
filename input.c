/* input.c - High level input functions
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

extern int A00255 (void);

extern zchar A00256 (zword, zword, bool);
extern zchar A00257 (int, zchar *, zword, zword, bool, bool);

extern void A00258 (zword, zword, zword, bool);

static bool truncate_question_mark(void);

/*
 * A00009
 *
 * Check if the given key is an input terminator.
 *
 */

bool A00009 (zchar key)
{

    if (key == ZC_TIME_OUT)
	return TRUE;
    if (key == ZC_RETURN)
	return TRUE;
    if (key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX)
	return TRUE;

    if (A00061 != 0)

	if (key >= ZC_ARROW_MIN && key <= ZC_MENU_CLICK) {

	    zword addr = A00061;
	    zbyte c;

	    do {
		LOW_BYTE (addr, c);
		if (c == 255 || key == A00192 (c))
		    return TRUE;
		addr++;
	    } while (c != 0);

	}

    return FALSE;

}/* A00009 */

/*
 * A00127, add or remove a menu and A00202 if successful.
 *
 * 	zargs[0] = number of menu
 *	zargs[1] = table of menu entries or 0 to remove menu
 *
 */

void A00127 (void)
{

    /* This opcode was only used for the Macintosh version of Journey.
       It controls menus with numbers greater than 2 (menus 0, 1 and 2
       are system menus). Frotz doesn't implement menus yet. */

    A00202 (FALSE);

}/* A00127 */

/*
 * A00032
 *
 * Ask the user a question; return true if the answer is yes.
 *
 */

bool A00032 (const char *s)
{
    zchar key;

    A00199 (s);
    A00199 ("? (y/n) >");

    key = A00256 (0, 0, FALSE);

    if (key == 'y' || key == 'Y') {
	A00199 ("y\n");
	return TRUE;
    } else {
	A00199 ("n\n");
	return FALSE;
    }

}/* A00032 */

/*
 * A00207
 *
 * Read a string from the current input stream.
 *
 */

void A00207 (int max, zchar *buffer)
{
    zchar key;

    buffer[0] = 0;

    do {

	key = A00257 (max, buffer, 0, 0, FALSE, FALSE);

    } while (key != ZC_RETURN);

}/* A00207 */

/*
 * A00250
 *
 * Ask the user to type in a number and return it.
 *
 */

int A00250 (void)
{
    zchar buffer[6];
    int value = 0;
    int i;

    A00207 (5, buffer);

    for (i = 0; buffer[i] != 0; i++)
	if (buffer[i] >= '0' && buffer[i] <= '9')
	    value = 10 * value + buffer[i] - '0';

    return value;

}/* A00250 */

/*
 * A00153, read a line of input and (in V5+) store the terminating key.
 *
 *	zargs[0] = address of text buffer
 *	zargs[1] = address of token buffer
 *	zargs[2] = timeout in tenths of a second (optional)
 *	zargs[3] = packed address of routine to be called on timeout
 *
 */

void A00153 (void)
{
    zchar buffer[INPUT_BUFFER_SIZE];
    zword addr;
    zchar key;
    zbyte max, size;
    zbyte c;
    int i;

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = 0;

    /* Get maximum input size */

    addr = zargs[0];

    LOW_BYTE (addr, max);

    if (A00035 <= V4)
	max--;

    if (max >= INPUT_BUFFER_SIZE)
	max = INPUT_BUFFER_SIZE - 1;

    /* Get initial input size */

    if (A00035 >= V5) {
	addr++;
	LOW_BYTE (addr, size);
    } else size = 0;

    /* Copy initial input to local buffer */

    for (i = 0; i < size; i++) {
	addr++;
	LOW_BYTE (addr, c);
	buffer[i] = A00192 (c);
    }

    buffer[i] = 0;

    /* Draw status line for V1 to V3 games */

    if (A00035 <= V3)
	A00174 ();

    /* Read input from current input stream */

    key = A00257 (
	max, buffer,		/* buffer and size */
	zargs[2],		/* timeout value   */
	zargs[3],		/* timeout routine */
	TRUE,	        	/* enable hot keys */
	A00035 == V6);	/* no script in V6 */

    if (key == ZC_BAD)
	return;

    /* Perform A00255 for V1 to V4 games */

    if (A00035 <= V4)
	A00255 ();

    /* Copy local buffer back to dynamic memory */

    for (i = 0; buffer[i] != 0; i++) {

	if (key == ZC_RETURN) {
	    if (buffer[i] >= 'A' && buffer[i] <= 'Z')
		buffer[i] += 'a' - 'A';
	    if (buffer[i] >= 0xc0 && buffer[i] <= 0xde && buffer[i] != 0xd7)
		buffer[i] += 0x20;

	}

	if (truncate_question_mark() && buffer[i] == '?') buffer[i] = ' ';

	A00203 ((zword) (zargs[0] + ((A00035 <= V4) ? 1 : 2) + i), A00193 (buffer[i]));

    }

    /* Add null character (V1-V4) or write input length into 2nd byte */

    if (A00035 <= V4)
	A00203 ((zword) (zargs[0] + 1 + i), 0);
    else
	A00203 ((zword) (zargs[0] + 1), i);

    /* Tokenise line if a token buffer is present */

    if (key == ZC_RETURN && zargs[1] != 0)
	A00258 (zargs[0], zargs[1], 0, FALSE);

    /* Store key */

    if (A00035 >= V5)
	store (A00193 (key));

}/* A00153 */

/*
 * A00154, read and store a key.
 *
 *	zargs[0] = input device (must be 1)
 *	zargs[1] = timeout in tenths of a second (optional)
 *	zargs[2] = packed address of routine to be called on timeout
 *
 */

void A00154 (void)
{
    zchar key;

    /* Supply default arguments */

    if (zargc < 2)
	zargs[1] = 0;

    /* Read input from the current input stream */

    key = A00256 (
	zargs[1],	/* timeout value   */
	zargs[2],	/* timeout routine */
	TRUE);  	/* enable hot keys */

    if (key == ZC_BAD)
	return;

    /* Store key */

    /* For timeouts, make sure A00193() won't try to convert
     * 0x00.  We should instead return 0x00 as is.
     * Thanks to Peter Seebach.
     */
    if (key == 0)
	store(key);
    else
	store (A00193 (key));
}/* A00154 */

/*
 * A00155, write the current mouse status into a table.
 *
 *	zargs[0] = address of table
 *
 */

void A00155 (void)
{
    zword btn;

    btn = 1;

    /** I don't remember what was going on here */
    /* Read the mouse position and which buttons are down */
/*
    btn = os_read_mouse ();
    A00070 = A00086;
    A00069 = A00085;
*/
    A00204 ((zword) (zargs[0] + 0), A00070);
    A00204 ((zword) (zargs[0] + 2), A00069);
    A00204 ((zword) (zargs[0] + 4), btn);	/* mouse button bits */
    A00204 ((zword) (zargs[0] + 6), 0);		/* menu selection */

}/* A00155 */

/*
 * truncate_question_mark
 *
 * check if this game is one that expects the A00259er to truncate a
 * trailing question mark from the input buffer.
 *
 * For some games, Infocom modified the A00259er to truncate trailing
 * question marks.  Presumably this was to make it easier to deal with
 * questions asked of the narrator or A00259er, such as "WHAT IS A
 * GRUE?".  This is a deviation from the Z-Machine Standard (written
 * after Infocom's demise).  Some A00259ers written later incorrectly
 * always truncate trailing punctuation.  In the interest of making sure
 * the original Infocom games play exactly as they did with Infocom's
 * own A00259ers, this function checks for those games that expect
 * the trailing question mark to be truncated.
 *
 */
static bool truncate_question_mark(void)
{
	if (A00075 == ZORK1) return TRUE;
	if (A00075 == ZORK2) return TRUE;
	if (A00075 == ZORK3) return TRUE;
	if (A00075 == MINIZORK) return TRUE;
	if (A00075 == SAMPLER1) return TRUE;
	if (A00075 == SAMPLER2) return TRUE;
	if (A00075 == ENCHANTER) return TRUE;
	if (A00075 == SORCERER) return TRUE;
	if (A00075 == SPELLBREAKER) return TRUE;
	if (A00075 == PLANETFALL) return TRUE;
	if (A00075 == STATIONFALL) return TRUE;
	if (A00075 == BALLYHOO) return TRUE;
	if (A00075 == BORDER_ZONE) return TRUE;
	if (A00075 == AMFV) return TRUE;
	if (A00075 == HHGG) return TRUE;
	if (A00075 == LGOP) return TRUE;
	if (A00075 == SUSPECT) return TRUE;

	return FALSE;
}
