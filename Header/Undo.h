#ifndef _UNDO_H_
#define _UNDO_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Undo.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Undo Header
// By Frank Gennari
#include "Logic.h"


enum {EMPTYx = 0, ADDx, DELETEx, MOVEx, ROTATEx, MODIFYVx, MODIFYWx};


int const default_stack_size = 128;
int const max_stack_size = 100000;


struct actionlist {
	
	char multiple, action, flags;
	int ID, location;
	double Value, old_location;
};


// external functions
void add_action(int action, int ID, int location, int old_location, char flags, double Value, char multiple);
component *undo_action(component *head);
component *redo_action(component *head);
void reset_actions();
int is_action_stack_empty();
int is_undo_stack_empty();

// internal functions
void shift_stack(actionlist *stack, int top, int stack_size);
void double_size(actionlist *&stack, int &top, int &stack_size);
component *find_comp(component *head, int ID, int position, int &is_head);
void ur_action_transform(int size_factor, int x_old, int x_new);
void stack_transform(actionlist *stack, int top, int size_factor, int x_old, int x_new);



#endif
