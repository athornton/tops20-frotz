#include "frotz.h"
#include <stdio.h>

zword cw(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zword retval;

    pc = (long) (pcp - zmp);
    fprintf(stderr, \
            "DEBUG: cw   entry: PC = 0x%lx; PCP = 0x%p; ZMP = 0x%p\n", \
            pc, pcp, zmp);

    retval = (zword) (256 * pcp[0] + pcp[1] );
    pcp += 2;
    pc = (long) (pcp - zmp);

    fprintf(stderr, \
            "DEBUG: cw   sp   : PC = 0x%lx; PCP = 0x%p; ZMP = 0x%p\n", \
            pc, pcp, zmp);

    /* Check for overflow */
    if (pc > 0xffff) {
        fprintf(stderr, "DEBUG: Code_word wrap detected: PC = 0x%lx\n", pc);
        /* Don't fix up...
        offset &= 0xffff;
        pcp = (zbyte *)((long) zmp + offset);
        */
    }

    fprintf(stderr, \
            "DEBUG: cw   exit : PC = 0x%lx; ret = 0x%x\n", \
            pc,retval);
    return retval;
}

long g_pc(void) {
    long pc;
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: g_pc entry:              PCP = 0x%p; ZMP = 0x%p\n", \
            pcp, zmp);
    pc = (long) (pcp - zmp);
    if ( pc > 0xffff) {
        fprintf(stderr, "DEBUG: PC wrap detected (get): 0x%lx\n", pc);
    }
    fprintf(stderr, \
            "DEBUG: g_pc exit : PC = 0x%lx, PCP = 0x%p, ZMP = 0x%p\n", \
            pc, pcp, zmp);
    return pc;
}

void s_pc(long pc) {
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: s_pc entry: PC = 0x%lx, PCP = 0x%p, ZMP = 0x%p\n", \
            pc, pcp, zmp);
    if ( pc > 0xffff) {
        fprintf(stderr, "DEBUG: PC wrap detected (set): 0x%lx\n", pc);
    }
    pcp = (zbyte *) (pc + (long) zmp);
    fprintf(stderr, \
            "DEBUG: s_pc exit : PC = 0x%lx, PCP = 0x%p, ZMP = 0x%p\n", \
            pc, pcp, zmp);
    return;
}
