/*
 * screen.c
 *
 * Generic screen manipulation
 *
 */

#include "frotz.h"

extern void A00241 (int, zword);

extern int A00242 (zword);

static struct {
    enum story A00063;
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

static font_height = 1;
static font_width = 1;

static bool input_redraw = FALSE;
static bool more_prompts = TRUE;
static bool discarding = FALSE;
static bool cursor = TRUE;

static input_window = 0;

static struct {
    zword y_pos;
    zword x_pos;
    zword y_size;
    zword x_size;
    zword y_cursor;
    zword x_cursor;
    zword left;
    zword right;
    zword nl_routine;
    zword nl_countdown;
    zword style;
    zword colour;
    zword font;
    zword font_size;
    zword attribute;
    zword line_count;
} wp[8], *cwp;

/*
 * winarg0
 *
 * Return the window number in zargs[0]. In V6 only, -3 refers to the
 * current window.
 *
 */

static zword winarg0 (void)
{
    short sz0;

    sz0 = s16(zargs[0]);

    if (A00025 == V6 && sz0 == -3)
        return cwin;

    if (sz0 >= ((A00025 == V6) ? 8 : 2))
        A00192 ("Illegal window");

    return (zargs[0] & 0xffff);

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
    short sz2;

    sz2=s16(zargs[2]);

    if (zargc < 3 || sz2 == -3)
        return cwin;

    if (sz2 >= 8)
        A00192 ("Illegal window");

    return (zargs[2] & 0xffff);

}/* winarg2 */

/*
 * update_cursor
 *
 * Move the hardware cursor to make it match the window properties.
 *
 */

static void update_cursor (void)
{

    A00219 (
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

    if (A00025 <= V4 && win == 0)
        lines = wp[0].y_size / hi (wp[0].font_size) - 1;

    wp[win].y_cursor = hi (wp[0].font_size) * lines + 1;
    wp[win].x_cursor = wp[win].left + 1;

    if (win == cwin)
        update_cursor ();

}/* reset_cursor */

/*
 * A00022
 *
 * Turn more prompts on/off.
 *
 */

void A00022 (bool flag)
{

    if (flag && !more_prompts)
        cwp->line_count = 0;

    more_prompts = flag;

}/* A00022 */

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
 * A00240
 *
 * Return maximum width of a line in the given window. This is used in
 * connection with the extended output stream #3 call in V6.
 *
 */

zword A00240 (zword win)
{

    if (A00025 == V6) {

        if (win >= 8)
            A00192 ("Illegal window");

        return wp[win].x_size - wp[win].left - wp[win].right;

    } else return 0xffff;

}/* A00240 */

/*
 * countdown
 *
 * Decrement the newline counter. Call the newline interrupt when the
 * counter hits zero. This is a helper function for A00260.
 *
 */

static void countdown (void)
{

    if (cwp->nl_countdown != 0)
        if (--cwp->nl_countdown == 0)
            A00242 (cwp->nl_routine);

}/* countdown */

/*
 * A00260
 *
 * Print a newline to the screen.
 *
 */

void A00260 (void)
{
    short slc, llc;
    if (discarding) return;

    /* Handle newline interrupts at the start (for most cases) */

    if (A00039 != INTERP_MSDOS || A00063 != ZORK_ZERO || A00027 != 393)
        countdown ();

    /* Check whether the last input line gets destroyed */

    if (input_window == cwin)
        input_redraw = TRUE;

    /* If the cursor has not reached the bottom line, then move it to
       the next line; otherwise scroll the window or reset the cursor
       to the top left. */

    cwp->x_cursor = cwp->left + 1;

    if (cwp->y_cursor + 2 * font_height - 1 > cwp->y_size)

        if (A00075) {

            zword y = cwp->y_pos;
            zword x = cwp->x_pos;

            A00217 (y,
                            x,
                            y + cwp->y_size - 1,
                            x + cwp->x_size - 1,
                            font_height);

        } else cwp->y_cursor = 1;

    else cwp->y_cursor += font_height;

    update_cursor ();

    /* See if we need to print a more prompt (unless the game has set
       the line counter to -999 in order to suppress more prompts). */
    slc = s16(cwp->line_count);
    if (A00075 && slc != -999) {

        zword above = (cwp->y_cursor - 1) / font_height;
        zword below = (cwp->y_size - cwp->y_cursor + 1) / font_height;

        cwp->line_count++; /* Assume it won't overflow */

        slc = s16(cwp->line_count);
        llc = s16(above + below -1);
        if (slc >= llc) {

            if (more_prompts)
                A00206 ();

            cwp->line_count = A00081;

        }

    }

    /* Handle newline interrupts at the end for Zork Zero under DOS */

    if (A00039 == INTERP_MSDOS && A00063 == ZORK_ZERO && A00027 == 393)
        countdown ();

}/* A00260 */

/*
 * A00258
 *
 * Display a single character on the screen.
 *
 */

void A00258 (zchar c)
{
    int width;

    if (discarding) return;

    if (c == ZC_INDENT && cwp->x_cursor != cwp->left + 1)
        c = ' ';

    if (units_left () < (width = A00197 (c))) {

        if (!A00073)
            { cwp->x_cursor = cwp->x_size - cwp->right; return; }

        A00260 ();

    }

    A00198 (c); cwp->x_cursor += width;

}/* A00258 */

/*
 * A00259
 *
 * Display a string of characters on the screen. If the word doesn't fit
 * then use wrapping or clipping depending on the current setting of the
 * A00073 flag.
 *
 */

void A00259 (const zchar *s)
{
    int width;

    if (discarding) return;

    if (*s == ZC_INDENT && cwp->x_cursor != cwp->left + 1)
        A00258 (*s++);

    if (units_left () < (width = A00224 (s))) {

        if (!A00073) {

            zchar c;

            while ((c = *s++) != 0)

                if (c == ZC_NEW_FONT || c == ZC_NEW_STYLE) {

                    int arg = (int) *s++;

                    if (c == ZC_NEW_FONT)
                        A00220 (arg);
                    if (c == ZC_NEW_STYLE)
                        A00221 (arg);

                } else A00258 (c);

            return;

        }

        if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
            width = A00224 (++s);

#ifdef AMIGA
        if (cwin == 0) Justifiable ();
#endif

        A00260 ();

    }

    A00199 (s); cwp->x_cursor += width;

}/* A00259 */

/*
 * A00261
 *
 * Display an input line on the screen. This is required during playback.
 *
 */

void A00261 (const zchar *buf, zchar key)
{
    int width;

    if (units_left () < (width = A00224 (buf)))
        A00260 ();

    A00199 (buf); cwp->x_cursor += width;

    if (key == ZC_RETURN)
        A00260 ();

}/* A00261 */

/*
 * A00262
 *
 * Remove an input line that has already been printed from the screen
 * as if it was deleted by the player. This could be necessary during
 * playback.
 *
 */

void A00262 (const zchar *buf)
{

    if (buf[0] != 0) {

        int width = A00224 (buf);

        zword y;
        zword x;

        cwp->x_cursor -= width;

        y = cwp->y_pos + cwp->y_cursor - 1;
        x = cwp->x_pos + cwp->x_cursor - 1;

        A00201 (y, x, y + font_height - 1, x + width - 1);
        A00219 (y, x);

    }

}/* A00262 */

/*
 * A00268
 *
 * Read an input line from the keyboard and return the terminating key.
 *
 */

zchar A00268 (int max, zchar *buf, zword timeout, bool continued)
{
    zchar key;
    int i;

    /* Make sure there is some space for input */

    if (cwin == 0 && units_left () + A00224 (buf) < 10 * font_width)
        A00260 ();

    /* Make sure the input line is visible */

    if (continued && input_redraw)
        A00261 (buf, -1);

    input_window = cwin;
    input_redraw = FALSE;

    /* Get input line from IO interface */

    cwp->x_cursor -= A00224 (buf);
    key = A00214 (max, buf, timeout, units_left (), continued);
    cwp->x_cursor += A00224 (buf);

    if (key != ZC_TIME_OUT)
        for (i = 0; i < 8; i++)
            wp[i].line_count = 0;

    /* Add a newline if the input was terminated normally */

    if (key == ZC_RETURN)
        A00260 ();

    return key;

}/* A00268 */

/*
 * A00267
 *
 * Read a single keystroke and return it.
 *
 */

zchar A00267 (zword timeout)
{
    zchar key;
    int i;

    key = A00213 (timeout, cursor);

    if (key != ZC_TIME_OUT)
        for (i = 0; i < 8; i++)
            wp[i].line_count = 0;

    return key;

}/* A00267 */

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

    A00073 = attr & 1;
    A00075 = attr & 2;
    A00074 = attr & 4;
    A00076 = attr & 8;

    /* Some story files forget to select wrapping for printing hints */

    if (A00063 == ZORK_ZERO && A00027 == 366)
        if (cwin == 0)
            A00073 = TRUE;
    if (A00063 == SHOGUN && A00027 <= 295)
        if (cwin == 0)
            A00073 = TRUE;

}/* update_attributes */

/*
 * A00018
 *
 * Set the right text style. This can be necessary when the fixed font
 * flag is changed, or when a new window is selected, or when the game
 * uses the set_text_style opcode.
 *
 */

void A00018 (void)
{
    zword style;

    if (A00025 != V6) {

        style = wp[0].style;

        if (cwin != 0 || A00034 & FIXED_FONT_FLAG)
            style |= FIXED_WIDTH_STYLE;

    } else style = cwp->style;

    if (!A00067 && A00065 && A00076) {

        A00186 (ZC_NEW_STYLE);
        A00186 (style);

    } else A00221 (style);

}/* A00018 */

/*
 * set_window
 *
 * Set the current window. In V6 every window has its own set of window
 * properties such as colours, text style, cursor position and size.
 *
 */

static void set_window (zword win)
{

    A00184 ();

    cwin = win; cwp = wp + win;

    update_attributes ();

    if (A00025 == V6) {

        A00218 (lo (cwp->colour), hi (cwp->colour));

        if (A00204 (cwp->font, &font_height, &font_width))
            A00220 (cwp->font);

        A00221 (cwp->style);

    } else A00018 ();

    if (A00025 != V6 && win != 0) {
        wp[win].y_cursor = 1;
        wp[win].x_cursor = 1;
    }

    update_cursor ();

}/* set_window */

/*
 * erase_window
 *
 * Erase a window to background colour.
 *
 */

static void erase_window (zword win)
{
    zword y = wp[win].y_pos;
    zword x = wp[win].x_pos;

    if (A00025 == V6 && win != cwin && A00039 != INTERP_AMIGA)
        A00218 (lo (wp[win].colour), hi (wp[win].colour));

    A00201 (y,
                   x,
                   y + wp[win].y_size - 1,
                   x + wp[win].x_size - 1);

    if (A00025 == V6 && win != cwin && A00039 != INTERP_AMIGA)
        A00218 (lo (cwp->colour), hi (cwp->colour));

    reset_cursor (win);

    wp[win].line_count = 0;

}/* erase_window */

/*
 * A00019
 *
 * Divide the screen into upper (1) and lower (0) windows. In V3 the upper
 * window appears below the status line.
 *
 */

void A00019 (zword height)
{
    zword stat_height = 0;
    short syc, sys;

    A00184 ();

    /* Calculate height of status line and upper window */

    if (A00025 != V6)
        height *= hi (wp[1].font_size);

    if (A00025 <= V3)
        stat_height = hi (wp[7].font_size);

    /* Cursor of upper window mustn't be swallowed by the lower window */

    wp[1].y_cursor += wp[1].y_pos - 1 - stat_height;

    wp[1].y_pos = 1 + stat_height;
    wp[1].y_size = height;

    syc=s16(wp[1].y_cursor);
    sys=s16(wp[1].y_size);
    if (syc > sys)
        reset_cursor (1);

    /* Cursor of lower window mustn't be swallowed by the upper window */

    wp[0].y_cursor += wp[0].y_pos - 1 - stat_height - height;

    wp[0].y_pos = 1 + stat_height + height;
    wp[0].y_size = A00044 - stat_height - height;

    syc=s16(wp[0].y_cursor);
    if (syc < 1)
        reset_cursor (0);

    /* Erase the upper window in V3 only */

    if (A00025 == V3 && height != 0)
        erase_window (1);

}/* A00019 */

/*
 * erase_screen
 *
 * Erase the entire screen to background colour.
 *
 */

static void erase_screen (zword win)
{
    int i;
    short sw;

    A00201 (1, 1, A00044, A00043);

    sw=s16(win);
    if (sw == -1) {
        A00019 (0);
        set_window (0);
        reset_cursor (0);
    }

    for (i = 0; i < 8; i++)
        wp[i].line_count = 0;

}/* erase_screen */

#ifdef AMIGA

/*
 * resize_screen
 *
 * Try to adapt the window properties to a new screen size.
 *
 */

void resize_screen (void)
{

    if (A00025 != V6) {

        wp[0].x_size = A00043;
        wp[1].x_size = A00043;
        wp[7].x_size = A00043;

        wp[0].y_size = A00044 - wp[1].y_size - wp[7].y_size;

    }

}/* resize_screen */

#endif

/*
 * A00017
 *
 * Prepare the screen for a new game.
 *
 */

void A00017 (void)
{

    /* Use default settings */

    A00218 (A00050, A00049);

    if (A00204 (TEXT_FONT, &font_height, &font_width))
        A00220 (TEXT_FONT);

    A00221 (0);

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
        cwp->colour = (A00049 << 8) | A00050;
        cwp->font = TEXT_FONT;
        cwp->font_size = (font_height << 8) | font_width;
        cwp->attribute = 8;
    }

