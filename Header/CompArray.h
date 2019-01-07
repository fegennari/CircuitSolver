#ifndef _COMPARRAY_H_
#define _COMPARRAY_H_

#include "Draw.h"

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, CompArray.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Component Arrays (names, etc.)
// By Frank Gennari
string const ff_full_name[num_ffs+1] = {" Flip Flop", " RS Latch", " RS Flip Flop", 
			 " D Flip Flop", " Transparent D Flip Flop", " JK Flip Flop", 
			 " JK Master/Slave Flip Flop", " Toggle Flip Flop"};
			 
unsigned char const add_window_id[num_choices+2] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 
			 11, 12, 13, 14, 43, 44, 45, 46, 47, 48, 36, 37, 38, 39, 15, 16, 17, 18, 
			 40, 21, 20, 19, 28, 42, 50, 27, 30, 31, 32, 22, 23, 24, 25, 29, 26, 49}; // comp id's
			 
unsigned char const draw_comp_in_add[num_choices+2] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
			 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 
			 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1}; // draw in add component window

char const has_val[MAX_NEW_USED_ID+1] = {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
			 2, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 2, 
			 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 1, 1}; // = no value, 1 = has value, 2 = has gate delay only

char const conn_class[MAX_NEW_USED_ID+1] = {0, 1, 1, 4, 4, 4, 4, 4, 4,
		  	 9, 9, 9, 9, 9, 9, 10, 5, 0, 6, 3, 3, 3, 7, 8, 7, 7, 0, 2, 3, 7, 2, 2,
		  	 11, 11, 11, 11, 12, 12, 13, 14, 6, 6, 15, 16, 16, 16, 16, 16, 16, 17, 3}; // for get_connections()
			 
string const comp_name[MAX_NEW_USED_ID+1] = {"Exit", "BUFFER", "INVERTER", 
			 "AND Gate", "NAND Gate", "OR Gate", "NOR Gate", "XOR Gate", "XNOR Gate", 
			 "3-Input AND Gate", "3-Input NAND Gate", "3-Input OR Gate", "3-Input NOR Gate", 
			 "3-Input XOR Gate", "3-Input XNOR Gate",
			 "Flip Flop", "OP Amp", "IC", "Transistor", "Capacitor", "Resistor",
			 "Diode", "Input", "Output", "Power", "Ground", "Wire", "Battery", "Inductor",
			 "Initial State", "Voltage Source", "Current Source",
			 "Voltage Controlled Voltage Source", "Current Controlled Voltage Source",
			 "Voltage Controlled Current Source", "Current Controlled Current Source",
			 "Tri-State Buffer", "Tri-State Inverter", "2:1 Mux", "Full Adder",
			 "NFET", "PFET", "Transformer",
			 "4-Input AND Gate", "4-Input NAND Gate", "4-Input OR Gate", "4-Input NOR Gate", 
			 "4-Input XOR Gate", "4-Input XNOR Gate", "Delay", "Transmission Line"};
		 
string const comp_label[MAX_NEW_USED_ID+1] = {" ", "G", "G", "G", "G", "G", "G", 
			 "G", "G", "G", "G", "G", "G", "G", "G", "FF", "OA", "U", "Q", "C", "R",
			 "D", " ", " ", " ", " ", " ", "B", "L", " ", "V", "I", "VCVS", "CCVS",
			 "VCCS", "CCCS", "G", "G", "MUX", "AD", "M", "M", "T", "G", "G", "G", 
			 "G", "G", "G", "GD", "TL"};

string const cunits[MAX_NEW_USED_ID+1] = {" ", " ", " ", " ", " ", " ", " ", " ", 
			 " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " ", "  F", 
			 "  Ohm", "  V", " ", " ", " ", " ", " ", "  V", "  H", " ", "  V", "  A", 
			 "  ", "  Ohm", "  S", "  ", " ", " ", " ", " ", "  W/L", "  W/L", "  H",
			 " ", " ", " ", " ", " ", " ", " ", " "};
			 
static draw_func const draw_comp[MAX_NEW_USED_ID+1] = {draw_NO, draw_BUFFER, draw_NOT, 
			 draw_AND, draw_NAND, draw_OR, draw_NOR, draw_XOR, draw_XNOR, 
			 draw_3AND, draw_3NAND, draw_3OR, draw_3NOR, draw_3XOR, draw_3XNOR,
			 draw_NO, draw_OPAMP, draw_IC, draw_NPN, draw_CAPACITOR, draw_RESISTOR,
			 draw_DIODE, draw_INPUT, draw_OUTPUT, draw_POWER, draw_GROUND, draw_CUSTOMWIRE, 
			 draw_BATTERY, draw_INDUCTOR, draw_STATE_SELECTION, draw_V_SOURCE, draw_I_SOURCE,
			 draw_VCV_SOURCE, draw_CCV_SOURCE, draw_VCC_SOURCE, draw_CCC_SOURCE, draw_TRI_STATE_BUFFER, 
			 draw_TRI_STATE_INVERTER, draw_MUX, draw_ADDER, draw_NFET, draw_PFET, draw_XFMR,
			 draw_4AND, draw_4NAND, draw_4OR, draw_4NOR, draw_4XOR, draw_4XNOR, draw_DELAY, draw_TLINE};
			 
static draw_func const draw_comp_rot[MAX_NEW_USED_ID+1] = {draw_NO, draw_BUFFER, draw_NOT, 
			 draw_AND, draw_NAND, draw_OR, draw_NOR, draw_XOR, draw_XNOR, 
			 draw_3AND, draw_3NAND, draw_3OR, draw_3NOR, draw_3XOR, draw_3XNOR,
			 draw_NO, draw_OPAMP, draw_IC, draw_NPN, draw_CAPACITOR2, draw_RESISTOR2,
			 draw_DIODE2, draw_INPUT, draw_OUTPUT, draw_POWER, draw_GROUND, draw_CUSTOMWIRE, 
			 draw_BATTERY2, draw_INDUCTOR2, draw_STATE_SELECTION, draw_V_SOURCE2, draw_I_SOURCE2,
			 draw_VCV_SOURCE2, draw_CCV_SOURCE2, draw_VCC_SOURCE2, draw_CCC_SOURCE2, draw_TRI_STATE_BUFFER, 
			 draw_TRI_STATE_INVERTER, draw_MUX, draw_ADDER, draw_NFET, draw_PFET, draw_XFMR2,
			 draw_4AND, draw_4NAND, draw_4OR, draw_4NOR, draw_4XOR, draw_4XNOR, draw_DELAY2, draw_TLINE2};



#endif

