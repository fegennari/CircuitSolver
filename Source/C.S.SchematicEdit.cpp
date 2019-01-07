#include "CircuitSolver.h"
#include "Draw.h"
#include "Logic.h"
#include "Undo.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.SchematicEdit.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Schematic Editing Functions
// By Frank Gennari
int const move_node = 1;


int copy_location(0);
component *clipboard = NULL;


extern int click_type, delete_pref, changed, changed2, changed3, changed4, has_menubar, skip_placement_check, use_rubberbanding, needs_redraw, drawing_color, cScale, is_char;
extern int sch_origin_grid_x, sch_origin_x, sch_origin_grid_y, sch_origin_y;
extern int x_blocks, y_blocks;
extern int o_size, object_size;
extern double supply_voltage, ground;
extern char typed_char;


int remove_one_comp(component *&comp, int position, int &temp_component, int &temp_location);
int proximity_remove(component *&head, string name, int pins, string description, int position, int delete_pref);
void draw_without_comp(component *head, string name, int pins, string description, component *comp);
void rubberband_attached(component *head, component *comp, int new_position);
void rubberband_at_location(component *head, int old_position, int comp_location, int offset);
int select_area(component *head, string name, int pins, string description, int position, int box_color);
int get_valid_position_with_scroll(component *head, string name, int pins, string description);
void copy_component(component *incomp, component *&outcomp);
int is_selected(component *comp, int area_select, int position, int position2);


int can_be_rotated(int id);

int get_scrolled_window_click();
int get_valid_position();
int in_selection_area(int location, int position1, int position2);
int is_in_window(int location);

void scroll_DW(component *head, string name, int pins, string description, int scroll_type, int scroll_param);

transistor Qnum_find(transistor &tr, int num);
int show_transistor(transistor *tr, int count);

special show_ic(special &comp, component *&head, string name, string description, int pins, string old_name, int is_opened);
int get_ic_pins(int comp_id);

component *comp_at_loc(component *head, component *&last, int loc, int height, int width, int wire_only, int pins_count);




component *remove_component(component *head, string name, int pins, string description) {

	int position, position2(-1), found(0), temp_component, temp_location, rem;
	component *temp = head, *temp2, *temp_next;

	if (head == 0) {
		beep();
		cout << "\nThere are no components to remove!" << endl;
		return head;	
	}
	if (click_type == 200)
		return remove_berserk(head, name, pins, description, -1, -1);
	
	cout << "\nClick on the component to remove. Right/command click to begin a selection box. Click on (D)elete again to remove multiple components immediately. Click on Cancel or another button to end." << endl;
	
	position = get_scrolled_window_click();
	
	if (click_type > 10 && click_type < 100) { // command/right click
		position2 = select_area(head, name, pins, description, position, RED);
		if (position2 == -1)
			return head;
		
		cout << "\nAre you sure you want to remove the components in the selected area?  ";
		
		if (decision())
			return remove_berserk(head, name, pins, description, position, position2);
		else {
			init_and_draw_circuit(head, name, pins, description, 1);
			return head;
		}
	}
	else {
		if (position == CANCEL) {
			cout << "\nCANCEL" << endl;
			return head;
		}
		if (position == 2)
			return remove_berserk(head, name, pins, description, -1, -1);
	}
	if (remove_one_comp(head, position, temp_component, temp_location) == 1) {
		temp = head->next;
		delete head;
		head = temp;
		found = 1;
	}
	while (temp != 0 && temp->next != 0) {	
		temp_next = temp->next;		
		rem = remove_one_comp(temp_next, position, temp_component, temp_location);
		
		if (rem == 1) {								
			if (temp_next->next != 0) {
				temp2 = temp_next->next;
				delete temp_next;
				temp->next = temp2;
			}
			else {
				delete temp_next;
				temp->next = 0;
			}
			init_and_draw_circuit(head, name, pins, description, 1);
			return head;
		}
		else if (rem == 2)
			found = 1;
				
		temp = temp->next;
	}
	if (!found) {
		if (proximity_remove(head, name, pins, description, position, delete_pref)) {
			return head;
		}
		beep();
		cout << "\nYou must click on the center of the component. No component was removed." << endl;
	}		
	init_and_draw_circuit(head, name, pins, description, 1);
	
	return head;
}




