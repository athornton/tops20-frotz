#include "frotz.h"
#include <stdio.h>

zbyte cb(void) {
    extern zbyte *pcp;
    zbyte byte;

    byte = *pcp;
    fprintf(stderr, \
            "DEBUG: cb   value: %x; PCP = %p\n", \
            byte, pcp);
    pcp++;
    fprintf(stderr, \
            "DEBUG: cb_inc   v: %x; PCP = %p\n", \
            byte, pcp);
    return byte;
}

zword cw(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zword retval;

    pc = (long) (pcp - zmp);
    fprintf(stderr, \
            "DEBUG: cw   entry: PC = %lx; PCP = %p; ZMP = %p\n", \
            pc, pcp, zmp);

    retval = (zword) (256 * pcp[0] + pcp[1] );
    pcp += 2;
    pc = (long) (pcp - zmp);

    fprintf(stderr, \
            "DEBUG: cw   sp   : PC = %lx; PCP = %p; ZMP = %p\n", \
            pc, pcp, zmp);

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
            "DEBUG: g_pc exit : PC = %lx, PCP = %p, ZMP = %p\n", \
            pc, pcp, zmp);
    return pc;
}

void s_pc(long pc) {
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: s_pc entry: PC = %lx, PCP = %p, ZMP = %p\n", \
            pc, pcp, zmp);
    if ( pc > 0xffff) {
        fprintf(stderr, "DEBUG: PC wrap detected (set): %lx\n", pc);
    }
    pcp = (zbyte *) (pc + (long) zmp);
    fprintf(stderr, \
            "DEBUG: s_pc exit : PC = %lx, PCP = %p, ZMP = %p\n", \
            pc, pcp, zmp);
    return;
}
