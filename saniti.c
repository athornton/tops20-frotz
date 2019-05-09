#include "frotz.h"

short A00247 ( zword z ) {
    short s;

    s = (short) (z & 0xffff) ;
    if ( ( s > 0 )  && ( s > 32767 ) ) {
        s = - ( 65536 - s );
    }

    return s;
}

zword A00248 ( zword z ) {
    return ( z & 0xffff);
}
