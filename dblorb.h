/*
 * dumb_blorb.h
 *
 * Blorb related functions specific to the Dumb interface.
 *
 */

#ifndef NO_BLORB

#include "../blorb/blorb.h"
#include "../blorb/blorblow.h"


typedef struct sampledata_struct {
	unsigned short channels;
	unsigned long samples;
	unsigned short bits;
	double rate;
} sampledata_t;


/*
 * The bb_result_t struct lacks a few members that would make things a
 * bit easier.  The myresource struct takes encapsulates the bb_result_t
 * struct and adds a type member and a filepointer.  I would like to
 * convince Andrew Plotkin to make a change in the reference Blorb code
 * to add these members.
 *
 */
typedef struct {
    bb_result_t bbres;
    unsigned long type;
    FILE *fp;
} myresource;

/* These are used only locally */
/*
extern bb_err_t		A00004;
extern bb_map_t		*A00005;
extern FILE		*A00006;
*/
extern bb_result_t	A00007;

bb_err_t dumb_blorb_init(char *);
void A00008(void);

#endif /* NO_BLORB */
