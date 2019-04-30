/*
 * fastmem.c
 *
 * Memory related functions (fast version without virtual memory)
 *
 */

#include <stdio.h>
#include <string.h>
#include "frotz.h"

#include <stdlib.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define far


extern void A00016 (int);
extern void A00017 (void);
extern void A00018 (void);
extern void call (zword, int, zword *, int);
extern void A00019 (zword);
extern void A00020 (void);
extern void A00021 (void);

extern void (*op0_opcodes[]) (void);
extern void (*op1_opcodes[]) (void);
extern void (*op2_opcodes[]) (void);
extern void (*var_opcodes[]) (void);

char save_name[MAX_FILE_NAME + 1] = DEFAULT_SAVE_NAME;
char auxilary_name[MAX_FILE_NAME + 1] = DEFAULT_AUXILARY_NAME;

zbyte far *zmp = NULL;
zbyte far *pcp = NULL;

static FILE *story_fp = NULL;

static zbyte far *undo[MAX_UNDO_SLOTS];

static undo_slots = 0;
static undo_count = 0;
static undo_valid = 0;

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

    if (A00056 == 0 || entry > A00058)
	return 0;

    addr = A00056 + 2 * entry;
    LOW_WORD (addr, val)

    return val;

}/* get_header_extension */

/*
 * A00240
 *
 * Set an entry in the header extension (former mouse table).
 *
 */

void A00240 (int entry, zword val)
{
    zword addr;

    if (A00056 == 0 || entry > A00058)
	return;

    addr = A00056 + 2 * entry;
    SET_WORD (addr, val)

}/* A00240 */

/*
 * restart_header
 *
 * Set all header fields which hold information about the A00235er.
 *
 */

void restart_header (void)
{
    zword screen_x_size;
    zword screen_y_size;
    zbyte font_x_size;
    zbyte font_y_size;

    int i;

    SET_BYTE (H_CONFIG, A00026)
    SET_WORD (H_FLAGS, A00034)

    if (A00025 >= V4) {
	SET_BYTE (H_INTERPRETER_NUMBER, A00039)
	SET_BYTE (H_INTERPRETER_VERSION, A00040)
	SET_BYTE (H_SCREEN_ROWS, A00041)
	SET_BYTE (H_SCREEN_COLS, A00042)
    }

    /* It's less trouble to use font size 1x1 for V5 games, especially
       because of a bug in the unreleased German version of "Zork 1" */

    if (A00025 != V6) {
	screen_x_size = (zword) A00042;
	screen_y_size = (zword) A00041;
	font_x_size = 1;
	font_y_size = 1;
    } else {
	screen_x_size = A00043;
	screen_y_size = A00044;
	font_x_size = A00046;
	font_y_size = A00045;
    }

    if (A00025 >= V5) {
	SET_WORD (H_SCREEN_WIDTH, screen_x_size)
	SET_WORD (H_SCREEN_HEIGHT, screen_y_size)
	SET_BYTE (H_FONT_HEIGHT, font_y_size)
	SET_BYTE (H_FONT_WIDTH, font_x_size)
	SET_BYTE (H_DEFAULT_BACKGROUND, A00049)
	SET_BYTE (H_DEFAULT_FOREGROUND, A00050)
    }

    if (A00025 == V6)
	for (i = 0; i < 8; i++)
	    A00194 ((zword) (H_USER_NAME + i), A00057[i]);

    SET_BYTE (H_STANDARD_HIGH, A00053)
    SET_BYTE (H_STANDARD_LOW, A00054)

}/* restart_header */

/*
 * A00236
 *
 * Allocate memory and load the story file.
 *
 */

