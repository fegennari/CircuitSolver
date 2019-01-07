#ifndef _XWINDOWS_INTERFACE_H_
#define _XWINDOWS_INTERFACE_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, XWindowsInterface.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// XWindows Circuit Solver/X Windows system header file
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
//#include <Xatom.h>  // ???
//#include <Xproto.h> // ???

#define xXWIN
#define CAN_SCROLL
#define CommandLine
#define USE_CC
//#define HAS_MS_TIMER
//#define TEXT_ONLY
//#define NEED_SWAP

int const window_nonclient_width = 9;
int const window_nonclient_height = 27;
int const MENU_X_OFFSET = 48;

int const menuwidth = 20;
int const Console_X = 660;
int const Console_Y = 360;

string const button_names[15] = {"Simulate", "Add", "Delete", "Open", "Save", "Clear", "Redraw", "Preferences", "Cancel", "Rotate", "Info", "Modify", "Undo", "Redo", "Exit"};
int const bn_lengths[15] = {8, 3, 6, 4, 4, 5, 6, 11, 6, 6, 4, 6, 4, 4, 4};

double const DELAY2 = 417;
int const DELAY = 89600;
long const time_constant = 52;



// Cursors
enum {DELETE_CURSOR, WHAT_CURSOR, EVAL_CURSOR, PROBEDC_CURSOR, PROBEAC_CURSOR, MODIFY_CURSOR, TRACE_CURSOR, GROUND_CURSOR, BDELETE_CURSOR};



// Wrappers for Mac->X Windows port
void MoveTo(int x, int y);
void LineTo(int x, int y);



// Global X Windows function prototypes (some are Mac/Win32 also)
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
void set_DW_title(char *title);
void init_progress_bar(int mode);
void update_progress_bar(int complete, int mode);
int update_timer(int init_time, int last_time);
void show_clock(); 
void show_timer();
void Window_setup();
void whiteout();
void wait(double time);
int get_time();
int read_specs();
int clock_microprocessor();
void beep();



#endif