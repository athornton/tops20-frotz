#include "frotz.h"
#include <stdio.h>

zword cw(void) {
    extern zbyte *pcp;
    zword retval;
    
    printf("cw entry: PCP is 0x%x\n", pcp);

    retval = (zword) (256 * pcp[0] + pcp[1] );
    pcp += 2;
    
    /* pcpt = ((unsigned int ) pcp) & 0xffff;
    pcp = (zbyte *) pcpt; */
    printf("cw exit: return value is 0x%x, PCP is 0x%x\n", retval, pcp);
    return retval;
}

long g_pc(void) {
    long v;
    extern zbyte *pcp, *zmp;
    printf("g_pc entry: PCP is 0x%x, ZMP is 0x%x\n", pcp, zmp);
    v = (long) (pcp - zmp);
    /*printf("g_pc mid: V is 0x%x, PCP is 0x%x, ZMP is 0x%x\n", v, pcp, zmp);
      v &= 0xffff; */
    printf("g_pc exit: V is 0x%x, PCP is 0x%x, ZMP is 0x%x\n", v, pcp, zmp);    
    return v;
}

void s_pc(long v) {
    extern zbyte *pcp, *zmp;
    long q;
    printf("s_pc entry: V is 0x%x, PCP is 0x%x, ZMP is 0x%x\n", v, pcp, zmp);
    q = (long ) ( zmp + v);
    /* q &= 0xffff; */
    pcp = (zbyte *) q;
    printf("s_pc exit: V is 0x%x, PCP is 0x%x, ZMP is 0x%x\n", v, pcp, zmp);
    return;
}