void A00236 (void)
{
    long size;
    zword addr;
    unsigned n;
    int i, j;

    static struct {
	enum story A00063;
	zword release;
	zbyte serial[6];
    } records[] = {
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

    if ((story_fp = fopen (A00062, "rb")) == NULL)
	A00202 ("Cannot open story file");

    /* Allocate memory for story header */

    if ((zmp = (zbyte far *) malloc (64)) == NULL)
	A00202 ("Out of memory");

    /* Load header into memory */

    if (fread (zmp, 1, 64, story_fp) != 64)
	A00202 ("Story file read error");

    /* Copy header fields to global variables */

    LOW_BYTE (H_VERSION, A00025)

    if (A00025 < V1 || A00025 > V8)
	A00202 ("Unknown Z-code version");

    LOW_BYTE (H_CONFIG, A00026)

    if (A00025 == V3 && (A00026 & CONFIG_BYTE_SWAPPED))
	A00202 ("Byte swapped story file");

    LOW_WORD (H_RELEASE, A00027)
    LOW_WORD (H_RESIDENT_SIZE, A00028)
    LOW_WORD (H_START_PC, A00029)
    LOW_WORD (H_DICTIONARY, A00030)
    LOW_WORD (H_OBJECTS, A00031)
    LOW_WORD (H_GLOBALS, A00032)
    LOW_WORD (H_DYNAMIC_SIZE, A00033)
    LOW_WORD (H_FLAGS, A00034)

    for (i = 0, addr = H_SERIAL; i < 6; i++, addr++)
	LOW_BYTE (addr, A00035[i])

    /* Auto-detect buggy story files that need special fixes */

    for (i = 0; records[i].A00063 != UNKNOWN; i++) {

	if (A00027 == records[i].release) {

	    for (j = 0; j < 6; j++)
		if (A00035[j] != records[i].serial[j])
		    goto no_match;

	    A00063 = records[i].A00063;

	}

    no_match:
        (void)0;
    }

    LOW_WORD (H_ABBREVIATIONS, A00036)
    LOW_WORD (H_FILE_SIZE, A00037)

    /* Calculate story file size in bytes */

    if (A00037 != 0) {

	A00064 = (long) 2 * A00037;

	if (A00025 >= V4)
	    A00064 *= 2;
	if (A00025 >= V6)
	    A00064 *= 2;

    } else {		/* some old games lack the file size entry */

	fseek (story_fp, 0, SEEK_END);
	A00064 = ftell (story_fp);
	fseek (story_fp, 64, SEEK_SET);

    }

    LOW_WORD (H_CHECKSUM, A00038)
    LOW_WORD (H_ALPHABET, A00055)
    LOW_WORD (H_FUNCTIONS_OFFSET, A00047)
    LOW_WORD (H_STRINGS_OFFSET, A00048)
    LOW_WORD (H_TERMINATING_KEYS, A00051)
    LOW_WORD (H_EXTENSION_TABLE, A00056)

    /* Zork Zero Macintosh doesn't have the graphics flag set */

    if (A00063 == ZORK_ZERO && A00027 == 296)
	A00034 |= GRAPHICS_FLAG;

    /* Adjust opcode tables */

    if (A00025 <= V4) {
	op0_opcodes[0x09] = z_pop;
	op1_opcodes[0x0f] = z_not;
    } else {
	op0_opcodes[0x09] = A00094;
	op1_opcodes[0x0f] = A00092;
    }

    /* Allocate memory for story data */

    if ((zmp = (zbyte far *) realloc (zmp, A00064)) == NULL)
	A00202 ("Out of memory");

    /* Load story file in chunks of 32KB */

    n = 0x8000;

    for (size = 64; size < A00064; size += n) {

	if (A00064 - size < 0x8000)
	    n = (unsigned) (A00064 - size);

	SET_PC (size)

	if (fread (pcp, 1, n, story_fp) != n)
	    A00202 ("Story file read error");

    }

    /* Read header extension table */

    A00058 = get_header_extension (HX_TABLE_SIZE);
    A00061 = get_header_extension (HX_UNICODE_TABLE);

}/* A00236 */

/*
 * A00237
 *
 * Allocate memory for multiple undo. It is important not to occupy
 * all the memory available, since the IO interface may need memory
 * during the game, e.g. for loading sounds or pictures.
 *
 */

void A00237 (void)
{
    void far *reserved;

    if (A00089 != 0)
	if ((reserved = malloc (A00089)) == NULL)
	    return;

    while (undo_slots < A00086 && undo_slots < MAX_UNDO_SLOTS) {

	void far *mem = malloc ((long) sizeof (stack) + A00033);

	if (mem == NULL)
	    break;

	undo[undo_slots++] = mem;

    }

    if (A00089 != 0)
	free (reserved);

}/* A00237 */

/*
 * A00238
 *
 * Close the story file and deallocate memory.
 *
 */

void A00238 (void)
{

    fclose (story_fp);

    while (undo_slots--)
	free (undo[undo_slots]);

    free (zmp);

}/* A00238 */

/*
 * A00194
 *
 * Write a byte value to the dynamic Z-machine memory.
 *
 */

void A00194 (zword addr, zbyte value)
{

    if (addr >= A00033)
	A00192 ("Store out of dynamic memory");

    if (addr == H_FLAGS + 1) {	/* flags register is modified */

	A00034 &= ~(SCRIPTING_FLAG | FIXED_FONT_FLAG);
	A00034 |= value & (SCRIPTING_FLAG | FIXED_FONT_FLAG);

	if (value & SCRIPTING_FLAG) {
	    if (!A00066)
		A00020 ();
	} else {
	    if (A00066)
		A00021 ();
	}

	A00018 ();

    }

    SET_BYTE (addr, value)

}/* A00194 */

/*
 * A00195
 *
 * Write a word value to the dynamic Z-machine memory.
 *
 */

void A00195 (zword addr, zword value)
{

    A00194 ((zword) (addr + 0), hi (value));
    A00194 ((zword) (addr + 1), lo (value));

}/* A00195 */

/*
 * A00152, re-load dynamic area, clear the stack and set the PC.
 *
 * 	no zargs used
 *
 */

void A00152 (void)
{
    static bool first_restart = TRUE;

    A00184 ();

    A00216 (RESTART_BEGIN);

    A00016 (0);

    if (!first_restart) {

	fseek (story_fp, 0, SEEK_SET);

	if (fread (zmp, 1, A00033, story_fp) != A00033)
	    A00202 ("Story file read error");

    } else first_restart = FALSE;

    restart_header ();
    A00017 ();

    sp = fp = stack + STACK_SIZE;

    if (A00025 != V6) {

	long pc = (long) A00029;
	SET_PC (pc)

    } else call (A00029, 0, NULL, 0);

    A00216 (RESTART_END);

}/* A00152 */

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

	LOW_BYTE (addr, len)
	addr++;

	for (i = 0; i < len; i++) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    if (c >= 'A' && c <= 'Z')
		c += 'a' - 'A';

	    default_name[i] = c;

	}

	default_name[i] = 0;

	if (strchr (default_name, '.') == NULL)
	    strcpy (default_name + i, ".AUX");

    } else strcpy (default_name, auxilary_name);

}/* get_default_name */

