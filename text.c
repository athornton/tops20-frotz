/*
 * text.c
 *
 * Text manipulation functions
 *
 */

#include "frotz.h"

enum string_type {
    LOW_STRING, ABBREVIATION, HIGH_STRING, EMBEDDED_STRING, VOCABULARY
};

extern zword A00268 (zword);
extern zword cw (void);

static zchar decoded[10];
static zword encoded[3];

static zchar zscii_to_latin1[] = {
    0xe4, 0xf6, 0xfc, 0xc4, 0xd6, 0xdc, 0xdf, 0xab,
    0xbb, 0xeb, 0xef, 0xff, 0xcb, 0xcf, 0xe1, 0xe9,
    0xed, 0xf3, 0xfa, 0xfd, 0xc1, 0xc9, 0xcd, 0xd3,
    0xda, 0xdd, 0xe0, 0xe8, 0xec, 0xf2, 0xf9, 0xc0,
    0xc8, 0xcc, 0xd2, 0xd9, 0xe2, 0xea, 0xee, 0xf4,
    0xfb, 0xc2, 0xca, 0xce, 0xd4, 0xdb, 0xe5, 0xc5,
    0xf8, 0xd8, 0xe3, 0xf1, 0xf5, 0xc3, 0xd1, 0xd5,
    0xe6, 0xc6, 0xe7, 0xc7, 0xfe, 0xf0, 0xde, 0xd0,
    0xa3, 0x00, 0x00, 0xa1, 0xbf
};

/*
 * A00182
 *
 * Map a ZSCII character onto the ISO Latin-1 alphabet.
 *
 */

zchar A00182 (zbyte c)
{

    if (c == 0xfc)
	return ZC_MENU_CLICK;
    if (c == 0xfd)
	return ZC_DOUBLE_CLICK;
    if (c == 0xfe)
	return ZC_SINGLE_CLICK;

    if (c >= 0x9b && A00063 != BEYOND_ZORK)

	if (A00061 != 0) {	/* game has its own Unicode table */

	    zbyte N;

	    LOW_BYTE (A00061, N)

	    if (c - 0x9b < N) {

		zword addr = A00061 + 1 + 2 * (c - 0x9b);
		zword unicode;

		unicode=lw(addr);

		return (unicode < 0x100) ? (zchar) unicode : '?';

	    } else return '?';

	} else				/* game uses standard set */

	    if (c <= 0xdf) {

		if (c == 0xdc || c == 0xdd)	/* Oe and oe ligatures */
		    return '?';			/* are not ISO-Latin 1 */

		return zscii_to_latin1[c - 0x9b];

	    } else return '?';

    return c;

}/* A00182 */

/*
 * A00183
 *
 * Map an ISO Latin-1 character onto the ZSCII alphabet.
 *
 */

zbyte A00183 (zchar c)
{
    int i;

    if (c == ZC_SINGLE_CLICK)
	return 0xfe;
    if (c == ZC_DOUBLE_CLICK)
	return 0xfd;
    if (c == ZC_MENU_CLICK)
	return 0xfc;

    if (c >= ZC_LATIN1_MIN)

	if (A00061 != 0) {	/* game has its own Unicode table */

	    zbyte N;
	    int i;

	    LOW_BYTE (A00061, N)

	    for (i = 0x9b; i < 0x9b + N; i++) {

		zword addr = A00061 + 1 + 2 * (i - 0x9b);
		zword unicode;

		unicode=lw(addr);

		if (c == unicode)
		    return (zbyte) i;

	    }

	    return '?';

	} else {			/* game uses standard set */

	    for (i = 0x9b; i <= 0xdf; i++)
		if (c == zscii_to_latin1[i - 0x9b])
		    return (zbyte) i;

	    return '?';

	}

    return c;

}/* A00183 */

/*
 * alphabet
 *
 * Return a character from one of the three character sets.
 *
 */