    /* Prepare lower/upper windows and status line */

    wp[0].attribute = 15;

    wp[0].left = A00082;
    wp[0].right = A00083;

    wp[0].x_size = A00043;
    wp[1].x_size = A00043;

    if (A00025 <= V3)
        wp[7].x_size = A00043;

    A00216 (RESTART_WPROP_SET);

    /* Clear the screen, unsplit it and select window 0 */

    erase_screen ((zword) (-1));

}/* A00017 */

/*
 * A00244
 *
 * Return false if the last mouse click occured outside the current
 * mouse window; otherwise write the mouse arrow coordinates to the
 * memory of the header extension table and return true.
 *
 */

bool A00244 (void)
{

    if (mwin >= 0) {

        if (A00072 < wp[mwin].y_pos || A00072 >= wp[mwin].y_pos + wp[mwin].y_size)
            return FALSE;
        if (A00071 < wp[mwin].x_pos || A00071 >= wp[mwin].x_pos + wp[mwin].x_size)
            return FALSE;

        A00060 = A00072 - wp[mwin].y_pos + 1;
        A00059 = A00071 - wp[mwin].x_pos + 1;

    } else {

        if (A00072 < 1 || A00072 > A00044)
            return FALSE;
        if (A00071 < 1 || A00071 > A00043)
            return FALSE;

        A00060 = A00072;
        A00059 = A00071;

    }

    if (A00025 != V6) {
        A00060 = (A00060 - 1) / A00045 + 1;
        A00059 = (A00059 - 1) / A00046 + 1;
    }

    A00241 (HX_MOUSE_Y, A00060);
    A00241 (HX_MOUSE_X, A00059);

    return TRUE;

}/* A00244 */

