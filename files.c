/*
 * files.c
 *
 * Transscription, recording and playback
 *
 */

#include <stdio.h>
#include <string.h>
#include "frotz.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

extern void A00022 (bool);

extern bool A00003 (zchar);

extern bool A00023 (const char *);

char script_name[MAX_FILE_NAME + 1] = DEFAULT_SCRIPT_NAME;
char command_name[MAX_FILE_NAME + 1] = DEFAULT_COMMAND_NAME;

#ifdef __MSDOS__
extern char A00024[];
#endif

static script_width = 0;

static FILE *sfp = NULL;
static FILE *rfp = NULL;
static FILE *pfp = NULL;

/*
 * A00020
 *
 * Open the transscript file. 'AMFV' makes this more complicated as it
 * turns transscription on/off several times to exclude some text from
 * the transscription file. This wasn't a problem for the original V4
 * A00235ers which always sent transscription to the printer, but it
 * means a problem to modern A00235ers that offer to open a new file
 * every time transscription is turned on. Our solution is to append to
 * the old transscription file in V1 to V4, and to ask for a new file
 * name in V5+.
 *
 */

void A00020 (void)
{
    static bool script_valid = FALSE;

    char new_name[MAX_FILE_NAME + 1];

    A00034 &= ~SCRIPTING_FLAG;

    if (A00025 >= V5 || !script_valid) {

	if (!A00212 (new_name, script_name, FILE_SCRIPT))
	    goto done;

	strcpy (script_name, new_name);

    }

    /* Opening in "at" mode doesn't work for A00254... */

    if ((sfp = fopen (script_name, "r+t")) != NULL || (sfp = fopen (script_name, "w+t")) != NULL) {

	fseek (sfp, 0, SEEK_END);

	A00034 |= SCRIPTING_FLAG;

	script_valid = TRUE;
	A00066 = TRUE;

	script_width = 0;

    } else A00189 ("Cannot open file\n");

done:

    SET_WORD (H_FLAGS, A00034)

}/* A00020 */

/*
 * A00021
 *
 * Stop transscription.
 *
 */

void A00021 (void)
{

    A00034 &= ~SCRIPTING_FLAG;
    SET_WORD (H_FLAGS, A00034)

    fclose (sfp); A00066 = FALSE;

}/* A00021 */

/*
 * A00252
 *
 * Write a newline to the transscript file.
 *
 */

void A00252 (void)
{

    if (fputc ('\n', sfp) == EOF)
	A00021 ();

    script_width = 0;

}/* A00252 */

/*
 * A00250
 *
 * Write a single character to the transscript file.
 *
 */

void A00250 (zchar c)
{

    if (c == ZC_INDENT && script_width != 0)
	c = ' ';

    if (c == ZC_INDENT)
	{ A00250 (' '); A00250 (' '); A00250 (' '); return; }
    if (c == ZC_GAP)
	{ A00250 (' '); A00250 (' '); return; }

#ifdef __MSDOS__
    if (c >= ZC_LATIN1_MIN)
	c = A00024[c - ZC_LATIN1_MIN];
#endif

    fputc (c, sfp); script_width++;

}/* A00250 */

/*
 * A00251
 *
 * Write a string to the transscript file.
 *
 */

void A00251 (const zchar *s)
{
    int width;
    int i;

    if (*s == ZC_INDENT && script_width != 0)
	A00250 (*s++);

    for (i = 0, width = 0; s[i] != 0; i++)

	if (s[i] == ZC_NEW_STYLE || s[i] == ZC_NEW_FONT)
	    i++;
	else if (s[i] == ZC_GAP)
	    width += 3;
	else if (s[i] == ZC_INDENT)
	    width += 2;
	else
	    width += 1;

    if (A00088 != 0 && script_width + width > A00088) {

	if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
	    s++;

	A00252 ();

    }

    for (i = 0; s[i] != 0; i++)

	if (s[i] == ZC_NEW_FONT || s[i] == ZC_NEW_STYLE)
	    i++;
	else
	    A00250 (s[i]);

}/* A00251 */

/*
 * A00253
 *
 * Send an input line to the transscript file.
 *
 */

void A00253 (const zchar *buf, zchar key)
{
    int width;
    int i;

    for (i = 0, width = 0; buf[i] != 0; i++)
	width++;

    if (A00088 != 0 && script_width + width > A00088)
	A00252 ();

    for (i = 0; buf[i] != 0; i++)
	A00250 (buf[i]);

    if (key == ZC_RETURN)
	A00252 ();

}/* A00253 */

/*
 * A00254
 *
 * Remove an input line from the transscript file.
 *
 */

void A00254 (const zchar *buf)
{
    int width;
    int i;

    for (i = 0, width = 0; buf[i] != 0; i++)
	width++;

    fseek (sfp, -width, SEEK_CUR); script_width -= width;

}/* A00254 */

/*
 * A00255
 *
 * Start sending a "debugging" A00070 to the transscript file.
 *
 */

