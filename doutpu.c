/*
 * dumb_output.c - Dumb interface, output functions
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
 * Or visit http://www.fsf.org/
 */

#include "dfrotz.h"

extern fs_t A00003;

static bool show_line_numbers = FALSE;
static bool show_line_types = -1;
static bool show_pictures = TRUE;
static bool visual_bell = TRUE;
static bool plain_ascii = FALSE;

static char latin1_to_ascii[] =
  "    !   c   L   >o< Y   |   S   ''  C   a   <<  not -   R   _   "
  "^0  +/- ^2  ^3  '   my  P   .   ,   ^1  o   >>  1/4 1/2 3/4 ?   "
  "A   A   A   A   Ae  A   AE  C   E   E   E   E   I   I   I   I   "
  "Th  N   O   O   O   O   Oe  *   O   U   U   U   Ue  Y   Th  ss  "
  "a   a   a   a   ae  a   ae  c   e   e   e   e   i   i   i   i   "
  "th  n   o   o   o   o   oe  :   o   u   u   u   ue  y   th  y   "
;

/* A00051 * A00052 */
static int screen_cells;

/* The in-memory state of the screen.  */
/* Each cell contains a style in the upper byte and a char in the lower. */
typedef unsigned short cell;
static cell *screen_data;

static cell make_cell(int style, char c) {return (style << 8) | (0xff & c);}
static char cell_char(cell c) {return c & 0xff;}
static int cell_style(cell c) {return c >> 8;}


/* A cell's style is REVERSE_STYLE, normal (0), or PICTURE_STYLE.
 * PICTURE_STYLE means the character is part of an ascii image outline
 * box.  (This just buys us the ability to turn box display on and off
 * with immediate effect.  No, not very useful, but I wanted to give
 * the rv bit some company in that huge byte I allocated for it.)  */
#define PICTURE_STYLE 16

static int current_style = 0;

/* Which cells have changed (1 byte per cell).  */
static char *screen_changes;

static int cursor_row = 0, cursor_col = 0;

/* Compression styles.  */
static enum {
  COMPRESSION_NONE, COMPRESSION_SPANS, COMPRESSION_MAX,
} compression_mode = COMPRESSION_SPANS;
static char *compression_names[] = {"NONE", "SPANS", "MAX"};
static int hide_lines = 0;

/* Reverse-video display styles.  */
static enum {
  RV_NONE, RV_DOUBLESTRIKE, RV_UNDERLINE, RV_CAPS,
} rv_mode = RV_NONE;
static char *rv_names[] = {"NONE", "DOUBLESTRIKE", "UNDERLINE", "CAPS"};
static char rv_blank_char = ' ';

static cell *dumb_row(int r) {return screen_data + r * A00052;}

static char *dumb_changes_row(int r)
{
    return screen_changes + r * A00052;
}

int A00210 (zchar z)
{
    if (plain_ascii && z >= ZC_LATIN1_MIN) {
	char *p = latin1_to_ascii + 4 * (z - ZC_LATIN1_MIN);
	return strchr(p, ' ') - p;
    }
    return 1;
}

int A00240 (const zchar *s)
{
    int width = 0;
    zchar c;

    while ((c = *s++) != 0) {
	if (c == ZC_NEW_STYLE || c == ZC_NEW_FONT)
	    s++;
	else
	    width += A00210(c);
    }
    return width;
}

void A00233(int row, int col)
{
    cursor_row = row - 1; cursor_col = col - 1;
    if (cursor_row >= A00051)
	cursor_row = A00051 - 1;
}

bool A00246(int UNUSED(win), int UNUSED(ypos_old),
                       int UNUSED(ypos_new), int UNUSED(xpos),
                       int UNUSED(ysize), int UNUSED(xsize))
{
    return FALSE;
}

/* Set a cell and update screen_changes.  */
static void dumb_set_cell(int row, int col, cell c)
{
    dumb_changes_row(row)[col] = (c != dumb_row(row)[col]);
    dumb_row(row)[col] = c;
}

void A00020(int row, int col, char c)
{
    dumb_set_cell(row, col, make_cell(PICTURE_STYLE, c));
}

/* Copy a cell and copy its changedness state.
 * This is used for scrolling.  */
static void dumb_copy_cell(int dest_row, int dest_col,
			   int src_row, int src_col)
{
    dumb_row(dest_row)[dest_col] = dumb_row(src_row)[src_col];
    dumb_changes_row(dest_row)[dest_col] = dumb_changes_row(src_row)[src_col];
}

