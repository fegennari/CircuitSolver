#ifndef _NETLIST_H_
#define _NETLIST_H_



#include "Logic.h"
#include "GridArray.h"
#include "StringTable.h"
#include "Equation.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Netlist.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Netlist definitions
// By Frank Gennari
int const NUMBER_LENGTH = 16;
int const nof           = 0;
int const GROUND_LOC    = 952;
int const MAX_LENGTH    = 5;
int const MAX_WIDTH     = 5;

// write netlist flags
#define SEPARATE_IO  0x01
#define APPEND_MODE  0x02
#define EQUIV_CHECK  0x04

// read netlist flags
#define HAS_INPUTS   0x01
#define HAS_OUTPUTS  0x02
#define HAS_ICS      0x04
#define NEEDS_MODELS 0x08
#define USE_AC_SRC   0x10

// error flags
#define TABLE_EMPTY  0x01
#define NO_MATCH     0x02
#define REPEAT_MATCH 0x04
#define BAD_VALUES   0x08

// net types
#define INPUT_TYPE   0x01
#define OUTPUT_TYPE  0x02


int const num_nl_id_types = 16;
int const nl = num_nl_id_types;

string const ground_string = "0";

char const nl_comp_id[MAX_NEW_USED_ID+1] = {nl, nl, nl, nl, nl, nl, nl, nl, nl,
	      nl, nl, nl, nl, nl, nl, nl, 12, 13, 10, 0, 1, 2, nl, nl, nl, nl, nl,
	      4, 3, nl, 4, 5, 6, 8, 7, 9, nl, nl, nl, nl, 11, 11, nl, nl, nl, nl,
	      nl, nl, nl, nl, 15};
	      
string const comp_symbol[MAX_NEW_USED_ID+1] = {"NONE", "BUF", "NOT",
			 "AND", "NAND", "OR", "NOR", "XOR", "XNOR",
			 "AND", "NAND", "OR", "NOR", "XOR", "XNOR",
			 "FF", "OPAMP", "IC", "Q", "C", "R", "D", "INPUT", "OUTPUT",
			 "VDD", "GND", "WIRE", "V", "L", "STATE", "V", "I",
			 //"FVCVS", "FCCVS", "FVCCS", "FCCCS",
			 "E", "H", "G", "F",
			 "TBUF", "TINV", "MUX", "ADDER", "M", "M", "XFMR",
			 "AND", "NAND", "OR", "NOR", "XOR", "XNOR", "DELAY", "T"};
			 
// for bounding box in place and route			 
int const comp_lengths[MAX_NEW_USED_ID+1] = {0, 5, 5, 5, 5, 5, 5,
			 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 2, 2, 1, 1, 0, 5,
			 5, 1, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 5, 5, 5, 5, 5, 5, 3, 5};
			 
int const comp_widths[MAX_NEW_USED_ID+1] = {0, 3, 3, 3, 3, 3, 3,
			 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 3, 3, 3, 1, 1, 2, 2, 0, 3,
			 3, 1, 3, 3, 3, 3, 3, 3, 4, 4, 4, 5, 3, 3, 5, 4, 4, 4, 4, 4, 4, 1, 3};

// 0 = none, 1 = 90, 2 = 180		 
int const comp_rotation[MAX_NEW_USED_ID+1] = {0, 2, 2, 2, 2, 2, 2,
			 2, 2, 2, 2, 2, 2, 2, 2, 0, 2, 0, 2, 1, 1, 1, 2, 2, 2, 2, 0, 1,
			 1, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1};
			 
			 
struct ICNet {

	int netnum;
	ICNet *next;
};


struct comp_nets {

	connections nets;
	comp_nets *next;
};


struct ICNet_list {

	ICNet *nets;
	ICNet_list *next;
};
			 

			 
#endif
