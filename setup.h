/*
 * Various status thingies for the interpreter and interface.
 *
 */

typedef struct frotz_setup_struct {
	int attribute_assignment;
	int attribute_testing;
	int context_lines;
	int object_locating;
	int object_movement;
	int left_margin;
	int right_margin;
	int ignore_errors;
	int interpreter_number;
	int piracy;
	int undo_slots;
	int expand_abbreviations;
	int script_cols;
	int sound;
	int err_report_mode;

	char *story_file;
        char *story_name;
        char *story_base;
        char *script_name;
        char *command_name;
        char *save_name;
        char *tmp_save_name;
        char *aux_name;
        char *story_path;
        char *zcode_path;
	char *restricted_path;

	bool restore_mode; /* for a save file passed from command line*/
	bool use_blorb;
	bool exec_in_blorb;
} f_setup_t;

extern f_setup_t f_setup;

/*** Story file header data ***/
/*
typedef struct zcode_header_struct {
	zbyte version;
	zbyte config;
	zword release;
	zword resident_size;
	zword start_pc;
	zword dictionary;
	zword objects;
	zword globals;
	zword dynamic_size;
	zword flags;
	zbyte serial[6];
	zword abbreviations;
	zword file_size;
	zword checksum;
	zbyte interpreter_number;
	zbyte interpreter_version;
	zbyte screen_rows;
	zbyte screen_cols;
	zword screen_width;
	zword screen_height;
	zbyte font_height;
	zbyte font_width;
	zword functions_offset;
	zword strings_offset;
	zbyte default_background;
	zbyte default_foreground;
	zword terminating_keys;
	zword line_width;
	zbyte standard_high;
	zbyte standard_low;
	zword alphabet;
	zword extension_table;
	zbyte user_name[8];

	zword hx_table_size;
	zword hx_mouse_x;
	zword hx_mouse_y;
	zword hx_unicode_table;
} z_header_t;
extern z_header_t z_header;
*/
