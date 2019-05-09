/* main.c - Frotz V2.40 main function
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
 * This is an A00259er for Infocom V1 to V6 games. It also supports
 * the recently defined V7 and V8 games.
 *
 */

#include "frotz.h"

#ifndef MSDOS_16BIT
#define cdecl
#endif

extern void A00259 (void);
extern void A00260 (void);
extern void A00261 (void);
extern void A00262 (void);

bool A00093 = FALSE;

/* Story file name, id number and size */

char *story_name = 0;

enum story A00075 = UNKNOWN;
long A00076 = 0;

/* Story file header data */

zbyte A00035 = 0;
zbyte A00036 = 0;
zword A00037 = 0;
zword A00038 = 0;
zword A00039 = 0;
zword A00040 = 0;
zword A00041 = 0;
zword A00042 = 0;
zword A00043 = 0;
zword A00044 = 0;
zbyte A00045[6] = { 0, 0, 0, 0, 0, 0 };
zword A00046 = 0;
zword A00047 = 0;
zword A00048 = 0;
zbyte A00049 = 0;
zbyte A00050 = 0;
zbyte A00051 = 0;
zbyte A00052 = 0;
zword A00053 = 0;
zword A00054 = 0;
zbyte A00055 = 1;
zbyte A00056 = 1;
zword A00057 = 0;
zword A00058 = 0;
zbyte A00059 = 0;
zbyte A00060 = 0;
zword A00061 = 0;
zword A00062 = 0;
zbyte A00063 = 1;
zbyte A00064 = 0;
zword A00065 = 0;
zword A00066 = 0;
zbyte A00067[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

zword A00068 = 0;
zword A00069 = 0;
zword A00070 = 0;
zword A00071 = 0;

/* Stack data */

zword stack[STACK_SIZE];
zword *sp = 0;
zword *fp = 0;
zword A00077 = 0;

/* IO streams */

bool A00079 = TRUE;
bool A00080 = FALSE;
bool A00081 = FALSE;
bool A00082 = FALSE;
bool A00083 = FALSE;
bool A00084 = FALSE;

/* Current window and mouse data */

int cwin = 0;
int mwin = 0;

int A00086 = 0;
int A00085 = 0;

/* Window attributes */

bool A00089 = FALSE;
bool A00090 = FALSE;
bool A00091 = FALSE;
bool A00092 = FALSE;

int opt_snd = 1;
char *option_zcode_path;


/* Size of memory to reserve (in bytes) */

long A00094 = 0;


/*
 * A00134, A00202 if the story file is a legal copy.
 *
 *	no zargs used
 *
 */
void A00134 (void)
{
    A00202 (!A00003.piracy);

}/* A00134 */


/*
 * main
 *
 * Prepare and run the game.
 *
 */
int cdecl main (int argc, char *argv[])
{
    A00241 ();

    A00224 (argc, argv);

    A00189 ();

    A00187 ();

    A00260 ();

    A00190 ();

    A00191 ();

    A00218 ();

    A00261 ();

    A00157 ();

    A00259 ();

    A00262 ();

    A00229 ();

    return 0;

}/* main */
