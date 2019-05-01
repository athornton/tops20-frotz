/*
 * frotz.h
 *
 * Global declarations and definitions
 *
 */

typedef int bool;

#define TRUE 1
#define FALSE 0

typedef unsigned char zbyte;
typedef unsigned short zword;

enum story {
    BEYOND_ZORK,
    SHERLOCK,
    ZORK_ZERO,
    SHOGUN,
    ARTHUR,
    JOURNEY,
    LURKING_HORROR,
    UNKNOWN
};

typedef unsigned char zchar;

/*** Constants that may be set at compile time ***/

#ifndef MAX_UNDO_SLOTS
#define MAX_UNDO_SLOTS 25
#endif
#ifndef MAX_FILE_NAME
#define MAX_FILE_NAME 80
#endif
#ifndef TEXT_BUFFER_SIZE
#define TEXT_BUFFER_SIZE 200
#endif
#ifndef INPUT_BUFFER_SIZE
#define INPUT_BUFFER_SIZE 200
#endif
#ifndef STACK_SIZE
#define STACK_SIZE 1024
#endif

#ifndef DEFAULT_SAVE_NAME
#define DEFAULT_SAVE_NAME "story.sav"
#endif
#ifndef DEFAULT_SCRIPT_NAME
#define DEFAULT_SCRIPT_NAME "story.scr"
#endif
#ifndef DEFAULT_COMMAND_NAME
#define DEFAULT_COMMAND_NAME "story.rec"
#endif
#ifndef DEFAULT_AUXILARY_NAME
#define DEFAULT_AUXILARY_NAME "story.aux"
#endif

/*** Story file header format ***/

#define H_VERSION 0
#define H_CONFIG 1
#define H_RELEASE 2
#define H_RESIDENT_SIZE 4
#define H_START_PC 6
#define H_DICTIONARY 8
#define H_OBJECTS 10
#define H_GLOBALS 12
#define H_DYNAMIC_SIZE 14
#define H_FLAGS 16
#define H_SERIAL 18
#define H_ABBREVIATIONS 24
#define H_FILE_SIZE 26
#define H_CHECKSUM 28
#define H_INTERPRETER_NUMBER 30
#define H_INTERPRETER_VERSION 31
#define H_SCREEN_ROWS 32
#define H_SCREEN_COLS 33
#define H_SCREEN_WIDTH 34
#define H_SCREEN_HEIGHT 36
#define H_FONT_HEIGHT 38 /* this is the font width in V5 */
#define H_FONT_WIDTH 39 /* this is the font height in V5 */
#define H_FUNCTIONS_OFFSET 40
#define H_STRINGS_OFFSET 42
#define H_DEFAULT_BACKGROUND 44
#define H_DEFAULT_FOREGROUND 45
#define H_TERMINATING_KEYS 46
#define H_LINE_WIDTH 48
#define H_STANDARD_HIGH 50
#define H_STANDARD_LOW 51
#define H_ALPHABET 52
#define H_EXTENSION_TABLE 54
#define H_USER_NAME 56

#define HX_TABLE_SIZE 0
#define HX_MOUSE_X 1
#define HX_MOUSE_Y 2
#define HX_UNICODE_TABLE 3

/*** Various Z-machine constants ***/

#define V1 1
#define V2 2
#define V3 3
#define V4 4
#define V5 5
#define V6 6
#define V7 7
#define V8 8

#define CONFIG_BYTE_SWAPPED 0x01 /* Story file is byte swapped         - V3  */
#define CONFIG_TIME         0x02 /* Status line displays time          - V3  */
#define CONFIG_TWODISKS     0x04 /* Story file occupied two disks      - V3  */
#define CONFIG_TANDY        0x08 /* Tandy licensed game                - V3  */
#define CONFIG_NOSTATUSLINE 0x10 /* Interpr can't support status lines - V3  */
#define CONFIG_SPLITSCREEN  0x20 /* Interpr supports split screen mode - V3  */
#define CONFIG_PROPORTIONAL 0x40 /* Interpr uses proportional font     - V3  */

