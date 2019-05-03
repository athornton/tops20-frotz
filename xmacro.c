#include "frotz.h"
#include <stdio.h>

zbyte cb(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zbyte byte;

    pc = (long) ( (long) pcp - (long) zmp);

    byte = (zmp[pc]) & 0xff;
    fprintf(stderr, \
            "DEBUG: cb   entry:              PCP = %p; ZMP = %p\n", \
            pcp, zmp);    
    fprintf(stderr, \
            "DEBUG: cb   value: 0x%02x; PC = 0x%lx\n", \
            byte, pc);
    pc = pc + 1;
    pcp = (zbyte *) ( (long ) zmp + (long) pc);
    fprintf(stderr, \
            "DEBUG: cb_inc   v: 0x%02x; PC = 0x%lx\n", \
            byte, pc);
    return byte;
}

zword cw(void) {
    extern zbyte *pcp;
    extern zbyte *zmp;
    long pc;
    zword v;

    pc = (long) ((long) pcp - (long) zmp);
    fprintf(stderr, \
            "DEBUG: cw   entry:             PC = 0x%lx\n", \
            pc);

    v = (zword) ((zbyte) (zmp[pc] & 0xff) * 256 \
                 + (zbyte) (zmp[pc+1] & 0xff ) ); 
    pcp = pcp + 2;
    pc = (long) ( (long) pcp - (long) zmp);

    fprintf(stderr, \
            "DEBUG: cw   exit : v = 0x%x; PC = 0x%lx\n", \
            v, pc);
    return v;
}

long g_pc(void) {
    long pc;
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: g_pc entry:              PCP = %p; ZMP = %p\n", \
            pcp, zmp);
    pc = (long) ( (long) pcp - (long) zmp);
    fprintf(stderr, \
            "DEBUG: g_pc exit : PC = 0x%lx\n", \
            pc);
    return pc;
}

void s_pc(long pc) {
    extern zbyte *pcp, *zmp;
    fprintf(stderr, \
            "DEBUG: s_pc entry: PC = 0x%lx\n", \
            pc);
    pcp = (zbyte *) (pc + (long) zmp);
    fprintf(stderr, \
            "DEBUG: s_pc exit : PC = 0x%lx\n", \
            pc);
    fprintf(stderr, \
            "DEBUG: s_pc exit :              PCP = %p; ZMP = %p\n", \
            pcp, zmp);    
    
    return;
}
