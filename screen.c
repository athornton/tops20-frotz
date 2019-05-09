/* screen.c - Generic screen manipulation
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

extern void A00268 (int, zword);

extern int A00269 (zword);

static struct {
    enum story A00075;
    int pic;
    int pic1;
    int pic2;
} mapper[] = {
    { ZORK_ZERO,  5, 497, 498 },
    { ZORK_ZERO,  6, 501, 502 },
    { ZORK_ZERO,  7, 499, 500 },
    { ZORK_ZERO,  8, 503, 504 },
    {    ARTHUR, 54, 170, 171 },
    {    SHOGUN, 50,  61,  62 },
    {   UNKNOWN,  0,   0,   0 }
};

/* These are usually out of date.  Always update before using. */
static int font_height = 1;
static int font_width = 1;

static bool input_redraw = FALSE;
static bool more_prompts = TRUE;
static bool discarding = FALSE;
static bool cursor = TRUE;

static int input_window = 0;

static Zwindow wp[8], *cwp = wp;

Zwindow * curwinrec() { return cwp;}


/*
 * winarg0
 *
 * Return the window number in zargs[0]. In V6 only, -3 refers to the
 * current window.
 *
 */
static zword winarg0 (void)
{
    if (A00035 == V6 && A00247( zargs[0] ) == -3)
	return cwin;

    if (zargs[0] >= ((A00035 == V6) ? 8 : 2))
	A00188 (ERR_ILL_WIN);

    return zargs[0];

}/* winarg0 */


/*
 * winarg2
 *
 * Return the (optional) window number in zargs[2]. -3 refers to the
 * current window. This optional window number was only used by some
 * V6 opcodes: set_cursor, set_margins, set_colour.
 *
 */
static zword winarg2 (void)
{
    if (zargc < 3 || A00247( zargs[2] ) == -3)
	return cwin;

    if (zargs[2] >= 8)
	A00188 (ERR_ILL_WIN);

    return zargs[2];

}/* winarg2 */


/*
 * update_cursor
 *
 * Move the hardware cursor to make it match the window properties.
 *
 */
static void update_cursor (void)
{
    A00233 (
	cwp->y_pos + cwp->y_cursor - 1,
	cwp->x_pos + cwp->x_cursor - 1);

}/* update_cursor */


/*
 * reset_cursor
 *
 * Reset the cursor of a given window to its initial position.
 *
 */
static void reset_cursor (zword win)
{
    int lines = 0;

    if (A00035 <= V4 && win == 0)
	lines = wp[0].y_size / hi (wp[0].font_size) - 1;

    wp[win].y_cursor = hi (wp[0].font_size) * lines + 1;
    wp[win].x_cursor = wp[win].left + 1;

    if (win == cwin)
	update_cursor ();

}/* reset_cursor */


/*
 * A00031
 *
 * Turn more prompts on/off.
 *
 */
void A00031 (bool flag)
{
    if (flag && !more_prompts)
	cwp->line_count = 0;

    more_prompts = flag;

}/* A00031 */


/*
 * units_left
 *
 * Return the #screen units from the cursor to the end of the line.
 *
 */
static int units_left (void)
{
    return cwp->x_size - cwp->right - cwp->x_cursor + 1;

}/* units_left */


/*
 * A00267
 *
 * Return maximum width of a line in the given window. This is used in
 * connection with the extended output stream #3 call in V6.
 *
 */
zword A00267 (zword win)
{
    if (A00035 == V6) {

	if (win >= 8)
	    A00188 (ERR_ILL_WIN);

	return wp[win].x_size - wp[win].left - wp[win].right;

    } else return 0xffff;

}/* A00267 */


/*
 * countdown
 *
 * Decrement the newline counter. Call the newline interrupt when the
 * counter hits zero. This is a helper function for A00208.
 *
 */
static void countdown (void)
{
    if (cwp->nl_countdown != 0)
	if (--cwp->nl_countdown == 0)
	    A00269 (cwp->nl_routine);

}/* countdown */


/*
 * A00208
 *
 * Print a newline to the screen.
 *
 */

