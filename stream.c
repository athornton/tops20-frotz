/* stream.c - IO stream implementation
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

extern bool A00271 (zchar);

extern bool A00272 (void);

extern void A00251 (void);
extern void A00252 (void);
extern void A00273 (zword, zword, bool);
extern void A00274 (void);
extern void A00253 (void);
extern void A00254 (void);
extern void A00026 (void);
extern void A00027 (void);

extern void A00275 (const zchar *);
extern void A00276 (void);
extern void A00277 (zchar);
extern void A00278 (const zchar *, zchar);
extern void A00279 (zchar);
extern void A00280 (const zchar *);
extern void A00281 (void);
extern void A00282 (const zchar *, zchar);
extern void A00283 (const zchar *);
extern void A00284 (void);
extern void A00285 (void);
extern void A00286 (zchar);
extern void A00287 (const zchar *);
extern void A00208 (void);
extern void A00288 (const zchar *, zchar);
extern void A00289 (const zchar *);
extern void A00290 (void);
extern void A00291 (void);

extern zchar A00292 (void);
extern zchar A00293 (zchar *);
extern zchar A00294 (zword);
extern zchar A00295 (int, zchar *, zword, bool);

extern int A00269 (zword);


/*
 * A00200
 *
 * Start printing a "debugging" A00084.
 *
 */
void A00200 (void)
{
    A00194 ();

    if (A00079)
	A00290 ();
    if (A00080 && A00090)
	A00284 ();

    A00084 = TRUE;

}/* A00200 */


/*
 * A00201
 *
 * Stop printing a "debugging" A00084.
 *
 */
void A00201 (void)
{
    A00194 ();

    if (A00079)
	A00291 ();
    if (A00080 && A00090)
	A00285 ();

    A00084 = FALSE;

}/* A00201 */


/*
 * A00131, open or close an output stream.
 *
 *	zargs[0] = stream to open (positive) or close (negative)
 *	zargs[1] = address to redirect output to (stream 3 only)
 *	zargs[2] = width of redirected output (stream 3 only, optional)
 *
 */
void A00131 (void)
{
    A00194 ();

    switch (A00247( zargs[0]) ) {

    case  1: A00079 = TRUE;
	     break;
    case -1: A00079 = FALSE;
	     break;
    case  2: if (!A00080) A00026 ();
	     break;
    case -2: if (A00080) A00027 ();
	     break;
    case  3: A00273 (zargs[1], zargs[2], zargc >= 3);
	     break;
    case -3: A00274 ();
	     break;
    case  4: if (!A00082) A00253 ();
	     break;
    case -4: if (A00082) A00254 ();
	     break;

    }

}/* A00131 */


/*
 * A00000
 *
 * Send a single character to the output stream.
 *
 */
void A00000 (zchar c)
{
    if (A00079)
	A00286 (c);
    if (A00080 && A00090)
	A00279 (c);

}/* A00000 */


/*
 * A00001
 *
 * Send a string of characters to the output streams.
 *
 */
void A00001 (const zchar *s)
{
    if (A00081 && !A00084)

	A00275 (s);

    else {

	if (A00079)
	    A00287 (s);
	if (A00080 && A00090)
	    A00280 (s);

    }

}/* A00001 */


/*
 * A00002
 *
 * Send a newline to the output streams.
 *
 */
void A00002 (void)
{
    if (A00081 && !A00084)
	A00276 ();

    else {

	if (A00079)
	    A00208 ();
	if (A00080 && A00090)
	    A00281 ();

    }

}/* A00002 */


/*
 * A00120, select an input stream.
 *
 *	zargs[0] = input stream to be selected
 *
 */
void A00120 (void)
{
    A00194 ();

    if (zargs[0] == 0 && A00083)
	A00252 ();
    if (zargs[0] == 1 && !A00083)
	A00251 ();

}/* A00120 */


/*
 * A00256
 *
 * Read a single keystroke from the current input stream.
 *
 */
zchar A00256 ( zword timeout, zword routine, bool hot_keys )
{
    zchar key = ZC_BAD;

    A00194 ();

    /* Read key from current input stream */

continue_input:

    do {

	if (A00083)
	    key = A00292 ();
	else
	    key = A00294 (timeout);

    } while (key == ZC_BAD);

    /* Verify mouse clicks */

    if (key == ZC_SINGLE_CLICK || key == ZC_DOUBLE_CLICK)
	if (!A00272 ())
	    goto continue_input;

    /* Copy key to the command file */

    if (A00082 && !A00083)
	A00277 (key);

    /* Handle timeouts */

    if (key == ZC_TIME_OUT)
	if (A00269 (routine) == 0)
	    goto continue_input;

    /* Handle hot keys */

    if (hot_keys && key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX) {

	if (A00035 == V4 && key == ZC_HKEY_UNDO)
	    goto continue_input;
	if (!A00271 (key))
	    goto continue_input;

	return ZC_BAD;

    }

    /* Return key */

    return key;

}/* A00256 */


/*
 * A00257
 *
 * Read a line of input from the current input stream.
 *
 */
zchar A00257 ( int max, zchar *buf,
			  zword timeout, zword routine,
			  bool hot_keys,
			  bool no_scripting )
{
    zchar key = ZC_BAD;

    A00194 ();

    /* Remove initial input from the transcript file or from the screen */

    if (A00080 && A00090 && !no_scripting)
	A00283 (buf);
    if (A00083)
	A00289 (buf);

    /* Read input line from current input stream */

continue_input:

    do {

	if (A00083)
	    key = A00293 (buf);
	else
	    key = A00295 (max, buf, timeout, key != ZC_BAD);

    } while (key == ZC_BAD);

    /* Verify mouse clicks */

    if (key == ZC_SINGLE_CLICK || key == ZC_DOUBLE_CLICK)
	if (!A00272 ())
	    goto continue_input;

    /* Copy input line to the command file */

    if (A00082 && !A00083)
	A00278 (buf, key);

    /* Handle timeouts */

    if (key == ZC_TIME_OUT)
	if (A00269 (routine) == 0)
	    goto continue_input;

    /* Handle hot keys */

    if (hot_keys && key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX) {

	if (!A00271 (key))
	    goto continue_input;

	return ZC_BAD;

    }

    /* Copy input line to transcript file or to the screen */

    if (A00080 && A00090 && !no_scripting)
	A00282 (buf, key);
    if (A00083)
	A00288 (buf, key);

    /* Return terminating key */

    return key;

}/* A00257 */