/*
 * A00153, restore [a part of] a Z-machine state from disk
 *
 *	zargs[0] = address of area to restore (optional)
 *	zargs[1] = number of bytes to restore
 *	zargs[2] = address of suggested file name
 *
 */

void A00153 (void)
{
    char new_name[MAX_FILE_NAME + 1];
    char default_name[MAX_FILE_NAME + 1];
    FILE *gfp;

    zword success = 0;

    if (zargc != 0) {

	/* Get the file name */

	get_default_name (default_name, (zargc >= 3) ? zargs[2] : 0);

	if (A00212 (new_name, default_name, FILE_LOAD_AUX) == 0)
	    goto finished;

	strcpy (auxilary_name, default_name);

	/* Open auxilary file */

	if ((gfp = fopen (new_name, "rb")) == NULL)
	    goto finished;

	/* Load auxilary file */

	success = fread (zmp + zargs[0], 1, zargs[1], gfp);

	/* Close auxilary file */

	fclose (gfp);

    } else {

	long pc;
	zword release;
	zword addr;
	int i;

	/* Get the file name */

	if (A00212 (new_name, save_name, FILE_RESTORE) == 0)
	    goto finished;

	strcpy (save_name, new_name);

	/* Open game file */

	if ((gfp = fopen (new_name, "rb")) == NULL)
	    goto finished;

	/* Load game file */

	release = (unsigned) fgetc (gfp) << 8;
	release |= fgetc (gfp);

	(void) fgetc (gfp);
	(void) fgetc (gfp);

	/* Check the release number */

	if (release == A00027) {

	    pc = (long) fgetc (gfp) << 16;
	    pc |= (unsigned) fgetc (gfp) << 8;
	    pc |= fgetc (gfp);

	    SET_PC (pc)

	    sp = stack + (fgetc (gfp) << 8);
	    sp += fgetc (gfp);
	    fp = stack + (fgetc (gfp) << 8);
	    fp += fgetc (gfp);

	    for (i = (int) (sp - stack); i < STACK_SIZE; i++) {
		stack[i] = (unsigned) fgetc (gfp) << 8;
		stack[i] |= fgetc (gfp);
	    }

	    fseek (story_fp, 0, SEEK_SET);

	    for (addr = 0; addr < A00033; addr++) {
		int skip = fgetc (gfp);
		for (i = 0; i < skip; i++)
		    zmp[addr++] = fgetc (story_fp);
		zmp[addr] = fgetc (gfp);
		(void) fgetc (story_fp);
	    }

	    /* Check for errors */

	    if (ferror (gfp) || ferror (story_fp) || addr != A00033)
		A00202 ("Error reading save file");

	    /* Reset upper window (V3 only) */

	    if (A00025 == V3)
		A00019 (0);

	    /* Initialise story header */

	    restart_header ();

	    /* Success */

	    success = 2;

	} else A00189 ("Invalid save file\n");

	/* Close game file */

	fclose (gfp);

    }

finished:

    if (A00025 <= V3)
	A00193 (success);
    else
	store (success);

}/* A00153 */

