#include "frotz.h"
#include <stdio.h>

/* The largest Z-Machine address is 512K for v8; so we can take just the
   bottom 19 bits for any address. */

void sb(zword addr, zbyte z) { /* replaces SET_BYTE */
    zmp[addr & 0xffff] = z & 0xff;
}

zbyte lb(zword addr) { /* replaces LOW_BYTE */
    return zmp[addr & 0xffff] & 0xff;
}


zbyte cb(void) { /* replaces CODE_BYTE */
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

void sw(long addr, zword value) { /* replaces SET_WORD */
    zmp[addr] = hi(value);
    zmp[addr+1] = lo(value);
}

zword lw(long addr) { /* replaces LOW_WORD */
    zword value;

    value = (zword) ( ( (zword) ((zmp[addr] & 0xff) * 256) +
                        (zword) ((zmp[addr + 1] & 0xff) ) ) & 0xffff );
    return value;
}

zword hw(long addr) { /* replaces HIGH_WORD...although it's the same thing
                         as LOW_WORD (?!?) */
    return lw(addr);
}
    
zword cw(void) { /* replaces CODE_WORD */
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

long g_pc(void) { /* replaces GET_PC) */
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

void s_pc(long pc) { /* replaces SET_PC */
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

short s16(zword z) { /* 16-bitify */
    short sz;
    sz = (short) (z & 0xffff);
    if ( ( sz > 0 ) && (sz > 32767 ) ) {
        sz = - (65536 - sz );
        } 
    return sz;
}

char c8(zbyte z) { /* 8-bitify) */
    char cz;
    cz = (char) (z & 0xff);
    if ( ( cz >0 ) && (cz > 127) ) {
        cz = - (128 - cz);
    }
    return cz;
}
