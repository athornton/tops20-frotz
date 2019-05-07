/* dumb-output.c
 * $Id: dumb-output.c,v 1.5 1998/07/08 03:45:41 al Exp $
 *
 * Copyright 1997,1998 Alfresco Petrofsky <alfresco@petrofsky.berkeley.edu>.
 * Any use permitted provided this notice stays intact.
 */

#include "dfrotz.h"

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

/* A00041 * A00042 */
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

static cell *dumb_row(int r) {return screen_data + r * A00042;}

static char *dumb_changes_row(int r)
{
  return screen_changes + r * A00042;
}

int A00197 (zchar z)
{
  if (plain_ascii && z >= ZC_LATIN1_MIN && z <= ZC_LATIN1_MAX) {
    char *p = latin1_to_ascii + 4 * (z - ZC_LATIN1_MIN);
    return strchr(p, ' ') - p;
  }
  return 1;
}

int A00224 (const zchar *s)
{
  int width = 0;
  zchar c;

  while ((c = *s++) != 0)
    if (c == ZC_NEW_STYLE || c == ZC_NEW_FONT)
      s++;
    else
      width += A00197(c);

  return width;
}

void A00219(int row, int col)
{
  cursor_row = row - 1; cursor_col = col - 1;
  if (cursor_row >= A00041)
    cursor_row = A00041 - 1;
}

/* Set a cell and update screen_changes.  */
static void dumb_set_cell(int row, int col, cell c)
{
  dumb_changes_row(row)[col] = (c != dumb_row(row)[col]);
  dumb_row(row)[col] = c;
}

void A00014(int row, int col, char c)
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

void A00221(int x)
{
  current_style = x & REVERSE_STYLE;
}

/* put a character in the cell at the cursor and advance the cursor.  */
static void dumb_display_char(char c)
{
  dumb_set_cell(cursor_row, cursor_col, make_cell(current_style, c));
  if (++cursor_col == A00042)
    if (cursor_row == A00041 - 1)
      cursor_col--;
    else {
      cursor_row++;
      cursor_col = 0;
    }
}

void A00011(char *s)
{
  /* copy to screen without marking it as a change.  */
  while (*s)
    dumb_row(cursor_row)[cursor_col++] = make_cell(0, *s++);
}

void A00012(int num_chars)
{
  /* Weird discard stuff.  Grep spec for 'pain in my butt'.  */
  /* The old characters should be on the screen just before the cursor.
   * Erase them.  */
  cursor_col -= num_chars;
  if (cursor_col < 0)
    cursor_col = 0;
  A00201(cursor_row + 1, cursor_col + 1,
		cursor_row + 1, cursor_col + num_chars);
}

void A00198 (zchar c)
{
  if (c >= ZC_LATIN1_MIN && c <= ZC_LATIN1_MAX) {
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

void A00199 (const zchar *s)
{
  zchar c;
  while ((c = *s++) != 0)
    if (c == ZC_NEW_FONT)
      s++;
    else if (c == ZC_NEW_STYLE)
      A00221(*s++);
    else
      A00198 (c);
}

void A00201 (int top, int left, int bottom, int right)
{
  int row, col;
  top--; left--; bottom--; right--;
  for (row = top; row <= bottom; row++)
    for (col = left; col <= right; col++)
      dumb_set_cell(row, col, make_cell(current_style, ' '));
}

void A00217 (int top, int left, int bottom, int right, int units)
{
  int row, col;
  top--; left--; bottom--; right--;
  if (units > 0) {
    for (row = top; row <= bottom - units; row++)
      for (col = left; col <= right; col++)
	dumb_copy_cell(row, col, row + units, col);
    A00201(bottom - units + 2, left + 1, bottom + 1, right + 1);
  } else if (units < 0) {
    for (row = bottom; row >= top - units; row--)
      for (col = left; col <= right; col++)
	dumb_copy_cell(row, col, row + units, col);
    A00201(top + 1, left + 1, top - units, right + 1);
  }
}

int A00204(int font, int *height, int *width)
{
    if (font == TEXT_FONT) {
      *height = 1; *width = 1; return 1;
    }
    return 0;
}

void A00218 (int x, int y) {}
void A00220 (int x) {}

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
	      && ((cell_style(c) != REVERSE_STYLE) || (rv_blank_char == ' '))));
}