/*
 * A00263
 *
 * Start printing a so-called debugging A00070. The contents of the
 * A00070 are passed to the A00070 stream, a Frotz specific output
 * stream with maximum priority.
 *
 */

void A00263 (void)
{

    if (cwin == 0) {		/* A00070s in window 0 only */

        A00221 (0);

        if (cwp->x_cursor != cwp->left + 1)
            A00260 ();

        A00258 (ZC_INDENT);

    } else discarding = TRUE;   /* discard A00070s in other windows */

}/* A00263 */

/*
 * A00264
 *
 * Stop printing a "debugging" A00070.
 *
 */

void A00264 (void)
{

    if (cwin == 0) {		/* A00070s in window 0 only */

        A00260 ();

        A00018 ();

    } else discarding = FALSE;  /* A00070 has been discarded */

}/* A00264 */

/*
 * A00091, turn text buffering on/off.
 *
 *	zargs[0] = new text buffering flag (0 or 1)
 *
 */

void A00091 (void)
{

    /* Infocom's V6 games rarely use the buffer_mode opcode. If they do
       then only to print text immediately, without any delay. This was
       used to give the player some sign of life while the game was
       spending much time on parsing a complicated input line. (To turn
       off word wrapping, V6 games use the window_style opcode instead.)
       Today we can afford to ignore buffer_mode in V6. */

    if (A00025 != V6) {

        A00184 ();

        wp[0].attribute &= ~8;

        if (zargs[0] != 0)
            wp[0].attribute |= 8;

        update_attributes ();

    }

}/* A00091 */

