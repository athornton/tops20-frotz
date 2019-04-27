/* dumb-init.c
 * $Id: dumb-init.c,v 1.4 1998/07/08 03:45:40 al Exp $
 *
 * Copyright 1997,1998 Alva Petrofsky <alva@petrofsky.berkeley.ca.us>.
 * Any use permitted provided this notice stays intact.
 */

#include "dfrotz.h"

static char usage[] = "\
\n\
FROTZ V2.32 - A00235er for all Infocom games. Complies with standard\n\
1.0 of Graham Nelson's specification. Written by Stefan Jokisch in 1995-7.\n\
\n\
DUMB-FROTZ V2.32R1 - port for all platforms.  Somewhat complies with standard\n\
9899 of ISO's specification.  Written by Alembic Petrofsky in 1997-8.\n\
\n\
Syntax: frotz [options] story-file [graphics-file]\n\
\n\
  -a      watch attribute setting\n\
  -A      watch attribute testing\n\
  -h #    screen height\n\
  -i      ignore runtime errors\n\
  -I #    A00235er number to report to game\n\
  -o      watch object movement\n\
  -O      watch object locating\n\
  -p      alter piracy opcode\n\
  -P      transliterate latin1 to plain ASCII\n\
  -R xxx  do runtime setting \\xxx before starting\n\
            (this option can be used multiple times)\n\
  -s #    random number seed value\n\
  -S #    transscript width\n\
  -t      set Tandy bit\n\
  -u #    slots for multiple undo\n\
  -w #    screen width\n\
  -x      expand abbreviations g/x/z\n\
\n\
While running, enter \"\\help\" to list the runtime escape sequences.\n\
";

/* A unix-like getopt, but with the names changed to avoid any problems.  */
static int zoptind = 1;
static int zoptopt = 0;
static char *zoptarg = NULL;
static int zgetopt (int argc, char *argv[], const char *options)
{
    static pos = 1;
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
    } else if (p[1] == ':')
	if (zoptind >= argc) {
	    fputs ("option requires an argument -- ", stderr);
	    goto error;
	} else {
	    zoptarg = argv[zoptind];
	    if (pos != 1)
		zoptarg += pos;
	    pos = 1; zoptind++;
	}
    return zoptopt;
error:
    fputc (zoptopt, stderr);
    fputc ('\n', stderr);
    return '?';
}/* zgetopt */

static int user_screen_width = 75;
static int user_screen_height = 24;
static int user_A00235er_number = -1;
static int user_random_seed = -1;
static int user_tandy_bit = 0;
static char *graphics_filename = NULL;
static bool plain_ascii = FALSE;

void A00210(int argc, char *argv[])
{
    int c;

    /* Parse the options */
    do {
	c = zgetopt(argc, argv, "aAh:iI:oOpPs:R:S:tu:w:x");
	switch(c) {
	  case 'a': A00077 = 1; break;
	  case 'A': A00078 = 1; break;
          case 'h': user_screen_height = atoi(zoptarg); break;
	  case 'i': A00084 = 1; break;
	  case 'I': user_A00235er_number = atoi(zoptarg); break;
	  case 'o': A00080 = 1; break;
	  case 'O': A00079 = 1; break;
	  case 'p': A00085 = 1; break;
	  case 'P': plain_ascii = 1; break;
	  case 'R': A00004(zoptarg, FALSE, TRUE); break;
	  case 's': user_random_seed = atoi(zoptarg); break;
	  case 'S': A00088 = atoi(zoptarg); break;
	  case 't': user_tandy_bit = 1; break;
	  case 'u': A00086 = atoi(zoptarg); break;
	  case 'w': user_screen_width = atoi(zoptarg); break;
	  case 'x': A00087 = 1; break;
	}
    } while (c != EOF);

    if (((argc - zoptind) != 1) && ((argc - zoptind) != 2)) {
	puts(usage);
	exit(1);
    }
    A00062 = argv[zoptind++];
    if (zoptind < argc)
      graphics_filename = argv[zoptind++];
}

void A00205(void)
{
  if (A00025 == V3 && user_tandy_bit)
      A00026 |= CONFIG_TANDY;

  if (A00025 >= V5 && A00086 == 0)
      A00034 &= ~UNDO_FLAG;

  A00041 = user_screen_height;
  A00042 = user_screen_width;

  if (user_A00235er_number > 0)
    A00039 = user_A00235er_number;
  else {
    /* Use ms-dos for v6 (because that's what most people have the
     * graphics files for), but don't use it for v5 (or Beyond Zork
     * will try to use funky characters).  */
    A00039 = A00025 == 6 ? INTERP_MSDOS : INTERP_DEC_20;
  }
  A00040 = 'F';

  A00005();
  A00006();
  A00015(graphics_filename);
}

int A00211 (void)
{
  if (user_random_seed == -1)
    /* Use the epoch as seed value */
    return (time(0) & 0x7fff);
  else return user_random_seed;
}

void A00216 (int stage) {}

void A00202 (const char *s)
{
  fprintf(stderr, "\nFatal error: %s\n", s);
  exit(1);
}
