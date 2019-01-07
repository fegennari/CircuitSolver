#ifndef _CIRCUIT_SOLVER_H_
#define _CIRCUIT_SOLVER_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, CircuitSolver.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Header
// By Frank Gennari
#include <cmath>
#include <fstream>

// necessary for MS Visual C++
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <algorithm>


#ifdef CS_SET_NAMESPACE
using namespace std; // need for MS Visual C++
#endif

// Other important Circuit Solver headers

// Only include one

#ifdef INCLUDE_MAC_OS
#include "MacOSInterface.h"
#else
#ifdef INCLUDE_WIN32
#include "Win32Interface.h"
#else
#ifdef INCLUDE_XWINDOWS
#include "XWindowsInterface.h"
#else
#ifdef INCLUDE_COMMON
#include "AllInterface.h"
#endif
#endif
#endif
#endif


#include "Graphics.h"
#include "ComponentID.h"


// OS conflicting character sets
#ifdef xWIN32
	#define MU_CHAR  (unsigned char)230 // Windows OEM µ
	#define OHM_CHAR (unsigned char)234
#else
#ifdef xMAC_OS
	#define MU_CHAR  'µ'
	#define OHM_CHAR '½'
#else
	#define MU_CHAR  'µ'
	#define OHM_CHAR "Ohm"
#endif
#endif



// Universal constants
#define pi				3.14159265359
#define e 				2.71828183
#define speed_of_light	2.99792458e8

#define CANCEL 8

// Type encodings
#define flipflopx	 4
#define transistorx	 5
#define icx			 6
#define wirex 		 9

#define atan_2arg    get_angle

#define MAX_INPUT_LETTER 26


// Lots of constants
double const two_pi       = 2*pi;
double const pi_over_4    = pi/4.0;
double const pi_over_180  = pi/180;
double const pi_under_180 = 180/pi;

double const default_voltage = 5.0;
double const TTL_supply = 5.0;
double const cmos_supply = 9.0;
double const diode_sat = 1.0e-14;
double const diode_vdrop = .7;
double const minimum = 1.0e-12;
double const no_voltage = 0.0;
double const default_tolerance = 0.0;
double const source_max = 1e6;
double const source_min = -1e6;
double const Rmax = 1e12;
double const Rmin = 1e-6;
double const Lmax = 1e6;
double const Lmin = 1e-12;
double const Cmax = 1e3;
double const Cmin = 1e-15;
double const CS_LIMIT = 1e9;
double const v_error = 1.0e-6;
double const A = 1.0e5;
double const UnknownV = 1.0e-20;
double const bound = 1.0e20;
double const max_domain = 1e20;
double const almost_infinite = 1e99;
double const almost_zero = 1e-99;
double const window_ratio = 3.0292;
double const default_vbe = 0.7;
double const default_vcesat = 0.2;
double const VcesatExtra = 0.1;
int const MAX_FUNCTIONS = 29;
int const MAX_CHARACTERS = 255;
int const MAX_ENTERED_CHARACTERS = 1024;
int const MAX_INPUT_CHARACTERS = 64;
int const MAX_SAVE_CHARACTERS = 255;//32; // must allow file path
int const MAX_BOXES = 12;
int const MAX_LEVEL = 1000;
int const num_menu_boxes = 14;
int const num_menu_slections = 30;
int const MENU_WIDTH = 64;
int const MENU_HEIGHT = 293; // Actually 286, but the Exit button is small
int const MENU_OFFSET = num_menu_boxes + 1;
int const subcircuit_flag = 20;
int const max_trs = Q_max_id - Q_base_id - 1;
int const max_ics = IC_max_id - IC_base_id - 1;
int const max_squares = Wire_max_id - Wire_base_id - 1;
int const default_num_buttons = 1007;
int const max_boxes = 1008; // Must be greater than default_num_buttons or at least the max subcircuit window size.
int const max_main_circuit_windows = 10000;
int const max_ic_windows = 1;
int const reserved_pos_value = -200000001;
int const max_subcircuits = 1024;
int const default_max_total_boxes = max_boxes*(max_subcircuits + 1);
int const max_drawing_grid = max_boxes;
int const max_grid_domain = 1000;
int const x_window = 832;
int const y_window = 274;
int const search_value = 10;
int const AC_mod_num = 50;
int const DC_mod_num = 250;
int const Graph_mod_num = 100;
int const MAX_POINTS = 20000;
int const MAX_DATA_SIZE = MAX_POINTS + 100;
int const default_points = 20000;
int const DEFAULT_NUM_PINS = 14;
int const max_pins = 64;
int const max_fit_pins = 18;
int const num_choices = 46;
int const num_menu_selections = 46;
int const max_matrix_size = 250;
int const def_win_title_len = 18;
int const pause_time = 2;
int const ascii_case_offset = 32;
int const key_val = 185396;
int const default_demo_windows = 40;
int const default_demo_nets = 200;
int const cancel_button_length = 48;
int const cancel_button_width = 20;


// Datafile names and path
char const cs_machine_specs[MAX_SAVE_CHARACTERS]    = "C.S.MachineSpecs";
char const transistor_datafile[MAX_SAVE_CHARACTERS] = "C.S.ComponentData";
char const ic_datafile[MAX_SAVE_CHARACTERS]         = "C.S.ICData";
char const data_folder[MAX_SAVE_CHARACTERS]         = "CircuitSolver.Data/";
char const circuit_folder[MAX_SAVE_CHARACTERS]      = "Circuits/";
char const init_window_title[def_win_title_len]     = "Circuit Solver - ";


// Prime numbers for hashtable
unsigned int const NUM_PRIMES = 20;

