/*
 * buffer.c
 *
 * Text buffering and word wrapping
 *
 */

#include "frotz.h"

extern void A00000 (zchar);
extern void A00001 (const zchar *);
extern void A00002 (void);

static zchar buffer[TEXT_BUFFER_SIZE];
static bufpos = 0;

static zchar prev_c = 0;

/*
 * A00184
 *
 * Copy the contents of the text buffer to the output streams.
 *
 */

void A00184 (void)
{
    static bool locked = FALSE;

    /* Make sure we stop when A00184 is called from A00184.
       Note that this is difficult to avoid as we might print a newline
       during A00184, which might cause a newline interrupt, that
       might execute any arbitrary opcode, which might flush the buffer. */

    if (locked || bufpos == 0)
	return;

    /* Send the buffer to the output streams */

    buffer[bufpos] = 0;

    locked = TRUE; A00001 (buffer); locked = FALSE;

    /* Reset the buffer */

    bufpos = 0;
    prev_c = 0;

}/* A00184 */

/*
 * A00186
 *
 * High level output function.
 *
 */

void A00186 (zchar c)
{
    static bool flag = FALSE;

    if (A00070 || A00067 || A00076) {

	if (!flag) {

	    /* Characters 0 and ZC_RETURN are special cases */

	    if (c == ZC_RETURN)
		{ A00185 (); return; }
	    if (c == 0)
		return;

	    /* Flush the buffer before a whitespace or after a hyphen */

	    if (c == ' ' || c == ZC_INDENT || c == ZC_GAP || prev_c == '-' && c != '-')
		A00184 ();

	    /* Set the flag if this is part one of a style or font change */

	    if (c == ZC_NEW_FONT || c == ZC_NEW_STYLE)
		flag = TRUE;

	    /* Remember the current character code */

	    prev_c = c;

	} else flag = FALSE;

	/* Insert the character into the buffer */

	buffer[bufpos++] = c;

	if (bufpos == TEXT_BUFFER_SIZE)
	    A00192 ("Text buffer overflow");

    } else A00000 (c);

}/* A00186 */

/*
 * A00185
 *
 * High level newline function.
 *
 */

void A00185 (void)
{

    A00184 (); A00002 ();

}/* A00185 */