void A00208 (void)
{
    if (discarding) return;

    /* Handle newline interrupts at the start (for most cases) */

    if (A00049 != INTERP_MSDOS || A00075 != ZORK_ZERO || A00037 != 393)
	countdown ();

    /* Check whether the last input line gets destroyed */

    if (input_window == cwin)
	input_redraw = TRUE;

    /* If the cursor has not reached the bottom line, then move it to
       the next line; otherwise scroll the window or reset the cursor
       to the top left. */

    cwp->x_cursor = cwp->left + 1;

    A00217(0, &font_height, &font_width);
    if (cwp->y_cursor + 2 * font_height - 1 > cwp->y_size)

	if (A00091) {

	    zword y = cwp->y_pos;
	    zword x = cwp->x_pos;

	    A00231 (y,
			    x,
			    y + cwp->y_size - 1,
			    x + cwp->x_size - 1,
			    font_height);

	} else cwp->y_cursor = 1;

    else cwp->y_cursor += font_height;

    update_cursor ();

    /* See if we need to print a more prompt (unless the game has set
       the line counter to -999 in order to suppress more prompts). */

    if (A00091 && A00247( cwp->line_count ) != -999) {

	zword above = (cwp->y_cursor - 1) / font_height;
	zword below = (cwp->y_size - cwp->y_cursor + 1) / font_height;

	cwp->line_count++;

	if (A00247( cwp->line_count ) >= A00247( above ) + below - 1) {

	    if (more_prompts)
		A00220 ();

	    cwp->line_count = A00003.context_lines;

	}

    }

    /* Handle newline interrupts at the end for Zork Zero under DOS */

    if (A00049 == INTERP_MSDOS && A00075 == ZORK_ZERO && A00037 == 393)
	countdown ();

}/* A00208 */


/*
 * A00286
 *
 * Display a single character on the screen.
 *
 */

void A00286 (zchar c)
{
    int width;

    if (discarding) return;

    if (c == ZC_INDENT && cwp->x_cursor != cwp->left + 1)
	c = ' ';

    if (units_left () < (width = A00210 (c))) {

	if (!A00089)
	    { cwp->x_cursor = cwp->x_size - cwp->right; return; }

	A00208 ();

    }

    A00211 (c); cwp->x_cursor += width;

}/* A00286 */


/*
 * A00287
 *
 * Display a string of characters on the screen. If the word doesn't fit
 * then use wrapping or clipping depending on the current setting of the
 * A00089 flag.
 *
 */
void A00287 (const zchar *s)
{
    int width;

    if (discarding) return;

    if (*s == ZC_INDENT && cwp->x_cursor != cwp->left + 1)
	A00286 (*s++);

    if (units_left () < (width = A00240 (s))) {

	if (!A00089) {

	    zchar c;

	    while ((c = *s++) != 0)

		if (c == ZC_NEW_FONT || c == ZC_NEW_STYLE) {

		    int arg = (int) *s++;

		    if (c == ZC_NEW_FONT)
			A00234 (arg);
		    if (c == ZC_NEW_STYLE)
			A00235 (arg);

		} else A00286 (c);

	    return;

	}

	if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
	    width = A00240 (++s);

#ifdef AMIGA
	if (cwin == 0) Justifiable ();
#endif

	A00208 ();

    }

    A00212 (s); cwp->x_cursor += width;

}/* A00287 */


/*
 * A00288
 *
 * Display an input line on the screen. This is required during playback.
 *
 */
void A00288 (const zchar *buf, zchar key)
{
    int width;

    if (units_left () < (width = A00240 (buf)))
	A00208 ();

    A00212 (buf); cwp->x_cursor += width;

    if (key == ZC_RETURN)
	A00208 ();

}/* A00288 */


/*
 * A00289
 *
 * Remove an input line that has already been printed from the screen
 * as if it was deleted by the player. This could be necessary during
 * playback.
 *
 */
void A00289 (const zchar *buf)
{
    if (buf[0] != 0) {

	int width = A00240 (buf);

	zword y;
	zword x;

	cwp->x_cursor -= width;

	y = cwp->y_pos + cwp->y_cursor - 1;
	x = cwp->x_pos + cwp->x_cursor - 1;

	A00217(0, &font_height, &font_width);
	A00214 (y, x, y + font_height - 1, x + width - 1, -1);
	A00233 (y, x);

    }

}/* A00289 */


/*
 * A00295
 *
 * Read an input line from the keyboard and return the terminating key.
 *
 */
zchar A00295 (int max, zchar *buf, zword timeout, bool continued)
{
    zchar key;
    int i;

    /* Make sure there is some space for input */

    if (cwin == 0 && units_left () + A00240 (buf) < 10 * font_width)
	A00208 ();

    /* Make sure the input line is visible */

    if (continued && input_redraw)
	A00288 (buf, -1);

    input_window = cwin;
    input_redraw = FALSE;

    /* Get input line from IO interface */

    cwp->x_cursor -= A00240 (buf);
    key = A00228 (max, buf, timeout, units_left (), continued);
    cwp->x_cursor += A00240 (buf);

    if (key != ZC_TIME_OUT)
	for (i = 0; i < 8; i++)
	    wp[i].line_count = 0;

    /* Add a newline if the input was terminated normally */

    if (key == ZC_RETURN)
	A00208 ();

    return key;

}/* A00295 */


/*
 * A00294
 *
 * Read a single keystroke and return it.
 *
 */
zchar A00294 (zword timeout)
{
    zchar key;
    int i;

    key = A00227 (timeout, cursor);

    if (key != ZC_TIME_OUT)
	for (i = 0; i < 8; i++)
	    wp[i].line_count = 0;

    return key;

}/* A00294 */