#define CONFIG_COLOUR       0x01 /* Interpr supports colour            - V5+ */
#define CONFIG_PICTURES	    0x02 /* Interpr supports pictures	       - V6  */
#define CONFIG_BOLDFACE     0x04 /* Interpr supports boldface style    - V4+ */
#define CONFIG_EMPHASIS     0x08 /* Interpr supports emphasis style    - V4+ */
#define CONFIG_FIXED        0x10 /* Interpr supports fixed width style - V4+ */
#define CONFIG_TIMEDINPUT   0x80 /* Interpr supports timed input       - V4+ */

#define SCRIPTING_FLAG	  0x0001 /* Outputting to transscription file  - V1+ */
#define FIXED_FONT_FLAG   0x0002 /* Use fixed width font               - V3+ */
#define REFRESH_FLAG 	  0x0004 /* Refresh the screen                 - V6  */
#define GRAPHICS_FLAG	  0x0008 /* Game wants to use graphics         - V5+ */
#define OLD_SOUND_FLAG	  0x0010 /* Game wants to use sound effects    - V3  */
#define UNDO_FLAG	  0x0010 /* Game wants to use UNDO feature     - V5+ */
#define MOUSE_FLAG	  0x0020 /* Game wants to use a mouse          - V5+ */
#define COLOUR_FLAG	  0x0040 /* Game wants to use colours          - V5+ */
#define SOUND_FLAG	  0x0080 /* Game wants to use sound effects    - V5+ */
#define MENU_FLAG	  0x0100 /* Game wants to use menus            - V6  */

#define INTERP_DEC_20 1
#define INTERP_APPLE_IIE 2
#define INTERP_MACINTOSH 3
#define INTERP_AMIGA 4
#define INTERP_ATARI_ST 5
#define INTERP_MSDOS 6
#define INTERP_CBM_128 7
#define INTERP_CBM_64 8
#define INTERP_APPLE_IIC 9
#define INTERP_APPLE_IIGS 10
#define INTERP_TANDY 11

#define BLACK_COLOUR 2
#define RED_COLOUR 3
#define GREEN_COLOUR 4
#define YELLOW_COLOUR 5
#define BLUE_COLOUR 6
#define MAGENTA_COLOUR 7
#define CYAN_COLOUR 8
#define WHITE_COLOUR 9
#define GREY_COLOUR 10		/* INTERP_MSDOS only */
#define LIGHTGREY_COLOUR 10 	/* INTERP_AMIGA only */
#define MEDIUMGREY_COLOUR 11 	/* INTERP_AMIGA only */
#define DARKGREY_COLOUR 12 	/* INTERP_AMIGA only */

#define REVERSE_STYLE 1
#define BOLDFACE_STYLE 2
#define EMPHASIS_STYLE 4
#define FIXED_WIDTH_STYLE 8

#define TEXT_FONT 1
#define PICTURE_FONT 2
#define GRAPHICS_FONT 3
#define FIXED_WIDTH_FONT 4

/*** Constants for os_restart_game */

#define RESTART_BEGIN 0
#define RESTART_WPROP_SET 1
#define RESTART_END 2

/*** Character codes ***/

#define ZC_TIME_OUT 0x00
#define ZC_NEW_STYLE 0x01
#define ZC_NEW_FONT 0x02
#define ZC_BACKSPACE 0x08
#define ZC_INDENT 0x09
#define ZC_GAP 0x0b
#define ZC_RETURN 0x0d
#define ZC_HKEY_MIN 0x0e
#define ZC_HKEY_RECORD 0x0e
#define ZC_HKEY_PLAYBACK 0x0f
#define ZC_HKEY_SEED 0x10
#define ZC_HKEY_UNDO 0x11
#define ZC_HKEY_RESTART 0x12
#define ZC_HKEY_QUIT 0x13
#define ZC_HKEY_DEBUG 0x14
#define ZC_HKEY_HELP 0x15
#define ZC_HKEY_MAX 0x15
#define ZC_ESCAPE 0x1b
#define ZC_ASCII_MIN 0x20
#define ZC_ASCII_MAX 0x7e
#define ZC_BAD 0x7f
#define ZC_ARROW_MIN 0x81
#define ZC_ARROW_UP 0x81
#define ZC_ARROW_DOWN 0x82
#define ZC_ARROW_LEFT 0x83
#define ZC_ARROW_RIGHT 0x84
#define ZC_ARROW_MAX 0x84
#define ZC_FKEY_MIN 0x85
#define ZC_FKEY_MAX 0x90
#define ZC_NUMPAD_MIN 0x91
#define ZC_NUMPAD_MAX 0x9a
#define ZC_SINGLE_CLICK 0x9b
#define ZC_DOUBLE_CLICK 0x9c
#define ZC_MENU_CLICK 0x9d
#define ZC_LATIN1_MIN 0xa0
#define ZC_LATIN1_MAX 0xff

