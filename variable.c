/*
 * variable.c
 *
 * Variable and stack related opcodes
 *
 */

#include "frotz.h"

/*
 * z_dec, decrement a variable.
 *
 * 	zargs[0] = variable to decrement
 *
 */

void z_dec (void)
{
    zword value;

    zword z0;
    short sv;
    
    z0 = zargs[0];
    z0 &= 0xffff;
    
    if (z0 == 0) {
        sv = s16(*sp);
        sv -= 1;
        *sp = ( (zword) (sv & 0xffff) );
    }
    else if (z0 < 16) {
        sv = s16(*(fp - z0));
        sv -= 1;
        *(fp - z0) = ( (zword) (sv & 0xffff) );
    }
    else {
	zword addr = h_globals + 2 * (zargs[0] - 16);
	value=lw(addr);
        sv=s16(value);
	sv--;
        value = (zword) sv;
        value &= 0xffff;
	sw(addr, value);
    }

}/* z_dec */

/*
 * z_dec_chk, decrement a variable and branch if now less than value.
 *
 * 	zargs[0] = variable to decrement
 * 	zargs[1] = value to check variable against
 *
 */

void z_dec_chk (void)
{
    zword value;
    zword z0, z1;
    short sv, sz1;
    
    z0 = zargs[0];
    z1 = zargs[1];

    z0 &= 0xffff;
    z1 &= 0xffff;
    
    if (z0 == 0) {
	sv = s16(*sp);
        sv -= 1;
        value = ( ( ( zword ) sv ) & 0xffff );
        *sp = value;
    }
    else if (z0 < 16) {
        sv = s16(*(fp - z0));
        sv -= 1;
	value = ( ( (zword) sv ) & 0xffff ) ;
        *(fp - z0) = value;
    }
    else {
	zword addr = h_globals + 2 * (z0 - 16);
	value=lw(addr);
	value--;
        value &= 0xffff;
	sw(addr, value);
    }
    sv = s16(value);
    sz1 = s16(z1);

    branch (sv < sz1);

}/* z_dec_chk */

/*
 * z_inc, increment a variable.
 *
 * 	zargs[0] = variable to increment
 *
 */

void z_inc (void)
{
    zword value;
    zword z0;
    short sv;

    z0 = zargs[0];
    z0 &= 0xffff;
    
    if (z0 == 0) {
        sv = s16(*sp);
        sv += 1;
        value = ( ( (zword) sv ) & 0xffff );
	*sp = value;
    }
    else if (z0 < 16) {
        sv = s16(*(fp -z0));
        sv +=1;
        value = ( ( (zword) sv) & 0xffff );
        *(fp - z0) = value;
        }
    else {
	zword addr = h_globals + 2 * (z0 - 16);
	value=lw(addr);
	value++;
        value &= 0xffff;
	sw(addr, value);
    }

}/* z_inc */

/*
 * z_inc_chk, increment a variable and branch if now greater than value.
 *
 * 	zargs[0] = variable to increment
 * 	zargs[1] = value to check variable against
 *
 */

void z_inc_chk (void)
{
    zword value;
    zword z0, z1;
    short sv, sz1;
    
    z0 = zargs[0];
    z1 = zargs[1];
    sz1 = s16(z1);
    
    if (z0 == 0) {
        sv = s16(*sp);
        sv += 1;
        value = ( ( (zword) sv ) & 0xffff );
	*sp = value;
    }
    else if (z0 < 16) {
        sv = s16(*(fp - z0));
        sv +=1;
        value = ( ( (zword) sv ) & 0xffff );
	*(fp - z0) = value;
    }
    else {
	zword addr = h_globals + 2 * (z0 - 16);
	value=lw(addr);
	value++;
        value &= 0xffff;
	sw(addr, value);
    }
    sv=s16(value);
    branch (sv > sz1);

}/* z_inc_chk */

/*
 * z_load, store the value of a variable.
 *
 *	zargs[0] = variable to store
 *
 */

void z_load (void)
{
    zword value;
    zword z0;
    z0 = zargs[0];
    z0 &= 0xffff;
    
    if (z0 == 0) {
	value = *sp;
    }
    else if (z0 < 16)
	value = *(fp - z0);
    else {
	zword addr = h_globals + 2 * (z0 - 16);
	value=lw(addr);
    }

    store (value & 0xffff);

}/* z_load */

/*
 * z_pop, pop a value off the game stack and discard it.
 *
 *	no zargs used
 *
 */

void z_pop (void)
{

    sp++;

}/* z_pop */

/*
 * z_pop_stack, pop n values off the game or user stack and discard them.
 *
 *	zargs[0] = number of values to discard
 *	zargs[1] = address of user stack (optional)
 *
 */

void z_pop_stack (void)
{

    if (zargc == 2) {		/* it's a user stack */

	zword size;
	zword addr = zargs[1];

	size=lw(addr);

	size += zargs[0];
	storew (addr, size);

    } else sp += zargs[0];	/* it's the game stack */

}/* z_pop_stack */

/*
 * z_pull, pop a value off...
 *
 * a) ...the game or a user stack and store it (V6)
 *
 *	zargs[0] = address of user stack (optional)
 *
 * b) ...the game stack and write it to a variable (other than V6)
 *
 *	zargs[0] = variable to write value to
 *
 */

void z_pull (void)
{
    zword value;

    if (h_version != V6) {	/* not a V6 game, pop stack and write */

	value = *sp++;

	if (zargs[0] == 0)
	    *sp = value;
	else if (zargs[0] < 16)
	    *(fp - zargs[0]) = value;
	else {
	    zword addr = h_globals + 2 * (zargs[0] - 16);
	    sw(addr, value);
	}

    } else {			/* it's V6, but is there a user stack? */

	if (zargc == 1) {	/* it's a user stack */

	    zword size;
	    zword addr = zargs[0];

	    size=lw(addr);

	    size++;
	    storew (addr, size);

	    addr += 2 * size;
	    value=lw(addr);

	} else value = *sp++;	/* it's the game stack */

	store (value);

    }

}/* z_pull */

/*
 * z_push, push a value onto the game stack.
 *
 *	zargs[0] = value to push onto the stack
 *
 */

void z_push (void)
{

    *--sp = zargs[0];

}/* z_push */

/*
 * z_push_stack, push a value onto a user stack then branch if successful.
 *
 *	zargs[0] = value to push onto the stack
 *	zargs[1] = address of user stack
 *
 */

void z_push_stack (void)
{
    zword size;
    zword addr = zargs[1];

    size=lw(addr);

    if (size != 0) {

	storew ((zword) (addr + 2 * size), zargs[0]);

	size--;
	storew (addr, size);

    }

    branch (size);

}/* z_push_stack */

/*
 * z_store, write a value to a variable.
 *
 * 	zargs[0] = variable to be written to
 *      zargs[1] = value to write
 *
 */

void z_store (void)
{
    zword value = zargs[1];

    if (zargs[0] == 0)
	*sp = value;
    else if (zargs[0] < 16)
	*(fp - zargs[0]) = value;
    else {
	zword addr = h_globals + 2 * (zargs[0] - 16);
	sw(addr, value);
    }

}/* z_store */