void A00235(int x)
{
    current_style = x & REVERSE_STYLE;
}

/* put a character in the cell at the cursor and advance the cursor.  */
static void dumb_display_char(char c)
{
    dumb_set_cell(cursor_row, cursor_col, make_cell(current_style, c));
    if (++cursor_col == A00052) {
	if (cursor_row == A00051 - 1)
	    cursor_col--;
	else {
	    cursor_row++;
	    cursor_col = 0;
	}
    }
}

void A00017(char *s)
{
    /* copy to screen without marking it as a change.  */
    while (*s)
	dumb_row(cursor_row)[cursor_col++] = make_cell(0, *s++);
}

void A00018(int num_chars)
{
  /* Weird discard stuff.  Grep spec for 'pain in my butt'.  */
  /* The old characters should be on the screen just before the cursor.
   * Erase them.  */
    cursor_col -= num_chars;

    if (cursor_col < 0)
	cursor_col = 0;
    A00214(cursor_row + 1, cursor_col + 1,
	cursor_row + 1, cursor_col + num_chars, -1);
}

void A00211 (zchar c)
{
    if (c >= ZC_LATIN1_MIN) {
	if (plain_ascii) {
	    char *ptr = latin1_to_ascii + 4 * (c - ZC_LATIN1_MIN);
	    do
		dumb_display_char(*ptr++);
	    while (*ptr != ' ');
	} else
	    dumb_display_char(c);
    } else if (c >= 32 && c <= 126) {
	dumb_display_char(c);
    } else if (c == ZC_GAP) {
	dumb_display_char(' '); dumb_display_char(' ');
    } else if (c == ZC_INDENT) {
	dumb_display_char(' '); dumb_display_char(' '); dumb_display_char(' ');
    }
  return;
}


/* Haxor your boxor? */
void A00212 (const zchar *s)
{
    zchar c;

    while ((c = *s++) != 0) {
	if (c == ZC_NEW_FONT)
	    s++;
	else if (c == ZC_NEW_STYLE)
	    A00235(*s++);
	else {
	    A00211 (c);
	}
    }
}

void A00214 (int top, int left, int bottom, int right, int UNUSED (win))
{
    int row, col;
    top--; left--; bottom--; right--;
    for (row = top; row <= bottom; row++) {
	for (col = left; col <= right; col++)
	    dumb_set_cell(row, col, make_cell(current_style, ' '));
    }
}

void A00231 (int top, int left, int bottom, int right, int units)
{
    int row, col;

    top--; left--; bottom--; right--;

    if (units > 0) {
	for (row = top; row <= bottom - units; row++) {
	    for (col = left; col <= right; col++)
		dumb_copy_cell(row, col, row + units, col);
	}
	A00214(bottom - units + 2, left + 1, bottom + 1, right + 1, -1 );
    } else if (units < 0) {
	for (row = bottom; row >= top - units; row--) {
	    for (col = left; col <= right; col++)
		dumb_copy_cell(row, col, row + units, col);
	}
	A00214(top + 1, left + 1, top - units, right + 1 , -1);
    }
}

int A00217(int font, int *height, int *width)
{
    if (font == TEXT_FONT) {
	*height = 1; *width = 1; return 1;
    }
    return 0;
}

void A00232 (int UNUSED (x), int UNUSED (y)) {}
void A00234 (int UNUSED (x)) {}

/* Print a cell to stdout.  */
static void show_cell(cell cel)
{
    char c = cell_char(cel);
    switch (cell_style(cel)) {
    case 0:
	putchar(c);
	break;
    case PICTURE_STYLE:
	putchar(show_pictures ? c : ' ');
	break;
    case REVERSE_STYLE:
	if (c == ' ')
	    putchar(rv_blank_char);
	else
	    switch (rv_mode) {
	    case RV_NONE: putchar(c); break;
	    case RV_CAPS: putchar(toupper(c)); break;
	    case RV_UNDERLINE: putchar('_'); putchar('\b'); putchar(c); break;
	    case RV_DOUBLESTRIKE: putchar(c); putchar('\b'); putchar(c); break;
	    }
	break;
    }
}

static bool will_print_blank(cell c)
{
    return (((cell_style(c) == PICTURE_STYLE) && !show_pictures)
	|| ((cell_char(c) == ' ')
	 && ((cell_style(c) != REVERSE_STYLE)
	|| (rv_blank_char == ' '))));
}

