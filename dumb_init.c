/*
 * dumb-init.c - Dumb interface, initialization
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

#include "dumb_frotz.h"
#include "dumb_blorb.h"
#ifndef NO_LIBGEN_H
#include <libgen.h>
#endif

static void print_version(void);

#define INFORMATION "\
An interpreter for all Infocom and other Z-Machine games.\n\
\n\
Syntax: dfrotz [options] story-file\n\
  -a   watch attribute setting    \t -P   alter piracy opcode\n\
  -A   watch attribute testing    \t -R <path> restricted read/write\n\
  -h # screen height              \t -s # random number seed value\n\
  -i   ignore fatal errors        \t -S # transcript width\n\
  -I # interpreter number         \t -t   set Tandy bit\n\
  -o   watch object movement      \t -u # slots for multiple undo\n\
  -O   watch object locating      \t -v   show version information\n\
  -L <file> load this save file   \t -w # screen width\n\
  -m   turn off MORE prompts      \t -x   expand abbreviations g/x/z\n\
  -p   plain ASCII output only    \t -Z # error checking (see below)\n"

#define INFO2 "\
Error checking: 0 none, 1 first only (default), 2 all, 3 exit after any error.\n\
For more options and explanations, please read the manual page.\n\n\
While running, enter \"\\help\" to list the runtime escape sequences.\n"


/* A unix-like getopt, but with the names changed to avoid any problems.  */
static int zoptind = 1;
static int zoptopt = 0;
static char *zoptarg = NULL;
static int zgetopt (int argc, char *argv[], const char *options)
{
    static int pos = 1;
    const char *p;
    if (zoptind >= argc || argv[zoptind][0] != '-' || argv[zoptind][1] == 0)
	return EOF;
    zoptopt = argv[zoptind][pos++];
    zoptarg = NULL;
    if (argv[zoptind][pos] == 0)
	{ pos = 1; zoptind++; }
    p = strchr (options, zoptopt);
    if (zoptopt == ':' || p == NULL) {
	fputs ("illegal option -- ", stderr);
	goto error;
    } else if (p[1] == ':') {
	if (zoptind >= argc) {
	    fputs ("option requires an argument -- ", stderr);
	    goto error;
	} else {
	    zoptarg = argv[zoptind];
	    if (pos != 1)
		zoptarg += pos;
	    pos = 1; zoptind++;
	}
    }
    return zoptopt;
error:
    fputc (zoptopt, stderr);
    fputc ('\n', stderr);
    return '?';
}/* zgetopt */

static int user_screen_width = 75;
static int user_screen_height = 24;
static int user_random_seed = -1;
static int user_tandy_bit = 0;
static char *graphics_filename = NULL;
static bool plain_ascii = FALSE;

f_setup_t f_setup;
bool do_more_prompts;
/*
 * os_process_arguments
 *
 * Handle command line switches.
 * Some variables may be set to activate special features of Frotz.
 *
 */
void os_process_arguments(int argc, char *argv[])
{
    int c;
    char *p = NULL;
    
    do_more_prompts = TRUE;
    /* Parse the options */
    do {
	c = zgetopt(argc, argv, "-aAh:iI:L:moOpPs:r:R:S:tu:vw:xZ:");
	switch(c) {
	  case 'a': f_setup.attribute_assignment = 1; break;
	  case 'A': f_setup.attribute_testing = 1; break;
	case 'h': user_screen_height = atoi(zoptarg); break;
	  case 'i': f_setup.ignore_errors = 1; break;
	  case 'I': f_setup.interpreter_number = atoi(zoptarg); break;
	case 'L': f_setup.restore_mode = 1;
		  f_setup.tmp_save_name = strdup(zoptarg);
		  break;
	  case 'm': do_more_prompts = FALSE; break;
	  case 'o': f_setup.object_movement = 1; break;
	  case 'O': f_setup.object_locating = 1; break;
	  case 'P': f_setup.piracy = 1; break;
	case 'p': plain_ascii = 1; break;
	case 'r': dumb_handle_setting(zoptarg, FALSE, TRUE); break;
	case 'R': f_setup.restricted_path = strndup(zoptarg, PATH_MAX); break;
	case 's': user_random_seed = atoi(zoptarg); break;
	  case 'S': f_setup.script_cols = atoi(zoptarg); break;
	case 't': user_tandy_bit = 1; break;
	  case 'u': f_setup.undo_slots = atoi(zoptarg); break;
	case 'v': print_version(); exit(2); break;
	case 'w': user_screen_width = atoi(zoptarg); break;
	  case 'x': f_setup.expand_abbreviations = 1; break;
	  case 'Z': f_setup.err_report_mode = atoi(zoptarg);
		if ((f_setup.err_report_mode < ERR_REPORT_NEVER) ||
		(f_setup.err_report_mode > ERR_REPORT_FATAL))
			f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
		break;
	}
    } while (c != EOF);

    if (((argc - zoptind) != 1) && ((argc - zoptind) != 2)) {
	printf("FROTZ V%s\tDumb interface.\n", VERSION);
	puts(INFORMATION);
	puts(INFO2);
	exit(1);
    }

    /* Save the story file name */

    f_setup.story_file = strdup(argv[zoptind]);
    f_setup.story_name = strdup(basename(argv[zoptind]));

    /* Now strip off the extension */
    p = strrchr(f_setup.story_name, '.');
    if ( p != NULL )
    {
	*p = '\0';	/* extension removed */
    }

    /* Create nice default file names */

    f_setup.script_name = malloc((strlen(f_setup.story_name) + strlen(EXT_SCRIPT)) * sizeof(char) + 1);
    strncpy(f_setup.script_name, f_setup.story_name, strlen(f_setup.story_name) + 1);
    strncat(f_setup.script_name, EXT_SCRIPT, strlen(EXT_SCRIPT) + 1);

    f_setup.command_name = malloc((strlen(f_setup.story_name) + strlen(EXT_COMMAND)) * sizeof(char) + 1);
    strncpy(f_setup.command_name, f_setup.story_name, strlen(f_setup.story_name) + 1);
    strncat(f_setup.command_name, EXT_COMMAND, strlen(EXT_COMMAND) + 1);

    if (!f_setup.restore_mode) {
      f_setup.save_name = malloc((strlen(f_setup.story_name) + strlen(EXT_SAVE)) * sizeof(char) + 1);
      strncpy(f_setup.save_name, f_setup.story_name, strlen(f_setup.story_name) + 1);
      strncat(f_setup.save_name, EXT_SAVE, strlen(EXT_SAVE) + 1);
    } else { /* Set our auto load save as the name save */
      f_setup.save_name = malloc((strlen(f_setup.tmp_save_name) + strlen(EXT_SAVE)) * sizeof(char) + 1);
      strncpy(f_setup.save_name, f_setup.tmp_save_name, strlen(f_setup.tmp_save_name) + 1);
      free(f_setup.tmp_save_name);
    }

    f_setup.aux_name = malloc((strlen(f_setup.story_name) + strlen(EXT_AUX)) * sizeof(char) + 1);
    strncpy(f_setup.aux_name, f_setup.story_name, strlen(f_setup.story_name) + 1);
    strncat(f_setup.aux_name, EXT_AUX, strlen(EXT_AUX) + 1);

}

