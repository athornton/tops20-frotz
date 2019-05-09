/*
 * dumb_frotz.h
 *
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
bool A00009 (zchar);

/* dumb-input.c */
bool A00010(const char *setting, bool show_cursor, bool startup);
void A00011(void);

/* dumb-output.c */
void A00012(void);
bool A00013(const char *setting, bool show_cursor,
				bool startup);
void A00014(bool show_cursor);
void A00015(bool show_cursor, char line_type);
void A00016(void);
void A00017(char *);
void A00018(int num_chars);
void A00019(void);
void A00020(int row, int col, char c);

/* dumb-pic.c */
void A00021(char *graphics_filename);
