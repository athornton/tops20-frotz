/* hotkey.c - Hot key functions
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

extern int A00249 (void);

extern int A00250 (void);

extern bool A00032 (const char *);

extern void A00251 (void);
extern void A00252 (void);
extern void A00253 (void);
extern void A00254 (void);

extern void A00022 (int);

/*
 * hot_key_debugging
 *
 * ...allows user to toggle cheating options on/off.
 *
 */
static bool hot_key_debugging (void)
{

    A00199 ("Debugging options\n");

    A00003.attribute_assignment = A00032 ("Watch attribute assignment");
    A00003.attribute_testing = A00032 ("Watch attribute testing");

    A00003.object_movement = A00032 ("Watch object movement");
    A00003.object_locating = A00032 ("Watch object locating");

    return FALSE;

}/* hot_key_debugging */


/*
 * hot_key_help
 *
 * ...displays a list of all hot keys.
 *
 */
static bool hot_key_help (void) {

    A00199 ("Help\n");

    A00199 (
	"\n"
	"Alt-D  debugging options\n"
	"Alt-H  help\n"
	"Alt-N  new game\n"
	"Alt-P  playback on\n"
	"Alt-R  recording on/off\n"
	"Alt-S  seed random numbers\n"
	"Alt-U  undo one turn\n"
	"Alt-X  exit game\n");

    return FALSE;

}/* hot_key_help */


/*
 * hot_key_playback
 *
 * ...allows user to turn playback on.
 *
 */
static bool hot_key_playback (void)
{
    A00199 ("Playback on\n");

    if (!A00083)
	A00251 ();

    return FALSE;

}/* hot_key_playback */


/*
 * hot_key_recording
 *
 * ...allows user to turn recording on/off.
 *
 */
static bool hot_key_recording (void)
{

    if (A00083) {
	A00199 ("Playback off\n");
	A00252 ();
    } else if (A00082) {
	A00199 ("Recording off\n");
	A00254 ();
    } else {
	A00199 ("Recording on\n");
	A00253 ();
    }

    return FALSE;

}/* hot_key_recording */


/*
 * hot_key_seed
 *
 * ...allows user to seed the random number seed.
 *
 */
static bool hot_key_seed (void)
{

    A00199 ("Seed random numbers\n");

    A00199 ("Enter seed value (or return to randomize): ");
    A00022 (A00250 ());

    return FALSE;

}/* hot_key_seed */


/*
 * hot_key_undo
 *
 * ...allows user to undo the previous turn.
 *
 */
static bool hot_key_undo (void)
{

    A00199 ("Undo one turn\n");

    if (A00249 ()) {

	if (A00035 >= V5) {		/* for V5+ games we must */
	    store (2);			/* store 2 (for success) */
	    return TRUE;		/* and abort the input   */
	}

	if (A00035 <= V3) {		/* for V3- games we must */
	    A00174 ();		/* draw the status line  */
	    return FALSE;		/* and continue input    */
	}

    } else A00199 ("No more undo information available.\n");

    return FALSE;

}/* hot_key_undo */


/*
 * hot_key_restart
 *
 * ...allows user to start a new game.
 *
 */
static bool hot_key_restart (void)
{
    A00199 ("New game\n");

    if (A00032 ("Do you wish to restart")) {

	A00157 ();
	return TRUE;

    } else return FALSE;

}/* hot_key_restart */


/*
 * hot_key_quit
 *
 * ...allows user to exit the game.
 *
 */
static bool hot_key_quit (void)
{

    A00199 ("Exit game\n");

    if (A00032 ("Do you wish to quit")) {

	A00151 ();
	return TRUE;

    } else return FALSE;

}/* hot_key_quit */


/*
 * A00271
 *
 * Perform the action associated with a so-called hot key. Return
 * true to abort the current input action.
 *
 */
bool A00271 (zchar key)
{
    if (cwin == 0) {

	bool aborting;

	aborting = FALSE;

	A00199 ("\nHot key -- ");

	switch (key) {
	    case ZC_HKEY_RECORD: aborting = hot_key_recording (); break;
	    case ZC_HKEY_PLAYBACK: aborting = hot_key_playback (); break;
	    case ZC_HKEY_SEED: aborting = hot_key_seed (); break;
	    case ZC_HKEY_UNDO: aborting = hot_key_undo (); break;
	    case ZC_HKEY_RESTART: aborting = hot_key_restart (); break;
	    case ZC_HKEY_QUIT: aborting = hot_key_quit (); break;
	    case ZC_HKEY_DEBUG: aborting = hot_key_debugging (); break;
	    case ZC_HKEY_HELP: aborting = hot_key_help (); break;
	}

	if (aborting)
	    return TRUE;

	A00199 ("\nContinue input...\n");

    }

    return FALSE;

}/* A00271 */