static zchar alphabet (int set, int index)
{

    if (A00055 != 0) {	/* game uses its own alphabet */

	zbyte c;

	zword addr = A00055 + 26 * set + index;
	LOW_BYTE (addr, c)

	return A00182 (c);

    } else			/* game uses default alphabet */

	if (set == 0)
	    return 'a' + index;
	else if (set == 1)
	    return 'A' + index;
	else if (A00025 == V1)
	    return " 0123456789.,!?_#'\"/\\<-:()"[index];
	else
	    return " ^0123456789.,!?_#'\"/\\-:()"[index];

}/* alphabet */

/*
 * load_string
 *
 * Copy a ZSCII string from the memory to the global "decoded" string.
 *
 */

static void load_string (zword addr, zword length)
{
    int resolution = (A00025 <= V3) ? 2 : 3;
    int i = 0;

    while (i < 3 * resolution)

	if (i < length) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    decoded[i++] = A00182 (c);

	} else decoded[i++] = 0;

}/* load_string */

/*
 * encode_text
 *
 * Encode the Unicode text in the global "decoded" string then write
 * the result to the global "encoded" array. (This is used to look up
 * words in the dictionary.) Up to V3 the vocabulary resolution is
 * two, since V4 it is three words. Because each word contains three
 * Z-characters, that makes six or nine Z-characters respectively.
 * Longer words are chopped to the proper size, shorter words are are
 * padded out with 5's. For word completion we pad with 0s and 31s,
 * the minimum and maximum Z-characters.
 *
 */

static void encode_text (int padding)
{
    static zchar again[] = { 'a', 'g', 'a', 'i', 'n', 0 };
    static zchar examine[] = { 'e', 'x', 'a', 'm', 'i', 'n', 'e', 0 };
    static zchar wait[] = { 'w', 'a', 'i', 't', 0 };

    zbyte zchars[12];
    const zchar *ptr = decoded;
    zchar c;
    int resolution = (A00025 <= V3) ? 2 : 3;
    int i = 0;

    /* Expand abbreviations that some old Infocom games lack */

    if (A00087)

	if (padding == 0x05 && decoded[1] == 0)

	    switch (decoded[0]) {
		case 'g': ptr = again; break;
		case 'x': ptr = examine; break;
		case 'z': ptr = wait; break;
	    }

    /* Translate string to a sequence of Z-characters */

    while (i < 3 * resolution)

	if ((c = *ptr++) != 0) {

	    int index, set;
	    zbyte c2;

	    /* Search character in the alphabet */

	    for (set = 0; set < 3; set++)
		for (index = 0; index < 26; index++)
		    if (c == alphabet (set, index))
			goto letter_found;

	    /* Character not found, store its ZSCII value */

	    c2 = A00183 (c);

	    zchars[i++] = 5;
	    zchars[i++] = 6;
	    zchars[i++] = c2 >> 5;
	    zchars[i++] = c2 & 0x1f;

	    continue;

	letter_found:

	    /* Character found, store its index */

	    if (set != 0)
		zchars[i++] = ((A00025 <= V2) ? 1 : 3) + set;

	    zchars[i++] = index + 6;

	} else zchars[i++] = padding;

    /* Three Z-characters make a 16bit word */

    for (i = 0; i < resolution; i++)

	encoded[i] =
	    (zchars[3 * i + 0] << 10) |
	    (zchars[3 * i + 1] << 5) |
	    (zchars[3 * i + 2]);

    encoded[resolution - 1] |= 0x8000;

}/* encode_text */

/*
 * A00096, test if a unicode character can be read and printed.
 *
 * 	zargs[0] = Unicode
 *
 */

void A00096 (void)
{
    zword c = zargs[0] & 0xffff;

    if (c >= 0x20 && c <= 0x7e)
	store (3);
    else if (c == 0xa0)
	store (1);
    else if (c >= 0xa1 && c <= 0xff)
	store (3);
    else
	store (0);

}/* A00096 */

