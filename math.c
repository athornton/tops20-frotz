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
    store (A00248((zword) (A00247( zargs[0] ) +
                                   A00247( zargs[1])) ));

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
    store (A00248((zword) (zargs[0] & zargs[1])));

}/* z_and */


/*
 * A00095, arithmetic SHIFT operation.
 *
 *	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right
 *
 */
void A00095 (void)
{
    if (A00247( zargs[1] ) > 0)
	store (A00248((zword) (A00247( zargs[0] )
                                       << A00247( zargs[1])) ));
    else
	store (A00248((zword) (A00247( zargs[0] )
                                       >> - A00247( zargs[1])) ));

}/* A00095 */


/*
 * z_div, signed 16bit division.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_div (void)
{
    if (A00248(zargs[1] == 0))
	A00188 (ERR_DIV_ZERO);

    store (A00248((zword) (A00247( zargs[0] )
                                   / A00247( zargs[1])) ));

}/* z_div */


/*
 * z_je, A00202 if the first value equals any of the following.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value (optional)
 *	...
 *	zargs[3] = fourth value (optional)
 *
 */
void z_je (void)
{

    zword z0 = A00248(zargs[0]);
    zword z1 = A00248(zargs[1]);
    zword z2 = A00248(zargs[2]);
    zword z3 = A00248(zargs[3]);
    
    A00202 (
	zargc > 1 && (z0 == z1 || (
	zargc > 2 && (z0 == z2 || (
        zargc > 3 && (z0 == z3))))));

}/* z_je */


/*
 * z_jg, A00202 if the first value is greater than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_jg (void)
{
    A00202 (A00247( zargs[0] ) > A00247( zargs[1]) );

}/* z_jg */


/*
 * z_jl, A00202 if the first value is less than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_jl (void)
{
    A00202 (A00247( zargs[0] ) < A00247( zargs[1]) );

}/* z_jl */


/*
 * z_jz, A00202 if value is zero.
 *
 * 	zargs[0] = value
 *
 */
void z_jz (void)
{
    A00202 (A00247( zargs[0] ) == 0);

}/* z_jz */


/*
 * A00126, logical SHIFT operation.
 *
 * 	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right (negative)
 *
 */
void A00126 (void)
{
    if (A00247( zargs[1] ) > 0)
	store (A00248((zword) (zargs[0]
                                       << A00247( zargs[1])) ));
    else
	store (A00248((zword) (zargs[0]
                                       >> - A00247( zargs[1])) ));

}/* A00126 */


/*
 * z_mod, remainder after signed 16bit division.
 *
 * 	zargs[0] = first value
 *	zargs[1] = second value
 *
 */
void z_mod (void)
{
    if (A00248(zargs[1]) == 0)
	A00188 (ERR_DIV_ZERO);

    store ((zword) (A00247( zargs[0] ) % A00247( zargs[1])) );

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
    store (A00248((zword) (A00247( zargs[0] ) * A00247( zargs[1])) ));

}/* z_mul */


/*
 * z_not, bitwise NOT operation.
 *
 * 	zargs[0] = value
 *
 */
void z_not (void)
{
    store (A00248((zword) ~zargs[0]));

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
    store (A00248((zword) (zargs[0] | zargs[1])));

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
    store (A00248((zword) (A00247( zargs[0] )
                                   - A00247( zargs[1])) ));

}/* z_sub */


/*
 * A00180, A00202 if all the flags of a bit mask are set in a value.
 *
 *	zargs[0] = value to be examined
 *	zargs[1] = bit mask
 *
 */
void A00180 (void)
{
    zword z0 = A00248(zargs[0]);
    zword z1 = A00248(zargs[1]);    
    A00202 ((z0 & z1) == z1);

}/* A00180 */