int remove_one_comp(component *&comp, int position, int &temp_component, int &temp_location) {

	if (comp->location == position || (get_order(comp->id) == wirex && (comp->id - Wire_base_id) == position)) {
		comp = hilite_component(comp, 1);			
		temp_component = comp->id;
		temp_location = comp->location;
		
		if (delete_pref)
			cout << "\nDo you want to remove the " << get_name(temp_component, 1) << "?  ";
			
		if (!delete_pref || decision()) {			
			add_action(DELETEx, temp_component, temp_location, temp_location, comp->flags, comp->value, 0);			
			cout << "\nThe component has been removed." << endl;
			mark_all_changed();			
			return 1;			
		}
		else {
			comp = hilite_component(comp, 0);
			return 2;
		}
	}
	return 0;
}




int proximity_remove(component *&head, string name, int pins, string description, int position, int delete_pref) {

	component *temp, *last;

	temp = comp_at_loc(head, last, position, 0, 0, 0, 0);

	if (temp == NULL) {
		return 0;
	}
	if (delete_pref)
		cout << "\nDo you want to remove the " << get_name(temp->id, 1) << "?  ";
				
	if (!delete_pref || decision()) {			
		add_action(DELETEx, temp->id, temp->location, temp->location, temp->flags, temp->value, 0);			
		cout << "\nThe component has been removed." << endl;
		mark_all_changed();

		if (last == NULL) { // temp is head
			head = temp->next;
		}
		else {
			last->next = temp->next;
		}
		delete [] temp;

		init_and_draw_circuit(head, name, pins, description, 1);
		return 1;
	}
	return 0;
}





component *remove_berserk(component *head, string name, int pins, string description, int position, int position2) {
	
	int one_loop_only(0), def_pos2(position2);
	char removed;
	component *temp, *temp2;
	
	set_special_cursor(BDELETE_CURSOR);
	cout << "Instant Delete" << endl;
	
	if (position > 0)
		one_loop_only = 1;
		
	do {
		if (head == 0)
			return 0;
		
		removed   = 0;
		temp      = temp2 = head;
		position2 = def_pos2;
		
		if (!one_loop_only) {
			position = get_scrolled_window_click();

			if (click_type > 10 && click_type < 100) { // command/right click
				position2 = select_area(head, name, pins, description, position, RED);
				if (position2 == -1)
					return head;
			}
			else if (is_char) {
				is_char = 0;
				scroll_DW(head, name, pins, description, 1, (typed_char - 36));
			}
			else if (position <= num_menu_boxes || click_type >= 100) 
				return head;
		}
		else if (position <= num_menu_boxes || click_type >= 100) 
			return head;
				
		while (head != 0 && is_selected(head, 1, position, position2)) {			
			add_action(DELETEx, head->id, head->location, head->location, head->flags, head->value, removed);
			
			temp = head;
			head = head->next;
			delete temp;
			
			mark_all_changed();
			removed = 1;
		}
		if (head) {			
			temp = head;
			temp2 = temp->next;
			
			while (temp2 != 0) {		
				if (is_selected(temp2, 1, position, position2)) {					
					add_action(DELETEx, temp2->id, temp2->location, temp2->location, temp2->flags, temp2->value, removed);
							
					temp->next = temp2->next;
					delete temp2;
					temp2 = temp->next;
									
					mark_all_changed();
					removed = 1;
				}
				else {
					temp = temp->next;
					temp2 = temp->next;
				}
			}
		}
		if (!removed) {
			removed = proximity_remove(head, name, pins, description, position, 0);
		}
		if (removed || position2 > 0) {
			init_and_draw_circuit(head, name, pins, description, 1);
			draw_status_bar();
			draw_status(" Remove");
		}
		if (removed && position2 > 0) {
			cout << "\nThe components in the selected area were removed." << endl;
			cout << "\nClick on the component to remove. Right/command click to begin a selection box. Click on (D)elete again to remove multiple components immediately. Click on Cancel or another button to end." << endl;
		}
	} while (position != CANCEL && !one_loop_only);
	
	return head;
}