/*
 * A00101, encode a ZSCII string for use in a dictionary.
 *
 *	zargs[0] = address of text buffer
 *	zargs[1] = length of ASCII string
 *	zargs[2] = offset of ASCII string within the text buffer
 *	zargs[3] = address to store encoded text in
 *
 * This is a V5+ opcode and therefore the dictionary resolution must be
 * three 16bit words.
 *
 */

void A00101 (void)
{
    int i;

    load_string ((zword) ( (zargs[0] + zargs[2]) & 0xffff), \
                 zargs[1] & 0xffff);

    encode_text (0x05);

    for (i = 0; i < 3; i++)
	A00195 ((zword) (zargs[3] + 2 * i), encoded[i]);

}/* A00101 */

/*
 * decode_text
 *
 * Convert encoded text to Unicode. The encoded text consists of 16bit
 * words. Every word holds 3 Z-characters (5 bits each) plus a spare
 * bit to mark the last word. The Z-characters translate to ZSCII by
 * looking at the current current character set. Some select another
 * character set, others refer to abbreviations.
 *
 * There are several different string types:
 *
 *    LOW_STRING - from the lower 64KB (byte address)
 *    ABBREVIATION - from the abbreviations table (word address)
 *    HIGH_STRING - from the end of the memory map (packed address)
 *    EMBEDDED_STRING - from the instruction stream (at PC)
 *    VOCABULARY - from the dictionary (byte address)
 *
 * The last type is only used for word completion.
 *
 */

#define outchar(c)	if (st==VOCABULARY) *ptr++=c; else A00186(c)

static void decode_text (enum string_type st, zword addr)
{
    zchar *ptr;
    long byte_addr;
    zchar c2;
    zword code;
    zbyte c, prev_c = 0;
    int shift_state = 0;
    int shift_lock = 0;
    int status = 0;

    /* Calculate the byte address if necessary */

    if (st == ABBREVIATION)

	byte_addr = (long) addr << 1;

    else if (st == HIGH_STRING) {

	if (A00025 <= V3)
	    byte_addr = (long) addr << 1;
	else if (A00025 <= V5)
	    byte_addr = (long) addr << 2;
	else if (A00025 <= V7)
	    byte_addr = ((long) addr << 2) + ((long) A00048 << 3);
	else /* A00025 == V8 */
	    byte_addr = (long) addr << 3;

	if (byte_addr >= A00064)
	    A00192 ("Print at illegal address");

    }

    /* Loop until a 16bit word has the highest bit set */

    if (st == VOCABULARY)
	ptr = decoded;

    do {

	int i;

	/* Fetch the next 16bit word */

	if (st == LOW_STRING || st == VOCABULARY) {
	    code=lw(addr);
	    addr += 2;
	} else if (st == HIGH_STRING || st == ABBREVIATION) {
	    code = hw(byte_addr);
	    byte_addr += 2;
	} else
            code=cw();

	/* Read its three Z-characters */

	for (i = 10; i >= 0; i -= 5) {

	    zword abbr_addr;
	    zword ptr_addr;

	    c = (code >> i) & 0x1f;

	    switch (status) {

	    case 0:	/* normal operation */

		if (shift_state == 2 && c == 6)
		    status = 2;

		else if (A00025 == V1 && c == 1)
		    A00185 ();

		else if (A00025 >= V2 && shift_state == 2 && c == 7)
		    A00185 ();

		else if (c >= 6)
		    outchar (alphabet (shift_state, c - 6));

		else if (c == 0)
		    outchar (' ');

		else if (A00025 >= V2 && c == 1)
		    status = 1;

		else if (A00025 >= V3 && c <= 3)
		    status = 1;

		else {

		    shift_state = (shift_lock + (c & 1) + 1) % 3;

		    if (A00025 <= V2 && c >= 4)
			shift_lock = shift_state;

		    break;

		}

		shift_state = shift_lock;

		break;

	    case 1:	/* abbreviation */

		ptr_addr = A00036 + 64 * (prev_c - 1) + 2 * c;

		abbr_addr=lw(ptr_addr);
		decode_text (ABBREVIATION, abbr_addr);

		status = 0;
		break;

	    case 2:	/* ZSCII character - first part */

		status = 3;
		break;

	    case 3:	/* ZSCII character - second part */

		c2 = A00182 ((prev_c << 5) | c);
		outchar (c2);

		status = 0;
		break;

	    }

	    prev_c = c;

	}

    } while (!(code & 0x8000));

    if (st == VOCABULARY)
	*ptr = 0;

}/* decode_text */

