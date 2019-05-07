/*
 * stream.c
 *
 * IO stream implementation
 *
 */

#include "frotz.h"

extern bool A00243 (zchar);

extern bool A00244 (void);

extern void A00228 (void);
extern void A00229 (void);
extern void A00245 (zword, zword, bool);
extern void A00246 (void);
extern void A00230 (void);
extern void A00231 (void);
extern void A00020 (void);
extern void A00021 (void);

extern void A00247 (const zchar *);
extern void A00248 (void);
extern void A00249 (zchar);
extern void A00250 (const zchar *, zchar);
extern void A00251 (zchar);
extern void A00252 (const zchar *);
extern void A00253 (void);
extern void A00254 (const zchar *, zchar);
extern void A00255 (const zchar *);
extern void A00256 (void);
extern void A00257 (void);
extern void A00258 (zchar);
extern void A00259 (const zchar *);
extern void A00260 (void);
extern void A00261 (const zchar *, zchar);
extern void A00262 (const zchar *);
extern void A00263 (void);
extern void A00264 (void);

extern zchar A00265 (void);
extern zchar A00266 (zchar *);
extern zchar A00267 (zword);
extern zchar A00268 (int, zchar *, zword, bool);

extern int A00242 (zword);

/*
 * A00190
 *
 * Start printing a "debugging" A00070.
 *
 */

void A00190 (void)
{

    A00184 ();

    if (A00065)
	A00263 ();
    if (A00066 && A00074)
	A00256 ();

    A00070 = TRUE;

}/* A00190 */

/*
 * A00191
 *
 * Stop printing a "debugging" A00070.
 *
 */

void A00191 (void)
{

    A00184 ();

    if (A00065)
	A00264 ();
    if (A00066 && A00074)
	A00257 ();

    A00070 = FALSE;

}/* A00191 */

/*
 * A00126, open or close an output stream.
 *
 *	zargs[0] = stream to open (positive) or close (negative)
 *	zargs[1] = address to redirect output to (stream 3 only)
 *	zargs[2] = width of redirected output (stream 3 only, optional)
 *
 */

void A00126 (void)
{

    A00184 ();

    switch ((short) zargs[0]) {

    case  1: A00065 = TRUE;
	     break;
    case -1: A00065 = FALSE;
	     break;
    case  2: if (!A00066) A00020 ();
	     break;
    case -2: if (A00066) A00021 ();
	     break;
    case  3: A00245 (zargs[1], zargs[2], zargc >= 3);
	     break;
    case -3: A00246 ();
	     break;
    case  4: if (!A00068) A00230 ();
	     break;
    case -4: if (A00068) A00231 ();
	     break;

    }

}/* A00126 */

/*
 * A00000
 *
 * Send a single character to the output stream.
 *
 */

void A00000 (zchar c)
{

    if (A00065)
	A00258 (c);
    if (A00066 && A00074)
	A00251 (c);

}/* A00000 */

/*
 * A00001
 *
 * Send a string of characters to the output streams.
 *
 */

void A00001 (const zchar *s)
{

    if (A00067 && !A00070)

	A00247 (s);

    else {

	if (A00065)
	    A00259 (s);
	if (A00066 && A00074)
	    A00252 (s);

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

    if (A00067 && !A00070)

	A00248 ();

    else {

	if (A00065)
	    A00260 ();
	if (A00066 && A00074)
	    A00253 ();

    }

}/* A00002 */

/*
 * A00115, select an input stream.
 *
 *	zargs[0] = input stream to be selected
 *
 */

void A00115 (void)
{

    A00184 ();

    if (zargs[0] == 0 && A00069)
	A00229 ();
    if (zargs[0] == 1 && !A00069)
	A00228 ();

}/* A00115 */

/*
 * A00233
 *
 * Read a single keystroke from the current input stream.
 *
 */

zchar A00233 ( zword timeout, zword routine,
			bool hot_keys )
{
    zchar key = ZC_BAD;

    A00184 ();

    /* Read key from current input stream */

continue_input:

    do {

	if (A00069)
	    key = A00265 ();
	else
	    key = A00267 (timeout);

    } while (key == ZC_BAD);

    /* Verify mouse clicks */

    if (key == ZC_SINGLE_CLICK || key == ZC_DOUBLE_CLICK)
	if (!A00244 ())
	    goto continue_input;

    /* Copy key to the command file */

    if (A00068 && !A00069)
	A00249 (key);

    /* Handle timeouts */

    if (key == ZC_TIME_OUT)
	if (A00242 (routine) == 0)
	    goto continue_input;

    /* Handle hot keys */

    if (hot_keys && key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX) {

	if (A00025 == V4 && key == ZC_HKEY_UNDO)
	    goto continue_input;
	if (!A00243 (key))
	    goto continue_input;

	return ZC_BAD;

    }

    /* Return key */

    return key;

}/* A00233 */

/*
 * A00234
 *
 * Read a line of input from the current input stream.
 *
 */

zchar A00234 ( int max, zchar *buf,
			  zword timeout, zword routine,
			  bool hot_keys,
			  bool no_scripting )
{
    zchar key = ZC_BAD;

    A00184 ();

    /* Remove initial input from the transscript file or from the screen */

    if (A00066 && A00074 && !no_scripting)
	A00255 (buf);
    if (A00069)
	A00262 (buf);

    /* Read input line from current input stream */

continue_input:

    do {

	if (A00069)
	    key = A00266 (buf);
	else
	    key = A00268 (max, buf, timeout, key != ZC_BAD);

    } while (key == ZC_BAD);

    /* Verify mouse clicks */

    if (key == ZC_SINGLE_CLICK || key == ZC_DOUBLE_CLICK)
	if (!A00244 ())
	    goto continue_input;

    /* Copy input line to the command file */

    if (A00068 && !A00069)
	A00250 (buf, key);

    /* Handle timeouts */

    if (key == ZC_TIME_OUT)
	if (A00242 (routine) == 0)
	    goto continue_input;

    /* Handle hot keys */

    if (hot_keys && key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX) {

	if (!A00243 (key))
	    goto continue_input;

	return ZC_BAD;

    }

    /* Copy input line to transscript file or to the screen */

    if (A00066 && A00074 && !no_scripting)
	A00254 (buf, key);
    if (A00069)
	A00261 (buf, key);

    /* Return terminating key */

    return key;

}/* A00234 */