int modify_component(component *head, string name) {

	int position, position2, Redraw(0), ID, order;
	component *temp = NULL;
	special comp;
	
	if (head == 0) {
		beep();
		cout << "\nThere are no components in the circuit to modify/change." << endl;
		return 0;
	}
	cout << "\nClick on the component you would like to modify/change. Right/command click to begin a selection box for moving components." << endl;
	
	position = get_valid_position();
	
	if (click_type > 10 && click_type < 100) { // command/right click
		position2 = select_area(head, name, 0, "", position, BLUE);
		if (position2 == -1)
			return 0;
		
		block_move(head, position, position2); 
		return 1;
	}	
	if (position == -1)
		return 0;
	
	while (head != 0) {	
		ID    = head->id;
		order = get_order(ID);
		
		if (head->location == position) { // change value
			head = hilite_component(head, 1);
		
			if (has_value(head->id)) {
				cout << "\nWould you like to change the value of the " << get_name(head->id, 1) << "?  ";
				if (decision()) {
					add_action(MODIFYVx, head->id, head->location, head->location, head->flags, head->value, 0);
					head->value = get_value(head->id);
					mark_all_changed();
					Redraw = 1;
				}
			}
			if (ID == GROUND_ID) {
				cout << "\nWould you like to change the value of ground?  "; // ground
				if (decision()) {
					add_action(MODIFYVx, head->id, head->location, head->location, head->flags, ground, 0);
					cout << "Enter the new value of ground:  ";
					head->value = ground = ind();
					cout << "\nGround is now set at ";
					out(ground);
					cout << "V." << endl;
					mark_all_changed();
				}
			}
			else if (ID == POWER_ID) {
				cout << "\nWould you like to change the value of power?  "; // power
				if (decision()) {
					add_action(MODIFYVx, head->id, head->location, head->location, head->flags, supply_voltage, 0);
					cout << "Enter the new value of power:  ";
					head->value = supply_voltage = ind();	
					cout << "\nPower is now set at ";
					out(supply_voltage);
					cout << "V." << endl;
					mark_all_changed();
					Redraw = 1;
				}
			}
			else if (ID == ST_SEL_ID) {
				cout << "\nWould you like to change the value of the initial state"; // initial state
				if (head->flags & ROTATED)
					cout << " from 1 to 0?  ";
				else
					cout << " from 0 to 1?  ";

				if (decision()) {
					add_action(MODIFYVx, head->id, head->location, head->location, head->flags, 0, 0);
					head->flags ^= ROTATED;
					mark_all_changed();
					Redraw = 1;
				}
			}
			else if (order == transistorx) { // transistor model
				transistor tr;
				tr = Qnum_find(tr, (ID - Q_base_id));
				cout << "\nWould you like to work with the " << tr.name << " transistor model?  "; // transistor
				if (decision()) {
					reset_cursor();
					if (has_menubar == 1) 
						delete_menubar();
					comp.selection = show_transistor(&tr, (max_trs + 1));
					if (has_menubar == 0)
						draw_menubar();
					mark_all_changed();
					Redraw = 1;
					if (comp.selection == 1)
						return (ID + 2);
					return 1;
				}
			}
			else if (order == icx) {  // IC/subcircuit model
				IC ic;
				find_ic(ic, (ID - IC_base_id));
				cout << "\nWould you like to work with the" << add_spaces(ic.name, ic.name.length()) << " IC model?  "; // ic
				if (decision()) {
					reset_cursor();
					if (has_menubar == 1) 
						delete_menubar();
					comp = show_ic(comp, temp, add_spaces(ic.name, ic.name.length()), add_spaces(ic.description, ic.description.length()), ic.pins, name, 0);
					if (has_menubar == 0) 
						draw_menubar();
					mark_all_changed();
					Redraw = 1;
					if (comp.selection == 1)
						return (ID + 2);
					return 1;
				}
			}
			cout << "\nWould you like to change the position of the " << get_name(head->id, 1) << "?  ";
			if (decision()) { // move component
				set_cursor(head->id, (head->flags & ROTATED));
				cout << "\nClick on the new position of the " << get_name(head->id, 1) << "." << endl;
				position2 = get_valid_position();
				if (position2 == -1 || check_placement(head, head->id, position2, head->state)) {
					head = hilite_component(head, 0);
					return 0;
				}
				add_action(MOVEx, head->id, position2, head->location, head->flags, head->value, 0);
				head->location = position2;
				set_special_cursor(MODIFY_CURSOR);
				mark_all_changed();
				Redraw = 1;
			}
			if (can_be_rotated(ID)) { // rotate
				cout << "Would you like to rotate the " << get_name(head->id, 1) << "?  ";
				if (decision()) {
					head->flags ^= ROTATED;
					add_action(ROTATEx, head->id, head->location, head->location, head->flags, head->value, 0);
					mark_all_changed();
					Redraw = 1;
				}
			}
		}
		else if (order == wirex && head->id - Wire_base_id == position) {
			cout << "\nWould you like to change the position of the wire?  ";
			if (decision()) { // move wire
				set_cursor(head->id, (head->flags & ROTATED));
				cout << "\nClick on the new position of the " << get_name(head->id, 1) << "." << endl;
				position2 = get_valid_position();
				if (position2 == -1 || check_placement(head, head->id, position2, head->state)) {
					head = hilite_component(head, 0);
					return 0;
				}
				add_action(MODIFYWx, Wire_base_id + position2, head->location, head->id, head->flags, head->value, 0);
				head->id = Wire_base_id + position2;
				set_special_cursor(MODIFY_CURSOR);
				mark_all_changed();
				Redraw = 1;
			}
		}
		head = hilite_component(head, 0);				
		head = head->next;
	}
	return Redraw;
}




