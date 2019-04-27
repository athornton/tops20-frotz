/*
 * hotkey.c
 *
 * Hot key functions
 *
 */

#include "frotz.h"

extern int A00225 (void);

extern int A00226 (void);

extern bool A00023 (const char *);

extern void A00227 (void);
extern void A00228 (void);
extern void A00229 (void);
extern void A00230 (void);

extern void A00016 (int);

/*
 * hot_key_debugging
 *
 * ...allows user to toggle cheating options on/off.
 *
 */

static bool hot_key_debugging (void)
{

    A00189 ("Debugging options\n");

    A00077 = A00023 ("Watch attribute assignment");
    A00078 = A00023 ("Watch attribute testing");
    A00080 = A00023 ("Watch object movement");
    A00079 = A00023 ("Watch object locating");

    return FALSE;

}/* hot_key_debugging */

/*
 * hot_key_help
 *
 * ...displays a list of all hot keys.
 *
 */

static bool hot_key_help (void) {

    A00189 ("Help\n");

    A00189 (
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

    A00189 ("Playback on\n");

    if (!A00069)
	A00227 ();

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

    if (A00069) {
	A00189 ("Playback off\n");
	A00228 ();
    } else if (A00068) {
	A00189 ("Recording off\n");
	A00230 ();
    } else {
	A00189 ("Recording on\n");
	A00229 ();
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

    A00189 ("Seed random numbers\n");

    A00189 ("Enter seed value (or return to randomize): ");
    A00016 (A00226 ());

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

    A00189 ("Undo one turn\n");

    if (A00225 ()) {

	if (A00025 >= V5) {		/* for V5+ games we must */
	    store (2);			/* store 2 (for success) */
	    return TRUE;		/* and abort the input   */
	}

	if (A00025 <= V3) {		/* for V3- games we must */
	    A00169 ();		/* draw the status line  */
	    return FALSE;		/* and continue input    */
	}

    } else A00189 ("No more undo information available.\n");

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

    A00189 ("New game\n");

    if (A00023 ("Do you wish to restart")) {

	A00152 ();
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

    A00189 ("Exit game\n");

    if (A00023 ("Do you wish to quit")) {

	A00146 ();
	return TRUE;

    } else return FALSE;

}/* hot_key_quit */

/*
 * A00242
 *
 * Perform the action associated with a so-called hot key. Return
 * true to abort the current input action.
 *
 */

bool A00242 (zchar key)
{

    if (cwin == 0) {

	bool aborting;

	A00189 ("\nHot key -- ");

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

	A00189 ("\nContinue input...\n");

    }

    return FALSE;

}/* A00242 */
