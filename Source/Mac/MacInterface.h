#ifndef _MAC_INTERFACE_H_
#define _MAC_INTERFACE_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, MacInterface.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Mac only interface header (resources)
// By Frank Gennari
#include <sioux.h>

#define xMAC_OS
#define CAN_SCROLL
//#define HAS_MS_TIMER
//#define TEXT_ONLY
//#define NEED_SWAP
//#define USE_CC

#define base_resource_id 127

double const DELAY2 = 11.17;
long const time_constant = 29600;
int const DELAY = 22400;


// Cursors
enum {DELETE_CURSOR, WHAT_CURSOR, EVAL_CURSOR, PROBEDC_CURSOR, PROBEAC_CURSOR, MODIFY_CURSOR, TRACE_CURSOR, GROUND_CURSOR, BDELETE_CURSOR};

int const base_res_id = 128;
int const base_s_cursor_id = 133;
int const init_cursor_id = 136;
int const max_cursor_id = 178;
int const cursor_offset = 100;
int const AC_cursor_frame0_id = 500;
int const DC_cursor_frame0_id = 600;
int const second_base_cursor_id = 1000;


// Global Mac function prototypes
void runtest();
void intro();
int prefs();
void clear_buffer();
void draw_menubar();
void delete_menubar();
int memory_error();
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
int get_event();
void wait(double time);
int get_time();
int read_specs();
int clock_microprocessor();
void beep();



#endif