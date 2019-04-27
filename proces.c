/*
 * process.c
 *
 * Interpreter loop and program control
 *
 */

#include "frotz.h"

zword zargs[8];
int zargc;

static finished = 0;

static void __extended__ (void);
static void __illegal__ (void);

void (*op0_opcodes[0x10]) (void) = {
    A00157,
    A00156,
    A00131,
    A00138,
    z_nop,
    A00158,
    A00153,
    A00152,
    A00155,
    A00094,
    A00146,
    A00125,
    A00169,
    A00179,
    __extended__,
    A00129
};

void (*op1_opcodes[0x10]) (void) = {
    z_jz,
    A00112,
    A00105,
    A00108,
    A00111,
    z_inc,
    z_dec,
    A00132,
    A00093,
    A00151,
    A00136,
    z_ret,
    A00117,
    A00137,
    A00118,
    A00092
};

void (*var_opcodes[0x40]) (void) = {
    __illegal__,
    z_je,
    z_jl,
    z_jg,
    A00099,
    A00114,
    z_jin,
    A00175,
    z_or,
    z_and,
    A00176,
    A00162,
    A00097,
    A00172,
    A00116,
    A00120,
    A00119,
    A00109,
    A00110,
    A00107,
    z_add,
    z_sub,
    z_mul,
    z_div,
    z_mod,
    A00093,
    A00092,
    A00164,
    A00177,
    __illegal__,
    __illegal__,
    __illegal__,
    A00093,
    A00174,
    A00173,
    A00144,
    A00148,
    A00133,
    A00135,
    A00147,
    A00142,
    A00141,
    A00171,
    A00167,
    A00093,
    A00104,
    A00102,
    A00165,
    A00106,
    A00168,
    A00091,
    A00126,
    A00115,
    A00170,
    A00149,
    A00160,
    z_not,
    A00092,
    A00092,
    A00178,
    A00101,
    A00098,
    A00139,
    A00095
};

void (*ext_opcodes[0x1d]) (void) = {
    A00158,
    A00153,
    A00121,
    A00090,
    A00163,
    A00100,
    A00127,
    A00103,
    A00166,
    A00159,
    A00154,
    A00140,
    A00096,
    __illegal__,
    __illegal__,
    __illegal__,
    A00124,
    A00180,
    A00181,
    A00113,
    A00161,
    A00130,
    A00150,
    A00123,
    A00143,
    A00145,
    A00134,
    A00122,
    A00128
};

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
	    zword addr = A00032 + 2 * (variable - 16);
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
 * A00235
 *
 * Z-code A00235er main loop
 *
 */

void A00235 (void)
{

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

    } while (finished == 0);

    finished--;

}/* A00235 */

/*
 * call
 *
 * Call a subroutine. Save PC and FP then load new PC and initialise
 * new stack frame. Note that the caller may legally provide less or
 * more arguments than the function actually has. The call type "ct"
 * can be 0 (A00093), 1 (A00092) or 2 (direct call).
 *
 */

void call (zword routine, int argc, zword *args, int ct)
{
    long pc;
    zword value;
    zbyte count;
    int i;

    if (sp - stack < 4)
	A00192 ("Stack overflow");

    GET_PC (pc)

    *--sp = (zword) (pc >> 9);		/* for historical reasons */
    *--sp = (zword) (pc & 0x1ff);	/* Frotz keeps its stack  */
    *--sp = (zword) (fp - stack - 1);	/* format compatible with */
    *--sp = (zword) (argc | (ct << 8));	/* Mark Howell's Zip      */

    fp = sp;

    /* Calculate byte address of routine */

    if (A00025 <= V3)
	pc = (long) routine << 1;
    else if (A00025 <= V5)
	pc = (long) routine << 2;
    else if (A00025 <= V7)
	pc = ((long) routine << 2) + ((long) A00047 << 3);
    else /* A00025 == V8 */
	pc = (long) routine << 3;

    if (pc >= A00064)
	A00192 ("Call to illegal address");

    SET_PC (pc)

    /* Initialise local variables */

    CODE_BYTE (count)

    if (count > 15)
	A00192 ("Call to non-routine");
    if (sp - stack < count)
	A00192 ("Stack overflow");

    value = 0;

    for (i = 0; i < count; i++) {

	if (A00025 <= V4)		/* V1 to V4 games provide default */
	    CODE_WORD (value)		/* values for all local variables */

	*--sp = (zword) ((argc-- > 0) ? args[i] : value);

    }

    /* Start main loop for direct calls */

    if (ct == 2)
	A00235 ();

}/* call */

