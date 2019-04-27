/* dumb-frotz.h
 * $Id: dumb-frotz.h,v 1.5 1998/07/08 03:45:40 al Exp $
 * Frotz os functions for a standard C library and a dumb terminal.
 * Now you can finally play Zork Zero on your Teletype.
 *
 * Copyright 1997, 1998 Alembic Petrofsky <alembic@petrofsky.berkeley.ca.us>.
 * Any use permitted provided this notice stays intact.
 */
#include "frotz.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* From input.c.  */
bool A00003 (zchar);

/* dumb-input.c */
bool A00004(const char *setting, bool show_cursor, bool startup);
void A00005(void);

/* dumb-output.c */
void A00006(void);
bool A00007(const char *setting, bool show_cursor,
				bool startup);
void A00008(bool show_cursor);
void A00009(bool show_cursor, char line_type);
void A00010(void);
void A00011(char *);
void A00012(int num_chars);
void A00013(void);
void A00014(int row, int col, char c);

/* dumb-pic.c */
void A00015(char *graphics_filename);