/*
 * A00100, draw a picture.
 *
 *	zargs[0] = number of picture to draw
 *	zargs[1] = y-coordinate of top left corner
 *	zargs[2] = x-coordinate of top left corner
 *
 */

void A00100 (void)
{
    zword pic = zargs[0];

    zword y = zargs[1];
    zword x = zargs[2];

    int i;

    A00184 ();

    if (y == 0)			/* use cursor line if y-coordinate is 0 */
        y = cwp->y_cursor;
    if (x == 0)                 /* use cursor column if x-coordinate is 0 */
        x = cwp->x_cursor;

    y += cwp->y_pos - 1;
    x += cwp->x_pos - 1;

    /* The following is necessary to make Amiga and Macintosh story
       files work with MCGA graphics files.  Some screen-filling
       pictures of the original Amiga release like the borders of
       Zork Zero were split into several MCGA pictures (left, right
       and top borders).  We pretend this has not happened. */

    for (i = 0; mapper[i].A00063 != UNKNOWN; i++)

        if (A00063 == mapper[i].A00063 && pic == mapper[i].pic) {

            int height1, width1;
            int height2, width2;

            int delta = 0;

            A00208 (pic, &height1, &width1);
            A00208 (mapper[i].pic2, &height2, &width2);

            if (A00063 == ARTHUR && pic == 54)
                delta = A00043 / 160;

            A00200 (mapper[i].pic1, y + height1, x + delta);
            A00200 (mapper[i].pic2, y + height1, x + width1 - width2 - delta);

        }

    A00200 (pic, y, x);

    if (A00063 == SHOGUN)

        if (pic == 3) {

            int height, width;

            A00208 (59, &height, &width);
            A00200 (59, y, A00043 - width + 1);

        }

}/* A00100 */