void block_move(component *head, int position, int position2) {
	
	int ref, new_ref, offset;
	char moved(0);
	component *temp = head;
	
	cout << "Click on a reference point for the move." << endl;
	ref = get_valid_position_with_scroll(head, "", 0, "");
	
	if (ref == CANCEL) {
		cout << "\nCANCEL" << endl;
		return;
	}
	else if (ref == -1)
		return;
		
	cout << "Click on the new location for that reference point." << endl;
	new_ref = get_valid_position_with_scroll(head, "", 0, "");
	
	if (new_ref == CANCEL) {
		cout << "\nCANCEL" << endl;
		return;
	}
	else if (new_ref == -1)
		return;
	
	offset = new_ref - ref;
	if (offset == 0)
		return;
	
	skip_placement_check = 1;
		
	while (temp != 0) {		
		if (get_order(temp->id) == wirex && in_selection_area((temp->id - Wire_base_id), position, position2) && !check_placement(head, temp->id + offset, temp->location, temp->state)) {
			add_action(MODIFYWx, temp->id + offset, temp->location, temp->id, temp->flags, temp->value, moved);
			temp->id += offset;
			moved = 1;
		}
		if (in_selection_area(temp->location, position, position2) && !check_placement(head, temp->id, temp->location + offset, temp->state)) {
			add_action(MOVEx, temp->id, temp->location + offset, temp->location, temp->flags, temp->value, moved);
			temp->location += offset;
			moved = 1;
		}
		temp = temp->next;
	}
	if (moved)
		mark_all_changed();

	skip_placement_check = 0;	
}