static void show_line_prefix(int row, char c)
{
    if (show_line_numbers) {
	if (row == -1)
	    printf("..");
	else
	    printf("%02d", (row + 1) % 100);
    }
    if (show_line_types)
	putchar(c);
    /* Add a separator char (unless there's nothing to separate).  */
    if (show_line_numbers || show_line_types)
	putchar(' ');
}

/* Print a row to stdout.  */
static void show_row(int r)
{
    if (r == -1) {
	show_line_prefix(-1, '.');
    } else {
	int c, last;
	show_line_prefix(r, (r == cursor_row) ? ']' : ' ');
	/* Don't print spaces at end of line.  */
	/* (Saves bandwidth and printhead wear.)  */
	/* TODO: compress spaces to tabs.  */
	for (last = A00052 - 1; last >= 0; last--)
	    if (!will_print_blank(dumb_row(r)[last]))
		break;
	for (c = 0; c <= last; c++)
	    show_cell(dumb_row(r)[c]);
    }
    putchar('\n');
}

/* Print the part of the cursor row before the cursor.  */
void A00015(bool show_cursor, char line_type)
{
    int i;
    show_line_prefix(show_cursor ? cursor_row : -1, line_type);
    if (show_cursor) {
	for (i = 0; i < cursor_col; i++)
	    show_cell(dumb_row(cursor_row)[i]);
    }
}

static void mark_all_unchanged(void)
{
    memset(screen_changes, 0, screen_cells);
}

/* Check if a cell is a blank or will display as one.
 * (Used to help decide if contents are worth printing.)  */
static bool is_blank(cell c)
{
    return ((cell_char(c) == ' ')
	|| ((cell_style(c) == PICTURE_STYLE) && !show_pictures));
}

/* Show the current screen contents, or what's changed since the last
 * call.
 *
 * If compressing, and show_cursor is true, and the cursor is past the
 * last nonblank character on the last line that would be shown, then
 * don't show that line (because it will be redundant with the prompt
 * line just below it).  */
void A00014(bool show_cursor)
{
    int r, c, first, last;
    char changed_rows[0x100];

    /* Easy case */
    if (compression_mode == COMPRESSION_NONE) {
	for (r = hide_lines; r < A00051; r++)
	    show_row(r);
	mark_all_unchanged();
	return;
    }

    /* Check which rows changed, and where the first and last change is.  */
    first = last = -1;
    memset(changed_rows, 0, A00051);
    for (r = hide_lines; r < A00051; r++) {
	for (c = 0; c < A00052; c++)
	    if (dumb_changes_row(r)[c] && !is_blank(dumb_row(r)[c]))
		break;
	changed_rows[r] = (c != A00052);
	if (changed_rows[r]) {
	    first = (first != -1) ? first : r;
	    last = r;
	}
    }

    if (first == -1)
	return;

    /* The show_cursor rule described above */
    if (show_cursor && (cursor_row == last)) {
	for (c = cursor_col; c < A00052; c++)
	    if (!is_blank(dumb_row(last)[c]))
		break;
	if (c == A00052)
	    last--;
    }

    /* Display the appropriate rows.  */
    if (compression_mode == COMPRESSION_MAX) {
	for (r = first; r <= last; r++)
	    if (changed_rows[r])
		show_row(r);
    } else {
	/* COMPRESSION_SPANS */
	for (r = first; r <= last; r++) {
	    if (changed_rows[r] || changed_rows[r + 1])
		show_row(r);
	    else {
		while (!changed_rows[r + 1])
		    r++;
		show_row(-1);
	    }
	}
	if (show_cursor && (cursor_row > last + 1))
	    show_row((cursor_row == last + 2) ? (last + 1) : -1);
    }

    mark_all_unchanged();
}

/* Unconditionally show whole screen.  For \s user command.  */
void A00016(void)
{
    int r;
    for (r = 0; r < A00054; r++)
	show_row(r);
}

/* Called when it's time for a more prompt but user has them turned off.  */
void A00019(void)
{
    A00014(FALSE);
    if (compression_mode == COMPRESSION_SPANS && hide_lines == 0) {
	show_row(-1);
    }
}

void A00229(void)
{
    A00014(FALSE);
}

void A00209 (int volume)
{
    if (visual_bell)
	printf("[%s-PITCHED BEEP]\n", (volume == 1) ? "HIGH" : "LOW");
    else
	putchar('\a'); /* so much for dumb.  */
}


/* To make the common code happy */