#undef outchar

/*
 * A00125, print a new line.
 *
 * 	no zargs used
 *
 */

void A00125 (void)
{

    A00185 ();

}/* A00125 */

/*
 * A00131, print a string embedded in the instruction stream.
 *
 *	no zargs used
 *
 */

void A00131 (void)
{

    decode_text (EMBEDDED_STRING, 0);

}/* A00131 */

/*
 * A00132, print a string from the lower 64KB.
 *
 *	zargs[0] = address of string to print
 *
 */

void A00132 (void)
{

    decode_text (LOW_STRING, zargs[0]);

}/* A00132 */

/*
 * A00133 print a single ZSCII character.
 *
 *	zargs[0] = ZSCII character to be printed
 *
 */

void A00133 (void)
{

    A00186 (A00182 (zargs[0]));

}/* A00133 */

/*
 * A00134, print a formatted table.
 *
 *	zargs[0] = address of formatted table to be printed
 *
 */

void A00134 (void)
{
    zword count;
    zword addr = zargs[0];

    bool first = TRUE;

    for (;;) {

	count=lw(addr);
	addr += 2;

	if (count == 0)
	    break;

	if (!first)
	    A00185 ();

	while (count--) {

	    zbyte c;

	    LOW_BYTE (addr, c)
	    addr++;

	    A00186 (A00182 (c));

	}

	first = FALSE;

    }

}/* A00134 */

/*
 * A00187
 *
 * Print a signed 16bit number.
 *
 */

void A00187 (zword value)
{
    int i;

    /* Print sign */

    if ((short) value < 0) {
	A00186 ('-');
	value = - (short) value;
    }

    /* Print absolute value */

    for (i = 10000; i != 0; i /= 10)
	if (value >= i || i == 1)
	    A00186 ('0' + (value / i) % 10);

}/* A00187 */

/*
 * A00135, print a signed number.
 *
 * 	zargs[0] = number to print
 *
 */

void A00135 (void)
{

    A00187 (zargs[0]);

}/* A00135 */

/*
 * A00188
 *
 * Print an object description.
 *
 */

void A00188 (zword object)
{
    zword addr = A00268 (object);
    zword code = 0x94a5;
    zbyte length;

    LOW_BYTE (addr, length)
    addr++;

    if (length != 0)
	code=lw(addr);

    if (code == 0x94a5) { 	/* encoded text 0x94a5 == empty string */

	A00189 ("object#");	/* supply a generic name */
	A00187 (object);		/* for anonymous objects */

    } else decode_text (LOW_STRING, addr);

}/* A00188 */

/*
 * A00136, print an object description.
 *
 * 	zargs[0] = number of object to be printed
 *
 */

void A00136 (void)
{

    A00188 (zargs[0] & 0xffff);

}/* A00136 */

/*
 * A00137, print the string at the given packed address.
 *
 * 	zargs[0] = packed address of string to be printed
 *
 */

void A00137 (void)
{

    decode_text (HIGH_STRING, zargs[0] & 0xffff);

}/* A00137 */

/*
 * A00138, print the string at PC, print newline then return true.
 *
 * 	no zargs used
 *
 */

void A00138 (void)
{

    decode_text (EMBEDDED_STRING, 0);
    A00185 ();
    ret (1);

}/* A00138 */

/*
 * A00189
 *
 * Print a string of ASCII characters.
 *
 */

void A00189 (const char *s)
{
    char c;

    while ((c = *s++) != 0)

	if (c == '\n')
	    A00185 ();
	else
	    A00186 (c);

}/* A00189 */

/*
 * A00140
 *
 * 	zargs[0] = Unicode
 *
 */

