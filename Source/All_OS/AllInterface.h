#ifndef _ALL_INTERFACE_H_
#define _ALL_INTERFACE_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, AllInterface.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// OS Independent system header file - no graphics

#define USE_VM
#define TEXT_ONLY
#define NEED_SWAP
#define USE_CC
//#define HAS_MS_TIMER

int const window_nonclient_width = 0;
int const window_nonclient_height = 0;
int const MENU_X_OFFSET = 0;

int const menuwidth = 0;
int const Console_X = 660;
int const Console_Y = 360;

string const button_names[15] = {""};
int const bn_lengths[15] = {0};

double const DELAY2 = 417.18;
int const DELAY = 89600;
long const time_constant = 52;


// Cursors, not actually used
enum {DELETE_CURSOR, WHAT_CURSOR, EVAL_CURSOR, PROBEDC_CURSOR, PROBEAC_CURSOR, MODIFY_CURSOR, TRACE_CURSOR, GROUND_CURSOR, BDELETE_CURSOR};


// Wrappers for Mac->Windows port
void MoveTo(int x, int y);
void LineTo(int x, int y);



// Define Mac point for cross platform compatibility
struct Point {
	int h, v;
};


// comment out if already defined
#define min(A, B) ((A < B) ? A : B)
#define max(A, B) ((A < B) ? B : A)


// Global Win32 function prototypes (some are Mac also)
void runtest();
void exit_error();
void intro();
int prefs();
void draw_menubar();
void delete_menubar();
int memory_error();
void memory_message();
void get_cursors();
void set_cursor(int ID);
void set_special_cursor(int type);
void reset_cursor();
int interrupt();
void get_continue();
int get_window_click();
Point GetDWClick();
void wait_for_click();
void draw_text_on_window(string text);
void set_DW_title(const char *title);
void init_progress_bar(int mode);
void update_progress_bar(int complete, int mode);
int update_timer(int init_time, int last_time);
void show_clock(); 
void show_timer();
void Window_setup(int is_graphing);
void whiteout();
void wait(double time);
int get_time();
int read_specs();
int clock_microprocessor();
void beep();



#endif