/*
 * A00225
 *
 * This function does the dirty work for A00154.
 *
 */

int A00225 (void)
{

    if (undo_slots == 0)	/* undo feature unavailable */

	return -1;

    else if (undo_valid == 0)	/* no saved game state */

	return 0;

    else {			/* undo possible */

	long pc;

	if (undo_count == 0)
	    undo_count = undo_slots;

	memcpy (stack, undo[undo_count - 1], sizeof (stack));
	memcpy (zmp, undo[undo_count - 1] + sizeof (stack), A00033);

	pc = ((long) stack[0] << 16) | stack[1];
	sp = stack + stack[2];
	fp = stack + stack[3];

	SET_PC (pc)

	restart_header ();

	undo_count--;
	undo_valid--;

	return 2;

    }

}/* A00225 */

/*
 * A00154, restore a Z-machine state from memory.
 *
 *	no zargs used
 *
 */

void A00154 (void)
{

    store ((zword) A00225 ());

}/* A00225 */

/*
 * A00158, save [a part of] the Z-machine state to disk.
 *
 *	zargs[0] = address of memory area to save (optional)
 *	zargs[1] = number of bytes to save
 *	zargs[2] = address of suggested file name
 *
 */

void A00158 (void)
{
    char new_name[MAX_FILE_NAME + 1];
    char default_name[MAX_FILE_NAME + 1];
    FILE *gfp;

    zword success = 0;

    if (zargc != 0) {

	/* Get the file name */

	get_default_name (default_name, (zargc >= 3) ? zargs[2] : 0);

	if (A00212 (new_name, default_name, FILE_SAVE_AUX) == 0)
	    goto finished;

	strcpy (auxilary_name, default_name);

	/* Open auxilary file */

	if ((gfp = fopen (new_name, "wb")) == NULL)
	    goto finished;

	/* Write auxilary file */

	success = fwrite (zmp + zargs[0], zargs[1], 1, gfp);

	/* Close auxilary file */

	fclose (gfp);

    } else {

	long pc;
	zword addr;
	zword nsp, nfp;
	int skip;
	int i;

	/* Get the file name */

	if (A00212 (new_name, save_name, FILE_SAVE) == 0)
	    goto finished;

	strcpy (save_name, new_name);

	/* Open game file */

	if ((gfp = fopen (new_name, "wb")) == NULL)
	    goto finished;

	/* Write game file */

	fputc ((int) hi (A00027), gfp);
	fputc ((int) lo (A00027), gfp);
	fputc ((int) hi (A00038), gfp);
	fputc ((int) lo (A00038), gfp);

	GET_PC (pc)

	fputc ((int) (pc >> 16) & 0xff, gfp);
	fputc ((int) (pc >> 8) & 0xff, gfp);
	fputc ((int) (pc) & 0xff, gfp);

	nsp = (int) (sp - stack);
	nfp = (int) (fp - stack);

	fputc ((int) hi (nsp), gfp);
	fputc ((int) lo (nsp), gfp);
	fputc ((int) hi (nfp), gfp);
	fputc ((int) lo (nfp), gfp);

	for (i = nsp; i < STACK_SIZE; i++) {
	    fputc ((int) hi (stack[i]), gfp);
	    fputc ((int) lo (stack[i]), gfp);
	}

	fseek (story_fp, 0, SEEK_SET);

	for (addr = 0, skip = 0; addr < A00033; addr++)
	    if (zmp[addr] != fgetc (story_fp) || skip == 255 || addr + 1 == A00033) {
		fputc (skip, gfp);
		fputc (zmp[addr], gfp);
		skip = 0;
	    } else skip++;

	/* Close game file and check for errors */

	if (fclose (gfp) == EOF || ferror (story_fp)) {
	    A00189 ("Error writing save file\n");
	    goto finished;
	}

	/* Success */

	success = 1;

    }

finished:

    if (A00025 <= V3)
	A00193 (success);
    else
	store (success);

}/* A00158 */