/*
 * A00102, erase the line starting at the cursor position.
 *
 *	zargs[0] = 1 + #units to erase (1 clears to the end of the line)
 *
 */

void A00102 (void)
{
    zword pixels = zargs[0];
    zword y, x;

    A00184 ();

    /* Clipping at the right margin of the current window */

    if (--pixels == 0 || pixels > units_left ())
        pixels = units_left ();

    /* Erase from cursor position */

    y = cwp->y_pos + cwp->y_cursor - 1;
    x = cwp->x_pos + cwp->x_cursor - 1;

    A00201 (y, x, y + font_height - 1, x + pixels - 1);

}/* A00102 */

/*
 * A00103, erase a picture with background colour.
 *
 *	zargs[0] = number of picture to erase
 *	zargs[1] = y-coordinate of top left corner (optional)
 *	zargs[2] = x-coordinate of top left corner (optional)
 *
 */

void A00103 (void)
{
    int height, width;

    zword y = zargs[1];
    zword x = zargs[2];

    A00184 ();

    if (y == 0)		/* use cursor line if y-coordinate is 0 */
        y = cwp->y_cursor;
    if (x == 0)         /* use cursor column if x-coordinate is 0 */
        x = cwp->x_cursor;

    A00208 (zargs[0], &height, &width);

    y += cwp->y_pos - 1;
    x += cwp->x_pos - 1;

    A00201 (y, x, y + height - 1, x + width - 1);

}/* A00103 */

/*
 * A00104, erase a window or the screen to background colour.
 *
 *	zargs[0] = window (-3 current, -2 screen, -1 screen & unsplit)
 *
 */

void A00104 (void)
{
    short sz0, sz1;

    A00184 ();
    sz0=s16(zargs[0]);
    sz1=s16(zargs[1]);

    if (sz0 == -1 || sz0 == -2)
        erase_screen ( (zargs[0] & 0xffff) );
    else
        erase_window (winarg0 ());

}/* A00104 */

/*
 * A00106, write the cursor coordinates into a table.
 *
 *	zargs[0] = address to write information to
 *
 */

void A00106 (void)
{
    zword y, x;

    A00184 ();

    y = cwp->y_cursor;
    x = cwp->x_cursor;

    if (A00025 != V6) {	/* convert to grid positions */
        y = (y - 1) / A00045 + 1;
        x = (x - 1) / A00046 + 1;
    }

    A00195 ((zword) (zargs[0] + 0), y);
    A00195 ((zword) (zargs[0] + 2), x);

}/* A00106 */