void os_init_screen(void)
{
    if (h_version == V3 && user_tandy_bit)
	h_config |= CONFIG_TANDY;

    if (h_version >= V5 && f_setup.undo_slots == 0)
	h_flags &= ~UNDO_FLAG;

    h_screen_rows = user_screen_height;
    h_screen_cols = user_screen_width;

    /* Use the ms-dos interpreter number for v6, because that's the
     * kind of graphics files we understand.  Otherwise, use DEC.  */
    if (f_setup.interpreter_number == INTERP_DEFAULT)
	h_interpreter_number = h_version == 6 ? INTERP_MSDOS : INTERP_DEC_20;
    else
 	h_interpreter_number = f_setup.interpreter_number;

    h_interpreter_version = 'F';

    dumb_init_input();
    dumb_init_output();
    dumb_init_pictures(graphics_filename);
}

int os_random_seed (void)
{
    if (user_random_seed == -1)
	/* Use the epoch as seed value */
	return (time(0) & 0x7fff);
    else return user_random_seed;
}

void os_restart_game (int UNUSED (stage)) {}

void os_fatal (const char *s, ...)
{
    fprintf(stderr, "\nFatal error: %s\n", s);
    if (f_setup.ignore_errors) {
	fprintf(stderr, "Continuing anyway...\n");
    } else {
	exit(1);
    }
}

FILE *os_load_story(void)
{
    FILE *fp;

#ifndef NO_BLORB
    switch (dumb_blorb_init(f_setup.story_file)) {
	case bb_err_NoBlorb:
	  break;
	case bb_err_Format:
	  printf("Blorb file loaded, but unable to build map.\n\n");
	  break;
	case bb_err_NotFound:
	  printf("Blorb file loaded, but lacks executable chunk.\n\n");
	  break;
	case bb_err_None:
	  break;
    }

    fp = fopen(f_setup.story_file, "rb");

    /* Is this a Blorb file containing Zcode? */
    if (f_setup.exec_in_blorb)
	 fseek(fp, blorb_res.data.startpos, SEEK_SET);
#else
    fp = fopen(f_setup.story_file, "rb");
#endif /* NO_BLORB */

    return fp;
}

/*
 * Seek into a storyfile, either a standalone file or the
 * ZCODE chunk of a blorb file (dumb does not support blorb
 * so this is just a wrapper for fseek)
 */
int os_storyfile_seek(FILE * fp, long offset, int whence)
{
    return fseek(fp, offset, whence);
}

/*
 * Tell the position in a storyfile, either a standalone file
 * or the ZCODE chunk of a blorb file (dumb does not support
 * blorb so this is just a wrapper for fseek)
 */
int os_storyfile_tell(FILE * fp)
{
    return ftell(fp);
}

void os_init_setup(void)
{
	f_setup.attribute_assignment = 0;
	f_setup.attribute_testing = 0;
	f_setup.context_lines = 0;
	f_setup.object_locating = 0;
	f_setup.object_movement = 0;
	f_setup.left_margin = 0;
	f_setup.right_margin = 0;
	f_setup.ignore_errors = 0;
	f_setup.piracy = 0;
	f_setup.undo_slots = MAX_UNDO_SLOTS;
	f_setup.expand_abbreviations = 0;
	f_setup.script_cols = 80;
	f_setup.sound = 1;
	f_setup.err_report_mode = ERR_DEFAULT_REPORT_MODE;
	f_setup.restore_mode = 0;
}

static void print_version(void)
{
    printf("FROTZ V%s\t", VERSION);
    printf("Dumb interface.\n");
    printf("Build date:\t%s\n", build_timestamp);
    printf("Commit date:\t%s\n", GIT_DATE);
    printf("Git commit:\t%s\n", GIT_HASH);
    printf("Git branch:\t%s\n", GIT_BRANCH);
    printf("  Frotz was originally written by Stefan Jokisch.\n");
    printf("  It complies with standard 1.0 of Graham Nelson's specification.\n");
    printf("  It was ported to Unix by Galen Hazelwood.\n");
    printf("  The core and dumb port are currently maintained by David Griffith.\n");
    printf("  Frotz's homepage is https://661.org/proj/if/frotz.\n\n");
    return;
}