/*
 * update_attributes
 *
 * Set the three enable_*** variables to make them match the attributes
 * of the current window.
 *
 */
static void update_attributes (void)
{
    zword attr = cwp->attribute;

    A00089 = attr & 1;
    A00091 = attr & 2;
    A00090 = attr & 4;
    A00092 = attr & 8;

    /* Some story files forget to select wrapping for printing hints */

    if (A00075 == ZORK_ZERO && A00037 == 366)
	if (cwin == 0)
	    A00089 = TRUE;
    if (A00075 == SHOGUN && A00037 <= 295)
	if (cwin == 0)
	    A00089 = TRUE;

}/* update_attributes */


/*
 * A00024
 *
 * Set the right text style. This can be necessary when the fixed font
 * flag is changed, or when a new window is selected, or when the game
 * uses the set_text_style opcode.
 *
 */
void A00024 (void)
{
    zword style;

    if (A00035 != V6) {

	style = wp[0].style;

	if (cwin != 0 || A00044 & FIXED_FONT_FLAG)
	    style |= FIXED_WIDTH_STYLE;

    } else style = cwp->style;

    if (!A00081 && A00079 && A00092) {

	A00196 (ZC_NEW_STYLE);
	A00196 (style);

    } else A00235 (style);

}/* A00024 */


/*
 * set_window
 *
 * Set the current window. In V6 every window has its own set of window
 * properties such as colours, text style, cursor position and size.
 *
 */
static void set_window (zword win)
{
    A00194 ();

    cwin = win; cwp = wp + win;

    update_attributes ();

    if (A00035 == V6) {

	A00232 (lo (cwp->colour), hi (cwp->colour));

	if (A00217 (cwp->font, &font_height, &font_width))
	    A00234 (cwp->font);

	A00235 (cwp->style);

    } else A00024 ();

    if (A00035 != V6 && win != 0) {
	wp[win].y_cursor = 1;
	wp[win].x_cursor = 1;
    }

    update_cursor ();

}/* set_window */


/*
 * A00030
 *
 * Erase a window to background colour.
 *
 */
void A00030 (zword win)
{
    zword y = wp[win].y_pos;
    zword x = wp[win].x_pos;

    if (A00035 == V6 && win != cwin && A00049 != INTERP_AMIGA)
	A00232 (lo (wp[win].colour), hi (wp[win].colour));

    A00214 (y,
		   x,
		   y + wp[win].y_size - 1,
		   x + wp[win].x_size - 1,
		   win);

    if (A00035 == V6 && win != cwin && A00049 != INTERP_AMIGA)
	A00232 (lo (cwp->colour), hi (cwp->colour));

    reset_cursor (win);

    wp[win].line_count = 0;

}/* A00030 */


/*
 * A00025
 *
 * Divide the screen into upper (1) and lower (0) windows. In V3 the upper
 * window appears below the status line.
 *
 */
void A00025 (zword height)
{
    zword stat_height = 0;

    A00194 ();

    /* Calculate height of status line and upper window */

    if (A00035 != V6)
	height *= hi (wp[1].font_size);

    if (A00035 <= V3)
	stat_height = hi (wp[7].font_size);

    /* Cursor of upper window mustn't be swallowed by the lower window */

    wp[1].y_cursor += wp[1].y_pos - 1 - stat_height;

    wp[1].y_pos = 1 + stat_height;
    wp[1].y_size = height;

    if (A00247( wp[1].y_cursor ) > A00247( wp[1].y_size) )
	reset_cursor (1);

    /* Cursor of lower window mustn't be swallowed by the upper window */

    wp[0].y_cursor += wp[0].y_pos - 1 - stat_height - height;

    wp[0].y_pos = 1 + stat_height + height;
    wp[0].y_size = A00054 - stat_height - height;

    if (A00247( wp[0].y_cursor ) < 1)
	reset_cursor (0);

    /* Erase the upper window in V3 only */

    if (A00035 == V3 && height != 0)
	A00030 (1);

}/* A00025 */


/*
 * erase_screen
 *
 * Erase the entire screen to background colour.
 *
 */
static void erase_screen (zword win)
{
    int i;

    A00214 (1, 1, A00054, A00053, -2);

    if (A00247( win ) == -1) {
	A00025 (0);
	set_window (0);
	reset_cursor (0);
    }

    for (i = 0; i < 8; i++)
	wp[i].line_count = 0;

}/* erase_screen */


/*
 * A00245
 *
 * Try to adapt the window properties to a new screen size.
 *
 */
