/*
 * math.c
 *
 * Arithmetic, compare and logical opcodes
 *
 */

#include "frotz.h"

    
/* A whole lot of masking here to cope with 36-bit systems */

/*
 * z_add, 16bit addition.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_add (void)
{
    short sz0, sz1;
    
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    
    store ((zword) (sz0 + sz1));

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

    store ((zword) ((zargs[0] & zargs[1]) & 0xffff));

}/* z_and */

/*
 * A00090, arithmetic SHIFT operation.
 *
 *	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right
 *
 */

void A00090 (void)
{
    short sz0, sz1;
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
        
    if (sz1 > 0)
	store (((zword) ( sz0 << sz1 )) & 0xffff );
    else
	store (((zword) ( sz0 >> -sz1 )) & 0xffff );

}/* A00090 */

/*
 * z_div, signed 16bit division.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_div (void)
{

    short sz0, sz1;
    
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    if (sz1 == 0)
	A00192 ("Division by zero");

    store ((zword) (sz0 / sz1));

}/* z_div */

/*
 * z_je, A00193 if the first value equals any of the following.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value (optional)
 *	...
 *	zargs[3] = fourth value (optional)
 *
 */

void z_je (void)
{

    A00193 (
	zargc > 1 && (zargs[0] == zargs[1] || (
	zargc > 2 && (zargs[0] == zargs[2] || (
	zargc > 3 && (zargs[0] == zargs[3]))))));

}/* z_je */

/*
 * z_jg, A00193 if the first value is greater than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_jg (void)
{
    short sz0, sz1;

    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    
    A00193 (sz0 > sz1);

}/* z_jg */

/*
 * z_jl, A00193 if the first value is less than the second.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_jl (void)
{
    short sz0, sz1;

    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    
    A00193 (sz0 < sz1);
}/* z_jl */

/*
 * z_jz, A00193 if value is zero.
 *
 * 	zargs[0] = value
 *
 */

void z_jz (void)
{
    short sz;
    
    sz = s16(zargs[0]);
    A00193 (sz == 0);

}/* z_jz */

/*
 * A00121, logical SHIFT operation.
 *
 * 	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right (negative)
 *
 */

void A00121 (void)
{
    short sz1;

    sz1 = s16(zargs[1]);
    if (sz1 > 0)
	store ((zword) (zargs[0] << sz1));
    else
	store ((zword) (zargs[0] >> -sz1));

}/* A00121 */

/*
 * z_mod, remainder after signed 16bit division.
 *
 * 	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_mod (void)
{

    short sz0, sz1;

    sz0=s16(zargs[0]);
    sz1=s16(zargs[1]);
    if (sz1 == 0)
	A00192 ("Division by zero");

    store ((zword) (sz0 % sz1));

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

    short sz0, sz1;
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);    
    
    store ((zword) (sz0 * sz1 ));

}/* z_mul */

/*
 * z_not, bitwise NOT operation.
 *
 * 	zargs[0] = value
 *
 */

void z_not (void)
{

    store (((zword) ~zargs[0]) & 0xffff);

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

    store (((zword) (zargs[0] | zargs[1])) & 0xffff);

}/* z_or */

/*
 * z_sub, 16bit substraction.
 *
 *	zargs[0] = first value
 *	zargs[1] = second value
 *
 */

void z_sub (void)
{

    short sz0, sz1;
    
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    store ((zword) (sz0 - sz1));

}/* z_sub */

/*
 * A00175, A00193 if all the flags of a bit mask are set in a value.
 *
 *	zargs[0] = value to be examined
 *	zargs[1] = bit mask
*
 */

void A00175 (void)
{

    A00193 (((zargs[0] & zargs[1]) & 0xffff) == (zargs[1] & 0xffff));

}/* A00175 */
