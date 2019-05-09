/* files.c - Transcription, recording and playback
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "frotz.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

extern void A00031 (bool);

extern bool A00009 (zchar);

extern bool A00032 (const char *);

/* char script_name[MAX_FILE_NAME + 1] = DEFAULT_SCRIPT_NAME; */
/* char command_name[MAX_FILE_NAME + 1] = DEFAULT_COMMAND_NAME; */

#ifdef __MSDOS__
extern char A00033[];
#endif

static int script_width = 0;

static FILE *sfp = NULL;
static FILE *rfp = NULL;
static FILE *pfp = NULL;

/*
 * A00026
 *
 * Open the transcript file. 'AMFV' makes this more complicated as it
 * turns transcription on/off several times to exclude some text from
 * the transcription file. This wasn't a problem for the original V4
 * A00259ers which always sent transcription to the printer, but it
 * means a problem to modern A00259ers that offer to open a new file
 * every time transcription is turned on. Our solution is to append to
 * the old transcription file in V1 to V4, and to ask for a new file
 * name in V5+.
 *
 */

void A00026 (void)
{
    static bool script_valid = FALSE;

    char *new_name;

    A00044 &= ~SCRIPTING_FLAG;

    if (A00035 >= V5 || !script_valid) {

	new_name = A00226(A00003.script_name, FILE_SCRIPT);
	if (new_name == NULL)
	    goto done;

	free(A00003.script_name);
	A00003.script_name = strdup(new_name);
    }

    /* Opening in "at" mode doesn't work for A00283... */

    if ((sfp = fopen (A00003.script_name, "r+t")) != NULL ||
		(sfp = fopen (A00003.script_name, "w+t")) != NULL) {

	fseek (sfp, 0, SEEK_END);

	A00044 |= SCRIPTING_FLAG;

	script_valid = TRUE;
	A00080 = TRUE;

	script_width = 0;

    } else A00199 ("Cannot open file\n");

done:

    SET_WORD (H_FLAGS, A00044);

}/* A00026 */

/*
 * A00027
 *
 * Stop transcription.
 *
 */

void A00027 (void)
{

    A00044 &= ~SCRIPTING_FLAG;
    SET_WORD (H_FLAGS, A00044);

    fclose (sfp); A00080 = FALSE;

}/* A00027 */

/*
 * A00281
 *
 * Write a newline to the transcript file.
 *
 */

void A00281 (void)
{

    if (fputc ('\n', sfp) == EOF)
	A00027 ();

    script_width = 0;

}/* A00281 */

/*
 * A00279
 *
 * Write a single character to the transcript file.
 *
 */

void A00279 (zchar c)
{

    if (c == ZC_INDENT && script_width != 0)
	c = ' ';

    if (c == ZC_INDENT)
	{ A00279 (' '); A00279 (' '); A00279 (' '); return; }
    if (c == ZC_GAP)
	{ A00279 (' '); A00279 (' '); return; }

#ifdef __MSDOS__
    if (c >= ZC_LATIN1_MIN)
	c = A00033[c - ZC_LATIN1_MIN];
#endif

#ifdef USE_UTF8
    if (c >= ZC_LATIN1_MIN)
    {
      if ( c < 0xc0) {
	fputc (0xc2, sfp); 
	fputc (c, sfp);
#ifdef HANDLE_OE_DIPTHONG
      } else if (c == 0xd6) {
	fputc (0xc5, sfp);
	fputc (0x92, sfp);
      } else if (c == 0xf6) {
	fputc (0xc5, sfp);
	fputc (0x93, sfp);
#endif /* HANDLE_OE_DIPTHONG */
      } else {
	fputc (0xc3, sfp);
	fputc (c - 0x40, sfp);
      }
    }
    else
    {
	fputc (c, sfp);
    }
#else
    fputc (c, sfp);
#endif
    script_width++;

}/* A00279 */

/*
 * A00280
 *
 * Write a string to the transcript file.
 *
 */

void A00280 (const zchar *s)
{
    int width;
    int i;

    if (*s == ZC_INDENT && script_width != 0)
	A00279 (*s++);

    for (i = 0, width = 0; s[i] != 0; i++)

	if (s[i] == ZC_NEW_STYLE || s[i] == ZC_NEW_FONT)
	    i++;
	else if (s[i] == ZC_GAP)
	    width += 3;
	else if (s[i] == ZC_INDENT)
	    width += 2;
	else
	    width += 1;

    if (A00003.script_cols != 0 && script_width + width > A00003.script_cols) {

	if (*s == ' ' || *s == ZC_INDENT || *s == ZC_GAP)
	    s++;

	A00281 ();

    }

    for (i = 0; s[i] != 0; i++)

	if (s[i] == ZC_NEW_FONT || s[i] == ZC_NEW_STYLE)
	    i++;
	else
	    A00279 (s[i]);

}/* A00280 */

