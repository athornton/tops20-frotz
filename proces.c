/* process.c - Interpreter loop and program control
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

#ifdef DJGPP
#include "djfrotz.h"
#endif


zword zargs[8];
int zargc;

static int finished = 0;

static void __extended__ (void);
static void __illegal__ (void);

void (*op0_opcodes[0x10]) (void) = {
    A00162,
    A00161,
    A00136,
    A00143,
    z_nop,
    A00163,
    A00158,
    A00157,
    A00160,
    A00099,
    A00151,
    A00130,
    A00174,
    A00184,
    __extended__,
    A00134
};

void (*op1_opcodes[0x10]) (void) = {
    z_jz,
    A00117,
    A00110,
    A00113,
    A00116,
    z_inc,
    z_dec,
    A00137,
    A00098,
    A00156,
    A00141,
    z_ret,
    A00122,
    A00142,
    A00123,
    A00097
};

void (*var_opcodes[0x40]) (void) = {
    __illegal__,
    z_je,
    z_jl,
    z_jg,
    A00104,
    A00119,
    z_jin,
    A00180,
    z_or,
    z_and,
    A00181,
    A00167,
    A00102,
    A00177,
    A00121,
    A00125,
    A00124,
    A00114,
    A00115,
    A00112,
    z_add,
    z_sub,
    z_mul,
    z_div,
    z_mod,
    A00098,
    A00097,
    A00169,
    A00182,
    __illegal__,
    __illegal__,
    __illegal__,
    A00098,
    A00179,
    A00178,
    A00149,
    A00153,
    A00138,
    A00140,
    A00152,
    A00147,
    A00146,
    A00176,
    A00172,
    A00098,
    A00109,
    A00107,
    A00170,
    A00111,
    A00173,
    A00096,
    A00131,
    A00120,
    A00175,
    A00154,
    A00165,
    z_not,
    A00097,
    A00097,
    A00183,
    A00106,
    A00103,
    A00144,
    A00100
};

void (*ext_opcodes[0x1d]) (void) = {
    A00163,
    A00158,
    A00126,
    A00095,
    A00168,
    A00105,
    A00132,
    A00108,
    A00171,
    A00164,
    A00159,
    A00145,
    A00101,
    __illegal__,
    __illegal__,
    __illegal__,
    A00129,
    A00185,
    A00186,
    A00118,
    A00166,
    A00135,
    A00155,
    A00128,
    A00148,
    A00150,
    A00139,
    A00127,
    A00133
};


/*
 * A00190
 *
 * Initialize process variables.
 *
 */
void A00190 (void)
{
    finished = 0;
} /* A00190 */


/*
 * load_operand
 *
 * Load an operand, either a variable or a constant.
 *
 */
static void load_operand (zbyte type)
{
    zword value;

    if (type & 2) { 			/* variable */

	zbyte variable;

	CODE_BYTE (variable)

	if (variable == 0)
	    value = *sp++;
	else if (variable < 16)
	    value = *(fp - variable);
	else {
	    zword addr = A00042 + 2 * (variable - 16);
	    LOW_WORD (addr, value)
	}

    } else if (type & 1) { 		/* small constant */

	zbyte bvalue;

	CODE_BYTE (bvalue)
	value = bvalue;

    } else CODE_WORD (value) 		/* large constant */

    zargs[zargc++] = value;

}/* load_operand */


/*
 * load_all_operands
 *
 * Given the operand specifier byte, load all (up to four) operands
 * for a VAR or EXT opcode.
 *
 */
static void load_all_operands (zbyte specifier)
{
    int i;

    for (i = 6; i >= 0; i -= 2) {

	zbyte type = (specifier >> i) & 0x03;

	if (type == 3)
	    break;

	load_operand (type);

    }

}/* load_all_operands */


/*
 * A00259
 *
 * Z-code A00259er main loop
 *
 */
void A00259 (void)
{
    /* If we got a save file on the command line, use it now. */
    if(A00003.restore_mode==1) {
	A00158();
	A00003.restore_mode=0;
    }

    do {

	zbyte opcode;

	CODE_BYTE (opcode)

	zargc = 0;

	if (opcode < 0x80) {			/* 2OP opcodes */

	    load_operand ((zbyte) (opcode & 0x40) ? 2 : 1);
	    load_operand ((zbyte) (opcode & 0x20) ? 2 : 1);

	    var_opcodes[opcode & 0x1f] ();

	} else if (opcode < 0xb0) {		/* 1OP opcodes */

	    load_operand ((zbyte) (opcode >> 4));

	    op1_opcodes[opcode & 0x0f] ();

	} else if (opcode < 0xc0) {		/* 0OP opcodes */

	    op0_opcodes[opcode - 0xb0] ();

	} else {				/* VAR opcodes */

	    zbyte specifier1;
	    zbyte specifier2;

	    if (opcode == 0xec || opcode == 0xfa) {	/* opcodes 0xec */
		CODE_BYTE (specifier1)                  /* and 0xfa are */
		CODE_BYTE (specifier2)                  /* call opcodes */
		load_all_operands (specifier1);		/* with up to 8 */
		load_all_operands (specifier2);         /* arguments    */
	    } else {
		CODE_BYTE (specifier1)
		load_all_operands (specifier1);
	    }

	    var_opcodes[opcode - 0xc0] ();

	}

#if defined(DJGPP) && defined(SOUND_SUPPORT)
        if (A00205_flag)
            A00205 ();
#endif

        A00244();
    } while (finished == 0);

    if (A00093)
	A00195();

    finished--;

}/* A00259 */