static void show_line_prefix(int row, char c)
{
  if (show_line_numbers)
    printf((row == -1) ? ".." : "%02d", (row + 1) % 100);
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
    for (last = A00042 - 1; last >= 0; last--)
      if (!will_print_blank(dumb_row(r)[last]))
	  break;
    for (c = 0; c <= last; c++)
      show_cell(dumb_row(r)[c]);
  }
  putchar('\n');
}

/* Print the part of the cursor row before the cursor.  */
void A00009(bool show_cursor, char line_type)
{
  int i;
  show_line_prefix(show_cursor ? cursor_row : -1, line_type);
  if (show_cursor)
    for (i = 0; i < cursor_col; i++)
      show_cell(dumb_row(cursor_row)[i]);
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
void A00008(bool show_cursor)
{
  int r, c, first, last;
  char changed_rows[0x100];

  /* Easy case */
  if (compression_mode == COMPRESSION_NONE) {
    for (r = hide_lines; r < A00041; r++)
      show_row(r);
    mark_all_unchanged();
    return;
  }

  /* Check which rows changed, and where the first and last change is.  */
  first = last = -1;
  memset(changed_rows, 0, A00041);
  for (r = hide_lines; r < A00041; r++) {
    for (c = 0; c < A00042; c++)
      if (dumb_changes_row(r)[c] && !is_blank(dumb_row(r)[c]))
	break;
    changed_rows[r] = (c != A00042);
    if (changed_rows[r]) {
      first = (first != -1) ? first : r;
      last = r;
    }
  }

  if (first == -1)
    return;

  /* The show_cursor rule described above */
  if (show_cursor && (cursor_row == last)) {
    for (c = cursor_col; c < A00042; c++)
      if (!is_blank(dumb_row(last)[c]))
	break;
    if (c == A00042)
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
void A00010(void)
{
  int r;
  for (r = 0; r < A00044; r++)
    show_row(r);
}

/* Called when it's time for a more prompt but user has them turned off.  */
void A00013(void)
{
  A00008(FALSE);
  if (compression_mode == COMPRESSION_SPANS && hide_lines == 0) {
    show_row(-1);
  }
}

void A00215(void)
{
  A00008(FALSE);
}

void A00196 (int volume)
{
  if (visual_bell)
    printf("[%s-PITCHED BEEP]\n", (volume == 1) ? "HIGH" : "LOW");
  else
    putchar('\a'); /* so much for dumb.  */
}

void A00209 (int x) {}
void A00203 (void) {}
void A00222 (int x, int y, int z) {}
void A00223 (void) {}

/* if val is '0' or '1', set *var accordingly, else toggle it.  */
static void toggle(bool *var, char val)
{
  *var = val == '1' || (val != '0' && !*var);
}

bool A00007(const char *setting, bool show_cursor,
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
    A00008(show_cursor);

  } else if (!strncmp(setting, "vb", 2)) {
    toggle(&visual_bell, setting[2]);
    printf("Visual bell %s\n", visual_bell ? "ON" : "OFF");
    A00196(1); A00196(2);

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
    A00008(show_cursor);

  } else if (!strcmp(setting, "set")) {
    printf("Compression Mode %s, hiding top %d lines\n",
	   compression_names[compression_mode], hide_lines);
    printf("Picture Boxes display %s\n", show_pictures ? "ON" : "OFF");
    printf("Visual Bell %s\n", visual_bell ? "ON" : "OFF");
    A00196(1); A00196(2);
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

void A00006(void)
{
    size_t sc;
    sc = sizeof(cell);
  if (A00025 == V3) {
    A00026 |= CONFIG_SPLITSCREEN;
    A00034 &= ~OLD_SOUND_FLAG;
  }

  if (A00025 >= V5) {
    A00034 &= ~SOUND_FLAG;
  }

  A00044 = A00041;
  A00043 = A00042;
  screen_cells = A00041 * A00042;

  A00046 = 1; A00045 = 1;

  if (show_line_types == -1)
    show_line_types = A00025 > 3;

  screen_data = malloc(screen_cells * sizeof(cell));
  screen_changes = malloc(screen_cells);
  A00201(1, 1, A00041, A00042);
  memset(screen_changes, 0, screen_cells);
}