void A00245 (void)
{
    /* V6 games are asked to redraw.  Other versions have no means for that
       so we do what we can. */
    if (A00035 == V6)
        A00044 |= REFRESH_FLAG;
    else {
        int scroll, h;

	wp[0].x_size = A00053;
	if (wp[0].x_cursor > A00053)
	    wp[0].x_cursor = A00053;
	wp[1].x_size = A00053;
        if (wp[1].x_cursor > A00053)
            wp[1].x_cursor = A00053;
	wp[7].x_size = A00053;
        if (wp[7].x_cursor > A00053)
            wp[7].x_cursor = A00053;

        h = A00054 - wp[1].y_size - wp[7].y_size;
        if (h > 0) {
            wp[0].y_size = h;
            scroll = wp[0].y_cursor - wp[0].y_size;
        } else {
            /* Just make a one line window at the bottom of the screen. */
            /*XXX We should probably adjust the other windows.  But how? */
            wp[0].y_size = 1;
            scroll = wp[0].y_pos + wp[0].y_cursor - A00054 - 1;
            wp[0].y_pos = A00054;
        }
        if (scroll > 0) {
            wp[0].y_cursor = wp[0].y_size;
            A00246(0, wp[0].y_pos + scroll, wp[0].y_pos,
                              wp[0].x_pos, wp[0].y_size, wp[0].x_size);
        }
    }

}/* A00245 */


/*
 * A00023
 *
 * Prepare the screen for a new game.
 *
 */
void A00023 (void)
{
    /* Use default settings */

    A00232 (A00060, A00059);

    if (A00217 (TEXT_FONT, &font_height, &font_width))
	A00234 (TEXT_FONT);

    A00235 (0);

    cursor = TRUE;

    /* Initialise window properties */

    mwin = 1;

    for (cwp = wp; cwp < wp + 8; cwp++) {
	cwp->y_pos = 1;
	cwp->x_pos = 1;
	cwp->y_size = 0;
	cwp->x_size = 0;
	cwp->y_cursor = 1;
	cwp->x_cursor = 1;
	cwp->left = 0;
	cwp->right = 0;
	cwp->nl_routine = 0;
	cwp->nl_countdown = 0;
	cwp->style = 0;
	cwp->colour = (A00059 << 8) | A00060;
	cwp->font = TEXT_FONT;
	cwp->font_size = (font_height << 8) | font_width;
	cwp->attribute = 8;
    }

    /* Prepare lower/upper windows and status line */

    wp[0].attribute = 15;

    wp[0].left = A00003.left_margin;
    wp[0].right = A00003.right_margin;

    wp[0].x_size = A00053;
    wp[1].x_size = A00053;

    if (A00035 <= V3)
	wp[7].x_size = A00053;

    A00230 (RESTART_WPROP_SET);

    /* Clear the screen, unsplit it and select window 0 */

    erase_screen ((zword) (-1));

}/* A00023 */


/*
 * A00272
 *
 * Return false if the last mouse click occured outside the current
 * mouse window; otherwise write the mouse arrow coordinates to the
 * memory of the header extension table and return true.
 *
 */
bool A00272 (void)
{

    if (mwin >= 0) {

	if (A00086 < wp[mwin].y_pos || A00086 >= wp[mwin].y_pos + wp[mwin].y_size)
	    return FALSE;
	if (A00085 < wp[mwin].x_pos || A00085 >= wp[mwin].x_pos + wp[mwin].x_size)
	    return FALSE;

	A00070 = A00086 - wp[mwin].y_pos + 1;
	A00069 = A00085 - wp[mwin].x_pos + 1;

    } else {

	if (A00086 < 1 || A00086 > A00054)
	    return FALSE;
	if (A00085 < 1 || A00085 > A00053)
	    return FALSE;

	A00070 = A00086;
	A00069 = A00085;

    }

    if (A00035 != V6) {
	A00070 = (A00070 - 1) / A00055 + 1;
	A00069 = (A00069 - 1) / A00056 + 1;
    }

    A00268 (HX_MOUSE_Y, A00070);
    A00268 (HX_MOUSE_X, A00069);

    return TRUE;

}/* A00272 */


/*
 * A00290
 *
 * Start printing a so-called debugging A00084. The contents of the
 * A00084 are passed to the A00084 stream, a Frotz specific output
 * stream with maximum priority.
 *
 */
void A00290 (void)
{
    if (cwin == 0) {		/* A00084s in window 0 only */

	A00235 (0);

	if (cwp->x_cursor != cwp->left + 1)
	    A00208 ();

	A00286 (ZC_INDENT);

    } else discarding = TRUE; 	/* discard A00084s in other windows */

}/* A00290 */


/*
 * A00291
 *
 * Stop printing a "debugging" A00084.
 *
 */
void A00291 (void)
{
    if (cwin == 0) {		/* A00084s in window 0 only */

	A00208 ();

	A00024 ();

    } else discarding = FALSE; 	/* A00084 has been discarded */

}/* A00291 */


/*
 * A00096, turn text buffering on/off.
 *
 *	zargs[0] = new text buffering flag (0 or 1)
 *
 */