component *move_component(component *head, int &move, int position, string name, int pins, string description) {
	
	int new_position(-1), previous_comp(-1), new_loc, new_id, order;
	char moved(0);
	component *temp = head, *last;
	
	move = 1;
	
	while (temp != 0) {
		order = get_order(temp->id);
		if (temp->location == position || (order == wirex && temp->id - Wire_base_id == position)) {
			move = 2;
					
			if (order != wirex)
				draw_without_comp(head, name, pins, description, temp);
			
			if (((order != wirex || get_order(previous_comp) != wirex) && !use_rubberbanding) || previous_comp == -1) {
				set_cursor(temp->id, (temp->flags & ROTATED));
				cout << "\nClick on the new location of the " << get_name(temp->id, 1) << "." << endl;
				do {
					new_position = get_scrolled_window_click();
					if (is_char) {
						is_char = 0;
						scroll_DW(head, name, pins, description, 1, (typed_char - 36));
						new_position = -2;
					}
				} while (new_position == -2);
			}	
			if (new_position != position && is_in_window(new_position) && ((order == wirex && get_order(previous_comp) == wirex) || !check_placement(head, temp->id, new_position, temp->state))) {	
				mark_all_changed();
					
				if (temp->location == position) {	
					add_action(MOVEx, temp->id, new_position, position, temp->flags, temp->value, moved);
					if (use_rubberbanding && order != wirex)
						rubberband_attached(head, temp, new_position);
					
					temp->location = new_position;	
					moved = 1;
				}
				else {
					add_action(MODIFYWx, Wire_base_id + new_position, temp->location, temp->id, temp->flags, temp->value, moved);
					temp->id = Wire_base_id + new_position;
					moved = 1;
				}
			}
			if (move_node)
				previous_comp = temp->id;	
		}
		temp = temp->next;
	}
	if (new_position == -1) { // no component found by simple position check
		temp = comp_at_loc(head, last, position, 0, 0, 0, 0);
		
		if (temp != NULL) {
			draw_without_comp(head, name, pins, description, temp);	
			cout << "\nClick on the new location of the " << get_name(temp->id, 1) << "." << endl;
			new_position = get_scrolled_window_click();
			new_loc      = temp->location + new_position - position;
			
			if (new_position != position && is_in_window(new_position) && (!check_placement(head, temp->id, new_loc, temp->state))) {	
				mark_all_changed();
				add_action(MOVEx, temp->id, new_loc, temp->location, temp->flags, temp->value, 0);
				
				if (use_rubberbanding)
					rubberband_attached(head, temp, new_loc);
				
				if (get_order(temp->id) == wirex) {
					new_id = temp->id + new_position - position;
					add_action(MODIFYWx, new_id, new_loc, temp->id, temp->flags, temp->value, 1);
					temp->id = new_id;
				}
				temp->location = new_loc;
			}
		}
	}
	if (new_position > 0)
		init_and_draw_circuit(head, name, pins, description, 1);
	
	return head;
}




void draw_without_comp(component *head, string name, int pins, string description, component *comp) {
	
	int old_id = comp->id;
	comp->id   = 0;
	init_and_draw_circuit(head, name, pins, description, 0);
	comp->id   = old_id;
}




void rubberband_attached(component *head, component *comp, int new_position) {
	
	int ic_pin_loc, numpins, offset(new_position - comp->location), temp, order;
	
	if (offset == 0)
		return;

	order = get_order(comp->id);
		
	if (order == wirex) {
		temp = comp->id;
		comp->id = 0; // necessary so that r_a_l() does not rubberband this wire
		rubberband_at_location(head, comp->location, -1, offset);
		rubberband_at_location(head, (temp - Wire_base_id), -1, offset);
		comp->id = temp;
	}
	else if (order == icx) {				
		numpins = get_ic_pins(comp->id);
		
		for (int i = 1; i <= numpins; ++i) {		
			if (i <= (numpins >> 1)) 
				ic_pin_loc = comp->location + cScale*(i - 1 + 2*x_blocks - numpins/4);	
			else 
				ic_pin_loc = comp->location + cScale*(numpins - i - 2*x_blocks - numpins/4);
				
			rubberband_at_location(head, ic_pin_loc, comp->location, offset);			
		}
	}
	else {
		connections x = get_connections(comp->location, comp->id, comp->flags);
				
		rubberband_at_location(head, x.in1, comp->location, offset);
		rubberband_at_location(head, x.in2, comp->location, offset);
		rubberband_at_location(head, x.in3, comp->location, offset);
		rubberband_at_location(head, x.out1, comp->location, offset);
		rubberband_at_location(head, x.out2, comp->location, offset);
		rubberband_at_location(head, x.out3, comp->location, offset);
	}
}