/*
 * A00282
 *
 * Send an input line to the transcript file.
 *
 */

void A00282 (const zchar *buf, zchar key)
{
    int width;
    int i;

    for (i = 0, width = 0; buf[i] != 0; i++)
	width++;

    if (A00003.script_cols != 0 && script_width + width > A00003.script_cols)
	A00281 ();

    for (i = 0; buf[i] != 0; i++)
	A00279 (buf[i]);

    if (key == ZC_RETURN)
	A00281 ();

}/* A00282 */

/*
 * A00283
 *
 * Remove an input line from the transcript file.
 *
 */

void A00283 (const zchar *buf)
{
    int width;
    int i;

    for (i = 0, width = 0; buf[i] != 0; i++)
	width++;

    fseek (sfp, -width, SEEK_CUR); script_width -= width;

}/* A00283 */

/*
 * A00284
 *
 * Start sending a "debugging" A00084 to the transcript file.
 *
 */

void A00284 (void)
{

    if (script_width != 0)
	A00281 ();

    A00279 (ZC_INDENT);

}/* A00284 */

/*
 * A00285
 *
 * Stop writing a "debugging" A00084.
 *
 */

void A00285 (void)
{

    A00281 ();

}/* A00285 */

/*
 * A00253
 *
 * Open a file to record the player's input.
 *
 */

void A00253 (void)
{
    char *new_name;

    new_name = A00226(A00003.command_name, FILE_RECORD);
    if (new_name != NULL) {

	free(A00003.command_name);
	A00003.command_name = strdup(new_name);

	if ((rfp = fopen (new_name, "wt")) != NULL)
	    A00082 = TRUE;
	else
	    A00199 ("Cannot open file\n");

    }

}/* A00253 */

/*
 * A00254
 *
 * Stop recording the player's input.
 *
 */

void A00254 (void)
{

    fclose (rfp); A00082 = FALSE;

}/* A00254 */

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

    if (c != ZC_RETURN) {
	if (c < ZC_HKEY_MIN || c > ZC_HKEY_MAX) {
	    record_code (A00193 (c), FALSE);
	    if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
		record_code (A00085, TRUE);
		record_code (A00086, TRUE);
	    }
	} else record_code (1000 + c - ZC_HKEY_MIN, TRUE);
    }

}/* record_char */

/*
 * A00277
 *
 * Copy a keystroke to the command file.
 *
 */

void A00277 (zchar key)
{

    record_char (key);

    if (fputc ('\n', rfp) == EOF)
	A00254 ();

}/* A00277 */

/*
 * A00278
 *
 * Copy a line of input to a command file.
 *
 */

void A00278 (const zchar *buf, zchar key)
{
    zchar c;

    while ((c = *buf++) != 0)
	record_char (c);

    record_char (key);

    if (fputc ('\n', rfp) == EOF)
	A00254 ();

}/* A00278 */

/*
 * A00251
 *
 * Open a file of commands for playback.
 *
 */

void A00251 (void)
{
    char *new_name;

    new_name = A00226(A00003.command_name, FILE_PLAYBACK);
    if (new_name != NULL) {

	free(A00003.command_name);
	A00003.command_name = strdup(new_name);

	if ((pfp = fopen (new_name, "rt")) != NULL) {

	    A00031 (A00032 ("Do you want MORE prompts"));

	    A00083 = TRUE;

	} else A00199 ("Cannot open file\n");

    }

}/* A00251 */

/*
 * A00252
 *
 * Stop playback of commands.
 *
 */

void A00252 (void)
{

    A00031 (TRUE);

    fclose (pfp); A00083 = FALSE;

}/* A00252 */

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

	if (c != '\n') {

	    if (c < 1000) {

		c = A00192 (c);

		if (c == ZC_SINGLE_CLICK || c == ZC_DOUBLE_CLICK) {
		    A00085 = replay_code ();
		    A00086 = replay_code ();
		}

		return c;

	    } else return ZC_HKEY_MIN + c - 1000;
	}

	ungetc ('\n', pfp);

	return ZC_RETURN;

    } else return ZC_BAD;

}/* replay_char */

/*
 * A00292
 *
 * Read a keystroke from a command file.
 *
 */

zchar A00292 (void)
{
    zchar key;

    key = replay_char ();

    if (fgetc (pfp) != '\n') {

	A00252 ();
	return ZC_BAD;

    } else return key;

}/* A00292 */

/*
 * A00293
 *
 * Read a line of input from a command file.
 *
 */

zchar A00293 (zchar *buf)
{
    zchar c;

    for (;;) {

	c = replay_char ();

	if (c == ZC_BAD || A00009 (c))
	    break;

	*buf++ = c;

    }

    *buf = 0;

    if (fgetc (pfp) != '\n') {

	A00252 ();
	return ZC_BAD;

    } else return c;

}/* A00293 */
