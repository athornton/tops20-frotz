/*
 * table.c
 *
 * Table handling opcodes
 *
 */

#include "frotz.h"

/*
 * A00098, copy a table or fill it with zeroes.
 *
 *	zargs[0] = address of table
 * 	zargs[1] = destination address or 0 for fill
 *	zargs[2] = size of table
 *
 * Note: Copying is safe even when source and destination overlap; but
 *       if zargs[1] is negative the table _must_ be copied forwards.
 *
 */

void A00098 (void)
{
    short ssz;
    zword addr;
    zword size = zargs[2];
    zbyte value;
    int i;

    /* TODO : this looks like it could use some masking */
    ssz=s16(size);
    if (zargs[1] == 0)      				/* zero table */

	for (i = 0; i < (size & 0xffff); i++)
	    A00194 ((zword) ((zargs[0] + i) & 0xffff), 0);

    else if (ssz < 0 || (zargs[0] & 0xffff) > (zargs[1] & 0xffff)) {
        /*copy forwards */

	for (i = 0; i < ((ssz < 0) ? - ssz : ssz); i++) {
	    addr = ((zargs[0] + i) & 0xffff);
	    LOW_BYTE (addr, value)
            A00194 ((zword) ((zargs[1] + i) & 0xfff), value);
	}
    }
    else						/* copy backwards */

	for (i = size - 1; i >= 0; i--) {
	    addr = zargs[0] + i;
	    LOW_BYTE (addr, value)
	    A00194 ((zword) (zargs[1] + i), value);
	}

}/* A00098 */

/*
 * A00119, store a value from a table of bytes.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry to store
 *
 */

void A00119 (void)
{
    zword addr = zargs[0] + zargs[1];
    zbyte value;

    LOW_BYTE (addr, value)

    store (value);

}/* A00119 */

/*
 * A00120, store a value from a table of words.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry to store
 *
 */

void A00120 (void)
{
    zword addr = (zargs[0] + 2 * zargs[1]) & 0xffff;
    zword value;

    value=lw(addr);

    store (value);

}/* A00120 */

/*
 * A00160, find and store the address of a target within a table.
 *
 *	zargs[0] = target value to be searched for
 *	zargs[1] = address of table
 *	zargs[2] = number of table entries to check value against
 *	zargs[3] = type of table (optional, defaults to 0x82)
 *
 * Note: The table is a word array if bit 7 of zargs[3] is set; otherwise
 *       it's a byte array. The lower bits hold the address step.
 *
 */

void A00160 (void)
{
    zword addr = zargs[1];
    int i;

    /* Supply default arguments */

    if (zargc < 4)
	zargs[3] = 0x82;

    /* Scan byte or word array */

    for (i = 0; i < zargs[2]; i++) {

	if (zargs[3] & 0x80) {	/* scan word array */

	    zword wvalue;

	    wvalue=lw(addr);

	    if (wvalue == zargs[0])
		goto finished;

	} else {		/* scan byte array */

	    zbyte bvalue;

	    LOW_BYTE (addr, bvalue)

	    if (bvalue == zargs[0])
		goto finished;

	}

	addr += zargs[3] & 0x7f;

    }

    addr = 0;

finished:

    store (addr);
    A00193 (addr);

}/* A00160 */

/*
 * A00173, write a byte into a table of bytes.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry
 *	zargs[2] = value to be written
 *
 */

void A00173 (void)
{

    A00194 ((zword) (zargs[0] + zargs[1]), zargs[2]);

}/* A00173 */

/*
 * A00174, write a word into a table of words.
 *
 *	zargs[0] = address of table
 *	zargs[1] = index of table entry
 *	zargs[2] = value to be written
 *
 */

void A00174 (void)
{

    A00195 ((zword) (zargs[0] + 2 * zargs[1]), zargs[2]);

}/* A00174 */