/*
 * A00113, store the value of a window property.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = number of window property to be stored
 *
 */

void A00113 (void)
{

    A00184 ();

    if (zargs[1] >= 16)
        A00192 ("Illegal window property");

    store (((zword *) (wp + winarg0 ())) [zargs[1]]);

}/* A00113 */

/*
 * A00123, select a window as mouse window.
 *
 *	zargs[0] = window number (-3 is the current) or -1 for the screen
 *
 */

void A00123 (void)
{
    short sz0;
    sz0 = s16(zargs[0]);

    mwin = (sz0 == -1) ? -1 : winarg0 ();

}/* A00123 */

/*
 * A00124, place a window on the screen.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = y-coordinate
 *	zargs[2] = x-coordinate
 *
 */

void A00124 (void)
{
    zword win = winarg0 ();

    A00184 ();

    wp[win].y_pos = zargs[1];
    wp[win].x_pos = zargs[2];

    if (win == cwin)
        update_cursor ();

}/* A00124 */

/*
 * A00127, get information on a picture or the graphics file.
 *
 *	zargs[0] = number of picture or 0 for the graphics file
 *	zargs[1] = address to write information to
 *
 */

void A00127 (void)
{
    zword pic = zargs[0];
    zword table = zargs[1];

    int height, width;
    int i;

    bool avail = A00208 (pic, &height, &width);

    for (i = 0; mapper[i].A00063 != UNKNOWN; i++)

        if (A00063 == mapper[i].A00063)

            if (pic == mapper[i].pic) {

                int height2, width2;

                avail &= A00208 (mapper[i].pic1, &height2, &width2);
                avail &= A00208 (mapper[i].pic2, &height2, &width2);

                height += height2;

            } else if (pic == mapper[i].pic1 || pic == mapper[i].pic2)

                avail = FALSE;

    A00195 ((zword) (table + 0), (zword) (height));
    A00195 ((zword) (table + 2), (zword) (width));

    A00193 (avail);

}/* A00127 */

/*
 * A00128, prepare a group of pictures for faster display.
 *
 *	zargs[0] = address of table holding the picture numbers
 *
 */

void A00128 (void)
{

    /* This opcode is used by Shogun and Zork Zero when the player
       encounters built-in games such as Peggleboz. Nowadays it is
       not very helpful to hold the picture data in memory because
       even a small disk cache avoids re-loading of data. */

}/* A00128 */

/*
 * A00139, print ASCII text in a rectangular area.
 *
 *	zargs[0] = address of text to be printed
 *	zargs[1] = width of rectangular area
 *	zargs[2] = height of rectangular area (optional)
 *	zargs[3] = number of char's to skip between lines (optional)
 *
 */

void A00139 (void)
{
    zword addr = zargs[0];
    zword x;
    int i, j;

    A00184 ();

    /* Supply default arguments */

    if (zargc < 3)
        zargs[2] = 1;
    if (zargc < 4)
        zargs[3] = 0;

    /* Write text in width x height rectangle */

    x = cwp->x_cursor;

    for (i = 0; i < zargs[2]; i++) {

        if (i != 0) {

            A00184 ();

            cwp->y_cursor += font_height;
            cwp->x_cursor = x;

            update_cursor ();

        }

        for (j = 0; j < zargs[1]; j++) {

            zbyte c;

            LOW_BYTE (addr, c)
            addr++;

            A00186 (c);

        }

        addr += zargs[3];

    }

}/* A00139 */

/*
 * A00145, set the value of a window property.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = number of window property to set
 *	zargs[2] = value to set window property to
 *
 */

void A00145 (void)
{

    A00184 ();

    if (zargs[1] >= 16)
        A00192 ("Illegal window property");

    ((zword *) (wp + winarg0 ())) [zargs[1]] = zargs[2];

}/* A00145 */

/*
 * A00161, scroll a window up or down.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = #screen units to scroll up (positive) or down (negative)
 *
 */

