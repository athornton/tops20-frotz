/*
 * dumb_blorb.c - Blorb routines
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 * Or visit http://www.fsf.org/
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <math.h>

#include "dfrotz.h"
#include "dblorb.h"

/* Don't build any of this if we don't want Blorb support. */
#ifndef NO_BLORB

extern fs_t A00003;

FILE *A00006;
bb_result_t A00007;
/* bb_map_t *A00005; */	/* Used only locally */

static int isblorb(FILE *);

#define UnsignedToFloat(u) (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)


/*
 * dumb_blorb_init
 *
 * Check if we're opening a Blorb file directly.  If not, check
 * to see if there's a seperate Blorb file that looks like it goes
 * along with this Zcode file.  If we have a Blorb file one way or the
 * other, make a Blorb map.  If we opened a Blorb file directly, that
 * means that our executable is in that file and therefore we will look
 * for a ZCOD chunk and record its location so os_load_story() can find it.
 * Make sure the Blorb file is opened and with the file pointer A00006.
 */
bb_err_t dumb_blorb_init(char *filename)
{
    FILE *fp;
    char *p;
    char *mystring;
    int  len1;
    int  len2;

    bb_err_t A00004;

    bb_map_t *A00005 = NULL;

    if ((fp = fopen(filename, "rb")) == NULL)
	return bb_err_Read;

    /* Is this really a Blorb file?  If not, maybe we're loading a naked
     * zcode file and our resources are in a seperate blorb file.
     */
    if (isblorb(fp)) {			/* Now we know to look */
	A00003.exec_in_blorb = 1;	/* for zcode in the blorb */
        A00006 = fopen(filename, "rb");
    } else {
	len1 = strlen(filename) + strlen(EXT_BLORB);
	len2 = strlen(filename) + strlen(EXT_BLORB3);

	mystring = malloc(len2 * sizeof(char) + 1);
        strncpy(mystring, filename, len1 * sizeof(char));
	p = strrchr(mystring, '.');
	if (p != NULL)
	    *p = '\0';

        strncat(mystring, EXT_BLORB, len1 * sizeof(char));

	/* Done monkeying with the initial file. */
	fclose(fp);
	fp = NULL;

	/* Check if foo.blb is there. */
        if ((A00006 = fopen(mystring, "rb")) == NULL) {
	    p = strrchr(mystring, '.');
	    if (p != NULL)
		*p = '\0';
            strncat(mystring, EXT_BLORB3, len2 * sizeof(char));
	    A00006 = fopen(mystring, "rb");
	}

	if (A00006 == NULL || !isblorb(fp))	/* No matching blorbs found. */
	    return bb_err_NoBlorb;

	/* At this point we know that we're using a naked zcode file */
	/* with resources in a seperate Blorb file. */
	A00003.use_blorb = 1;
    }

    /* Create a Blorb map from this file.
     * This will fail if the file is not a valid Blorb file.
     * From this map, we can now pick out any resource we need.
     */
    A00004 = bb_create_map(fp, &A00005);
    if (A00004 != bb_err_None)
	return bb_err_Format;

    /* Locate the EXEC chunk within the blorb file and record its
     * location so os_load_story() can find it.
     */
    if (A00003.exec_in_blorb) {
	A00004 = bb_load_chunk_by_type(A00005, bb_method_FilePos,
		&A00007, bb_ID_ZCOD, 0);
	A00003.exec_in_blorb = 1;
    }

    fclose(fp);
    return A00004;
}


/*
 * ux_blorb_stop
 *
 * Basically just close the Blorb file.
 *
 */
void ux_blorb_stop(void)
{
    if (A00006 != NULL)
	fclose(A00006);
    A00006 = NULL;
}

/*
 **********************************************
 * These functions are internal to ux_blorb.c
 *
 **********************************************
 */

/*
 * isblorb
 *
 * Returns 1 if this file is a Blorb file, 0 if not.
 *
 * FIXME Is there a potential endian problem here?
 */
static int isblorb(FILE *fp)
{
    char mybuf[4];

    if (fp == NULL)
	return 0;

    fread(mybuf, 1, 4, fp);
    if (strncmp(mybuf, "FORM", 4))
	return 0;

    fseek(fp, 4, SEEK_CUR);
    fread(mybuf, 1, 4, fp);

    if (strncmp(mybuf, "IFRS", 4))
	return 0;

    return 1;
}

#endif /* NO_BLORB */
