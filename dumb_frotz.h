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
bool is_terminator ();

/* dumb-input.c */
bool dumb_handle_setting();
void dumb_init_input();

/* dumb-output.c */
void dumb_init_output();
bool dumb_output_handle_setting();
void dumb_show_screen();
void dumb_show_prompt();
void dumb_dump_screen();
void dumb_display_user_input();
void dumb_discard_old_input();
void dumb_elide_more_prompt();
void dumb_set_picture_cell();

/* dumb-pic.c */
void dumb_init_pictures();