/*** File types ***/

#define FILE_RESTORE 0
#define FILE_SAVE 1
#define FILE_SCRIPT 2
#define FILE_PLAYBACK 3
#define FILE_RECORD 4
#define FILE_LOAD_AUX 5
#define FILE_SAVE_AUX 6

/*** Data access macros ***/

#define SET_BYTE(addr,v)  { zmp[addr] = v; }
#define LOW_BYTE(addr,v)  { v = zmp[addr]; }
#define CODE_BYTE(v)	  { v = *pcp++;    }

extern zbyte *pcp;
extern zbyte *zmp;

#define lo(v)	(v & 0xff)
#define hi(v)	((v & 0xff00) >> 8)

#define SET_WORD(addr,v)  { zmp[addr] = hi(v); zmp[addr+1] = lo(v); }
#define LOW_WORD(addr,v)  { v = ((zword) zmp[addr] << 8) | zmp[addr+1]; }
#define HIGH_WORD(addr,v) { v = ((zword) zmp[addr] << 8) | zmp[addr+1]; }
/***
#define CODE_WORD(v)      { v = ((zword) pcp[0] << 8) | pcp[1]; pcp += 2; }
#define GET_PC(v)         { v = pcp - zmp; }
#define SET_PC(v)         { pcp = zmp + v; }
***/

/*** Non-macro versions ***/
extern zword cw(void);
extern long g_pc(void);
extern void s_pc(long v);

/*** Story file header data ***/

extern zbyte A00025;
extern zbyte A00026;
extern zword A00027;
extern zword A00028;
extern zword A00029;
extern zword A00030;
extern zword A00031;
extern zword A00032;
extern zword A00033;
extern zword A00034;
extern zbyte A00035[6];
extern zword A00036;
extern zword A00037;
extern zword A00038;
extern zbyte A00039;
extern zbyte A00040;
extern zbyte A00041;
extern zbyte A00042;
extern zword A00043;
extern zword A00044;
extern zbyte A00045;
extern zbyte A00046;
extern zword A00047;
extern zword A00048;
extern zbyte A00049;
extern zbyte A00050;
extern zword A00051;
extern zword A00052;
extern zbyte A00053;
extern zbyte A00054;
extern zword A00055;
extern zword A00056;
extern zbyte A00057[8];

extern zword A00058;
extern zword A00059;
extern zword A00060;
extern zword A00061;

/*** Various data ***/

extern const char *A00062;

extern enum story A00063;
extern long A00064;

extern zword stack[STACK_SIZE];
extern zword *sp;
extern zword *fp;

extern zword zargs[8];
extern zargc;

extern bool A00065;
extern bool A00066;
extern bool A00067;
extern bool A00068;
extern bool A00069;
extern bool A00070;

extern cwin;
extern mwin;

extern A00071;
extern A00072;

extern bool A00073;
extern bool A00074;
extern bool A00075;
extern bool A00076;

extern A00077;
extern A00078;
extern A00079;
extern A00080;
extern A00081;
extern A00082;
extern A00083;
extern A00084;
extern A00085;
extern A00086;
extern A00087;
extern A00088;

extern long A00089;

/*** Z-machine opcodes ***/

