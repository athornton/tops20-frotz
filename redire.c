/*
 * redirect.c
 *
 * Output redirection to Z-machine memory
 *
 */

#include "frotz.h"

#define MAX_NESTING 16

extern zword A00239 (zword);

static depth = -1;

static struct {
    zword xsize;
    zword table;
    zword width;
    zword total;
} redirect[MAX_NESTING];

/*
 * A00244
 *
 * Begin output redirection to the memory of the Z-machine.
 *
 */

void A00244 (zword table, zword xsize, bool buffering)
{

    if (++depth < MAX_NESTING) {

	if (!buffering)
	    xsize = 0xffff;
	if (buffering && (short) xsize <= 0)
	    xsize = A00239 ((zword) (- (short) xsize));

	A00195 (table, 0);

	redirect[depth].table = table;
	redirect[depth].width = 0;
	redirect[depth].total = 0;
	redirect[depth].xsize = xsize;

	A00067 = TRUE;

   } else A00192 ("Nesting stream #3 too deep");

}/* A00244 */

/*
 * A00247
 *
 * Redirect a newline to the memory of the Z-machine.
 *
 */

void A00247 (void)
{
    zword size;
    zword addr;

    redirect[depth].total += redirect[depth].width;
    redirect[depth].width = 0;

    addr = redirect[depth].table;

    size=lw(addr);
    addr += 2;

    if (redirect[depth].xsize != 0xffff) {

	redirect[depth].table = addr + size;
	size = 0;

    } else A00194 ((zword) (addr + (size++)), 13);

    A00195 (redirect[depth].table, size);

}/* A00247 */

/*
 * A00246
 *
 * Redirect a string of characters to the memory of the Z-machine.
 *
 */

void A00246 (const zchar *s)
{
    zword size;
    zword addr;
    zchar c;

    if (A00025 == V6) {

	int width = A00224 (s);

	if (redirect[depth].xsize != 0xffff)

	    if (redirect[depth].width + width > redirect[depth].xsize) {

		if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
		    width = A00224 (++s);

		A00247 ();

	    }

	redirect[depth].width += width;

    }

    addr = redirect[depth].table;

    size=lw(addr);
    addr += 2;

    while ((c = *s++) != 0)
	A00194 ((zword) (addr + (size++)), A00183 (c));

    A00195 (redirect[depth].table, size);

}/* A00246 */

/*
 * A00245
 *
 * End of output redirection.
 *
 */

void A00245 (void)
{

    if (depth >= 0) {

	if (redirect[depth].xsize != 0xffff)
	    A00247 ();

	if (A00025 == V6) {

	    A00052 = (redirect[depth].xsize != 0xffff) ?
		redirect[depth].total : redirect[depth].width;

	    sw(H_LINE_WIDTH, A00052);

	}

	if (depth == 0)
	    A00067 = FALSE;

	depth--;

    }

}/* A00245 */
