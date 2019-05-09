/* fastmem.c - Memory related functions (fast version without virtual memory)
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

/*
 * New undo mechanism added by Jim Dunleavy <jim.dunleavy@erha.ie>
 */

#include <stdio.h>
#include <string.h>
#include "frotz.h"

#ifdef MSDOS_16BIT

#include <alloc.h>

#define malloc(size)	farmalloc (size)
#define realloc(size,p)	farrealloc (size,p)
#define free(size)	farfree (size)

#else

#include <stdlib.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define far

#endif

extern void A00022 (int);
extern void A00023 (void);
extern void A00024 (void);
extern void call (zword, int, zword *, int);
extern void A00025 (zword);
extern void A00026 (void);
extern void A00027 (void);


extern zword A00028 (FILE *, FILE *);
extern zword A00029 (FILE *, FILE *);

extern void A00030 (zword);

extern void (*op0_opcodes[]) (void);
extern void (*op1_opcodes[]) (void);
extern void (*op2_opcodes[]) (void);
extern void (*var_opcodes[]) (void);

/* char save_name[MAX_FILE_NAME + 1] = DEFAULT_SAVE_NAME; */
char auxilary_name[MAX_FILE_NAME + 1] = DEFAULT_AUXILARY_NAME;

zbyte far *zmp = NULL;
zbyte far *pcp = NULL;

static FILE *story_fp = NULL;

/*
 * Data for the undo mechanism.
 * This undo mechanism is based on the scheme used in Evin Robertson's
 * Nitfol A00259er.
 * Undo blocks are stored as differences between states.
 */

typedef struct undo_struct undo_t;
struct undo_struct {
    undo_t *next;
    undo_t *prev;
    long pc;
    long diff_size;
    zword A00077;
    zword stack_size;
    zword frame_offset;
    /* undo diff and stack data follow */
};

static undo_t *first_undo = NULL, *last_undo = NULL, *curr_undo = NULL;
static zbyte *undo_mem = NULL, *prev_zmp, *undo_diff;

static int undo_count = 0;


/*
 * get_header_extension
 *
 * Read a value from the header extension (former mouse table).
 *
 */
zword get_header_extension (int entry)
{
    zword addr;
    zword val;

    if (A00066 == 0 || entry > A00068)
	return 0;

    addr = A00066 + 2 * entry;
    LOW_WORD (addr, val);

    return val;

}/* get_header_extension */


/*
 * A00268
 *
 * Set an entry in the header extension (former mouse table).
 *
 */
void A00268 (int entry, zword val)
{
    zword addr;

    if (A00066 == 0 || entry > A00068)
	return;

    addr = A00066 + 2 * entry;
    SET_WORD (addr, val);

}/* A00268 */


/*
 * restart_header
 *
 * Set all header fields which hold information about the A00259er.
 *
 */
void restart_header (void)
{
    zword screen_x_size;
    zword screen_y_size;
    zbyte font_x_size;
    zbyte font_y_size;

    int i;

    SET_BYTE (H_CONFIG, A00036);
    SET_WORD (H_FLAGS, A00044);

    if (A00035 >= V4) {
	SET_BYTE (H_INTERPRETER_NUMBER, A00049);
	SET_BYTE (H_INTERPRETER_VERSION, A00050);
	SET_BYTE (H_SCREEN_ROWS, A00051);
	SET_BYTE (H_SCREEN_COLS, A00052);
    }

    /* It's less trouble to use font size 1x1 for V5 games, especially
       because of a bug in the unreleased German version of "Zork 1" */

    if (A00035 != V6) {
	screen_x_size = (zword) A00052;
	screen_y_size = (zword) A00051;
	font_x_size = 1;
	font_y_size = 1;
    } else {
	screen_x_size = A00053;
	screen_y_size = A00054;
	font_x_size = A00056;
	font_y_size = A00055;
    }

    if (A00035 >= V5) {
	SET_WORD (H_SCREEN_WIDTH, screen_x_size);
	SET_WORD (H_SCREEN_HEIGHT, screen_y_size);
	SET_BYTE (H_FONT_HEIGHT, font_y_size);
	SET_BYTE (H_FONT_WIDTH, font_x_size);
	SET_BYTE (H_DEFAULT_BACKGROUND, A00059);
	SET_BYTE (H_DEFAULT_FOREGROUND, A00060);
    }

    if (A00035 == V6)
	for (i = 0; i < 8; i++)
	    A00203 ((zword) (H_USER_NAME + i), A00067[i]);

    SET_BYTE (H_STANDARD_HIGH, A00063);
    SET_BYTE (H_STANDARD_LOW, A00064);

}/* restart_header */