void A00096 (void)
{
    /* Infocom's V6 games rarely use the buffer_mode opcode. If they do
       then only to print text immediately, without any delay. This was
       used to give the player some sign of life while the game was
       spending much time on parsing a complicated input line. (To turn
       off word wrapping, V6 games use the window_style opcode instead.)
       Today we can afford to ignore buffer_mode in V6. */

    if (A00035 != V6) {

	A00194 ();

	wp[0].attribute &= ~8;

	if (zargs[0] != 0)
	    wp[0].attribute |= 8;

	update_attributes ();

    }

}/* A00096 */


/*
 * A00105, draw a picture.
 *
 *	zargs[0] = number of picture to draw
 *	zargs[1] = y-coordinate of top left corner
 *	zargs[2] = x-coordinate of top left corner
 *
 */
void A00105 (void)
{
    zword pic = zargs[0];

    zword y = zargs[1];
    zword x = zargs[2];

    int i;

    A00194 ();

    if (y == 0)			/* use cursor line if y-coordinate is 0 */
	y = cwp->y_cursor;
    if (x == 0)    		/* use cursor column if x-coordinate is 0 */
	x = cwp->x_cursor;

    y += cwp->y_pos - 1;
    x += cwp->x_pos - 1;

    /* The following is necessary to make Amiga and Macintosh story
       files work with MCGA graphics files.  Some screen-filling
       pictures of the original Amiga release like the borders of
       Zork Zero were split into several MCGA pictures (left, right
       and top borders).  We pretend this has not happened. */

    for (i = 0; mapper[i].A00075 != UNKNOWN; i++)

	if (A00075 == mapper[i].A00075 && pic == mapper[i].pic) {

	    int height1, width1;
	    int height2, width2;

	    int delta = 0;

	    A00222 (pic, &height1, &width1);
	    A00222 (mapper[i].pic2, &height2, &width2);

	    if (A00075 == ARTHUR && pic == 54)
		delta = A00053 / 160;

	    A00213 (mapper[i].pic1, y + height1, x + delta);
	    A00213 (mapper[i].pic2, y + height1, x + width1 - width2 - delta);

	}

    A00213 (pic, y, x);

    if (A00075 == SHOGUN)

	if (pic == 3) {

	    int height, width;

	    A00222 (59, &height, &width);
	    A00213 (59, y, A00053 - width + 1);

	}

}/* A00105 */


/*
 * A00107, erase the line starting at the cursor position.
 *
 *	zargs[0] = 1 + #units to erase (1 clears to the end of the line)
 *
 */
void A00107 (void)
{
    zword pixels = zargs[0];
    zword y, x;

    A00194 ();

    /* Clipping at the right margin of the current window */

    if (--pixels == 0 || pixels > units_left ())
	pixels = units_left ();

    /* Erase from cursor position */

    y = cwp->y_pos + cwp->y_cursor - 1;
    x = cwp->x_pos + cwp->x_cursor - 1;

    A00217(0, &font_height, &font_width);
    A00214 (y, x, y + font_height - 1, x + pixels - 1, -1);

}/* A00107 */


/*
 * A00108, erase a picture with background colour.
 *
 *	zargs[0] = number of picture to erase
 *	zargs[1] = y-coordinate of top left corner (optional)
 *	zargs[2] = x-coordinate of top left corner (optional)
 *
 */
void A00108 (void)
{
    int height, width;

    zword y = zargs[1];
    zword x = zargs[2];

    A00194 ();

    if (y == 0)		/* use cursor line if y-coordinate is 0 */
	y = cwp->y_cursor;
    if (x == 0)    	/* use cursor column if x-coordinate is 0 */
	x = cwp->x_cursor;

    A00222 (zargs[0], &height, &width);

    y += cwp->y_pos - 1;
    x += cwp->x_pos - 1;

    A00214 (y, x, y + height - 1, x + width - 1, -1);

}/* A00108 */


/*
 * A00109, erase a window or the screen to background colour.
 *
 *	zargs[0] = window (-3 current, -2 screen, -1 screen & unsplit)
 *
 */
void A00109 (void)
{
    A00194 ();

    if (A00247( zargs[0] ) == -1 || A00247( zargs[0] ) == -2)
	erase_screen (zargs[0]);
    else
	A00030 (winarg0 ());

}/* A00109 */


/*
 * A00111, write the cursor coordinates into a table.
 *
 *	zargs[0] = address to write information to
 *
 */
void A00111 (void)
{
    zword y, x;

    A00194 ();

    y = cwp->y_cursor;
    x = cwp->x_cursor;

    if (A00035 != V6) {	/* convert to grid positions */
	y = (y - 1) / A00055 + 1;
	x = (x - 1) / A00056 + 1;
    }

    A00204 ((zword) (zargs[0] + 0), y);
    A00204 ((zword) (zargs[0] + 2), x);

}/* A00111 */


/*
 * A00118, store the value of a window property.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = number of window property to be stored
 *
 */
