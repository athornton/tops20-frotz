#include "frotz.h"

short sanitize_16( zword z) {
    short s;

    s = (short) (z & 0xffff) ;
    if ( ( s > 0 )  && ( s > 32767 ) ) {
        s = - (65536 - s);
    }

    return s;

}
