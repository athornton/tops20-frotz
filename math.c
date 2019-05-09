/* math.c - Arithmetic, compare and logical opcodes
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

/*
 * z_add, 16bit addition.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_add (void)
{
    store (truncate_zword((zword) (sanitize_16( zargs[0] ) +
                                   sanitize_16( zargs[1])) ));

}/* z_add */


/*
 * z_and, bitwise AND operation.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_and (void)
{
    store (truncate_zword((zword) (zargs[0] & zargs[1])));

}/* z_and */


/*
 * z_art_shift, arithmetic SHIFT operation.
 *
 *	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right
 *
 */
void z_art_shift (void)
{
    if (sanitize_16( zargs[1] ) > 0)
	store (truncate_zword((zword) (sanitize_16( zargs[0] )
                                       << sanitize_16( zargs[1])) ));
    else
	store (truncate_zword((zword) (sanitize_16( zargs[0] )
                                       >> - sanitize_16( zargs[1])) ));

}/* z_art_shift */


/*
 * z_div, signed 16bit division.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_div (void)
{
    if (truncate_zword(zargs[1] == 0))
	runtime_error (ERR_DIV_ZERO);

    store (truncate_zword((zword) (sanitize_16( zargs[0] )
                                   / sanitize_16( zargs[1])) ));

}/* z_div */


/*
 * z_je, branch if the first value equals any of the following.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value (optional)
 *	...
 *	zargs[3] = fourth value (optional)
 *
 */
void z_je (void)
{

    zword z0 = truncate_zword(zargs[0]);
    zword z1 = truncate_zword(zargs[1]);
    zword z2 = truncate_zword(zargs[2]);
    zword z3 = truncate_zword(zargs[3]);
    
    branch (
	zargc > 1 && (z0 == z1 || (
	zargc > 2 && (z0 == z2 || (
        zargc > 3 && (z0 == z3))))));

}/* z_je */


/*
 * z_jg, branch if the first value is greater than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_jg (void)
{
    branch (sanitize_16( zargs[0] ) > sanitize_16( zargs[1]) );

}/* z_jg */


/*
 * z_jl, branch if the first value is less than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_jl (void)
{
    branch (sanitize_16( zargs[0] ) < sanitize_16( zargs[1]) );

}/* z_jl */


/*
 * z_jz, branch if value is zero.
 *
 * 	zargs[0] = value
 *
 */
void z_jz (void)
{
    branch (sanitize_16( zargs[0] ) == 0);

}/* z_jz */


/*
 * z_log_shift, logical SHIFT operation.
 *
 * 	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right (negative)
 *
 */
void z_log_shift (void)
{
    if (sanitize_16( zargs[1] ) > 0)
	store (truncate_zword((zword) (zargs[0]
                                       << sanitize_16( zargs[1])) ));
    else
	store (truncate_zword((zword) (zargs[0]
                                       >> - sanitize_16( zargs[1])) ));

}/* z_log_shift */


/*
 * z_mod, remainder after signed 16bit division.
 *
 * 	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_mod (void)
{
    if (truncate_zword(zargs[1]) == 0)
	runtime_error (ERR_DIV_ZERO);

    store ((zword) (sanitize_16( zargs[0] ) % sanitize_16( zargs[1])) );

}/* z_mod */


/*
 * z_mul, 16bit multiplication.
 *
 * 	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_mul (void)
{
    store (truncate_zword((zword) (sanitize_16( zargs[0] ) * sanitize_16( zargs[1])) ));

}/* z_mul */


/*
 * z_not, bitwise NOT operation.
 *
 * 	zargs[0] = value
 *
 */
void z_not (void)
{
    store (truncate_zword((zword) ~zargs[0]));

}/* z_not */


/*
 * z_or, bitwise OR operation.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_or (void)
{
    store (truncate_zword((zword) (zargs[0] | zargs[1])));

}/* z_or */


/*
 * z_sub, 16bit subtraction.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_sub (void)
{
    store (truncate_zword((zword) (sanitize_16( zargs[0] )
                                   - sanitize_16( zargs[1])) ));

}/* z_sub */


/*
 * z_test, branch if all the flags of a bit mask are set in a value.
 *
 *	zargs[0] = value to be examined
 *	zargs[1] = bit mask
 *
 */
void z_test (void)
{
    zword z0 = truncate_zword(zargs[0]);
    zword z1 = truncate_zword(zargs[1]);    
    branch ((z0 & z1) == z1);

}/* z_test */
