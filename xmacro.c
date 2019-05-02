#include "frotz.h"
#include <stdio.h>

zbyte cb(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;

    pc = (long) (pcp - zmp);
    zbyte byte;

    byte = *pcp;
    fprintf(stderr, \
            "DEBUG: cb   value: %02x; PC = %lx\n", \
            byte, pc);
    pcp++;
    pc = (long) (pcp - zmp);
    fprintf(stderr, \
            "DEBUG: cb_inc   v: %02x; PC = %lx\n", \
            byte, pc);
    return byte;
}

zword cw(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zword retval;

    pc = (long) (pcp - zmp);
    fprintf(stderr, \
            "DEBUG: cw   entry: PC = %lx\n", \
            pc);

    retval = (zword) (256 * pcp[0] + pcp[1] );
    pcp += 2;
    pc = (long) (pcp - zmp);

    fprintf(stderr, \
            "DEBUG: cw   sp   : PC = %lx\n", \
            pc);

    /* Check for overflow */
    if (pc > 0xffff) {
        fprintf(stderr, "DEBUG: Code_word wrap detected: PC = %lx\n", pc);
        /* Don't fix up...
        offset &= ffff;
        pcp = (zbyte *)((long) zmp + offset);
        */
    }

    fprintf(stderr, \
            "DEBUG: cw   exit : PC = %lx; ret = %x\n", \
            pc,retval);
    return retval;
}

long g_pc(void) {
    long pc;
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: g_pc entry:              PCP = %p; ZMP = %p\n", \
            pcp, zmp);
    pc = (long) (pcp - zmp);
    if ( pc > 0xffff) {
        fprintf(stderr, "DEBUG: PC wrap detected (get): %lx\n", pc);
    }
    fprintf(stderr, \
            "DEBUG: g_pc exit : PC = %lx\n", \
            pc);
    return pc;
}

void s_pc(long pc) {
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: s_pc entry: PC = %lx\n", \
            pc);
    if ( pc > 0xffff) {
        fprintf(stderr, "DEBUG: PC wrap detected (set): %lx\n", pc);
    }
    pcp = (zbyte *) (pc + (long) zmp);
    fprintf(stderr, \
            "DEBUG: s_pc exit : PC = %lx\n", \
            pc);
    return;
}