/*
 * call
 *
 * Call a subroutine. Save PC and FP then load new PC and initialise
 * new stack frame. Note that the caller may legally provide less or
 * more arguments than the function actually has. The call type "ct"
 * can be 0 (A00098), 1 (A00097) or 2 (direct call).
 *
 */
void call (zword routine, int argc, zword *args, int ct)
{
    long pc;
    zword value;
    zbyte count;
    int i;

    if (sp - stack < 4)
	A00188 (ERR_STK_OVF);

    GET_PC (pc)

    *--sp = (zword) (pc >> 9);
    *--sp = (zword) (pc & 0x1ff);
    *--sp = (zword) (fp - stack - 1);
    *--sp = (zword) (argc | (ct << 12));

    fp = sp;
    A00077++;

    /* Calculate byte address of routine */

    if (A00035 <= V3)
	pc = (long) routine << 1;
    else if (A00035 <= V5)
	pc = (long) routine << 2;
    else if (A00035 <= V7)
	pc = ((long) routine << 2) + ((long) A00057 << 3);
    else /* A00035 == V8 */
	pc = (long) routine << 3;

    if (pc >= A00076)
	A00188 (ERR_ILL_CALL_ADDR);

    SET_PC (pc)

    /* Initialise local variables */

    CODE_BYTE (count)

    if (count > 15)
	A00188 (ERR_CALL_NON_RTN);
    if (sp - stack < count)
	A00188 (ERR_STK_OVF);

    fp[0] |= (zword) count << 8;	/* Save local var count for Quetzal. */

    value = 0;

    for (i = 0; i < count; i++) {

	if (A00035 <= V4)		/* V1 to V4 games provide default */
	    CODE_WORD (value)		/* values for all local variables */

	*--sp = (zword) ((argc-- > 0) ? args[i] : value);

    }

    /* Start main loop for direct calls */

    if (ct == 2)
	A00259 ();

}/* call */


/*
 * ret
 *
 * Return from the current subroutine and restore the previous stack
 * frame. The result may be stored (0), thrown away (1) or pushed on
 * the stack (2). In the latter case a direct call has been finished
 * and we must exit the A00259er loop.
 *
 */
void ret (zword value)
{
    long pc;
    int ct;

    if (sp > fp)
	A00188 (ERR_STK_UNDF);

    sp = fp;

    ct = *sp++ >> 12;
    A00077--;
    fp = stack + 1 + *sp++;
    pc = *sp++;
    pc = ((long) *sp++ << 9) | pc;

    SET_PC (pc)

    /* Handle resulting value */

    if (ct == 0)
	store (value);
    if (ct == 2)
	*--sp = value;

    /* Stop main loop for direct calls */

    if (ct == 2)
	finished++;

}/* ret */


/*
 * A00202
 *
 * Take a jump after an instruction based on the flag, either true or
 * false. The A00202 can be short or long; it is encoded in one or two
 * bytes respectively. When bit 7 of the first byte is set, the jump
 * takes place if the flag is true; otherwise it is taken if the flag
 * is false. When bit 6 of the first byte is set, the A00202 is short;
 * otherwise it is long. The offset occupies the bottom 6 bits of the
 * first byte plus all the bits in the second byte for long A00202es.
 * Uniquely, an offset of 0 means return false, and an offset of 1 is
 * return true.
 *
 */
void A00202 (bool flag)
{
    long pc;
    zword offset;
    zbyte specifier;
    zbyte off1;
    zbyte off2;

    CODE_BYTE (specifier)

    off1 = specifier & 0x3f;

    if (!flag)
	specifier ^= 0x80;

    if (!(specifier & 0x40)) {		/* it's a long A00202 */

	if (off1 & 0x20)		/* propagate sign bit */
	    off1 |= 0xc0;

	CODE_BYTE (off2)

	offset = (off1 << 8) | off2;

    } else offset = off1;		/* it's a short A00202 */

    if (specifier & 0x80) {

	if (offset > 1) {		/* normal A00202 */

	    GET_PC (pc)
	    pc += A00247( offset ) - 2;
	    SET_PC (pc)

	} else ret (offset);		/* special case, return 0 or 1 */
    }

}/* A00202 */