void A00140 (void)
{

    A00186 ((zargs[0] <= 0xff) ? zargs[0] : '?');

}/* A00140 */

/*
 * lookup_text
 *
 * Scan a dictionary searching for the given word. The first argument
 * can be
 *
 * 0x00 - find the first word which is >= the given one
 * 0x05 - find the word which exactly matches the given one
 * 0x1f - find the last word which is <= the given one
 *
 * The return value is 0 if the search fails.
 *
 */

static zword lookup_text (int padding, zword dct)
{
    zword entry_addr;
    zword entry_count;
    zword entry;
    zword addr;
    zbyte entry_len;
    zbyte sep_count;
    int resolution = (A00025 <= V3) ? 2 : 3;
    int entry_number;
    int lower, upper;
    int i;
    bool sorted;

    encode_text (padding);

    LOW_BYTE (dct, sep_count)		/* skip word separators */
    dct += 1 + sep_count;
    LOW_BYTE (dct, entry_len)		/* get length of entries */
    dct += 1;
    entry_count = lw(dct);		/* get number of entries */
    dct += 2;

    if ((short) entry_count < 0) {	/* bad luck, entries aren't sorted */

	entry_count = - (short) entry_count;
	sorted = FALSE;

    } else sorted = TRUE;		/* entries are sorted */

    lower = 0;
    upper = entry_count - 1;

    while (lower <= upper) {

	if (sorted)                             /* binary search */
	    entry_number = (lower + upper) / 2;
	else                                    /* linear search */
	    entry_number = lower;

	entry_addr = dct + entry_number * entry_len;

	/* Compare word to dictionary entry */

	addr = entry_addr;

	for (i = 0; i < resolution; i++) {
	    entry=lw(addr);
	    if (encoded[i] != entry)
		goto continuing;
	    addr += 2;
	}

	return entry_addr;		/* exact match found, return now */

    continuing:

	if (sorted)				/* binary search */

	    if (encoded[i] > entry)
		lower = entry_number + 1;
	    else
		upper = entry_number - 1;

	else lower++;                           /* linear search */

    }

    /* No exact match has been found */

    if (padding == 0x05)
	return 0;

    entry_number = (padding == 0x00) ? lower : upper;

    if (entry_number == -1 || entry_number == entry_count)
	return 0;

    return dct + entry_number * entry_len;

}/* lookup_text */

/*
 * tokenise_text
 *
 * Translate a single word to a token and append it to the token
 * buffer. Every token consists of the address of the dictionary
 * entry, the length of the word and the offset of the word from
 * the start of the text buffer. Unknown words cause empty slots
 * if the flag is set (such that the text can be scanned several
 * times with different dictionaries); otherwise they are zero.
 *
 */

static void tokenise_text (zword text, zword length, zword from, zword parse, zword dct, bool flag)
{
    zword addr;
    zbyte token_max, token_count;

    LOW_BYTE (parse, token_max)
    parse++;
    LOW_BYTE (parse, token_count)

    if (token_count < token_max) {	/* sufficient space left for token? */

	A00194 (parse++, token_count + 1);

	load_string ((zword) (text + from), length);

	addr = lookup_text (0x05, dct);

	if (addr != 0 || !flag) {

	    parse += 4 * token_count;

	    A00195 ((zword) (parse + 0), addr);
	    A00194 ((zword) (parse + 2), length);
	    A00194 ((zword) (parse + 3), from);

	}

    }

}/* tokenise_text */

/*
 * A00234
 *
 * Split an input line into words and translate the words to tokens.
 *
 */