void A00161 (void)
{
    short sz1;
    zword win = winarg0 ();
    zword y, x;

    A00184 ();

    /* Use the correct set of colours when scrolling the window */

    if (win != cwin && A00039 != INTERP_AMIGA)
        A00218 (lo (wp[win].colour), hi (wp[win].colour));

    y = wp[win].y_pos;
    x = wp[win].x_pos;

    sz1 = s16(zargs[1]);
    A00217 (y,
                    x,
                    y + wp[win].y_size - 1,
                    x + wp[win].x_size - 1,
                    sz1);

    if (win != cwin && A00039 != INTERP_AMIGA)
        A00218 (lo (cwp->colour), hi (cwp->colour));

}/* A00161 */

/*
 * A00164, set the foreground and background colours.
 *
 *	zargs[0] = foreground colour
 *	zargs[1] = background colour
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */

void A00164 (void)
{
    short sfg, sbg;
    zword win = (A00025 == V6) ? winarg2 () : 0;

    zword fg = zargs[0];
    zword bg = zargs[1];


    A00184 ();
    sfg=s16(fg);
    sbg=s16(bg);
    if (sfg == -1)	/* colour -1 is the colour at the cursor */
        fg = A00207 ();
    if (sbg == -1)
        bg = A00207 ();

    if (fg == 0)		/* colour 0 means keep current colour */
        fg = lo (wp[win].colour);
    if (bg == 0)
        bg = hi (wp[win].colour);

    if (fg == 1)		/* colour 1 is the system default colour */
        fg = A00050;
    if (bg == 1)
        bg = A00049;

    if (A00025 == V6 && A00039 == INTERP_AMIGA)

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

    if (win == cwin || A00025 != V6)
        A00218 (fg, bg);

}/* A00164 */

/*
 * A00163, set the font for text output and store the previous font.
 *
 *      zargs[0] = number of font or 0 to keep current font
 *
 */

void A00163 (void)
{
    zword win = (A00025 == V6) ? cwin : 0;
    zword font = zargs[0];

    if (font != 0) {

        if (A00063 == JOURNEY && font == 4)	/* Journey uses fixed fonts */
            font = 1;				/* for most A00236er #'s */

        if (A00204 (font, &font_height, &font_width)) {

            store (wp[win].font);

            wp[win].font = font;
            wp[win].font_size = (font_height << 8) | font_width;

            if (!A00067 && A00065 && A00076) {

                A00186 (ZC_NEW_FONT);
                A00186 (font);

            } else A00220 (font);

        } else store (0);

    } else store (wp[win].font);

}/* A00163 */

/*
 * A00165, set the cursor position or turn the cursor on/off.
 *
 *	zargs[0] = y-coordinate or -2/-1 for cursor on/off
 *	zargs[1] = x-coordinate
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */

void A00165 (void)
{
    short sy;
    zword win = (A00025 == V6) ? winarg2 () : 1;

    zword y = zargs[0];
    zword x = zargs[1];
    sy=s16(y);

    A00184 ();

    /* Supply default arguments */

    if (zargc < 3) {
        zargs[2] = -3;
        zargs[2] &= 0xffff;
    }

    /* Handle cursor on/off */

    if (sy < 0) {

        if (sy == -2)
            cursor = TRUE;
        if (sy == -1)
            cursor = FALSE;

        return;

    }

    /* Convert grid positions to screen units if this is not V6 */

    if (A00025 != V6) {

        if (cwin == 0)
            return;

        y = (y - 1) * A00045 + 1;
        x = (x - 1) * A00046 + 1;

    }

    /* Protect the margins */

    if (x <= wp[win].left || x > wp[win].x_size - wp[win].right)
        x = wp[win].left + 1;

    /* Move the cursor */

    wp[win].y_cursor = y;
    wp[win].x_cursor = x;

    if (win == cwin)
        update_cursor ();

}/* A00165 */

/*
 * A00166, set the left and right margins of a window.
 *
 *	zargs[0] = left margin in pixels
 *	zargs[1] = right margin in pixels
 *	zargs[2] = window (-3 is the current one, optional)
 *
 */

void A00166 (void)
{
    zword win = winarg2 ();

    A00184 ();

    wp[win].left = zargs[0];
    wp[win].right = zargs[1];

    /* Protect the margins */

    if (wp[win].x_cursor <= zargs[0] || wp[win].x_cursor > wp[win].x_size - zargs[1]) {

        wp[win].x_cursor = zargs[0] + 1;

        if (win == cwin)
            update_cursor ();

    }

}/* A00166 */