void A00118 (void)
{
    A00194 ();

    if (zargs[1] >= 16)
	A00188 (ERR_ILL_WIN_PROP);

    store (((zword *) (wp + winarg0 ())) [zargs[1]]);

}/* A00118 */


/*
 * A00128, select a window as mouse window.
 *
 *	zargs[0] = window number (-3 is the current) or -1 for the screen
 *
 */
void A00128 (void)
{
    mwin = (A00247( zargs[0] ) == -1) ? -1 : winarg0 ();

}/* A00128 */


/*
 * A00129, place a window on the screen.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = y-coordinate
 *	zargs[2] = x-coordinate
 *
 */
void A00129 (void)
{
    zword win = winarg0 ();

    A00194 ();

    wp[win].y_pos = zargs[1];
    wp[win].x_pos = zargs[2];

    if (win == cwin)
	update_cursor ();

}/* A00129 */


/*
 * A00132, get information on a picture or the graphics file.
 *
 *	zargs[0] = number of picture or 0 for the graphics file
 *	zargs[1] = address to write information to
 *
 */
void A00132 (void)
{
    zword pic = zargs[0];
    zword table = zargs[1];

    int height, width;
    int i;

    bool avail = A00222 (pic, &height, &width);

    for (i = 0; mapper[i].A00075 != UNKNOWN; i++)

	if (A00075 == mapper[i].A00075) {

	    if (pic == mapper[i].pic) {

		int height2, width2;

		avail &= A00222 (mapper[i].pic1, &height2, &width2);
		avail &= A00222 (mapper[i].pic2, &height2, &width2);

		height += height2;

	    } else if (pic == mapper[i].pic1 || pic == mapper[i].pic2)

		avail = FALSE;
	}

    A00204 ((zword) (table + 0), (zword) (height));
    A00204 ((zword) (table + 2), (zword) (width));

    A00202 (avail);

}/* A00132 */


/*
 * A00133, prepare a group of pictures for faster display.
 *
 *	zargs[0] = address of table holding the picture numbers
 *
 */
void A00133 (void)
{
    /* This opcode is used by Shogun and Zork Zero when the player
       encounters built-in games such as Peggleboz. Nowadays it is
       not very helpful to hold the picture data in memory because
       even a small disk cache avoids re-loading of data. */

}/* A00133 */


/*
 * A00144, print ASCII text in a rectangular area.
 *
 *	zargs[0] = address of text to be printed
 *	zargs[1] = width of rectangular area
 *	zargs[2] = height of rectangular area (optional)
 *	zargs[3] = number of char's to skip between lines (optional)
 *
 */
void A00144 (void)
{
    zword addr = zargs[0];
    zword x;
    int i, j;

    A00194 ();

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = 1;
    if (zargc < 4)
	zargs[3] = 0;

    /* Write text in width x height rectangle */

    x = cwp->x_cursor;

    for (i = 0; i < zargs[2]; i++) {

	if (i != 0) {

	    A00194 ();
	    A00217(0, &font_height, &font_width);
	    cwp->y_cursor += font_height;
	    cwp->x_cursor = x;

	    update_cursor ();

	}

	for (j = 0; j < zargs[1]; j++) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    A00196 (c);

	}

	addr += zargs[3];

    }

}/* A00144 */


/*
 * A00150, set the value of a window property.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = number of window property to set
 *	zargs[2] = value to set window property to
 *
 */
void A00150 (void)
{
    A00194 ();

    if (zargs[1] >= 16)
	A00188 (ERR_ILL_WIN_PROP);

    ((zword *) (wp + winarg0 ())) [zargs[1]] = zargs[2];

}/* A00150 */


/*
 * A00166, scroll a window up or down.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = #screen units to scroll up (positive) or down (negative)
 *
 */
void A00166 (void)
{
    zword win = winarg0 ();
    zword y, x;

    A00194 ();

    /* Use the correct set of colours when scrolling the window */

    if (win != cwin && A00049 != INTERP_AMIGA)
	A00232 (lo (wp[win].colour), hi (wp[win].colour));

    y = wp[win].y_pos;
    x = wp[win].x_pos;

    A00231 (y,
		    x,
		    y + wp[win].y_size - 1,
		    x + wp[win].x_size - 1,
		    A00247( zargs[1]) );

    if (win != cwin && A00049 != INTERP_AMIGA)
	A00232 (lo (cwp->colour), hi (cwp->colour));

}/* A00166 */


