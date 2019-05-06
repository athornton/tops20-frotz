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
    /*    short sz0, sz1; */

    /*
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    */
    
    /*    store (z16(sz0 + sz1)); */
        
    /*    store ((zword) ((sz0 + sz1) & 0xffff) );*/
    store ( (zword) (zargs[0] + zargs[1]) &0xffff);

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
 * z_art_shift, arithmetic SHIFT operation.
 *
 *	zargs[0] = value
 *	zargs[1] = #positions to shift left (positive) or right
 *
 */

void z_art_shift (void)
{
    short sz0, sz1;
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
        
    if (sz1 > 0)
	store (((zword) ( sz0 << sz1 )) & 0xffff );
    else
	store (((zword) ( sz0 >> -sz1 )) & 0xffff );

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

    short sz0, sz1;
    
    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    if (sz1 == 0)
	runtime_error ("Division by zero");

    store ((zword) ((sz0 / sz1) & 0xffff));

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

    branch (
	zargc > 1 && (zargs[0] == zargs[1] || (
	zargc > 2 && (zargs[0] == zargs[2] || (
	zargc > 3 && (zargs[0] == zargs[3]))))));

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
    short sz0, sz1;

    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    
    branch (sz0 > sz1);

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
    short sz0, sz1;

    sz0 = s16(zargs[0]);
    sz1 = s16(zargs[1]);
    
    branch (sz0 < sz1);
}/* z_jl */

/*
 * z_jz, branch if value is zero.
 *
 * 	zargs[0] = value
 *
 */

void z_jz (void)
{
    short sz;
    
    sz = s16(zargs[0]);
    branch (sz == 0);

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
    short sz1;

    sz1 = s16(zargs[1]);
    if (sz1 > 0)
	store ((zword) (zargs[0] << sz1));
    else
	store ((zword) (zargs[0] >> -sz1));

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

    short sz0, sz1;

    sz0=s16(zargs[0]);
    sz1=s16(zargs[1]);
    if (sz1 == 0)
	runtime_error ("Division by zero");

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
 * z_test, branch if all the flags of a bit mask are set in a value.
 *
 *	zargs[0] = value to be examined
 *	zargs[1] = bit mask
*
 */

void z_test (void)
{

    branch (((zargs[0] & zargs[1]) & 0xffff) == (zargs[1] & 0xffff));

}/* z_test */