/*
 * A00260
 *
 * Allocate memory and load the story file.
 *
 * Data collected from http://www.russotto.net/zplet/ivl.html
 *
 */
void A00260 (void)
{
    long size;
    zword addr;
    unsigned n;
    int i, j;

    static struct {
	enum story A00075;
	zword release;
	zbyte serial[6];
    } records[] = {
	{	   ZORK1,   2, "AS000C" },
	{	   ZORK1,   5, ""       },
	{	   ZORK1,  15, "UG3AU5" },
	{	   ZORK1,  23, "820428" },
	{	   ZORK1,  25, "820515" },
	{	   ZORK1,  26, "820803" },
	{	   ZORK1,  28, "821013" },
	{	   ZORK1,  30, "830330" },
	{	   ZORK1,  75, "830929" },
	{	   ZORK1,  76, "840509" },
	{	   ZORK1,  88, "840726" },
	{	   ZORK1,  52, "871125" },
	{	  ZORK1G,   3, "880113" },
	{	   ZORK2,   7, "UG3AU5" },
	{	   ZORK2,  15, "820308" },
	{	   ZORK2,  17, "820427" },
	{	   ZORK2,  18, "820512" },
	{	   ZORK2,  18, "820517" },
	{	   ZORK2,  19, "820721" },
	{	   ZORK2,  22, "830331" },
	{	   ZORK2,  23, "830411" },
	{	   ZORK2,  48, "840904" },
	{	   ZORK3,  10, "820818" },
	{	   ZORK3,  12, "821025" },
	{	   ZORK3,  15, "830331" },
	{	   ZORK3,  15, "840518" },
	{	   ZORK3,  16, "830410" },
	{	   ZORK3,  17, "840727" },
	{	MINIZORK,  34, "871124" },
	{	SAMPLER1,  26, "840731" },
	{	SAMPLER1,  53, "850407" },
	{	SAMPLER1,  55, "850823" },
	{	SAMPLER2,  97, "870601" },
	{      ENCHANTER,  10, "830810" },
	{      ENCHANTER,  15, "831107" },
	{      ENCHANTER,  16, "831118" },
	{      ENCHANTER,  24, "851118" },
	{      ENCHANTER,  29, "860820" },
	{	SORCERER,   4, "840131" },
	{	SORCERER,   6, "840508" },
	{	SORCERER,  13, "851021" },
	{	SORCERER,  15, "851108" },
	{	SORCERER,  18, "860904" },
	{	SORCERER,  67, "0"      },
	{	SORCERER,  63, "850916" },
	{	SORCERER,  87, "860904" },
	{   SPELLBREAKER,  63, "850916" },
	{   SPELLBREAKER,  87, "860904" },
	{     PLANETFALL,  20, "830708" },
	{     PLANETFALL,  26, "831014" },
	{     PLANETFALL,  29, "840118" },
	{     PLANETFALL,  37, "851003" },
	{     PLANETFALL,  10, "880531" },
	{    STATIONFALL, 107, "870430" },
	{	BALLYHOO,  97, "851218" },
	{    BORDER_ZONE,   9, "871008" },
	{	    AMFV,  77, "850814" },
	{	    AMFV,  79, "851122" },
	{	    HHGG,  47, "840914" },
	{	    HHGG,  56, "841221" },
	{	    HHGG,  58, "851002" },
	{	    HHGG,  59, "851108" },
	{	    HHGG,  31, "871119" },
	{	    LGOP,   0, "BLOWN!" },
	{	    LGOP,  50, "860711" },
	{	    LGOP,  59, "860730" },
	{	    LGOP,  59, "861114" },
	{	    LGOP, 118, "860325" },
	{	    LGOP,   4, "880405" },
	{	 SUSPECT,  14, "841005" },
	{       SHERLOCK,  21, "871214" },
	{       SHERLOCK,  26, "880127" },
	{    BEYOND_ZORK,  47, "870915" },
	{    BEYOND_ZORK,  49, "870917" },
	{    BEYOND_ZORK,  51, "870923" },
	{    BEYOND_ZORK,  57, "871221" },
	{      ZORK_ZERO, 296, "881019" },
	{      ZORK_ZERO, 366, "890323" },
	{      ZORK_ZERO, 383, "890602" },
	{      ZORK_ZERO, 393, "890714" },
	{         SHOGUN, 292, "890314" },
	{         SHOGUN, 295, "890321" },
	{         SHOGUN, 311, "890510" },
	{         SHOGUN, 322, "890706" },
	{         ARTHUR,  54, "890606" },
	{         ARTHUR,  63, "890622" },
	{         ARTHUR,  74, "890714" },
	{        JOURNEY,  26, "890316" },
	{        JOURNEY,  30, "890322" },
	{        JOURNEY,  77, "890616" },
	{        JOURNEY,  83, "890706" },
	{ LURKING_HORROR, 203, "870506" },
	{ LURKING_HORROR, 219, "870912" },
	{ LURKING_HORROR, 221, "870918" },
	{        UNKNOWN,   0, "------" }
    };

    /* Open story file */

    if ((story_fp = os_load_story()) == NULL)
        A00215 ("Cannot open story file");

    /* Allocate memory for story header */

    if ((zmp = (zbyte far *) malloc (64)) == NULL)
	A00215 ("Out of memory");

    /* Load header into memory */

    if (fread (zmp, 1, 64, story_fp) != 64)
	A00215 ("Story file read error");

    /* Copy header fields to global variables */

    LOW_BYTE (H_VERSION, A00035);

    if (A00035 < V1 || A00035 > V8)
	A00215 ("Unknown Z-code version");

    LOW_BYTE (H_CONFIG, A00036);

    if (A00035 == V3 && (A00036 & CONFIG_BYTE_SWAPPED))
	A00215 ("Byte swapped story file");

    LOW_WORD (H_RELEASE, A00037);
    LOW_WORD (H_RESIDENT_SIZE, A00038);
    LOW_WORD (H_START_PC, A00039);
    LOW_WORD (H_DICTIONARY, A00040);
    LOW_WORD (H_OBJECTS, A00041);
    LOW_WORD (H_GLOBALS, A00042);
    LOW_WORD (H_DYNAMIC_SIZE, A00043);
    LOW_WORD (H_FLAGS, A00044);

    for (i = 0, addr = H_SERIAL; i < 6; i++, addr++)
	LOW_BYTE (addr, A00045[i]);

    /* Auto-detect buggy story files that need special fixes */

    A00075 = UNKNOWN;

    for (i = 0; records[i].A00075 != UNKNOWN; i++) {

	if (A00037 == records[i].release) {

	    for (j = 0; j < 6; j++)
		if (A00045[j] != records[i].serial[j])
		    goto no_match;

	    A00075 = records[i].A00075;

	}

    no_match: ; /* null statement */

    }

    LOW_WORD (H_ABBREVIATIONS, A00046);
    LOW_WORD (H_FILE_SIZE, A00047);

    /* Calculate story file size in bytes */

    if (A00047 != 0) {

	A00076 = (long) 2 * A00047;

	if (A00035 >= V4)
	    A00076 *= 2;
	if (A00035 >= V6)
	    A00076 *= 2;

    } else {		/* some old games lack the file size entry */
	A00238 (story_fp, 0, SEEK_END);
	A00076 = A00239 (story_fp);
	A00238 (story_fp, 64, SEEK_SET);
    }

    LOW_WORD (H_CHECKSUM, A00048);
    LOW_WORD (H_ALPHABET, A00065);
    LOW_WORD (H_FUNCTIONS_OFFSET, A00057);
    LOW_WORD (H_STRINGS_OFFSET, A00058);
    LOW_WORD (H_TERMINATING_KEYS, A00061);
    LOW_WORD (H_EXTENSION_TABLE, A00066);

    /* Zork Zero Macintosh doesn't have the graphics flag set */

    if (A00075 == ZORK_ZERO && A00037 == 296)
	A00044 |= GRAPHICS_FLAG;

    /* Adjust opcode tables */

    if (A00035 <= V4) {
	op0_opcodes[0x09] = z_pop;
	op1_opcodes[0x0f] = z_not;
    } else {
	op0_opcodes[0x09] = A00099;
	op1_opcodes[0x0f] = A00097;
    }

    /* Allocate memory for story data */

    if ((zmp = (zbyte far *) realloc (zmp, A00076)) == NULL)
	A00215 ("Out of memory");

    /* Load story file in chunks of 32KB */

    n = 0x8000;

    for (size = 64; size < A00076; size += n) {

	if (A00076 - size < 0x8000)
	    n = (unsigned) (A00076 - size);

	SET_PC (size);

	if (fread (pcp, 1, n, story_fp) != n)
	    A00215 ("Story file read error");

    }

    /* Read header extension table */

    A00068 = get_header_extension (HX_TABLE_SIZE);
    A00071 = get_header_extension (HX_UNICODE_TABLE);

}/* A00260 */