/*
 * A00169, set the foreground and background colours.
 *
 *	zargs[0] = foreground colour
 *	zargs[1] = background colour
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */
void A00169 (void)
{
    zword win = (A00035 == V6) ? winarg2 () : 0;

    zword fg = zargs[0];
    zword bg = zargs[1];

    A00194 ();

    if (A00247( fg ) == -1)	/* colour -1 is the colour at the cursor */
	fg = A00221 ();
    if (A00247( bg ) == -1)
	bg = A00221 ();

    if (fg == 0)		/* colour 0 means keep current colour */
	fg = lo (wp[win].colour);
    if (bg == 0)
	bg = hi (wp[win].colour);

    if (fg == 1)		/* colour 1 is the system default colour */
	fg = A00060;
    if (bg == 1)
	bg = A00059;

    if (A00035 == V6 && A00049 == INTERP_AMIGA)

	/* Changing colours of window 0 affects the entire screen */

	if (win == 0) {

	    int i;

	    for (i = 1; i < 8; i++) {

		zword bg2 = hi (wp[i].colour);
		zword fg2 = lo (wp[i].colour);

		if (bg2 < 16)
		    bg2 = (bg2 == lo (wp[0].colour)) ? fg : bg;
		if (fg2 < 16)
		    fg2 = (fg2 == lo (wp[0].colour)) ? fg : bg;

		wp[i].colour = (bg2 << 8) | fg2;

	    }

	}

    wp[win].colour = (bg << 8) | fg;

    if (win == cwin || A00035 != V6)
	A00232 (fg, bg);

}/* A00169 */


/*
 * A00168, set the font for text output and store the previous font.
 *
 * 	zargs[0] = number of font or 0 to keep current font
 *
 */
void A00168 (void)
{
    zword win = (A00035 == V6) ? cwin : 0;
    zword font = zargs[0];

    if (font != 0) {

	if (A00217 (font, &font_height, &font_width)) {

	    store (wp[win].font);

	    wp[win].font = font;
	    wp[win].font_size = (font_height << 8) | font_width;

	    if (!A00081 && A00079 && A00092) {

		A00196 (ZC_NEW_FONT);
		A00196 (font);

	    } else A00234 (font);

	} else store (0);

    } else store (wp[win].font);

}/* A00168 */


/*
 * A00170, set the cursor position or turn the cursor on/off.
 *
 *	zargs[0] = y-coordinate or -2/-1 for cursor on/off
 *	zargs[1] = x-coordinate
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */
void A00170 (void)
{
    zword win = (A00035 == V6) ? winarg2 () : 1;

    zword y = zargs[0];
    zword x = zargs[1];

    A00194 ();

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = -3;

    /* Handle cursor on/off */

    if (A00247( y ) < 0) {

	if (A00247( y ) == -2)
	    cursor = TRUE;
	if (A00247( y ) == -1)
	    cursor = FALSE;

	return;

    }

    /* Convert grid positions to screen units if this is not V6 */

    if (A00035 != V6) {

	if (cwin == 0)
	    return;

	y = (y - 1) * A00055 + 1;
	x = (x - 1) * A00056 + 1;

    }

    /* Protect the margins */

    if (y == 0)			/* use cursor line if y-coordinate is 0 */
	y = wp[win].y_cursor;
    if (x == 0)			/* use cursor column if x-coordinate is 0 */
	x = wp[win].x_cursor;
    if (x <= wp[win].left || x > wp[win].x_size - wp[win].right)
	x = wp[win].left + 1;

    /* Move the cursor */

    wp[win].y_cursor = y;
    wp[win].x_cursor = x;

    if (win == cwin)
	update_cursor ();

}/* A00170 */


/*
 * A00171, set the left and right margins of a window.
 *
 *	zargs[0] = left margin in pixels
 *	zargs[1] = right margin in pixels
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */
void A00171 (void)
{
    zword win = winarg2 ();

    A00194 ();

    wp[win].left = zargs[0];
    wp[win].right = zargs[1];

    /* Protect the margins */

    if (wp[win].x_cursor <= zargs[0] || wp[win].x_cursor > wp[win].x_size - zargs[1]) {

	wp[win].x_cursor = zargs[0] + 1;

	if (win == cwin)
	    update_cursor ();

    }

}/* A00171 */


/*
 * A00173, set the style for text output.
 *
 * 	zargs[0] = style flags to set or 0 to reset text style
 *
 */
void A00173 (void)
{
    zword win = (A00035 == V6) ? cwin : 0;
    zword style = zargs[0];

    wp[win].style |= style;

    if (style == 0)
	wp[win].style = 0;

    A00024 ();

}/* A00173 */


/*
 * A00172, select the current window.
 *
 *	zargs[0] = window to be selected (-3 is the current one)
 *
 */
void A00172 (void)
{
    set_window (winarg0 ());

}/* A00172 */


/*
 * pad_status_line
 *
 * Pad the status line with spaces up to the given position.
 *
 */
static void pad_status_line (int column)
{
    int spaces;

    A00194 ();

    spaces = units_left () / A00210 (' ') - column;

    /* while (spaces--) */
    /* Justin Wesley's fix for narrow displays (Agenda PDA) */
    while (spaces-- > 0)
	A00286 (' ');

}/* pad_status_line */


/*
 * A00174, display the status line for V1 to V3 games.
 *
 *	no zargs used
 *
 */