void A00234 (zword text, zword token, zword dct, bool flag)
{
    zword addr1;
    zword addr2;
    zbyte length;
    zbyte c;

    /* Use standard dictionary if the given dictionary is zero */

    if (dct == 0)
	dct = A00030;

    /* Remove all tokens before inserting new ones */

    A00194 ((zword) (token + 1), 0);

    /* Move the first pointer across the text buffer searching for the
       beginning of a word. If this succeeds, store the position in a
       second pointer. Move the first pointer searching for the end of
       the word. When it is found, "tokenise" the word. Continue until
       the end of the buffer is reached. */

    addr1 = text;
    addr2 = 0;

    if (A00025 >= V5) {
	addr1++;
	LOW_BYTE (addr1, length)
    }

    do {

	zword sep_addr;
	zbyte sep_count;
	zbyte separator;

	/* Fetch next ZSCII character */

	addr1++;

	if (A00025 >= V5 && addr1 == text + 2 + length)
	    c = 0;
	else
	    LOW_BYTE (addr1, c)

	/* Check for separator */

	sep_addr = dct;

	LOW_BYTE (sep_addr, sep_count)
	sep_addr++;

	do {

	    LOW_BYTE (sep_addr, separator)
	    sep_addr++;

	} while (c != separator && --sep_count != 0);

	/* This could be the start or the end of a word */

	if (sep_count == 0 && c != ' ' && c != 0) {

	    if (addr2 == 0)
		addr2 = addr1;

	} else if (addr2 != 0) {

	    tokenise_text (
		text,
		(zword) (addr1 - addr2),
		(zword) (addr2 - text),
		token, dct, flag );

	    addr2 = 0;

	}

	/* Translate separator (which is a word in its own right) */

	if (sep_count != 0)

	    tokenise_text (
		text,
		(zword) (1),
		(zword) (addr1 - text),
		token, dct, flag );

    } while (c != 0);

}/* A00234 */

/*
 * A00178, make a lexical analysis of a ZSCII string.
 *
 *	zargs[0] = address of string to analyze
 *	zargs[1] = address of token buffer
 *	zargs[2] = address of dictionary (optional)
 *	zargs[3] = set when unknown words cause empty slots (optional)
 *
 */

void A00178 (void)
{

    /* Supply default arguments */

    if (zargc < 3)
	zargs[2] = 0;
    if (zargc < 4)
	zargs[3] = 0;

    /* Call A00234 to do the real work */

    A00234 (zargs[0], zargs[1], zargs[2], zargs[3] != 0);

}/* A00178 */

/*
 * completion
 *
 * Scan the vocabulary to complete the last word on the input line
 * (similar to "tcsh" under Unix). The return value is
 *
 *    2 ==> completion is impossible
 *    1 ==> completion is ambiguous
 *    0 ==> completion is successful
 *
 * The function also returns a string in its second argument. In case
 * of 2, the string is empty; in case of 1, the string is the longest
 * extension of the last word on the input line that is common to all
 * possible completions (for instance, if the last word on the input
 * is "fo" and its only possible completions are "follow" and "folly"
 * then the string is "ll"); in case of 0, the string is an extension
 * to the last word that results in the only possible completion.
 *
 */

int completion (const zchar *buffer, zchar *result)
{
    zword minaddr;
    zword maxaddr;
    zchar *ptr;
    zchar c;
    int len;
    int i;

    *result = 0;

    /* Copy last word to "decoded" string */

    len = 0;

    while ((c = *buffer++) != 0)

	if (c != ' ') {

	    if (len < 9)
		decoded[len++] = c;

	} else len = 0;

    decoded[len] = 0;

    /* Search the dictionary for first and last possible extensions */

    minaddr = lookup_text (0x00, A00030);
    maxaddr = lookup_text (0x1f, A00030);

    if (minaddr == 0 || maxaddr == 0 || minaddr > maxaddr)
	return 2;

    /* Copy first extension to "result" string */

    decode_text (VOCABULARY, minaddr);

    ptr = result;

    for (i = len; (c = decoded[i]) != 0; i++)
	*ptr++ = c;
    *ptr = 0;

    /* Merge second extension with "result" string */

    decode_text (VOCABULARY, maxaddr);

    for (i = len, ptr = result; (c = decoded[i]) != 0; i++, ptr++)
	if (*ptr != c) break;
    *ptr = 0;

    /* Search was ambiguous or successful */

    return (minaddr == maxaddr) ? 0 : 1;

}/* completion */