/*
 * A00261
 *
 * Allocate memory for multiple undo. It is important not to occupy
 * all the memory available, since the IO interface may need memory
 * during the game, e.g. for loading sounds or pictures.
 *
 */
void A00261 (void)
{
    void far *reserved;

    reserved = NULL;	/* makes compilers shut up */

    if (A00094 != 0) {
	if ((reserved = malloc (A00094)) == NULL)
	    return;
    }

    /* Allocate A00043 bytes for previous dynamic zmp state
       + 1.5 A00043 for Quetzal diff + 2. */
    undo_mem = malloc ((A00043 * 5) / 2 + 2);
    if (undo_mem != NULL) {
	prev_zmp = undo_mem;
	undo_diff = undo_mem + A00043;
	memmove (prev_zmp, zmp, A00043);
    } else
	A00003.undo_slots = 0;

    if (A00094 != 0)
	free (reserved);

}/* A00261 */


/*
 * free_undo
 *
 * Free count undo blocks from the beginning of the undo list.
 *
 */
static void free_undo (int count)
{
    undo_t *p;

    if (count > undo_count)
	count = undo_count;
    while (count--) {
	p = first_undo;
	if (curr_undo == first_undo)
	    curr_undo = curr_undo->next;
	first_undo = first_undo->next;
	free (p);
	undo_count--;
    }
    if (first_undo)
	first_undo->prev = NULL;
    else
	last_undo = NULL;
}/* free_undo */


