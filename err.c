/* err.c - Runtime error reporting functions
 *	Written by Jim Dunleavy <jim.dunleavy@erha.ie>
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
/*fs_t A00003; */

/* Define stuff for stricter Z-code error checking, for the generic
   Unix/DOS/etc terminal-window interface. Feel free to change the way
   player prefs are specified, or replace report_zstrict_error()
   completely if you want to change the way errors are reported. */

/* int err_report_mode = ERR_DEFAULT_REPORT_MODE; */

static int error_count[ERR_NUM_ERRORS];

static char *err_A00084s[] = {
    "Text buffer overflow",
    "Store out of dynamic memory",
    "Division by zero",
    "Illegal object",
    "Illegal attribute",
    "No such property",
    "Stack overflow",
    "Call to illegal address",
    "Call to non-routine",
    "Stack underflow",
    "Illegal opcode",
    "Bad stack frame",
    "Jump to illegal address",
    "Can't save while in interrupt",
    "Nesting stream #3 too deep",
    "Illegal window",
    "Illegal window property",
    "Print at illegal address",
    "@jin called with object 0",
    "@get_child called with object 0",
    "@get_parent called with object 0",
    "@get_sibling called with object 0",
    "@get_prop_addr called with object 0",
    "@get_prop called with object 0",
    "@put_prop called with object 0",
    "@clear_attr called with object 0",
    "@set_attr called with object 0",
    "@test_attr called with object 0",
    "@move_object called moving object 0",
    "@move_object called moving into object 0",
    "@remove_object called with object 0",
    "@get_next_prop called with object 0"
};

static void print_long (unsigned long value, int base);

/*
 * A00187
 *
 * Initialise error reporting.
 *
 */
void A00187 (void)
{
    int i;

    /* Initialize the counters. */

    for (i = 0; i < ERR_NUM_ERRORS; i++)
        error_count[i] = 0;
}


/*
 * A00188
 *
 * An error has occurred. Ignore it, pass it to A00215 or report
 * it according to err_report_mode.
 *
 * errnum : Numeric code for error (1 to ERR_NUM_ERRORS)
 *
 */
void A00188 (int errnum)
{
    int wasfirst;

    if (errnum <= 0 || errnum > ERR_NUM_ERRORS)
	return;

    if (A00003.err_report_mode == ERR_REPORT_FATAL
	|| errnum <= ERR_MAX_FATAL) {
	A00194 ();
	A00215 (err_A00084s[errnum - 1]);
	return;
    }

    wasfirst = (error_count[errnum - 1] == 0);
    error_count[errnum - 1]++;

    if ((A00003.err_report_mode == ERR_REPORT_ALWAYS)
	|| (A00003.err_report_mode == ERR_REPORT_ONCE && wasfirst)) {
	long pc;

	GET_PC (pc);
	A00199 ("Warning: ");
	A00199 (err_A00084s[errnum - 1]);
	A00199 (" (PC = ");
	print_long (pc, 16);
	A00196 (')');

	if (A00003.err_report_mode == ERR_REPORT_ONCE) {
	    A00199 (" (will ignore further occurrences)");
	} else {
	    A00199 (" (occurence ");
	    print_long (error_count[errnum - 1], 10);
	    A00196 (')');
	}
	A00195 ();
    }
} /* report_error */


/*
 * print_long
 *
 * Print an unsigned 32bit number in decimal or hex.
 *
 */
static void print_long (unsigned long value, int base)
{
    unsigned long i;
    char c;

    for (i = (base == 10 ? 1000000000 : 0x10000000); i != 0; i /= base)
	if (value >= i || i == 1) {
	    c = (value / i) % base;
	    A00196 (c + (c <= 9 ? '0' : 'a' - 10));
	}

}/* print_long */
