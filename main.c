/*
 * main.c
 *
 * Frotz V2.32 main function
 *
 * This is an A00235er for Infocom V1 to V6 games. It also supports
 * the recently defined V7 and V8 games. Please report bugs to
 *
 *    s.jokisch@avu.de
 *
 * Frotz is freeware. It may be used and distributed freely provided
 * no commercial profit is involved. (c) 1995-1997 Stefan Jokisch
 *
 */

#include "frotz.h"

extern void A00235 (void);
extern void A00236 (void);
extern void A00237 (void);
extern void A00238 (void);

/* Story file name, id number and size */

const char *A00062 = 0;

enum story A00063 = UNKNOWN;
long A00064 = 0;

/* Story file header data */

zbyte A00025 = 0;
zbyte A00026 = 0;
zword A00027 = 0;
zword A00028 = 0;
zword A00029 = 0;
zword A00030 = 0;
zword A00031 = 0;
zword A00032 = 0;
zword A00033 = 0;
zword A00034 = 0;
zbyte A00035[6] = { 0, 0, 0, 0, 0, 0 };
zword A00036 = 0;
zword A00037 = 0;
zword A00038 = 0;
zbyte A00039 = 0;
zbyte A00040 = 0;
zbyte A00041 = 0;
zbyte A00042 = 0;
zword A00043 = 0;
zword A00044 = 0;
zbyte A00045 = 1;
zbyte A00046 = 1;
zword A00047 = 0;
zword A00048 = 0;
zbyte A00049 = 0;
zbyte A00050 = 0;
zword A00051 = 0;
zword A00052 = 0;
zbyte A00053 = 1;
zbyte A00054 = 0;
zword A00055 = 0;
zword A00056 = 0;
zbyte A00057[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

zword A00058 = 0;
zword A00059 = 0;
zword A00060 = 0;
zword A00061 = 0;

/* Stack data */

zword stack[STACK_SIZE];
zword *sp = 0;
zword *fp = 0;

/* IO streams */

bool A00065 = TRUE;
bool A00066 = FALSE;
bool A00067 = FALSE;
bool A00068 = FALSE;
bool A00069 = FALSE;
bool A00070 = FALSE;

/* Current window and mouse data */

int cwin = 0;
int mwin = 0;

int A00072 = 0;
int A00071 = 0;

/* Window attributes */

bool A00073 = FALSE;
bool A00074 = FALSE;
bool A00075 = FALSE;
bool A00076 = FALSE;

/* User options */

int A00077 = 0;
int A00078 = 0;
int A00081 = 0;
int A00079 = 0;
int A00080 = 0;
int A00082 = 0;
int A00083 = 0;
int A00084 = 0;
int A00085 = 0;
int A00086 = MAX_UNDO_SLOTS;
int A00087 = 0;
int A00088 = 80;

/* Size of memory to reserve (in bytes) */

long A00089 = 0;

/*
 * A00192
 *
 * An error has occured. Ignore it or pass it to A00202.
 *
 */

void A00192 (const char *s)
{

    if (!A00084)
	{ A00184 (); A00202 (s); }

}/* A00192 */

/*
 * A00129, A00193 if the story file is a legal copy.
 *
 *	no zargs used
 *
 */

void A00129 (void)
{

    A00193 (!A00085);

}/* A00129 */

/*
 * main
 *
 * Prepare and run the game.
 *
 */

int main (int argc, char *argv[])
{

    A00210 (argc, argv);

    A00236 ();

    A00205 ();

    A00237 ();

    A00152 ();

    A00235 ();

    A00238 ();

    A00215 ();

    return 0;

}/* main */
