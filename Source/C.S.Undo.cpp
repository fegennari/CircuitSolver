#include "CircuitSolver.h"
#include "Undo.h"

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Undo.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Undo
// By Frank Gennari
int const conserve_RAM = 0;

int action_top, undo_top, action_stack_size, undo_stack_size;

actionlist *action_stack;
actionlist *undo_stack;


extern int needs_redraw, changed, changed2, changed3, changed4, skip_placement_check;
extern double supply_voltage, ground;


int point_transform(int location, int size_factor, int x_old, int x_new);




void add_action(int action, int ID, int location, int old_location, char flags, double Value, char multiple) {
	
	if (action_top < 0)
		action_top = 0;
	
	action_stack[action_top].action       = (char)action;
	action_stack[action_top].ID           = ID;
	action_stack[action_top].location     = location;
	action_stack[action_top].old_location = old_location;
	action_stack[action_top].Value        = Value;
	action_stack[action_top].multiple     = multiple;
	action_stack[action_top].flags        = flags;
	
	if (action_top < action_stack_size) // push(action_stack)
		++action_top;
	else if (conserve_RAM)
		shift_stack(action_stack, action_top, action_stack_size);
	else 
		double_size(action_stack, action_top, action_stack_size);		
}





component *undo_action(component *head) {
	
	component *comp, *temp;
	int last_action, top_less_one, is_head, undo_again, error, ID;
	
	do {
		undo_again = is_head = error = 0;
		top_less_one = action_top - 1;
		
		if (top_less_one < 0) {
			cout << "\nThere is nothing to undo." << endl;
			return head;
		}
		last_action = action_stack[top_less_one].action;
			
		switch (last_action) {
		
			case EMPTYx:
				cout << "\nThere is nothing to undo." << endl;
				return head;
				
			case ADDx:
				comp = find_comp(head, action_stack[top_less_one].ID, action_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (is_head) {
						action_stack[top_less_one].old_location = head->value;
						head = comp->next;
						delete comp;
					}			
					else {
						action_stack[top_less_one].old_location = comp->next->value;
						temp = comp->next->next;
						delete comp->next;
						comp->next = temp;
					}
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if (top_less_one > 0 && get_order(action_stack[top_less_one-1].ID) == wirex && action_stack[top_less_one-1].multiple == 1 && action_stack[top_less_one-1].Value == 1) 
					undo_again = 1;
					
				if (action_stack[top_less_one].multiple == 1) // ???
					undo_again = 1;
				break;
				
			case DELETEx:
				skip_placement_check = 1;
				ID   = action_stack[top_less_one].ID;
				head = draw_individual_component(head, ID, action_stack[top_less_one].location, 0, action_stack[top_less_one].flags);
				if (action_stack[top_less_one].flags & ROTATED) {
					ID = -ID;
				}
				head = update_circuit(ID, action_stack[top_less_one].location, head, 2, action_stack[top_less_one].Value);
				if (action_stack[top_less_one].multiple == 1) {
					undo_again = 1;
				}
				skip_placement_check = 0;
				mark_all_changed();
				break;
				
			case MOVEx:		
				comp = find_comp(head, action_stack[top_less_one].ID, action_stack[top_less_one].location, is_head);			
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					comp->location = (int)action_stack[top_less_one].old_location;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if ((action_stack[top_less_one].multiple == 1 && !(get_order(action_stack[top_less_one].ID) == wirex && action_stack[top_less_one].Value == 1)) || (top_less_one > 0 && get_order(action_stack[top_less_one-1].ID) == wirex && action_stack[top_less_one-1].multiple == 1 && action_stack[top_less_one-1].Value == 1)) 
					undo_again = 1;
				break;
				
			case ROTATEx: // can't have multiple
				comp = find_comp(head, action_stack[top_less_one].ID, action_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head) {
						comp = comp->next;
					}
					comp->flags ^= ROTATED;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				break;
				
			case MODIFYVx:
				comp = find_comp(head, action_stack[top_less_one].ID, action_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					if (comp->id == GROUND_ID) {
						needs_redraw = 0;
						ground = action_stack[top_less_one].Value;
						swap(comp->value, action_stack[top_less_one].Value);
					}
					else if (comp->id == POWER_ID) {
						supply_voltage = action_stack[top_less_one].Value;
						swap(comp->value, action_stack[top_less_one].Value);
					}
					else if (comp->id == ST_SEL_ID) {
						comp->flags ^= ROTATED;
						action_stack[top_less_one].flags ^= ROTATED;
					}
					else
						swap(comp->value, action_stack[top_less_one].Value);

					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				break;
				
			case MODIFYWx:
				comp = find_comp(head, action_stack[top_less_one].ID, action_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					comp->id = (int)action_stack[top_less_one].old_location;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if (action_stack[top_less_one].multiple == 1) 
					undo_again = 1;
				break;
				
			default:
				internal_error();
				cerr << "\nError: Undefined operation: Undo" << endl;
				return head;
		}
		if (undo_top < 0)
			undo_top = 0;
		
		if (!undo_again)
			cout << "\nUndo" << endl;
		
		if (!error) {
			undo_stack[undo_top] = action_stack[top_less_one];
		
			if (undo_top < undo_stack_size) // push(undo_stack)
				++undo_top;
			else if (conserve_RAM)
				shift_stack(undo_stack, undo_top, undo_stack_size);
			else 
				double_size(undo_stack, undo_top, undo_stack_size);
		}	
		action_stack[top_less_one].action = EMPTYx; // pop(action_stack)
		--action_top;
	
	} while (undo_again);
	
	return head;
}





component *redo_action(component *head) {
	
	component *comp, *temp;
	int last_undo, top_less_one, is_head, redo_again, error, ID;
	
	do {
		redo_again = is_head = error = 0;
		top_less_one = undo_top - 1;
		
		if (top_less_one < 0) {
			cout << "\nThere is nothing to redo." << endl;
			return head;
		}
		last_undo = undo_stack[top_less_one].action;
		
		switch (last_undo) {
		
			case EMPTYx:
				cout << "\nThere is nothing to redo." << endl;
				return head;
				
			case ADDx:	
				skip_placement_check = 1;
				ID   = undo_stack[top_less_one].ID;
				head = draw_individual_component(head, ID, undo_stack[top_less_one].location, 0, undo_stack[top_less_one].flags);
				if (undo_stack[top_less_one].flags & ROTATED) {
					ID = -ID;
				}
				head = update_circuit(ID, undo_stack[top_less_one].location, head, 2, undo_stack[top_less_one].old_location);
				if (get_order(undo_stack[top_less_one].ID) == wirex && undo_stack[top_less_one].multiple == 1 && undo_stack[top_less_one].Value == 1) 
					redo_again = 1;
					
				if (top_less_one > 0 && undo_stack[top_less_one-1].multiple == 1) // ???
					redo_again = 1;
				
				mark_all_changed();
				skip_placement_check = 0;	
				break;
				
			case DELETEx:
				comp = find_comp(head, undo_stack[top_less_one].ID, undo_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (is_head) {
						head = comp->next;
						delete comp;
					}			
					else {
						temp = comp->next->next;
						delete comp->next;
						comp->next = temp;
					}
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if (top_less_one > 0 && undo_stack[top_less_one-1].multiple == 1) 
					redo_again = 1;
				break;
				
			case MOVEx:
				comp = find_comp(head, undo_stack[top_less_one].ID, (int)undo_stack[top_less_one].old_location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					comp->location = undo_stack[top_less_one].location;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if ((top_less_one > 0 && undo_stack[top_less_one-1].multiple == 1 && !(get_order(undo_stack[top_less_one-1].ID) == wirex && undo_stack[top_less_one-1].Value == 1)) || (get_order(undo_stack[top_less_one].ID) == wirex && undo_stack[top_less_one].multiple == 1 && undo_stack[top_less_one].Value == 1)) 
					redo_again = 1;
				break;
				
			case ROTATEx: // can't have multiple
				comp = find_comp(head, -undo_stack[top_less_one].ID, undo_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					comp->flags ^= ROTATED;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				break;
				
			case MODIFYVx:
				comp = find_comp(head, undo_stack[top_less_one].ID, undo_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					if (comp->id == GROUND_ID) {
						needs_redraw = 0;
						ground = undo_stack[top_less_one].Value;
						swap(comp->value, undo_stack[top_less_one].Value);
					}
					else if (comp->id == POWER_ID) {
						supply_voltage = undo_stack[top_less_one].Value;
						swap(comp->value, undo_stack[top_less_one].Value);
					}
					else if (comp->id == ST_SEL_ID) {
						comp->flags ^= ROTATED;
						undo_stack[top_less_one].flags ^= ROTATED;
					}
					else
						swap(comp->value, undo_stack[top_less_one].Value);

					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				break;
				
			case MODIFYWx:
				comp = find_comp(head, (int)undo_stack[top_less_one].old_location, undo_stack[top_less_one].location, is_head);
				if (comp != 0) {
					if (!is_head)
						comp = comp->next;
					comp->id = undo_stack[top_less_one].ID;
					mark_all_changed();
					needs_redraw = 1;
				}
				else
					error = 1;
				if (top_less_one > 0 && undo_stack[top_less_one-1].multiple == 1) 
					redo_again = 1;
				break;
				
			default:
				internal_error();
				cerr << "\nError: Undefined operation: Redo" << endl;
				return head;
		}
		if (action_top < 0)
			action_top = 0;
		
		if (!redo_again)
			cout << "\nRedo" << endl;
		
		if (!error) {	
			action_stack[action_top] = undo_stack[top_less_one];
		
			if (action_top < action_stack_size) // push(action_stack)
				++action_top;
			else if (conserve_RAM)
				shift_stack(action_stack, action_top, action_stack_size);
			else 
				double_size(action_stack, action_top, action_stack_size);
		}
		undo_stack[top_less_one].action = EMPTYx; // pop(undo_stack)
		--undo_top;
	
	} while (redo_again);
		
	return head;
}





void reset_actions() {
	
	unsigned int i;
	
	action_stack_size = undo_stack_size = default_stack_size;
	
	delete [] action_stack;
	delete [] undo_stack;
	
	action_stack = memAlloc(action_stack, default_stack_size+1);
	undo_stack   = memAlloc(undo_stack,   default_stack_size+1);

	for (i = 0; (int)i <= action_stack_size; ++i)
		action_stack[i].action = EMPTYx;
		
	for (i = 0; (int)i <= undo_stack_size; ++i)
		undo_stack[i].action = EMPTYx;
		
	action_top = undo_top = 0;
}




int is_action_stack_empty() {

	return (action_top == 0);
}




int is_undo_stack_empty() {

	return (undo_top == 0);
}




void shift_stack(actionlist *stack, int top, int stack_size) {
	
	unsigned int i, max_i = (unsigned)min(top, stack_size);
	
	for (i = 1; i <= max_i; ++i) 
		stack[i-1] = stack[i];
		
	stack[top].action = EMPTYx;
}




void double_size(actionlist *&stack, int &top, int &stack_size) {
	
	unsigned int i;
	actionlist *new_stack = NULL;
	
	if (2*stack_size >= max_stack_size) {
		shift_stack(stack, top, stack_size);
		return;
	}
	new_stack = memAlloc(new_stack, 2*stack_size+1);
	
	for (i = 0; (int)i <= stack_size; ++i)
		new_stack[i] = stack[i];
	
	stack_size *= 2;
		
	for (; (int)i < stack_size; ++i)
		new_stack[i].action = EMPTYx;
		
	delete [] stack;
	stack = new_stack;
	++top;
}





component *find_comp(component *head, int ID, int position, int &is_head) {
	
	component *head_next;
	
	if (head == 0)
		return head;
	
	if (head->id == ID && head->location == position) {
		is_head = 1;
		return head;
	}
	head_next = head->next;
	
	while (head_next != 0) {				
		if (head_next->id == ID && head_next->location == position)
			return head;
			
		if (get_order(head_next->id) == wirex && head_next->id == ID && head_next->location == position)
			return head;
			
		head = head_next;
		head_next = head->next;
	}
	cout << "\nThe command undo/redo could not be completed because the modified component could not be found. "
		 << "\nPossible problem: The current sequence of undo/redo operations is unsupported for efficiency reasons and will be skipped." << endl;
	
	return 0;
}




void ur_action_transform(int size_factor, int x_old, int x_new) {

	stack_transform(action_stack, action_top, size_factor, x_old, x_new);
	stack_transform(undo_stack,   undo_top,   size_factor, x_old, x_new);
}


void stack_transform(actionlist *stack, int top, int size_factor, int x_old, int x_new) {

	for (unsigned int i = 0; (int)i < top; ++i) {
		switch (stack[i].action) {
			case EMPTYx:
				break;
			case MOVEx: // update a.old_location and next section
				stack[i].old_location = point_transform((int)stack[i].old_location, size_factor, x_old, x_new);
			case ADDx:
			case DELETEx:
			case ROTATEx:
			case MODIFYVx: // update a.location, a.ID (wire)
				stack[i].location = point_transform(stack[i].location, size_factor, x_old, x_new);
				if (get_order(stack[i].ID) == wirex)
					stack[i].ID = Wire_base_id + point_transform((stack[i].ID - Wire_base_id), size_factor, x_old, x_new);
				break;
			case MODIFYWx: // update a.location, a.ID, a.old_location (as wire ID)
				stack[i].location = point_transform(stack[i].location, size_factor, x_old, x_new);
				stack[i].ID = Wire_base_id + point_transform((stack[i].ID - Wire_base_id), size_factor, x_old, x_new);
				stack[i].old_location = Wire_base_id + point_transform(((int)stack[i].old_location - Wire_base_id), size_factor, x_old, x_new);
				break;
		}
	}
}