void A00174 (void)
{
    zword global0;
    zword global1;
    zword global2;
    zword addr;

    bool brief = FALSE;

    /* One V5 game (Wishbringer Solid Gold) contains this opcode by
       accident, so just return if the version number does not fit */

    if (A00035 >= V4)
	return;

    /* Read all relevant global variables from the memory of the
       Z-machine into local variables */

    addr = A00042;
    LOW_WORD (addr, global0)
    addr += 2;
    LOW_WORD (addr, global1)
    addr += 2;
    LOW_WORD (addr, global2)

    /* Frotz uses window 7 for the status line. Don't forget to select
       reverse and fixed width text style */

    set_window (7);

    A00196 (ZC_NEW_STYLE);
    A00196 (REVERSE_STYLE | FIXED_WIDTH_STYLE);

    /* If the screen width is below 55 characters then we have to use
       the brief status line format */

    if (A00052 < 55)
	brief = TRUE;

    /* Print the object description for the global variable 0 */

    A00196 (' ');
    A00198 (global0);

    /* A header flag tells us whether we have to display the current
       time or the score/moves information */

    if (A00036 & CONFIG_TIME) {	/* print hours and minutes */

	zword hours = (global1 + 11) % 12 + 1;

	pad_status_line (brief ? 15 : 20);

	A00199 ("Time: ");

	if (hours < 10)
	    A00196 (' ');
	A00197 (hours);

	A00196 (':');

	if (global2 < 10)
	    A00196 ('0');
	A00197 (global2);

	A00196 (' ');

	A00196 ((global1 >= 12) ? 'p' : 'a');
	A00196 ('m');

    } else {				/* print score and moves */

	pad_status_line (brief ? 15 : 30);

	A00199 (brief ? "S: " : "Score: ");
	A00197 (global1);

	pad_status_line (brief ? 8 : 14);

	A00199 (brief ? "M: " : "Moves: ");
	A00197 (global2);

    }

    /* Pad the end of the status line with spaces */

    pad_status_line (0);

    /* Return to the lower window */

    set_window (0);

}/* A00174 */


/*
 * A00176, split the screen into an upper (1) and lower (0) window.
 *
 *	zargs[0] = height of upper window in screen units (V6) or #lines
 *
 */
void A00176 (void)
{
    A00025 (zargs[0]);

}/* A00176 */


/*
 * A00185, change the width and height of a window.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = new height in screen units
 *	zargs[2] = new width in screen units
 *
 */
void A00185 (void)
{
    zword win = winarg0 ();

    A00194 ();

    wp[win].y_size = zargs[1];
    wp[win].x_size = zargs[2];

    /* Keep the cursor within the window */

    if (wp[win].y_cursor > zargs[1] || wp[win].x_cursor > zargs[2])
	reset_cursor (win);

}/* A00185 */


/*
 * A00186, set / clear / toggle window attributes.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = window attribute flags
 *	zargs[2] = operation to perform (optional, defaults to 0)
 *
 */
void A00186 (void)
{
    zword win = winarg0 ();
    zword flags = zargs[1];

    A00194 ();

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = 0;

    /* Set window style */

    switch (zargs[2]) {
	case 0: wp[win].attribute = flags; break;
	case 1: wp[win].attribute |= flags; break;
	case 2: wp[win].attribute &= ~flags; break;
	case 3: wp[win].attribute ^= flags; break;
    }

    if (cwin == win)
	update_attributes ();

}/* A00186 */


/*
 * get_window_colours
 *
 * Get the colours for a given window.
 *
 */
void get_window_colours (zword win, zbyte* fore, zbyte* back)
{
    *fore = lo (wp[win].colour);
    *back = hi (wp[win].colour);

}/* get_window_colours */


/*
 * get_wf
 *
 * Get the font for a given window.
 *
 */
zword get_wf (zword win)
{
    zword font = wp[win].font;

    if (font == TEXT_FONT) {

        if (A00035 != V6) {

            if (win != 0 || A00044 & FIXED_FONT_FLAG)

                font = FIXED_WIDTH_FONT;

        } else {

            if (wp[win].style & FIXED_WIDTH_STYLE)

                font = FIXED_WIDTH_FONT;

        }
    }
    return font;

}/* get_wf */


/*
 * colour_in_use
 *
 * Check if a colour is set in any window.
 *
 */
int colour_in_use (zword colour)
{
    int max = (A00035 == V6) ? 8 : 2;
    int i;

    for (i = 0; i < max; i++) {
        zword bg = hi (wp[i].colour);
        zword fg = lo (wp[i].colour);

        if (colour == fg || colour == bg)
            return 1;
    }

    return 0;

}/* colour_in_use */


/*
 * get_current_window
 *
 * Get the currently active window.
 *
 */
zword get_current_window (void)
{
    return cwp - wp;

}/* get_current_window */