/*
 * A00168, set the style for text output.
 *
 *      zargs[0] = style flags to set or 0 to reset text style
 *
 */

void A00168 (void)
{
    zword win = (A00025 == V6) ? cwin : 0;
    zword style = zargs[0];

    wp[win].style |= style;

    if (style == 0)
        wp[win].style = 0;

    A00018 ();

}/* A00168 */

/*
 * A00167, select the current window.
 *
 *	zargs[0] = window to be selected (-3 is the current one)
 *
 */

void A00167 (void)
{

    set_window (winarg0 ());

}/* A00167 */

/*
 * pad_status_line
 *
 * Pad the status line with spaces up to the given position.
 *
 */

static void pad_status_line (int column)
{
    int spaces;

    A00184 ();

    spaces = units_left () / A00197 (' ') - column;

    while (spaces--)
        A00258 (' ');

}/* pad_status_line */

/*
 * A00169, display the status line for V1 to V3 games.
 *
 *	no zargs used
 *
 */

void A00169 (void)
{
    zword global0;
    zword global1;
    zword global2;
    zword addr;

    bool brief = FALSE;

    /* One V5 game (Wishbringer Solid Gold) contains this opcode by
       accident, so just return if the version number does not fit */

    if (A00025 >= V4)
        return;

    /* Read all relevant global variables from the memory of the
       Z-machine into local variables */

    addr = A00032;
    global0 = lw(addr);
    addr += 2;
    global1 = lw(addr);
    addr += 2;
    global2 = lw(addr);

    /* Frotz uses window 7 for the status line. Don't forget to select
       reverse and fixed width text style */

    set_window (7);

    A00186 (ZC_NEW_STYLE);
    A00186 (REVERSE_STYLE | FIXED_WIDTH_STYLE);

    /* If the screen width is below 55 characters then we have to use
       the brief status line format */

    if (A00042 < 55)
        brief = TRUE;

    /* Print the object description for the global variable 0 */

    A00186 (' ');
    A00188 (global0);

    /* A header flag tells us whether we have to display the current
       time or the score/moves information */

    if (A00026 & CONFIG_TIME) {	/* print hours and minutes */

        zword hours = (global1 + 11) % 12 + 1;

        pad_status_line (brief ? 15 : 20);

        A00189 ("Time: ");

        if (hours < 10)
            A00186 (' ');
        A00187 (hours);

        A00186 (':');

        if (global2 < 10)
            A00186 ('0');
        A00187 (global2);

        A00186 (' ');

        A00186 ((global1 >= 12) ? 'p' : 'a');
        A00186 ('m');

    } else {				/* print score and moves */

        pad_status_line (brief ? 15 : 30);

        A00189 (brief ? "S: " : "Score: ");
        A00187 (global1);

        pad_status_line (brief ? 8 : 14);

        A00189 (brief ? "M: " : "Moves: ");
        A00187 (global2);

    }

    /* Pad the end of the status line with spaces */

    pad_status_line (0);

    /* Return to the lower window */

    set_window (0);

}/* A00169 */

/*
 * A00171, split the screen into an upper (1) and lower (0) window.
 *
 *	zargs[0] = height of upper window in screen units (V6) or #lines
 *
 */

void A00171 (void)
{

    A00019 (zargs[0]);

}/* A00171 */

/*
 * A00180, change the width and height of a window.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = new height in screen units
 *	zargs[2] = new width in screen units
 *
 */

void A00180 (void)
{
    zword win = winarg0 ();

    A00184 ();

    wp[win].y_size = zargs[1];
    wp[win].x_size = zargs[2];

    /* Keep the cursor within the window */

    if (wp[win].y_cursor > zargs[1] || wp[win].x_cursor > zargs[2])
        reset_cursor (win);

}/* A00180 */

/*
 * A00181, set / clear / toggle window attributes.
 *
 *	zargs[0] = window (-3 is the current one)
 *	zargs[1] = window attribute flags
 *	zargs[2] = operation to perform (optional, defaults to 0)
 *
 */

void A00181 (void)
{
    zword win = winarg0 ();
    zword flags = zargs[1];

    A00184 ();

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

}/* A00181 */