void rubberband_at_location(component *head, int old_position, int comp_location, int offset) {

	if (old_position == 0 || comp_location == old_position)
		return;
		
	while (head != 0) {
		if (head->location == old_position && (get_order(head->id) == wirex || head->id == INPUT_ID || head->id == OUTPUT_ID || head->id == ST_SEL_ID || head->id == POWER_ID || head->id == GROUND_ID)) {
			add_action(MOVEx, head->id, head->location + offset, head->location, head->flags, head->value, 1);
			head->location += offset;
		}
		if (get_order(head->id) == wirex && head->id - Wire_base_id == old_position) {
			add_action(MODIFYWx, head->id + offset, head->location, head->id, head->flags, head->value, 1);
			head->id += offset;
		}
		head = head->next;
	}
}




void copy_components(component *head) {

	int position, position2(0), found(0), copy_area(0);
	component *temp = head, *new_component;

	if (head == 0) {
		beep();
		cout << "\nThere are no components to copy!" << endl;
		return;	
	}
	cout << "\nClick on the component to copy. Right/command click to begin a selection box. Click on Cancel or another button to end." << endl;
	
	position = get_scrolled_window_click();
	
	if (click_type > 10 && click_type < 100) { // command/right click
		position2 = select_area(head, "", 0, "", position, BLUE);
		if (position2 == -1)
			return;
			
		copy_area = 1;
	}
	while (temp != 0) {
		if (is_selected(temp, copy_area, position, position2)) {							
			if ((new_component = new component) != 0) {
				if (!found) {
					destroyL(clipboard);
					clipboard = 0;
				}
				copy_component(temp, new_component);
				new_component->next = clipboard;
				clipboard           = new_component;
				copy_location       = position;
			}
			else 
				out_of_memory();
				
			found = 1;
		}
		temp = temp->next;
	}
	if (!found)
		cout << "\nNothing was copied." << endl;
	
	if (copy_area)
		needs_redraw = 1;
}




component *cut_components(component *head) { // copy + delete

	int position, position2(0), cut_area(0), cut_head(0), is_cut_run(0);
	char removed(0);
	component *temp = head, *last = head, *clipboard_end = NULL, *head_end = NULL;

	if (head == 0) {
		beep();
		cout << "\nThere are no components to cut!" << endl;
		return head;	
	}
	cout << "\nClick on the component to cut. Right/command click to begin a selection box. Click on Cancel or another button to end." << endl;
	
	position = get_scrolled_window_click();
	
	if (click_type > 10 && click_type < 100) { // command/right click
		position2 = select_area(head, "", 0, "", position, RED);
		if (position2 == -1)
			return head;
			
		cut_area = 1;
	}	
	while (temp != 0) {
		if (is_selected(temp, cut_area, position, position2)) {				
			if (is_cut_run == 0) {
				if (temp == head) {
					cut_head = 1;
					clipboard = temp;
				}
				else {
					last->next = 0;
					head_end = last;
					if (clipboard_end == 0)
						clipboard = temp;
					else
						clipboard_end->next = temp;
				}
				is_cut_run = 1;
			}
			add_action(DELETEx, temp->id, temp->location, temp->location, temp->flags, temp->value, removed);
			
			if (!removed)
				copy_location = position;
			
			mark_all_changed();
			needs_redraw = 1;		
			removed = 1;
		}
		else if (is_cut_run) {
			last->next = 0;
			clipboard_end = last;
			if (cut_head == 1) {
				head = temp;
				cut_head = 0;
			}
			else {
				head_end->next = temp;
			}
			is_cut_run = 0;
		}
		last = temp;
		temp = temp->next;
	}
	if (removed) {
		if (clipboard_end != 0)
			clipboard_end->next = 0;
			
		if (cut_head)
			head = 0;
	}
	else
		cout << "\nNothing was cut." << endl;
			
	if (cut_area)
		needs_redraw = 1;

	return head;
}