void A00255 (void)
{

    if (script_width != 0)
	A00252 ();

    A00250 (ZC_INDENT);

}/* A00255 */

/*
 * A00256
 *
 * Stop writing a "debugging" A00070.
 *
 */

void A00256 (void)
{

    A00252 ();

}/* A00256 */

/*
 * A00229
 *
 * Open a file to record the player's input.
 *
 */

void A00229 (void)
{
    char new_name[MAX_FILE_NAME + 1];

    if (A00212 (new_name, command_name, FILE_RECORD)) {

	strcpy (command_name, new_name);

	if ((rfp = fopen (new_name, "wt")) != NULL)
	    A00068 = TRUE;
	else
	    A00189 ("Cannot open file\n");

    }

}/* A00229 */

/*
 * A00230
 *
 * Stop recording the player's input.
 *
 */

void A00230 (void)
{

    fclose (rfp); A00068 = FALSE;

}/* A00230 */

/*
 * record_code
 *
 * Helper function for record_char.
 *
 */

static void record_code (int c, bool force_encoding)
{

    if (force_encoding || c == '[' || c < 0x20 || c > 0x7e) {

	int i;

	fputc ('[', rfp);

	for (i = 10000; i != 0; i /= 10)
	    if (c >= i || i == 1)
		fputc ('0' + (c / i) % 10, rfp);

	fputc (']', rfp);

    } else fputc (c, rfp);

}/* record_code */

/*
 * record_char
 *
 * Write a character to the command file.
 *
 */

static void record_char (zchar c)
{

    if (c != ZC_RETURN)

	if (c < ZC_HKEY_MIN || c > ZC_HKEY_MAX) {

	    record_code (A00183 (c), FALSE);

	    if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
		record_code (A00071, TRUE);
		record_code (A00072, TRUE);
	    }

	} else record_code (1000 + c - ZC_HKEY_MIN, TRUE);

}/* record_char */

/*
 * A00248
 *
 * Copy a keystroke to the command file.
 *
 */

void A00248 (zchar key)
{

    record_char (key);

    if (fputc ('\n', rfp) == EOF)
	A00230 ();

}/* A00248 */

/*
 * A00249
 *
 * Copy a line of input to a command file.
 *
 */

void A00249 (const zchar *buf, zchar key)
{
    zchar c;

    while ((c = *buf++) != 0)
	record_char (c);

    record_char (key);

    if (fputc ('\n', rfp) == EOF)
	A00230 ();

}/* A00249 */

/*
 * A00227
 *
 * Open a file of commands for playback.
 *
 */

void A00227 (void)
{
    char new_name[MAX_FILE_NAME + 1];

    if (A00212 (new_name, command_name, FILE_PLAYBACK)) {

	strcpy (command_name, new_name);

	if ((pfp = fopen (new_name, "rt")) != NULL) {

	    A00022 (A00023 ("Do you want MORE prompts"));

	    A00069 = TRUE;

	} else A00189 ("Cannot open file\n");

    }

}/* A00227 */

/*
 * A00228
 *
 * Stop playback of commands.
 *
 */

void A00228 (void)
{

    A00022 (TRUE);

    fclose (pfp); A00069 = FALSE;

}/* A00228 */

/*
 * replay_code
 *
 * Helper function for replay_key and replay_line.
 *
 */

static int replay_code (void)
{
    int c;

    if ((c = fgetc (pfp)) == '[') {

	int c2;

	c = 0;

	while ((c2 = fgetc (pfp)) != EOF && c2 >= '0' && c2 <= '9')
	    c = 10 * c + c2 - '0';

	return (c2 == ']') ? c : EOF;

    } else return c;

}/* replay_code */

/*
 * replay_char
 *
 * Read a character from the command file.
 *
 */

static zchar replay_char (void)
{
    int c;

    if ((c = replay_code ()) != EOF) {

	if (c != '\n')

	    if (c < 1000) {

		c = A00182 (c);

		if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
		    A00071 = replay_code ();
		    A00072 = replay_code ();
		}

		return c;

	    } else return ZC_HKEY_MIN + c - 1000;

	ungetc ('\n', pfp);

	return ZC_RETURN;

    } else return ZC_BAD;

}/* replay_char */

/*
 * A00264
 *
 * Read a keystroke from a command file.
 *
 */

zchar A00264 (void)
{
    zchar key;

    key = replay_char ();

    if (fgetc (pfp) != '\n') {

	A00228 ();
	return ZC_BAD;

    } else return key;

}/* A00264 */

/*
 * A00265
 *
 * Read a line of input from a command file.
 *
 */

zchar A00265 (zchar *buf)
{
    zchar c;

    for (;;) {

	c = replay_char ();

	if (c == ZC_BAD || A00003 (c))
	    break;

	*buf++ = c;

    }

    *buf = 0;

    if (fgetc (pfp) != '\n') {

	A00228 ();
	return ZC_BAD;

    } else return c;

}/* A00265 */