void 	z_add (void);
void 	z_and (void);
void 	A00090 (void);
void 	A00091 (void);
void 	A00092 (void);
void 	A00093 (void);
void 	A00094 (void);
void 	A00095 (void);
void	A00096 (void);
void 	A00097 (void);
void 	A00098 (void);
void 	z_dec (void);
void 	A00099 (void);
void 	z_div (void);
void 	A00100 (void);
void 	A00101 (void);
void 	A00102 (void);
void 	A00103 (void);
void 	A00104 (void);
void 	A00105 (void);
void 	A00106 (void);
void 	A00107 (void);
void 	A00108 (void);
void 	A00109 (void);
void 	A00110 (void);
void 	A00111 (void);
void 	A00112 (void);
void 	A00113 (void);
void 	z_inc (void);
void 	A00114 (void);
void 	A00115 (void);
void 	A00116 (void);
void 	z_je (void);
void 	z_jg (void);
void 	z_jin (void);
void 	z_jl (void);
void 	A00117 (void);
void 	z_jz (void);
void 	A00118 (void);
void 	A00119 (void);
void 	A00120 (void);
void 	A00121 (void);
void 	A00122 (void);
void 	z_mod (void);
void 	A00123 (void);
void 	A00124 (void);
void 	z_mul (void);
void 	A00125 (void);
void 	z_nop (void);
void 	z_not (void);
void 	z_or (void);
void 	A00126 (void);
void 	A00127 (void);
void 	A00128 (void);
void 	A00129 (void);
void 	z_pop (void);
void 	A00130 (void);
void 	A00131 (void);
void 	A00132 (void);
void 	A00133 (void);
void 	A00134 (void);
void 	A00135 (void);
void 	A00136 (void);
void 	A00137 (void);
void 	A00138 (void);
void 	A00139 (void);
void	A00140 (void);
void 	A00141 (void);
void 	A00142 (void);
void 	A00143 (void);
void 	A00144 (void);
void 	A00145 (void);
void 	A00146 (void);
void 	A00147 (void);
void 	A00148 (void);
void 	A00149 (void);
void 	A00150 (void);
void 	A00151 (void);
void 	A00152 (void);
void 	A00153 (void);
void 	A00154 (void);
void 	z_ret (void);
void 	A00155 (void);
void 	A00156 (void);
void 	A00157 (void);
void 	A00158 (void);
void 	A00159 (void);
void 	A00160 (void);
void 	A00161 (void);
void 	A00162 (void);
void 	A00163 (void);
void 	A00164 (void);
void 	A00165 (void);
void 	A00166 (void);
void 	A00167 (void);
void 	A00168 (void);
void 	A00169 (void);
void 	A00170 (void);
void 	A00171 (void);
void 	A00172 (void);
void 	A00173 (void);
void 	A00174 (void);
void 	z_sub (void);
void 	A00175 (void);
void 	A00176 (void);
void 	A00177 (void);
void 	A00178 (void);
void 	A00179 (void);
void 	A00180 (void);
void 	A00181 (void);

/*** Various global functions ***/

zchar	A00182 (zbyte);
zbyte	A00183 (zchar);

void 	A00184 (void);
void	A00185 (void);
void	A00186 (zchar);
void	A00187 (zword);
void	A00188 (zword);
void 	A00189 (const char *);

void 	A00190 (void);
void 	A00191 (void);

void	A00192 (const char *);

void	ret (zword);
void 	store (zword);
void 	A00193 (bool);

void	A00194 (zword, zbyte);
void	A00195 (zword, zword);

/*** Interface functions ***/

void 	A00196 (int);
int  	A00197 (zchar);
void 	A00198 (zchar);
void 	A00199 (const zchar *);
void 	A00200 (int, int, int);
void 	A00201 (int, int, int, int);
void 	A00202 (const char *);
void 	A00203 (void);
int  	A00204 (int, int *, int *);
void 	A00205 (void);
void 	A00206 (void);
int  	A00207 (void);
int  	A00208 (int, int *, int *);
void 	A00209 (int);
void 	A00210 (int, char *[]);
int	A00211 (void);
int  	A00212 (char *, const char *, int);
zchar	A00213 (int, int);
zchar	A00214 (int, zchar *, int, int, int);
void 	A00215 (void);
void 	A00216 (int);
void 	A00217 (int, int, int, int, int);
void 	A00218 (int, int);
void 	A00219 (int, int);
void 	A00220 (int);
void 	A00221 (int);
void 	A00222 (int, int, int);
void 	A00223 (void);
int  	A00224 (const zchar *);