/*
 * A00262
 *
 * Close the story file and deallocate memory.
 *
 */
void A00262 (void)
{
    if (story_fp != NULL)
	fclose (story_fp);
    story_fp = NULL;

    if (undo_mem) {
	free_undo (undo_count);
	free (undo_mem);
    }

    undo_mem = NULL;
    undo_count = 0;

    if (zmp)
	free (zmp);
    zmp = NULL;
}/* A00262 */


/*
 * A00203
 *
 * Write a byte value to the dynamic Z-machine memory.
 *
 */
void A00203 (zword addr, zbyte value)
{
    addr = A00248(addr);
    if (addr >= A00043)
	A00188 (ERR_STORE_RANGE);

    if (addr == H_FLAGS + 1) {	/* flags register is modified */

	A00044 &= ~(SCRIPTING_FLAG | FIXED_FONT_FLAG);
	A00044 |= value & (SCRIPTING_FLAG | FIXED_FONT_FLAG);

	if (value & SCRIPTING_FLAG) {
	    if (!A00080)
		A00026 ();
	} else {
	    if (A00080)
		A00027 ();
	}

	A00024 ();

    }

    SET_BYTE (addr, value);

}/* A00203 */


/*
 * A00204
 *
 * Write a word value to the dynamic Z-machine memory.
 *
 */
