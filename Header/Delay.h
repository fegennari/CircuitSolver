#ifndef _DELAY_H_
#define _DELAY_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Delay.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Delay header
// By Frank Gennari
#define NO_DELAY -1

int const NUM_GD_TYPES = 13;


int const comp_fanin[MAX_NEW_USED_ID+1] = {0, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 
			 3, 3, 1, 2, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			 2, 2, 3, 3, 1, 1, 0, 4, 4, 4, 4, 4, 4, 1};
			 
int const gate_delay_types[NUM_GD_TYPES] = {BUFFER_ID, NOT_ID, AND_ID, NAND_ID, OR_ID, 
			 NOR_ID, XOR_ID, XNOR_ID, T_ST_BUF_ID, T_ST_INV_ID, MUX_ID, ADDER_ID, FF_ID};

int const gate_delay_mapping[FF_max_id+1] = {NO_DELAY, BUFFER_ID, NOT_ID, AND_ID, NAND_ID, 
			 OR_ID, NOR_ID, XOR_ID, XNOR_ID, AND_ID, NAND_ID, OR_ID, NOR_ID, XOR_ID, XNOR_ID,
			 NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, 
			 NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, 
			 NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, T_ST_BUF_ID, T_ST_INV_ID, 
			 MUX_ID, ADDER_ID, NO_DELAY, NO_DELAY, NO_DELAY, AND_ID, NAND_ID, OR_ID, NOR_ID, 
			 XOR_ID, XNOR_ID, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, 
			 NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY, NO_DELAY,
			 FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID,
			 FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID,
			 FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID,
			 FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID, FF_ID};
			 
			
#endif