/*
 * ret
 *
 * Return from the current subroutine and restore the previous stack
 * frame. The result may be stored (0), thrown away (1) or pushed on
 * the stack (2). In the latter case a direct call has been finished
 * and we must exit the A00235er loop.
 *
 */

void ret (zword value)
{
    long pc;
    int ct;

    if (sp > fp)
	A00192 ("Stack underflow");

    sp = fp;

    ct = *sp++ >> 8;
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
 * A00193
 *
 * Take a jump after an instruction based on the flag, either true or
 * false. The A00193 can be short or long; it is encoded in one or two
 * bytes respectively. When bit 7 of the first byte is set, the jump
 * takes place if the flag is true; otherwise it is taken if the flag
 * is false. When bit 6 of the first byte is set, the A00193 is short;
 * otherwise it is long. The offset occupies the bottom 6 bits of the
 * first byte plus all the bits in the second byte for long A00193es.
 * Uniquely, an offset of 0 means return false, and an offset of 1 is
 * return true.
 *
 */

void A00193 (bool flag)
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

    if (!(specifier & 0x40)) {		/* it's a long A00193 */

	if (off1 & 0x20)		/* propagate sign bit */
	    off1 |= 0xc0;

	CODE_BYTE (off2)

	offset = (off1 << 8) | off2;

    } else offset = off1;		/* it's a short A00193 */

    if (specifier & 0x80)

	if (offset > 1) {		/* normal A00193 */

	    GET_PC (pc)
	    pc += (short) offset - 2;
	    SET_PC (pc)

	} else ret (offset);		/* special case, return 0 or 1 */

}/* A00193 */

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
	zword addr = A00032 + 2 * (variable - 16);
	SET_WORD (addr, value)
    }

}/* store */

/*
 * A00241
 *
 * Call the A00235er loop directly. This is necessary when
 *
 * - a sound effect has been finished
 * - a read instruction has timed out
 * - a newline countdown has hit zero
 *
 * The A00235er returns the result value on the stack.
 *
 */

int A00241 (zword addr)
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

    return (short) *sp++;

}/* A00241 */

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

    A00192 ("Illegal opcode");

}/* __illegal__ */

/*
 * A00094, store the current stack frame for later use with A00177.
 *
 *	no zargs used
 *
 */

void A00094 (void)
{

    store ((zword) (fp - stack));

}/* A00094 */

/*
 * A00177, go back to the given stack frame and return the given value.
 *
 *	zargs[0] = value to return
 *	zargs[1] = stack frame
 *
 */

void A00177 (void)
{

    if (zargs[1] > STACK_SIZE)
	A00192 ("Bad stack frame");

    fp = stack + zargs[1];

    ret (zargs[0]);

}/* A00177 */

/*
 * A00092, call a subroutine and discard its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */

void A00092 (void)
{

    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 1);

}/* A00092 */

/*
 * A00093, call a subroutine and store its result.
 *
 * 	zargs[0] = packed address of subroutine
 *	zargs[1] = first argument (optional)
 *	...
 *	zargs[7] = seventh argument (optional)
 *
 */

void A00093 (void)
{

    if (zargs[0] != 0)
	call (zargs[0], zargc - 1, zargs + 1, 0);
    else
	store (0);

}/* A00093 */

/*
 * A00095, A00193 if subroutine was called with >= n arg's.
 *
 * 	zargs[0] = number of arguments
 *
 */

void A00095 (void)
{

    if (fp == stack + STACK_SIZE)
	A00193 (zargs[0] == 0);
    else
	A00193 (zargs[0] <= (*fp & 0xff));

}/* A00095 */

/*
 * A00117, jump unconditionally to the given address.
 *
 *	zargs[0] = PC relative address
 *
 */

void A00117 (void)
{
    long pc;

    GET_PC (pc)

    pc += (short) zargs[0] - 2;

    if (pc >= A00064)
	A00192 ("Jump to illegal address");

    SET_PC (pc)

}/* A00117 */

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
 * A00146, stop game and exit A00235er.
 *
 *	no zargs used
 *
 */

void A00146 (void)
{

    finished = 9999;

}/* A00146 */

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
 * A00155, return from a subroutine with a value popped off the stack.
 *
 *	no zargs used
 *
 */

void A00155 (void)
{

    ret (*sp++);

}/* A00155 */

/*
 * A00156, return from a subroutine with false (0).
 *
 * 	no zargs used
 *
 */

void A00156 (void)
{

    ret (0);

}/* A00156 */

/*
 * A00157, return from a subroutine with true (1).
 *
 * 	no zargs used
 *
 */

void A00157 (void)
{

    ret (1);

}/* A00157 */