void A00204 (zword addr, zword value)
{
    A00203 (A00248(addr + 0), hi (value));
    A00203 (A00248(addr + 1), lo (value));

}/* A00204 */


/*
 * A00157, re-load dynamic area, clear the stack and set the PC.
 *
 * 	no zargs used
 *
 */
void A00157 (void)
{
    static bool first_restart = TRUE;

    A00194 ();

    A00230 (RESTART_BEGIN);

    A00022 (0);

    if (!first_restart) {

	A00238 (story_fp, 0, SEEK_SET);

	if (fread (zmp, 1, A00043, story_fp) != A00043)
	    A00215 ("Story file read error");

    } else first_restart = FALSE;

    restart_header ();
    A00023 ();

    sp = fp = stack + STACK_SIZE;
    A00077 = 0;

    if (A00035 != V6) {

	long pc = (long) A00039;
	SET_PC (pc);

    } else call (A00039, 0, NULL, 0);

    A00230 (RESTART_END);

}/* A00157 */


/*
 * get_default_name
 *
 * Read a default file name from the memory of the Z-machine and
 * copy it to a string.
 *
 */
static void get_default_name (char *default_name, zword addr)
{
    if (addr != 0) {

	zbyte len;
	int i;

	LOW_BYTE (addr, len);
	addr++;

	for (i = 0; i < len; i++) {

	    zbyte c;

	    LOW_BYTE (addr, c);
	    addr++;

	    if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';

	    default_name[i] = c;

	}

	default_name[i] = 0;

	if (strchr (default_name, '.') == NULL)
	    strncat(default_name, EXT_AUX, strlen(default_name) - strlen(EXT_AUX) + 1);
    } else {
	free(default_name);
	default_name = strdup(A00003.aux_name);
    }

}/* get_default_name */


/*
 * A00158, restore [a part of] a Z-machine state from disk
 *
 *	zargs[0] = address of area to restore (optional)
 *	zargs[1] = number of bytes to restore
 *	zargs[2] = address of suggested file name
 *
 */
void A00158 (void)
{
    char *new_name;
    char default_name[MAX_FILE_NAME + 1];
    FILE *gfp = NULL;

    zword success = 0;

    if (zargc != 0) {

	/* Get the file name */

	get_default_name (default_name, (zargc >= 3) ? zargs[2] : 0);

	new_name = A00226(default_name, FILE_LOAD_AUX);
	if (new_name == NULL)
	    goto finished;

	free(A00003.aux_name);
	A00003.aux_name = strdup(default_name);

	/* Open auxilary file */

	if ((gfp = fopen (new_name, "rb")) == NULL)
	    goto finished;

	/* Load auxilary file */

	success = fread (zmp + zargs[0], 1, zargs[1], gfp);

	/* Close auxilary file */

	fclose (gfp);

    } else {

        /***
//	long pc;
//	zword release;
//	zword addr;
//	int i;
***/

	/* Get the file name */

	new_name = A00226(A00003.save_name, FILE_RESTORE);
	if (new_name == NULL)
	    goto finished;

	free(A00003.save_name);
	A00003.save_name = strdup(new_name);

	/* Open game file */

	if ((gfp = fopen (new_name, "rb")) == NULL)
	    goto finished;

	success = A00029 (gfp, story_fp);

	if (A00247( success ) >= 0) {

	    /* Close game file */

	    fclose (gfp);

	    if (A00247( success ) > 0) {
		zbyte old_screen_rows;
		zbyte old_screen_cols;

		/* In V3, reset the upper window. */
		if (A00035 == V3)
		    A00025 (0);

		LOW_BYTE (H_SCREEN_ROWS, old_screen_rows);
		LOW_BYTE (H_SCREEN_COLS, old_screen_cols);

		/* Reload cached header fields. */
		restart_header ();

		/*
		 * Since QUETZAL files may be saved on many different machines,
		 * the screen sizes may vary a lot. Erasing the status window
		 * seems to cover up most of the resulting badness.
		 */
		if (A00035 > V3 && A00035 != V6
		    && (A00051 != old_screen_rows
		    || A00052 != old_screen_cols))
		    A00030 (1);
	    }
	} else
	    A00215 ("Error reading save file");
    }

finished:

    if (gfp == NULL && A00003.restore_mode)
	A00215 ("Error reading save file");

    if (A00035 <= V3)
	A00202 (success);
    else
	store (success);

}/* A00158 */


