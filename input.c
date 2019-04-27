/*
 * input.c
 *
 * High level input functions
 *
 */

#include "frotz.h"

extern int A00231 (void);

extern zchar A00232 (zword, zword, bool);
extern zchar A00233 (int, zchar *, zword, zword, bool, bool);

extern void A00234 (zword, zword, zword, bool);

/*
 * A00003
 *
 * Check if the given key is an input terminator.
 *
 */

bool A00003 (zchar key)
{

    if (key == ZC_TIME_OUT)
	return TRUE;
    if (key == ZC_RETURN)
	return TRUE;
    if (key >= ZC_HKEY_MIN && key <= ZC_HKEY_MAX)
	return TRUE;

    if (A00051 != 0)

	if (key >= ZC_ARROW_MIN && key <= ZC_MENU_CLICK) {

	    zword addr = A00051;
	    zbyte c;

	    do {
		LOW_BYTE (addr, c)
		if (c == 255 || key == A00182 (c))
		    return TRUE;
		addr++;
	    } while (c != 0);

	}

    return FALSE;

}/* A00003 */

/*
 * A00122, add or remove a menu and A00193 if successful.
 *
 * 	zargs[0] = number of menu
 *	zargs[1] = table of menu entries or 0 to remove menu
 *
 */

void A00122 (void)
{

    /* This opcode was only used for the Macintosh version of Journey.
       It controls menus with numbers greater than 2 (menus 0, 1 and 2
       are system menus). Frotz doesn't implement menus yet. */

    A00193 (FALSE);

}/* A00122 */

/*
 * A00023
 *
 * Ask the user a question; return true if the answer is yes.
 *
 */

bool A00023 (const char *s)
{
    zchar key;

    A00189 (s);
    A00189 ("? (y/n) >");

    key = A00232 (0, 0, FALSE);

    if (key == 'y' || key == 'Y') {
	A00189 ("y\n");
	return TRUE;
    } else {
	A00189 ("n\n");
	return FALSE;
    }

}/* A00023 */

/*
 * read_string
 *
 * Read a string from the current input stream.
 *
 */

void read_string (int max, zchar *buffer)
{
    zchar key;

    buffer[0] = 0;

    do {

	key = A00233 (max, buffer, 0, 0, FALSE, FALSE);

    } while (key != ZC_RETURN);

}/* read_string */

/*
 * A00226
 *
 * Ask the user to type in a number and return it.
 *
 */

int A00226 (void)
{
    zchar buffer[6];
    int value = 0;
    int i;

    read_string (5, buffer);

    for (i = 0; buffer[i] != 0; i++)
	if (buffer[i] >= '0' && buffer[i] <= '9')
	    value = 10 * value + buffer[i] - '0';

    return value;

}/* A00226 */

/*
 * A00148, read a line of input and (in V5+) store the terminating key.
 *
 *	zargs[0] = address of text buffer
 *	zargs[1] = address of token buffer
 *	zargs[2] = timeout in tenths of a second (optional)
 *	zargs[3] = packed address of routine to be called on timeout
 *
 */

void A00148 (void)
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

    LOW_BYTE (addr, max)

    if (A00025 <= V4)
	max--;

    if (max >= INPUT_BUFFER_SIZE)
	max = INPUT_BUFFER_SIZE - 1;

    /* Get initial input size */

    if (A00025 >= V5) {
	addr++;
	LOW_BYTE (addr, size)
    } else size = 0;

    /* Copy initial input to local buffer */

    for (i = 0; i < size; i++) {
	addr++;
	LOW_BYTE (addr, c)
	buffer[i] = A00182 (c);
    }

    buffer[i] = 0;

    /* Draw status line for V1 to V3 games */

    if (A00025 <= V3)
	A00169 ();

    /* Read input from current input stream */

    key = A00233 (
	max, buffer,		/* buffer and size */
	zargs[2],		/* timeout value   */
	zargs[3],		/* timeout routine */
	TRUE,	        	/* enable hot keys */
	A00025 == V6);	/* no script in V6 */

    if (key == ZC_BAD)
	return;

    /* Perform A00231 for V1 to V4 games */

    if (A00025 <= V4)
	A00231 ();

    /* Copy local buffer back to dynamic memory */

    for (i = 0; buffer[i] != 0; i++) {

	if (key == ZC_RETURN) {

	    if (buffer[i] >= 'A' && buffer[i] <= 'Z')
		buffer[i] += 'a' - 'A';
	    if (buffer[i] >= 0xc0 && buffer[i] <= 0xde && buffer[i] != 0xd7)
		buffer[i] += 0x20;

	}

	A00194 ((zword) (zargs[0] + ((A00025 <= V4) ? 1 : 2) + i), A00183 (buffer[i]));

    }

    /* Add null character (V1-V4) or write input length into 2nd byte */

    if (A00025 <= V4)
	A00194 ((zword) (zargs[0] + 1 + i), 0);
    else
	A00194 ((zword) (zargs[0] + 1), i);

    /* Tokenise line if a token buffer is present */

    if (key == ZC_RETURN && zargs[1] != 0)
	A00234 (zargs[0], zargs[1], 0, FALSE);

    /* Store key */

    if (A00025 >= V5)
	store (A00183 (key));

}/* A00148 */

/*
 * A00149, read and store a key.
 *
 *	zargs[0] = input device (must be 1)
 *	zargs[1] = timeout in tenths of a second (optional)
 *	zargs[2] = packed address of routine to be called on timeout
 *
 */

void A00149 (void)
{
    zchar key;

    /* Supply default arguments */

    if (zargc < 2)
	zargs[1] = 0;

    /* Read input from the current input stream */

    key = A00232 (
	zargs[1],	/* timeout value   */
	zargs[2],	/* timeout routine */
	TRUE);  	/* enable hot keys */

    if (key == ZC_BAD)
	return;

    /* Store key */

    store (A00183 (key));

}/* A00149 */

/*
 * A00150, write the current mouse status into a table.
 *
 *	zargs[0] = address of table
 *
 */

void A00150 (void)
{

    A00195 ((zword) (zargs[0] + 0), A00060);
    A00195 ((zword) (zargs[0] + 2), A00059);
    A00195 ((zword) (zargs[0] + 4), 1);		/* mouse button bits */
    A00195 ((zword) (zargs[0] + 6), 0);		/* menu selection */

}/* A00150 */