void A00219(void) {}
void A00223 (int UNUSED (a)) {}
void A00216 (int UNUSED (a)) {}
void A00236 (int UNUSED (a), int UNUSED (b), int UNUSED (c), zword UNUSED (d)) {}
void A00237 (int UNUSED (a)) {}


/* if val is '0' or '1', set *var accordingly, else toggle it.  */
static void toggle(bool *var, char val)
{
    *var = val == '1' || (val != '0' && !*var);
}

bool A00013(const char *setting, bool show_cursor,
				bool startup)
{
    char *p;
    int i;

    if (!strncmp(setting, "pb", 2)) {
	toggle(&show_pictures, setting[2]);
	printf("Picture outlines display %s\n", show_pictures ? "ON" : "OFF");
	if (startup)
	    return TRUE;
	for (i = 0; i < screen_cells; i++)
	    screen_changes[i] = (cell_style(screen_data[i]) == PICTURE_STYLE);
	A00014(show_cursor);
    } else if (!strncmp(setting, "vb", 2)) {
	toggle(&visual_bell, setting[2]);
	printf("Visual bell %s\n", visual_bell ? "ON" : "OFF");
	A00209(1); A00209(2);
    } else if (!strncmp(setting, "ln", 2)) {
	toggle(&show_line_numbers, setting[2]);
	printf("Line numbering %s\n", show_line_numbers ? "ON" : "OFF");
    } else if (!strncmp(setting, "lt", 2)) {
	toggle(&show_line_types, setting[2]);
	printf("Line-type display %s\n", show_line_types ? "ON" : "OFF");

    } else if (*setting == 'c') {
	switch (setting[1]) {
	case 'm': compression_mode = COMPRESSION_MAX; break;
	case 's': compression_mode = COMPRESSION_SPANS; break;
	case 'n': compression_mode = COMPRESSION_NONE; break;
	case 'h': hide_lines = atoi(&setting[2]); break;
	default: return FALSE;
	}
	printf("Compression mode %s, hiding top %d lines\n",
	    compression_names[compression_mode], hide_lines);
    } else if (*setting == 'r') {
	switch (setting[1]) {
	case 'n': rv_mode = RV_NONE; break;
	case 'o': rv_mode = RV_DOUBLESTRIKE; break;
	case 'u': rv_mode = RV_UNDERLINE; break;
	case 'c': rv_mode = RV_CAPS; break;
	case 'b': rv_blank_char = setting[2] ? setting[2] : ' '; break;
	default: return FALSE;
	}
	printf("Reverse-video mode %s, blanks reverse to '%c': ",
	    rv_names[rv_mode], rv_blank_char);

	for (p = "sample reverse text"; *p; p++)
	    show_cell(make_cell(REVERSE_STYLE, *p));
	putchar('\n');
	for (i = 0; i < screen_cells; i++)
	    screen_changes[i] = (cell_style(screen_data[i]) == REVERSE_STYLE);
	A00014(show_cursor);
    } else if (!strcmp(setting, "set")) {

	printf("Compression Mode %s, hiding top %d lines\n",
	    compression_names[compression_mode], hide_lines);
	printf("Picture Boxes display %s\n", show_pictures ? "ON" : "OFF");
	printf("Visual Bell %s\n", visual_bell ? "ON" : "OFF");
	A00209(1); A00209(2);
	printf("Line Numbering %s\n", show_line_numbers ? "ON" : "OFF");
	printf("Line-Type display %s\n", show_line_types ? "ON" : "OFF");
	printf("Reverse-Video mode %s, Blanks reverse to '%c': ",
	    rv_names[rv_mode], rv_blank_char);
	for (p = "sample reverse text"; *p; p++)
	    show_cell(make_cell(REVERSE_STYLE, *p));
	putchar('\n');
    } else
	return FALSE;
    return TRUE;
}

void A00012(void)
{
    if (A00035 == V3) {
	A00036 |= CONFIG_SPLITSCREEN;
	A00044 &= ~OLD_SOUND_FLAG;
    }

    if (A00035 >= V5) {
	A00044 &= ~SOUND_FLAG;
    }

    A00054 = A00051;
    A00053 = A00052;
    screen_cells = A00051 * A00052;

    A00056 = 1; A00055 = 1;

    if (show_line_types == -1)
	show_line_types = A00035 > 3;

    screen_data = malloc(screen_cells * sizeof(cell));
    screen_changes = malloc(screen_cells);
    A00214(1, 1, A00051, A00052, -2);
    memset(screen_changes, 0, screen_cells);
}