/*
 * mem_diff
 *
 * Set diff to a Quetzal-like difference between a and b,
 * copying a to b as we go.  It is assumed that diff points to a
 * buffer which is large enough to hold the diff.
 * mem_size is the number of bytes to compare.
 * Returns the number of bytes copied to diff.
 *
 */
static long mem_diff (zbyte *a, zbyte *b, zword mem_size, zbyte *diff)
{
    unsigned size = mem_size;
    zbyte *p = diff;
    unsigned j;
    zbyte c;

    for (;;) {
	for (j = 0; size > 0 && (c = *a++ ^ *b++) == 0; j++)
	    size--;
	if (size == 0) break;
	size--;
	if (j > 0x8000) {
	    *p++ = 0;
	    *p++ = 0xff;
	    *p++ = 0xff;
	    j -= 0x8000;
	}
	if (j > 0) {
	    *p++ = 0;
	    j--;
	    if (j <= 0x7f) {
		*p++ = j;
	    } else {
		*p++ = (j & 0x7f) | 0x80;
		*p++ = (j & 0x7f80) >> 7;
	    }
	}
	*p++ = c;
	*(b - 1) ^= c;
    }
    return p - diff;
}/* mem_diff */


/*
 * mem_undiff
 *
 * Applies a quetzal-like diff to dest
 *
 */
static void mem_undiff (zbyte *diff, long diff_length, zbyte *dest)
{
    zbyte c;

    while (diff_length) {
	c = *diff++;
	diff_length--;
	if (c == 0) {
	    unsigned runlen;

	    if (!diff_length)
		return;  /* Incomplete run */
	    runlen = *diff++;
	    diff_length--;
	    if (runlen & 0x80) {
		if (!diff_length)
		    return; /* Incomplete extended run */
		c = *diff++;
		diff_length--;
		runlen = (runlen & 0x7f) | (((unsigned) c) << 7);
	    }

	    dest += runlen + 1;
	} else {
	    *dest++ ^= c;
	}
    }
}/* mem_undiff */


/*
 * A00249
 *
 * This function does the dirty work for A00159.
 *
 */
int A00249 (void)
{
    if (A00003.undo_slots == 0)	/* undo feature unavailable */

	return -1;

    if (curr_undo == NULL)		/* no saved game state */

	return 0;

    /* undo possible */

    memmove (zmp, prev_zmp, A00043);
    SET_PC (curr_undo->pc);
    sp = stack + STACK_SIZE - curr_undo->stack_size;
    fp = stack + curr_undo->frame_offset;
    A00077 = curr_undo->A00077;
    mem_undiff ((zbyte *) (curr_undo + 1), curr_undo->diff_size, prev_zmp);
    memmove (sp, (zbyte *)(curr_undo + 1) + curr_undo->diff_size,
	    curr_undo->stack_size * sizeof (*sp));

    curr_undo = curr_undo->prev;

    restart_header ();

    return 2;

}/* A00249 */


/*
 * A00159, restore a Z-machine state from memory.
 *
 *	no zargs used
 *
 */
void A00159 (void)
{
    store ((zword) A00249 ());

}/* A00159 */


/*
 * A00163, save [a part of] the Z-machine state to disk.
 *
 *	zargs[0] = address of memory area to save (optional)
 *	zargs[1] = number of bytes to save
 *	zargs[2] = address of suggested file name
 *
 */