const unsigned long PRIMES[] =
{
  53,         97,         193,       389,       769,
  1543,       3079,       6151,      12289,     24593,
  49157,      98317,      196613,    393241,    786433,
  1572869,    3145739,    6291469,   12582917,  25165843,
  50331653,   100663319,  201326611, 402653189, 805306457
};

// Structs
struct point {
	double x, y;
};


struct transistor {
	string name;
	char z;
	double q_icmax, q_cbmax, q_vcemax, q_vbemax, q_beta, q_pdmax, q_tf, q_vbe, q_vcesat;
	transistor *next;
};


struct IC {
	string name, description;
	int pins;
	IC *next;
};


struct box {
	int v_boxes, h_boxes;
};


struct connections {
	int in1, in2, in3, out1, out2, out3;
};


struct complex {
	double Im, Re;
};




// Global function prototypes
int valid_number(double number);
void out(double value);
void write_units(ofstream &outfile, double value);
char get_units_and_scale(double value, double &scaled);
char *xin(int size);
int in();
double ind();
char inc();
unsigned int inp();
unsigned int inpg0();
char *double_to_char(double number);
void string_to_char(char *output, string input, int num_chars);
int comp_char(char A, char B);
int compare_array(const void *A, const void *B);
int get_num_exp(double &number);
int infile_file(ifstream &infile, const char *ifname);
int outfile_file(ofstream &outfile, const char *ifname, int app);
int overwrite_file(const char *name);
int filestream_check(ifstream &filestream);
int filestream_check(ofstream &filestream);
int decision();
int xdecision(int id);
int which_box(Point click);
int which_box(point click);
int which_box_g(point click);
void set_buttons(int boxes, int grid_color, int is_schematic_window);
void init_menu(int number, int color, int is_schematic_window);
void draw_LINE();
int shift_and_remove_zeros(char *display, char *display2);
char *convert_to_constparam255(const char *display);
void print_ch(unsigned char *ch);
void print_err_ch(unsigned char *ch);
int sign(int num);
void delay(double time);
void randome(long &s1ptr, long &s2ptr, double &ranptr);
void read_data();
void check_datafiles(int &found_comp_data, int &found_ic_data);
void out_of_memory();
void demo_error();
void internal_error();
int box_clicked();

int got_cancel_click();

void reset_timer(int is_ms);
void start_timer();
void stop_timer();
int  elapsed_time();
void show_elapsed_time();
void format_time(int time, char *time_string);
void print_date_and_time();

int transform_DW_point(int position, int new_x, int old_x);
int scale_DW_point(int position, int scale_factor);
int is_vertical (int position1, int position2);
int is_horizontal (int position1, int position2);
int is_diagonal (int position1, int position2);

void draw_string(const char *display);
void draw_string2(const char *display);
void draw_string2(const unsigned char *display);




// Template prototypes
template<class T> int roundx(T num);
template<class T> T add_spaces(T S, int size);
template<class T> T remove_spaces(T S, int size);
template<class T> T *memAlloc(T *data, int size);
template<class T> T *memAlloc_init(T *data, int size, T value);
template<class T> void destroyL(T *list);
template<class T> void get_print_plural(T val);
template<class T> T *double_array_size(T *arr, unsigned int &capacity);



// Template functions (must be included in header file)
template<class T> int roundx(T num) {
		
	int num2 = (int)num;
	
	if (fabs(num - num2) >= .5) {
		if (num >= 0)
			return (int)ceil(num);
		else
			return (int)floor(num);
	}	
	if (num >= 0)
		return (int)floor(num);
		
	return (int)ceil(num);
}



template<class T> T add_spaces(T S, int size) {

	int count(0);
	
	while (count < size && S[count] != '\0') {
		if (S[count] == '_')
			S[count] = ' ';
			
			++count;
	}
	return S;
}




template<class T> T remove_spaces(T S, int size) {

	int count(0);
	
	while (count < size && S[count] != '\0') {
		if (S[count] == ' ')
			S[count] = '_';
			
			++count;
	}
	return S;
}




template<class T> T *memAlloc(T *data, int size) {

	if (size <= 0)
		return NULL;

	data = new T[size];
	
	if (data == NULL)
		out_of_memory();
		
	return data;
}




template<class T> T *memAlloc_init(T *data, int size, T value) {

	data = memAlloc(data, size);
	
	if (data == NULL || size <= 0)
		return NULL;
	
	if (/*value != (T)NULL && */(value == 0 || sizeof(T) == 1)) {
		memset(data, (int)value, size*sizeof(T));
	}
	else {
		for (int i = 0; i < size; ++i)
			data[i] = value;
	}
	return data;
}




template<class T> void destroyL(T *list) {
	
	T *temp;
	
	while (list != 0) {
		temp = list->next;
		delete list;
		list = temp;
	}
}




template<class T> int countL(T *list) {
	
	int lcount;
		
	for (lcount = 0; list != 0; ++lcount)
		list = list->next;
		
	return lcount;
}




template<class T> void get_print_plural(T val) {

	if (val > 1) {
		cout << "s";
	}
}




template<class T> T *double_array_size(T *arr, unsigned int &capacity) {

	unsigned int  i, old_cap = capacity;
	T            *arr2 = NULL;
	
	if (capacity <= 0)
		capacity = 1;
	else
		capacity *= 2;	
		
	arr2 = memAlloc(arr2, capacity);
	
	for (i = 0; i < old_cap; ++i) {
		arr2[i] = arr[i];
	}
	for (; i < capacity; ++i) {
		arr2[i] = NULL;
	}
	delete [] arr;
	
	return arr2;
}



// MAY NOT NEED
#ifdef NEED_SWAP

#include <vector>

/*template<class T> void swap(T &A, T &B);

template<class T> void swap(T &A, T &B) {

	T temp = A;
	A      = B;
	B      = temp;
}*/
#endif


#endif


