// Visualization.h
// Viz - Design Place and Route/Visualization Package Header
// Written by Frank Gennari 11/24/00

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Analog Analysis.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.
#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_



#include "CircuitSolver.h"
#include "Logic.h"
#include "Draw.h"
#include "StringTable.h"


#define NO_OBJECT  -1
#define NOWHERE    -1
#define LARGE_COST -1
#define NODE_WIRE  -1
#define DEF_TYPE   0
#define DEF_PLACE  0


enum {xNOOP = 0, xOPEN, xSAVE, xREDRAW, xGRID, xBLANK, xEXIT, xCLICK}; // buttons


// colors
int const VIZ_GRID_COLOR = AQUA;
int const WIRE_COLOR     = BLACK;
int const TEXT_COLOR     = BLACK;
int const DEFAULT_COLOR  = BLACK;

// sizes
int const VIZ_NUM_BOXES      = 5000;
int const VIZ_MAX_OBJECTS    = 10000;
int const VIZ_MAX_B_CHAR_LEN = 9;
int const VIZ_NUM_BUTTONS    = 6;
int const VIZ_MAX_LEGEND     = 10;
int const VIZ_NUM_SETTINGS   = 11;
int const VIZ_MAX_BITS       = 4096;

// button definitions
string const VIZ_BUTTON_NAMES1[VIZ_NUM_BUTTONS] = {" Open", " ", " Redraw", " Grid Off", " ", " Exit"};
int const VIZ_BUTTON_LENGTHS1[VIZ_NUM_BUTTONS]  = {5, 1, 7, 9, 1, 5};

// secondary button functions
string const VIZ_BUTTON_NAMES2[VIZ_NUM_BUTTONS] = {" Open", " Save", " Redraw", " Grid On", " ", " Cancel"};
int const VIZ_BUTTON_LENGTHS2[VIZ_NUM_BUTTONS]  = {5, 5, 7, 8, 1, 7};


string const color_names[num_colors+1] = {"default", "red", "green", "blue", 
			 "yellow", "pink", "purple", "brown", "aqua", "white", "black", 
			 "dkgrey", "ltblue", "almond", "ltgrey", "tan", "dkgreen", "orange"};

string const viz_settings[VIZ_NUM_SETTINGS] = {"name_on_top", "min_block_x", 
			 "min_block_y", "init_block_spacing", "min_block_spacing", 
			 "pick_first_placement", "trade_quality_for_speed", 
			 "ask_to_save", "do_fill", "draw_bit_slashes", "center_pull"};


// viz default settings
int const DEF_name_on_top = 0; // else name inside of block
int const DEF_min_block_x = 2;
int const DEF_min_block_y = 2;
int const DEF_init_block_spacing = 6;
int const DEF_min_block_spacing = 2;
int const DEF_pick_first_placement = 0; // alot faster
int const DEF_trade_quality_for_speed = 0;
int const DEF_ask_to_save = 0;
int const DEF_show_legend = 0;
int const DEF_do_fill     = 1;
int const DEF_draw_bit_slashes = 1;
double const DEF_center_pull = 0.1;

double const arrow_width  = 0.5;
double const arrow_length = 0.55;
double const arrow_offset = 0.25; 



struct vizWire {

	int start, end, s_arrow, e_arrow, source, dest;
	int bits, num_points, point_cap, dest_match_wire_id;
	char location_set; 
	int *other_points;	
};


struct vizObject {
	
	int location, ports, xsize, ysize, bit_width, color, is_filled, next_wire;
	string name, type;
	vizWire *wire;
};


struct vizLegendEntry {
	
	char fill;
	int color;
	string text;
};






#endif