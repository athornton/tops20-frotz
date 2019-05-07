perl -pi -e 's/([ \(])\(short\) ([^\s;]+)/$1sanitize_16( $2 )/g ;' *.c
