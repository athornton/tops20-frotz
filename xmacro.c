#include "frotz.h"
#include <stdio.h>

/* The largest Z-Machine address is 512K for v8; so we can take just the
   bottom 19 bits. */

zbyte cb(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zbyte byte;

    pc = (long) ( ( (long) pcp - (long) zmp ) & 0x7ffff );

    byte = (zmp[pc]) & 0xff;
    /* fprintf(stderr,                                           \
            "DEBUG: cb   entry:                    PCP = %p\n", \
            pcp);
    fprintf(stderr, \
            "DEBUG: cb   value: 0x%02x; PC = 0x%lx\n", \
            byte, pc); */
    pc = (pc + 1) & 0x7ffff;
    pcp = (zbyte *) ( (long ) zmp + (long) pc);
    /* fprintf(stderr,                                  \
            "DEBUG: cb_inc   v: 0x%02x; PC = 0x%lx\n", \
            byte, pc);
    fprintf(stderr, \
            "DEBUG: cb   exit :                    PCP = %p\n", \
            pcp); */

    return byte;
}

zword cw(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zword v;

    pc = (long) ( ( (long) pcp - (long) zmp ) & 0x7ffff) ;
    /* fprintf(stderr,                                           \
            "DEBUG: cw   entry:                    PCP = %p\n", \
            pcp);
    fprintf(stderr, \
            "DEBUG: cw   entry:             PC = 0x%lx\n", \
            pc); */

    v = (zword) ((zbyte) (zmp[pc] & 0xff) * 256 \
                 + (zbyte) (zmp[pc+1] & 0xff ) );
    pc = (pc + 2) & 0x7ffff ;
    pcp = (zbyte *) ( (long ) zmp + (long) pc);

    /* fprintf(stderr,                                    \
            "DEBUG: cw   exit : v = 0x%x; PC = 0x%lx\n", \
            v, pc);
    fprintf(stderr, \
            "DEBUG: cw   exit :                    PCP = %p\n", \
            pcp); */
    return v;
}

long g_pc(void) {
    long pc;
    extern zbyte *pcp, *zmp;
    /* fprintf(stderr,                                           \
            "DEBUG: g_pc entry:                    PCP = %p\n", \
            pcp); */
    pc = (long) ( ( (long) pcp - (long) zmp) & 0x7ffff) ;
    /* fprintf(stderr,                          \
            "DEBUG: g_pc exit : PC = 0x%lx\n", \
            pc); */
    return pc;
}

void s_pc(long pc) {
    extern zbyte *pcp, *zmp;
    /* fprintf(stderr,                          \
            "DEBUG: s_pc entry: PC = 0x%lx\n", \
            pc); */
    pcp = (zbyte *) (pc + (long) zmp);
    /* fprintf(stderr,                          \
            "DEBUG: s_pc exit : PC = 0x%lx\n", \
            pc);
    fprintf(stderr, \
            "DEBUG: s_pc exit :                    PCP = %p\n", \
            pcp); */

    return;
}

short s16(zword z) {
    short sz;
    sz = (short) (z & 0xffff);
    if ( ( sz > 0 ) && (sz > 32767 ) ) {
        sz = - (65536 - sz );
    }
    return sz;
}
    