void A00163 (void)
{
    char *new_name;
    char default_name[MAX_FILE_NAME + 1];
    FILE *gfp;

    zword success = 0;

    if (zargc != 0) {

	/* Get the file name */

	get_default_name (default_name, (zargc >= 3) ? zargs[2] : 0);

	new_name = A00226(default_name, FILE_SAVE_AUX);
	if (new_name == NULL)
	    goto finished;

	free(A00003.aux_name);
	A00003.aux_name = strdup(default_name);

	/* Open auxilary file */

	if ((gfp = fopen (new_name, "wb")) == NULL)
	    goto finished;

	/* Write auxilary file */

	success = fwrite (zmp + zargs[0], zargs[1], 1, gfp);

	/* Close auxilary file */

	fclose (gfp);

    } else {

        /***
//	long pc;
//	zword addr;
//	zword nsp, nfp;
//	int skip;
//	int i;
***/

	/* Get the file name */

	new_name = A00226(A00003.save_name, FILE_SAVE);
	if (new_name == NULL)
	    goto finished;

	free(A00003.save_name);
	A00003.save_name = strdup(new_name);

	/* Open game file */

	if ((gfp = fopen (new_name, "wb")) == NULL)
	    goto finished;

	success = A00028 (gfp, story_fp);

	/* Close game file and check for errors */

	if (fclose (gfp) == EOF || ferror (story_fp)) {
	    A00199 ("Error writing save file\n");
	    goto finished;
	}

	/* Success */

	success = 1;
    }

finished:

    if (A00035 <= V3)
	A00202 (success);
    else
	store (success);

}/* A00163 */


/*
 * A00255
 *
 * This function does the dirty work for A00164.
 *
 */
int A00255 (void)
{
    long diff_size;
    zword stack_size;
    undo_t *p;
    long pc;

    if (A00003.undo_slots == 0)	/* undo feature unavailable */
	return -1;

    /* save undo possible */

    while (last_undo != curr_undo) {
	p = last_undo;
	last_undo = last_undo->prev;
	free (p);
	undo_count--;
    }
    if (last_undo)
	last_undo->next = NULL;
    else
	first_undo = NULL;

    if (undo_count == A00003.undo_slots)
	free_undo (1);

    diff_size = mem_diff (zmp, prev_zmp, A00043, undo_diff);
    stack_size = stack + STACK_SIZE - sp;
    do {
	p = malloc (sizeof (undo_t) + diff_size + stack_size * sizeof (*sp));
	if (p == NULL)
	    free_undo (1);
    } while (!p && undo_count);
    if (p == NULL)
	return -1;
    pc = p->pc;
    GET_PC (pc);	/* Turbo C doesn't like seeing p->pc here */
    p->pc = pc;
    p->A00077 = A00077;
    p->diff_size = diff_size;
    p->stack_size = stack_size;
    p->frame_offset = fp - stack;
    memmove (p + 1, undo_diff, diff_size);
    memmove ((zbyte *)(p + 1) + diff_size, sp, stack_size * sizeof (*sp));

    if (!first_undo) {
	p->prev = NULL;
	first_undo = p;
    } else {
	last_undo->next = p;
	p->prev = last_undo;
    }
    p->next = NULL;
    curr_undo = last_undo = p;
    undo_count++;
    return 1;

}/* A00255 */


/*
 * A00164, save the current Z-machine state for a future undo.
 *
 *	no zargs used
 *
 */
void A00164 (void)
{
    store ((zword) A00255 ());

}/* A00164 */


/*
 * A00184, check the story file integrity.
 *
 *	no zargs used
 *
 */
void A00184 (void)
{
    zword checksum = 0;
    long i;

    /* Sum all bytes in story file except header bytes */

    A00238 (story_fp, 64, SEEK_SET);

    for (i = 64; i < A00076; i++)
	checksum += fgetc (story_fp);

    /* Branch if the checksums are equal */

    A00202 (checksum == A00048);

}/* A00184 */