component *paste_components(component *head) {

	int position, offset, ID, loc;
	char pasted(0);
	component *temp = clipboard, *temp2;

	if (clipboard == 0) {
		beep();
		cout << "\nThere are no components in the clipboard to paste!" << endl;
		return head;	
	}
	cout << "\nClick on the paste location. Click on Cancel or another button to end." << endl;
	
	position = get_scrolled_window_click();
	
	offset = position - copy_location;
	
	while (temp != 0) {
		ID = temp->id;
		loc = temp->location + offset;

		if (get_order(ID) == wirex) {
			ID += offset;
		}
		if (temp->flags & ROTATED) {
			ID = -ID;
		}
		temp2 = update_circuit(ID, loc, head, temp->state, temp->value);
		
		if (temp2 != head) {
			add_action(ADDx, abs(ID), loc, loc, temp->flags, temp->value, pasted);
			mark_all_changed();
			needs_redraw = 1;
			pasted       = 1;
			head         = temp2;
		}
		temp = temp->next;
	}
	return head;
}



int select_area(component *head, string name, int pins, string description, int position, int box_color) {
	
	int position2, loc, offset, top_limit, bot_limit, x1, x2, y1, y2, px, py;
	int const window_overlap = 3;
	
	cout << "\nClick again to select the opposite corner of the selection box." << endl;	
	position2 = get_valid_position_with_scroll(head, name, pins, description);

	if ((click_type <= 10 || click_type >= 100) && position2 == CANCEL) {
		cout << "\nCANCEL" << endl;
		return -1;
	}
	else if (position2 == -1)
		return -1;
	
	sch_origin_grid_x = sch_origin_x;
	sch_origin_grid_y = sch_origin_y;

	// redraw selected components
	top_limit = x_blocks*(y_blocks + window_overlap);
	bot_limit = -window_overlap*x_blocks;
	o_size    = (int)(sign(o_size)*6*object_size/sqrt((double)x_blocks*y_blocks));
	offset    = x_blocks*sch_origin_y + sch_origin_x;
	x1        = position%x_blocks;
	x2        = position2%x_blocks;
	y1        = position/x_blocks;
	y2        = position2/x_blocks;

	if (x1 > x2) {
		swap(x1, x2);
	}
	if (y1 > y2) {
		swap(y1, y2);
	}
	drawing_color = HILITE_COLOR;
	
	while (head != 0) {
		loc = head->location;
		px  = loc%x_blocks;
		py  = loc/x_blocks;

		if (px >= x1 && px <= x2 && py >= y1 && py <= y2) {
			loc -= offset;
			
			if (get_order(head->id) == wirex || (loc >= bot_limit && (loc < top_limit && (head->flags & BASE_CIRCUIT)))) {
				head = draw_individual_component(head, head->id, head->location, head->value, head->flags);
			}
		}
		head = head->next;
	}
	drawing_color = BLACK;
	draw_box_outline(box_color, position, position2);
	wait(0.1);
	
	sch_origin_grid_x = 0;
	sch_origin_grid_y = 0;
	
	return position2;
}




int get_valid_position_with_scroll(component *head, string name, int pins, string description) {
	
	int position;
	
	do {
		position = get_scrolled_window_click();
		
		if (position == CANCEL) {
			cout << "\nCANCEL" << endl;
			return -1;
		}
		else if (is_char) {
			is_char = 0;
			scroll_DW(head, name, pins, description, 1, (typed_char - 36));
			position = -1;
		}
	} while (position <= num_menu_boxes);
	
	return position;
}




component *hilite_component(component *comp, int hilite_on) {

	if (hilite_on)
		drawing_color = HILITE_COLOR;
	else
		drawing_color = BLACK;
		
	comp          = draw_individual_component(comp, comp->id, comp->location, comp->value, comp->flags);
	drawing_color = BLACK;
	
	return comp;
}




void copy_component(component *incomp, component *&outcomp) {

	outcomp->id       = incomp->id;
	outcomp->location = incomp->location;
	outcomp->state    = incomp->state;
	outcomp->value    = incomp->value;
	outcomp->flags    = incomp->flags;
}




int is_selected(component *comp, int area_select, int position, int position2) {

	return ((area_select && (in_selection_area(comp->location, position, position2) || (get_order(comp->id) == wirex && in_selection_area((comp->id - Wire_base_id), position, position2))))
			|| (!area_select && (comp->location == position || (get_order(comp->id) == wirex && (comp->id - Wire_base_id) == position))));
}