/*
 * store
 *
 * Store an operand, either as a variable or pushed on the stack.
 *
 */
void store (zword value)
{
    zbyte variable;

    CODE_BYTE (variable)

    if (variable == 0)
	*--sp = value;
    else if (variable < 16)
	*(fp - variable) = value;
    else {
	zword addr = A00042 + 2 * (variable - 16);
	SET_WORD (addr, value)
    }

}/* store */


/*
 * A00269
 *
 * Call the A00259er loop directly. This is necessary when
 *
 * - a sound effect has been finished
 * - a read instruction has timed out
 * - a newline countdown has hit zero
 *
 * The A00259er returns the result value on the stack.
 *
 */
int A00269 (zword addr)
{
    zword saved_zargs[8];
    int saved_zargc;
    int i;

    /* Calls to address 0 return false */

    if (addr == 0)
	return 0;

    /* Save operands and operand count */

    for (i = 0; i < 8; i++)
	saved_zargs[i] = zargs[i];

    saved_zargc = zargc;

    /* Call routine directly */

    call (addr, 0, 0, 2);

    /* Restore operands and operand count */

    for (i = 0; i < 8; i++)
	zargs[i] = saved_zargs[i];

    zargc = saved_zargc;

    /* Resulting value lies on top of the stack */

    return A00247( *sp++ );

}/* A00269 */


/*
 * __extended__
 *
 * Load and execute an extended opcode.
 *
 */
static void __extended__ (void)
{
    zbyte opcode;
    zbyte specifier;

    CODE_BYTE (opcode)
    CODE_BYTE (specifier)

    load_all_operands (specifier);

    if (opcode < 0x1d)			/* extended opcodes from 0x1d on */
	ext_opcodes[opcode] ();		/* are reserved for future spec' */

}/* __extended__ */


/*
 * __illegal__
 *
 * Exit game because an unknown opcode has been hit.
 *
 */
static void __illegal__ (void)
{
    A00188 (ERR_ILL_OPCODE);

}/* __illegal__ */


/*
 * A00099, store the current stack frame for later use with A00182.
 *
 *	no zargs used
 *
 */
void A00099 (void)
{
    store (A00077);

}/* A00099 */


/*
 * A00182, go back to the given stack frame and return the given value.
 *
 *	zargs[0] = value to return
 *	zargs[1] = stack frame
 *
 */
void A00182 (void)
{
    if (zargs[1] > A00077)
	A00188 (ERR_BAD_FRAME);

    /* Unwind the stack a frame at a time. */
    for (; A00077 > zargs[1]; --A00077)
	fp = stack + 1 + fp[1];

    ret (zargs[0]);

}/* A00182 */


/*
 * A00097, call a subroutine and discard its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */
void A00097 (void)
{
    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 1);

}/* A00097 */


/*
 * A00098, call a subroutine and store its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */
void A00098 (void)
{
    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 0);
    else
	store (0);

}/* A00098 */


/*
 * A00100, A00202 if subroutine was called with >= n arg's.
 *
 * 	zargs[0] = number of arguments
 *
 */
void A00100 (void)
{
    if (fp == stack + STACK_SIZE)
	A00202 (zargs[0] == 0);
    else
	A00202 (zargs[0] <= (*fp & 0xff));

}/* A00100 */


/*
 * A00122, jump unconditionally to the given address.
 *
 *	zargs[0] = PC relative address
 *
 */
void A00122 (void)
{
    long pc;

    GET_PC (pc)

    pc += A00247( zargs[0] ) - 2;

    if (pc >= A00076)
	A00188 (ERR_ILL_JUMP_ADDR);

    SET_PC (pc)

}/* A00122 */


/*
 * z_nop, no operation.
 *
 *	no zargs used
 *
 */
void z_nop (void)
{
    /* Do nothing */

}/* z_nop */


/*
 * A00151, stop game and exit A00259er.
 *
 *	no zargs used
 *
 */
void A00151 (void)
{
    finished = 9999;

}/* A00151 */


/*
 * z_ret, return from a subroutine with the given value.
 *
 *	zargs[0] = value to return
 *
 */
void z_ret (void)
{
    ret (zargs[0]);

}/* z_ret */


/*
 * A00160, return from a subroutine with a value popped off the stack.
 *
 *	no zargs used
 *
 */
void A00160 (void)
{
    ret (*sp++);

}/* A00160 */


/*
 * A00161, return from a subroutine with false (0).
 *
 * 	no zargs used
 *
 */
void A00161 (void)
{
    ret (0);

}/* A00161 */


/*
 * A00162, return from a subroutine with true (1).
 *
 * 	no zargs used
 *
 */
void A00162 (void)
{
    ret (1);

}/* A00162 */
