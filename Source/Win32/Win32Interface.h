#ifndef _WIN32_INTERFACE_H_
#define _WIN32_INTERFACE_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Win32Interface.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Win32 Circuit Solver/Windows system header file
#include <windows.h>
#include <windowsx.h>

#define xWIN32
#define USE_VM
#define HAS_MS_TIMER
#define USE_SLEEP
//#define TEXT_ONLY
//#define WindowsGUI

// need these for cygwin
//#define USE_CC
//#define NEED_SWAP

int const window_nonclient_width = 9;
int const window_nonclient_height = 27;
int const MENU_X_OFFSET = 48;

int const menuwidth = 20;
int const Console_X = 660;
int const Console_Y = 360;

string const button_names[15] = {"Simulate", "Add", "Delete", "Open", "Save", "New/Clear", "Redraw", "Preferences", "Cancel", "Rotate", "Info", "Modify", "Undo", "Redo", "Exit"};
int const bn_lengths[15] = {8, 3, 6, 4, 4, 9, 6, 11, 6, 6, 4, 6, 4, 4, 4};

double const DELAY2 = 417.18;
int const DELAY = 89600;
long const time_constant = 52;



// Mac stuff from Windows port - must remove later

// Cursors
enum {DELETE_CURSOR, WHAT_CURSOR, EVAL_CURSOR, PROBEDC_CURSOR, PROBEAC_CURSOR, MODIFY_CURSOR, TRACE_CURSOR, GROUND_CURSOR, BDELETE_CURSOR};

//int const base_s_cursor_id = 133;
//int const init_cursor_id = 136;
//int const max_cursor_id = 178;
//int const cursor_offset = 100;
//int const AC_cursor_frame0_id = 500;
//int const DC_cursor_frame0_id = 600;


// Win32 stuff for included "windows.h"


// Wrappers for Mac->Windows port
void MoveTo(int x, int y);
void LineTo(int x, int y);




// Define Mac point for cross platform compatibility
struct Point {
	int h, v;
};


// Global Win32 function prototypes (some are Mac also)
void runtest();
void exit_error();
void intro();
void demo_message();
int prefs();
void draw_menubar();
void delete_menubar();
int memory_error();
void memory_message();
void get_cursors();
void set_cursor(int ID, int is_rotated);
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