/*
 * A00231
 *
 * This function does the dirty work for A00159.
 *
 */

int A00231 (void)
{
    long pc;

    if (undo_slots == 0)	/* undo feature unavailable */

	return -1;

    else {			/* save undo possible */

	if (undo_count == undo_slots)
	    undo_count = 0;

	GET_PC (pc)

	stack[0] = (zword) (pc >> 16);
	stack[1] = (zword) (pc & 0xffff);
	stack[2] = (zword) (sp - stack);
	stack[3] = (zword) (fp - stack);

	memcpy (undo[undo_count], stack, sizeof (stack));
	memcpy (undo[undo_count] + sizeof (stack), zmp, A00033);

	if (++undo_count == undo_slots)
	    undo_count = 0;
	if (++undo_valid > undo_slots)
	    undo_valid = undo_slots;

	return 1;

    }

}/* A00231 */

/*
 * A00159, save the current Z-machine state for a future undo.
 *
 *	no zargs used
 *
 */

void A00159 (void)
{

    store ((zword) A00231 ());

}/* A00159 */

/*
 * A00179, check the story file integrity.
 *
 *	no zargs used
 *
 */

void A00179 (void)
{
    zword checksum = 0;
    long i;

    /* Sum all bytes in story file except header bytes */

    fseek (story_fp, 64, SEEK_SET);

    for (i = 64; i < A00064; i++)
	checksum += fgetc (story_fp);

    /* Branch if the checksums are equal */

    A00193 (checksum == A00038);

}/* A00179 */
