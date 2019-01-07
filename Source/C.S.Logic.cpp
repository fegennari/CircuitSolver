#include "CircuitSolver.h"
#include "Logic.h"
#include "Draw.h"
#include "Undo.h"
#include "CompArray.h"
#include "StringTable.h"



// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Logic.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic (actually circuit design and testing graphical interface)
// By Frank Gennari
#ifdef xWIN32
	char OHM_TEXT[] = {' ', ' ', (char)234, '\0'};
#else
#ifdef xMAC_OS
	char OHM_TEXT[] = {' ', ' ', '½', '\0'};
#else
	char OHM_TEXT[] = {' ', ' ', 'O', 'h', 'm', '\0'};
#endif
#endif


#define NUM_COMP_NUMS   21


int const enable_text_scroll        = 0;
int const show_hierarchy            = 0;
int const show_values_on_screen     = 1;
int const show_comp_numbers         = 1;
int const useMHO                    = 0;
int const begin_in_schematic_window = 1; 
int const ask_to_open               = 0;
int const allow_save_off_screen     = 1;
int const warn_clear                = 0;

int const MAX_ERROR_MESSAGES        = 100;
int const window_overlap            = 3;


int changed(0), changed2(0), changed3(0), Opened(1), num_buttons, is_ic, grid_color(8), save_pref(0), delete_pref(1), must_exit(0), has_menubar(0), force_begin_in_schematic_window(0), last_component(0), 
	was_in_schematic_window(0), click_type(0), current_circuit_window_id(0), print_text(1), update_error(0), surpress_update_error(0), skip_placement_check(0), GV(0), recalculate(2), title_set(0),
	use_rubberbanding(1), auto_route(1), cScale(1), SAVE(0), surpress_show_values(0), LINE_THICKNESS(1), VALUE_TEXT_SIZE(9);
unsigned char units[6] = {0};
string ic_name = " ", component_name, comp_units;
box new_num;
char current_name[MAX_SAVE_CHARACTERS + 1] = ".";
const char *hasValue = has_val;
component **circuit_window;


extern transistor Add;
extern int graph_enable, o_size, object_size, x_limit, y_limit, last_known_value, conflict, num_components, OPEN, text_size, need_node, changed4, needs_redraw, complex_form, useDIALOG, max_total_boxes, freq, saveBOXES, drawing_color, dont_whiteout_double, DEMO, enablePAINT, looks_like_IC, mos_delay_not_wl;
extern int useDELAY, force_init_states, dont_print_tt; // for logic engine
extern int found_click, is_char, line_weight;
extern int sch_origin_x, sch_origin_y, sch_origin_grid_x, sch_origin_grid_y;
extern int x_blocks, y_blocks, x_screen, y_screen, x_screen_real, y_screen_real;
extern char NAME[], *conflict_flag, typed_char;

extern int EDGE, num_iterations;
extern double supply_voltage, ground;


int circuit_check(component *head, int show_error, int is_top_level);
void draw_transistor_by_id(int id, int position);
void draw_component_numbers(component *head);
void draw_voltage(int position);
component *revert(component *head);
int save_cir(component *& head);
int can_be_rotated(int id);
void print_circuit_info(component *head, int flag); // for debugging


component *evaluate_logic(component *head);
void set_gate_delays(component *head, comp_sim *comps, int nodes, int num_comps, int mode);

void draw_text(string text, int box_num, int t_size);

int in_selection_area(int location, int position1, int position2);

component *pick_transistor(component *head, string name, int pins, string description);
double get_q_type(int id);
transistor Qnum_find(transistor &tr, int num);
void display_info(transistor *tr);
 
int count_io(component *head);

void label_io(int location, int ID, int use_alpha, char name_char);

component *analog_analysis_init(component *head, complex *DWGRID);

component *simulation_cleanup(component *head);

component *import_netlist(component *head, ifstream &infile, StringTable &net_names, string &input_buffer);
void export_netlist(component *& head);

void write_circuit_as_text(component *head);

component *link_components(component *head);

point get_center(int box_num);
int get_scrolled_window_click();
int get_valid_position();
int is_in_window(int location);

component *pick_MOSFET(component *head, string name, int pins, string description);

special show_ic(component *&head, string name, string description, int pins, string old_name, int is_opened);

component *reroute_diag_wire(component *head, int w_start, int &comp_id);

int show_transistor(transistor *tr, int count);
void Q_values();
void NewQ();

void paint_menu_buttons();

void draw_string2(unsigned char *display);
void set_text_size(int size);

void resize_DW(component *&head, string name, int pins, string description, int new_num_boxes);
void resize_comps(component *&head, string name, int pins, string description, int size_factor);
void set_grid_res(component *&head, string name, int pins, string description, int grid_factor);
void zoom_DW(component *&head, string name, int pins, string description, int zoom_factor);
void reset_DW_defaults(component *&head, string name, int pins, string description);
void scroll_DW(component *head, string name, int pins, string description, int scroll_type, int scroll_param);
void window_resize(component *head, string name, int pins, string description);
void reset_origin();
void fit_circuit_to_screen();
int get_window_param();

void create_logic_function();

void clipping_check();

int rlc_eval(component *&head);




void logic() {

	int ok(0), Decision;
	component *head = NULL, *tail;

	//clipping_check();

	#ifdef TEXT_ONLY
		cout << "Schematic capture is not available when running in text mode." << endl;
		if (OPEN) {
			cout << "Would you like to simulate this design without graphics?  ";
			if (decision()) {
				simulate_only();
			}
			else {
				cout << "Would you like to save this design in another format?  ";
				if (decision()) {
					save_only();
				}
			}
		}
		else {
			cout << "Do you want to open a netlist directly for simulation?  ";
			if (decision()) {
				supply_voltage = default_voltage;
				ground = is_ic = 0;
				head   = evaluate_logic(head);
				destroyL(head); // head should be NULL, but just in case...
			}
		}
		return;
	#endif

	num_buttons = default_num_buttons;
	x_blocks = y_blocks = x_screen = y_screen = 1;
	new_num.h_boxes = new_num.v_boxes = 0;
	must_exit = title_set = 0;		
	supply_voltage = default_voltage;
	ground = 0;
	
	strcpy(current_name, ".");
	reset_actions();
	
	if (DEMO)
		save_pref = 0;
	
	while (head == 0 && ok == 0) {		 
	 	is_ic = 0;
	 	
		if (OPEN != 1 && ask_to_open) {
			cout << "\nOpen saved circuit?  ";
		}
		if (OPEN == 1 || (ask_to_open && decision())) {
			head = open_circuit(head);
			set_buttons(num_buttons, NO_COLOR, 1);
			fit_circuit_to_screen();						
			changed = changed2 = 0;
		
			if (head != 0)	{
				ok   = 1;
				tail = head;
				
				while (tail->next != 0) {
					tail = tail->next;
				}
				if (!is_ic) {
					head = draw_component_at_click(tail->id, head, " ", 0, " "); 
					was_in_schematic_window = 1;
				}	
				else { 
					head = pick_ic(head, 1, " ", 0, " ");
					destroyL(head);
					set_DW_title(init_window_title);
					whiteout();
					was_in_schematic_window = 0;
					return;
				}
			}
			else
				OPEN = 0;			
		}
		else
			ok = 1;						
	} 
	do {
		if (!must_exit) {
			head = select_component(head, " ", 0, " ");	
		}
		Decision = 0;
		
 		if (save_pref && head != 0 && circuit_has_changed()) {
			if (!useDIALOG) {
 				cout << "\nSave circuit before exiting";
			}
 			Decision = xdecision(1);
 		
 			if (Decision == 1) {
 				save_circuit(head);
 				changed = changed2 = 0;
 			}
 			else if (Decision == 2 && must_exit) {
 				force_begin_in_schematic_window = 1;
 				must_exit = 0;
 			}
 		}
 	} while (Decision == 2);
 		
 	changed = changed2 = was_in_schematic_window = 0;
 	
 	destroyL(head);
 	set_DW_title(init_window_title);
 	whiteout();
}




void simulate_only() {

	component *head = NULL;
	
	supply_voltage = default_voltage;
	ground = is_ic = 0;
	num_buttons = default_num_buttons;

	head = open_circuit(head);

	if (head == 0)
		return;

	x_blocks    = new_num.h_boxes;
	y_blocks    = new_num.v_boxes;
	num_buttons = 0;

	if (!count_io(head)) {
		cout << "Only digital circuits with inputs and outputs defined can be simulated with this option." << endl;
	}
	else {
		head = evaluate_logic(head);
		head = simulation_cleanup(head);
	}
	destroyL(head);
}




void save_only() {

	component *head = NULL;
	
	supply_voltage = default_voltage;
	ground = is_ic = 0;

	head = open_circuit(head);

	if (head == 0)
		return;

	x_blocks = new_num.h_boxes;
	y_blocks = new_num.v_boxes;

	save_circuit(head);
	destroyL(head);
}




int circuit_has_changed() {

	//return changed;
	return !is_action_stack_empty();
}





component *select_component(component *head, string name, int pins, string description) {
	
	int component(last_component), normal;

	if (force_begin_in_schematic_window || (head == 0 && begin_in_schematic_window && !was_in_schematic_window)) {
		force_begin_in_schematic_window = 0;
		head = draw_component_at_click(component, head, name, pins, description);
		if (must_exit) {
			changed3 = changed4 = 1;
 			return head;
 		}
	}
	was_in_schematic_window = 0;
	
	do {	
		reset_cursor();
		fill_boxes_with_logic_icons();
		
		cout << "\nClick on the graphics window to make a selection (first box to Exit)." << endl;		
		component = box_clicked();
 		normal = 0;

		if (component > num_choices) {
			continue;
		}
 		if (component == FUNCTION_SEL)
 			create_logic_function();
 		
 		if (component != 0 && (component <= num_choices || (component > Q_base_id && component < Wire_max_id))) {
 			component = add_window_id[component];
 			normal = 1;
 		}
 		if (component <= num_choices)
 			cout << get_name(component, 1) << endl;
 		 		
 		switch (component) {		
	 		case IC_ID:
	 			must_exit = 0;
	 			head = pick_ic(head, 0, name, pins, description);
	 			break;
	 			
	 		case FF_ID:
	 			head = pick_ff(head, name, pins, description);
	 			break;
	 					
	 		case TRANSISTOR_ID:
	 			head = pick_transistor(head, name, pins, description);
	 			break;
	 		
	 		case DEP_SRC_ID:
	 			head = pick_source(head, name, pins, description);
	 			break;
	 			
	 		case FET_ID:
	 			head = pick_MOSFET(head, name, pins, description);
	 			break;
	 			
	 		default:
	 			if (normal) {
	 				if (component == ST_SEL_ID) {
	 					cout << "Enter Initial State (1 or 0, >0 assumes 1):  ";
						if (LOGIC(in()) == 0) {
	 						component = -component;
						}
	 				}
	 				head = draw_component_at_click(component, head, name, pins, description);
	 			}
 		}
 	} while (component != 0 && !must_exit);
 	
 	changed3       = changed4 = 1;
 	last_component = component;
 	
	if (last_component > MAX_USED_ID || last_component == IC_ID || last_component == CUSWIRE_ID || last_component == TRANSISTOR_ID || last_component == FF_ID) {
 		last_component = 0;
 	}
 	return head;
}





void fill_boxes_with_logic_icons() {	
	
	unsigned int i;
	int draw_id, temp_llic(looks_like_IC);
	
	object_size = (int)(sqrt(float(x_limit*y_limit))/pi);
	looks_like_IC = 1;
	set_buttons(num_menu_selections, NO_COLOR, 0);
	init_menu(num_menu_selections, BLACK, 0);

	for (i = 0; (int)i <= num_choices; ++i) {
		draw_id = add_window_id[i];
		if (draw_id <= MAX_USED_ID && draw_comp_in_add[i])
			draw_comp[draw_id](i);
	}
	draw_NOT(0);
	draw_FF(FF_SEL, NO_FF);
	draw_CK(FF_SEL);
	draw_CK0(FF_SEL);
	draw_DEPENDENT_SOURCE(DEP_SRC_SEL);
	draw_XFMR(XFMR_SEL);
	
	draw_text(" Expression", FUNCTION_SEL, 12);
	
	looks_like_IC = temp_llic;
}





component *pick_ff(component *head, string name, int pins, string description) {
	
	int flipflop; // id = 15
	
	EDGE = get_edge();
	init_menu(8, BLACK, 0);
	
	for (unsigned int i = 1; (int)i <= num_ffs; ++i) {
		draw_custom_ff(-(int)i, i);
		if (EDGE == 0 && i != 1)
			draw_CK0(i);
	}
	draw_LARROW(0);
	
	draw_text(" BACK", reserved_pos_value, 12);
	
	do {
		cout << "\nChoose a type of Flip Flop." << endl;
	
		flipflop = box_clicked();
	
	} while (flipflop > num_ffs && flipflop != 0);
	
	if (flipflop != 0) {
		if (EDGE == 0)
			flipflop = flipflop + num_ffs;
		return draw_component_at_click((FF_base_id + flipflop), head, name, pins, description);
	}	
	return head;
}





component *pick_source(component *head, string name, int pins, string description) {

	init_menu(4, BLACK, 0);
	
	draw_VCV_SOURCE(0);
	draw_CCV_SOURCE(1);
	draw_VCC_SOURCE(2);
	draw_CCC_SOURCE(3);
	
	cout << "\nChoose a type of dependent source." << endl;
	
	return draw_component_at_click((DEP_SRC_ID + box_clicked()), head, name, pins, description);
}



inline char BooleanNot(char in) {
	if (in == 1) return 0;
	else if (in == 0) return 1;
	return 2;
}

int get_edge() {
	
	init_menu(2, BLACK, 0);
	
	draw_text(" Positive Edge Triggered (Rising)", reserved_pos_value, 12);
	draw_text(" Negative Edge Triggered (Falling)", -1, 12);
	
	cout << "\nSelect a type of FlipFlop." << endl;
	
	return BooleanNot(box_clicked());
}





string get_name_of_ff(int id) {
	
	if (id <= num_ffs)
		return ff_full_name[id];
		
	if (id > num_ffs && id <= 2*num_ffs) 
		return get_name_of_ff(id - num_ffs);
		
	return " Unknown FlipFlop";
}





component *draw_component_at_click(int component_id, component *head, string name, int pins, string description) {
		
	int position, move, abs_id, old_comp_id, ID;
	char flags;
	complex *DWGRID = NULL;
	component *temp;
	
	if (!title_set) {
		char window_title[MAX_SAVE_CHARACTERS + def_win_title_len] = {0};
		strcpy(window_title, init_window_title);
		set_DW_title(strcat(window_title, current_name));
		title_set = 1;
	}
	if (!has_menubar) 
		draw_menubar();
	
	init_and_draw_circuit(head, name, pins, description, 1);

	cout << "\nClick on a button for circuit design/simulation functions.  Right click to move components." << endl;
	
	if (component_id == NO_ID && enable_text_scroll)
		scroll_text("Circuit Solver: Created by Frank Gennari", 120);
	
	if (get_order(abs(component_id)) == wirex) {
		component_id = CUSWIRE_ID;
	}
	print_text  = enablePAINT = 1;
	found_click = 0;
	
 	do {
		abs_id = abs(component_id);
 		set_cursor(abs_id, (component_id < 0));
		move = 0;
		
		if (print_text) {
	 		if (component_id != 0) {
	 			draw_status_bar();
				draw_status(get_name(abs_id, 0));
				cout << "\nClick to place the ";
				
				if (abs_id == POWER_ID) {
					out(supply_voltage);
					cout << "V ";
				}
				if (abs_id == ST_SEL_ID) {
					cout << "(Flip to change states, currently at ";
					if (abs(component_id) > 0)
						cout << "1";
					else
						cout << "0";
					cout << ")";
				}
				cout << get_name(abs(component_id), 1) << ".";
			}
			cout << "\nClick on a button for circuit design/simulation functions.  Right click to move components." << endl;
		}		
		do {			
			position = get_scrolled_window_click();
			if (click_type == -2) {
				init_and_draw_circuit(head, name, pins, description, 1);
				wait(0.1);
			}
			if (click_type == -3 && has_menubar == 1) {
				paint_menu_buttons();
				wait(0.1);
			}
		} while (click_type == -2 || click_type == -3);
				
		if (click_type > 10 && click_type < 100) 
			head = move_component(head, move, position, name, pins, description); // Move						
		else if (is_char) {
			is_char = 0;
			scroll_DW(head, name, pins, description, 1, (typed_char - 36));
		}
		else {
			head = process_button_click(head, name, pins, description, DWGRID, position, component_id);
		}
		if (must_exit)
			return head;
		
		if (component_id != NO_ID && is_in_window(position) && !move) {	
			mark_all_changed();
			old_comp_id = component_id;
			
			if (component_id < 0) {
				flags = ROTATED;
			}
			else {
				flags = 0;
			}
			if (component_id == CUSWIRE_ID) { // user drawn wire
				component_id = draw_CLINE(position);
				head = split_wires(head, position, (component_id - Wire_base_id));
				
				if (auto_route && component_id != 0) {
					if (is_diagonal(position, (component_id - Wire_base_id))) {
						head = reroute_diag_wire(head, position, component_id);
						needs_redraw = 1;
					}
				}
			}
			head = draw_individual_component(head, abs(component_id), position, 0, flags);
			temp = update_circuit(component_id, position, head, 2, 0);
			
			if (print_text > 0)
				--print_text;
			 
			if (temp != head) {
				add_action(ADDx, abs(component_id), position, position, flags, temp->value, 0); 
			}
			head         = temp;
			component_id = old_comp_id;
			
			if (head != 0) {
				ID = head->id;
			
				if (needs_redraw || (head->value < 0 && head->location == position && head->id == abs(component_id) && (ID == DIODE_ID || ID == BATTERY_ID || (ID >= V_SOURCE_ID && ID <= CCCS_ID))))
					init_and_draw_circuit(head, name, pins, description, 1);
			} 
		}
		else if (print_text > 0)
			--print_text;
			
	} while (position != 1);
	
	if (has_menubar) 
		delete_menubar();
	
	reset_cursor();
	
	return head;	
}




component *process_button_click(component *head, string name, int pins, string description, complex *DWGRID, int position, int &component_id) {
	
	int Decision, modret(0), old_GV, param(1), abs_id(abs(component_id));
	char window_title[MAX_SAVE_CHARACTERS + def_win_title_len] = {0};

	static int last_pos(0);
	
	must_exit = 0;

	if (position > num_menu_selections) {
		if (needs_redraw)
			init_and_draw_circuit(head, name, pins, description, 1);
		
		return head;
	}	
	switch (position) {
		case 0: // Run/Evaluate/Simulate
			if (changed3 || Opened)
				init_and_draw_circuit(head, name, pins, description, 1);
			
			if (head == 0) {
				beep();
				cout << "\nThere is no circuit to simulate!" << endl;
				cout << "Would you like to bypass the graphical interface and open a netlist directly for logic simulation?  ";
				if (decision()) {
					head = evaluate_logic(head);
					destroyL(head); // head should be NULL, but just in case...
					head = NULL;
				}
				print_text = 2;
				break;
			}
			if (!count_io(head)) 
				head = analog_analysis_init(head, DWGRID);
			else {
				if (recalculate && !(changed3 || Opened)) {
					if (recalculate == 1)
						changed3 = 1;
					else if (recalculate == 2) {	
						cout << "\nRecalculate Circuit?  ";
						changed3 = decision();
					}
				}
				head = evaluate_logic(head);
			}
			head = simulation_cleanup(head);
			if (needs_redraw)
				init_and_draw_circuit(head, name, pins, description, 1);
			
			print_text = 2;
			break;
			
		//	1 = Add
		
		case 2: // Remove
			draw_status_bar();
			draw_status(" Remove");
			set_special_cursor(DELETE_CURSOR);
			head = remove_component(head, name, pins, description);
			reset_cursor();
			print_text = 2;
			break;

		case 3: // Open
			reset_cursor();
			draw_status_bar();
			draw_status(" Open");
			if (is_ic) {
				beep();
				cout << "\nCannot open circuit until IC modifications are finished!" << endl;
				break;
			}
			if (save_pref && head != 0 && circuit_has_changed()) {
				cout << "\nSave existing circuit";
				Decision = xdecision(1);
				if (Decision == 1)
					save_circuit(head);
				else if (Decision == 2)
					break;
			}
			head = open_circuit(head);
			set_buttons(num_buttons, NO_COLOR, 1);
			fit_circuit_to_screen();			
			changed = changed2 = 0;
			
			if (is_ic) {
				head = pick_ic(head, 1, name, pins, description);
				is_ic = 0;
				destroyL(head);
				must_exit = 1;
				if (has_menubar == 1) 
					delete_menubar();
				return 0;
			}
			init_and_draw_circuit(head, name, pins, description, 1);
			
			if (has_menubar == 1)
				paint_menu_buttons();
		
			print_text = 2;
			break;
			
		case 4: // Save as
			draw_status_bar();
			draw_status(" Save");
			reset_cursor();
			save_circuit(head);
			print_text = 2;
			break;
			
		case 5: // Clear
			draw_status_bar();
			draw_status(" Clear");
			if (warn_clear && head != 0) {
				cout << "\nWarning: This will erase the entire circuit and cannot be undone. Proceed?  ";
				if (!decision()) // Add dialog box?
					break;
			}
			reset_cursor();
		case 26: // Close
			if (save_pref && head != 0 && circuit_has_changed()) {
				cout << "\nSave circuit";
				Decision = xdecision(1);
				if (Decision == 1)
					save_circuit(head);
				else if (Decision == 2)
					break;
			}
			destroyL(head);
			head = 0;		
			reset_actions();
			init_buttons(1);
			
			if (is_ic)
				draw_IC_ON_SCREEN(name, pins, description);
			
			print_text = 2;
			break;
		
		case 6: // Redraw
			if (last_pos == 6) {
				position        = 0;
				LINE_THICKNESS  = 2;
				VALUE_TEXT_SIZE = 16;
			}
			init_and_draw_circuit(head, name, pins, description, 1);
			LINE_THICKNESS  = 1;
			VALUE_TEXT_SIZE = 9;

			if (has_menubar == 1)
				paint_menu_buttons();
			break;
			
		case 7: // Preferences
			reset_cursor();
			old_GV = GV;
			set_circuit_prefs();
			if (old_GV == 0 && GV == 1) {
				set_gate_delays(head, NULL, 0, 0, 0);
			}
			else if (old_GV == 1 && GV == 0) {
				cout << "\nCircuit must now be recalculated." << endl;
				changed3 = 1;
			}
			init_and_draw_circuit(head, name, pins, description, 1);
			print_text = 2;	
			break;
		
		case CANCEL: // Stop/Cancel = 8
			cout << "\nCANCEL" << endl;
			reset_cursor();
			get_continue();
			if (has_menubar == 1)
				paint_menu_buttons(); 
			break;
			
		case 9: // Flip/Rotate
			if (can_be_rotated(abs_id)) {
				draw_status_bar();
				if (abs_id == ST_SEL_ID) {
					draw_status(" State inverted");
				}
				else if (IS_ROT90(abs_id)) {
					draw_status(" Components Flipped 90 Degrees");
				}
				else {
					draw_status(" Components Flipped 180 Degrees");
				}
				o_size       = -o_size;
				component_id = -component_id;
				draw_REVERSE();
				wait(.2);
			}
			else {
				beep();
				cout << "\nThis component cannot be flipped or inverted." << endl;
			}
			break;
			
		case 10: // Info
			draw_status_bar();
			draw_status(" Info");
			set_special_cursor(WHAT_CURSOR);
			get_info(head);
			reset_cursor();
			print_text = 2;
			break;
			
		case 11: // Modify
			draw_status_bar();
			draw_status(" Modify");
			set_special_cursor(MODIFY_CURSOR);
			modret = modify_component(head, name);
			if (modret) {
				init_and_draw_circuit(head, name, pins, description, 1);
				if (modret > 1)
					component_id = modret - 2;
			}
			print_text = 2;
			break;
		case 12: // Undo
			needs_redraw = 0;
			head = undo_action(head);
			if (needs_redraw)
				init_and_draw_circuit(head, name, pins, description, 1);
			break;
		
		case 13: // Redo
			needs_redraw = 0;
			head = redo_action(head);
			if (needs_redraw)
				init_and_draw_circuit(head, name, pins, description, 1);
			break;
			
		case 14: // Exit (Exit from schematic capture section)	
			must_exit = 1;
			if (has_menubar) 
				delete_menubar();
			reset_cursor();
			break;
			
		case 15: // zoom
			param = get_window_param();
			zoom_DW(head, name, pins, description, param);
			//resize_comps(head, name, pins, description, param);
			break;
			
		case 16: // resize DW
			param = get_window_param();
			resize_DW(head, name, pins, description, param);
			break;
			
		case 17: // grid resolution
			param = get_window_param();
			set_grid_res(head, name, pins, description, param);
			break;
			
		case 18: // cut
			head = cut_components(head);
			break;
			
		case 19: // copy
			copy_components(head);
			break;
			
		case 20: // paste
			head = paste_components(head);
			break;
			
		case 21: // reset window defaults
			reset_DW_defaults(head, name, pins, description);
			break;
			
		case 22: // save
			draw_status_bar();
			draw_status(" Save");
			reset_cursor();
			if (!is_ic)
				SAVE = 1;
			save_circuit(head);
			if (!is_ic)
				SAVE = 0;
			break;
			
		case 23: // revert
			head = revert(head);
			if (needs_redraw)
				init_and_draw_circuit(head, name, pins, description, 1);
			break;
			
		case 24: // print
			cout << "\nSorry, print is not yet supported. Please use the Print Screen button on your keyboard to take a screenshot." << endl;
			break;
			
		case 25: // New
			draw_status_bar();
			draw_status(" New");
			reset_cursor();
			if (save_pref && head != 0 && circuit_has_changed()) {
				cout << "\nSave circuit";
				Decision = xdecision(1);
				if (Decision == 1)
					save_circuit(head);
				else if (Decision == 2)
					break;
			}
			destroyL(head);
			head = 0;			
			reset_actions();
			init_buttons(1);
			
			if (is_ic)
				draw_IC_ON_SCREEN(name, pins, description);
			
			strcpy(current_name, ".");
			strcpy(window_title, init_window_title);
			set_DW_title(strcat(window_title, "."));
			print_text = 2;
			break;
			
		// case 26, close, is defined above
		
		case 27: // Fit to Screen
			fit_circuit_to_screen();
			break;

		case 28: // Reset Origin
			reset_origin();
			needs_redraw = 1;
			break;
			
		case 29: // Multiple circuits - switch windows by changing to different head (component *)
			cout << "\nSorry, this option is not yet implemented." << endl;
			/*
			circuit_window[current_circuit_window_id] = head;
			current_circuit_window_id = active_circuit;
			head = circuit_window[current_circuit_window_id];
			*/
			break;

		case 30: // RLC Circuit Simulation/Analysis
			if (rlc_eval(head)) {
				if (needs_redraw) {
					init_and_draw_circuit(head, name, pins, description, 1);
					needs_redraw = 0;
				}
			}
			else {
				cerr << "Could not run RLC simulation." << endl;
			}
			break;
	}
	last_pos = position;

	return head;
}





component *draw_individual_component(component *head, int component_id, int position, double value, char flags) {
	
	int SIGN = sign(o_size), temp_size(o_size), order;
	
	if (component_id == 0) {
		return head;
	}
	sch_origin_grid_x = sch_origin_x;
	sch_origin_grid_y = sch_origin_y;
		
	if (flags & ROTATED) {
		o_size = -abs(o_size);
	}
	else {
		o_size = abs(o_size);
	}
	if (component_id == POWER_ID)
		draw_voltage(position + 1);
		
	if ((component_id < CAP_ID && component_id != FF_ID) || (component_id >= INPUT_ID && component_id <= GROUND_ID) || (component_id >= T_ST_BUF_ID && component_id <= PFET_ID) || (component_id >= QAND_ID && component_id <= QXNOR_ID))
		draw_comp[component_id](position);
		
	else if ((component_id >= V_SOURCE_ID && component_id <= MAX_DEP_SRC_ID) || component_id == DIODE_ID || component_id == BATTERY_ID || component_id == DELAY_ID || component_id == TLINE_ID) {
		temp_size = o_size;
		
		if (value < 0) 
			o_size = -abs(o_size);
		else 
			o_size = abs(o_size);
			
		if (flags & ROTATED)
			draw_comp_rot[component_id](position);
		else
			draw_comp[component_id](position);
			
		o_size = temp_size;
	}
	else {
		switch (component_id) {			
			case FF_ID:
				draw_FF(position, (component_id - FF_base_id));
				break;
				
			case PNP_ID: // 111
				draw_PNP(position);
				break;
					
			case CAP_ID:
			case RES_ID:
			case INDUCTOR_ID:
			case XFMR_ID:
				if (flags & ROTATED)
					draw_comp_rot[component_id](position);
				else
					draw_comp[component_id](position);
				break;
				
			case ST_SEL_ID:
				if (flags & ROTATED)
					draw_char('0', position, 3, 12);
				else
					draw_char('1', position, 1, 12);
				break;

			case CUSWIRE_ID: // should not happen
				cerr << "Error: Wire with undefined endpoint cannot be drawn." << endl;
				return head;
		}
	}
	order = get_order(component_id);

	if (order == wirex) { // wire
		draw_LINE(component_id, position);
	}
	else if (order == flipflopx) { // flipflop
		if (component_id != PRSL_ID)
			draw_ckFF(position, (component_id - FF_base_id));
		else
			draw_FF(position, (component_id - FF_base_id));
			
		if (component_id > NRSL_ID && component_id != PRSL_ID)
			draw_CK0(position);
			
		draw_FF_name((component_id - FF_base_id), position);
	}
	else if (order == transistorx) { // transistor
		draw_transistor_by_id(component_id, position);
	}	
	else if (order == icx) { // ic	
		IC ic;
		find_ic(ic, (component_id - IC_base_id));		
		draw_FULL_IC(position, ic.pins);
		draw_text(ic.name, -position, 12);
	}					
	draw_NODES(get_connections(position, component_id, flags));
	o_size = SIGN*abs(o_size);
	sch_origin_grid_x = 0;
	sch_origin_grid_y = 0;
	
	return head;
}




void draw_transistor_by_id(int id, int position) {

	transistor tr;
	
	tr = Qnum_find(tr, (id - Q_base_id));
	
	switch (tr.z) {
		case 'n':
		case 'N':
			draw_NPN(position);
			break;
			
		case 'p':
		case 'P':
			draw_PNP(position);
			break;
			
		case 'f':
		case 'F':
			draw_NFET(position);
			break;
			
		case 'g':
		case 'G':
			draw_PFET(position);
			break;
			
		default:
			draw_TRANSISTOR(position);
	}
	draw_text(shift_chars(tr.name), -(position + sign(o_size)*2), 10);
}




void draw_voltage(int position) {

	label_io(position, 0, 2, ' '); // supply voltage is kind of like an I/O
}




component *split_wires(component *head, int start, int end) {
	
	int end_position, s1x, s1y, s2x, s2y, e1x, e1y, e2x, e2y;
	component *temp = head;
	
	s1x = start%x_blocks;
	s1y = start/x_blocks;
	e1x = end%x_blocks;
	e1y = end/x_blocks;
		
	while (temp != 0) {		
		if (get_order(temp->id) == wirex) {
			end_position = temp->id - Wire_base_id;
			s2x = temp->location%x_blocks;
			s2y = temp->location/x_blocks;
			e2x = end_position%x_blocks;
			e2y = end_position/x_blocks;
			if (s1x == s2x && s2x == e2x && ((s1y > s2y && s1y < e2y) || (s1y < s2y && s1y > e2y))) 
				head = split_one_wire(temp, head, start);
			
			if (s1y == s2y && s2y == e2y && ((s1x > s2x && s1x < e2x) || (s1x < s2x && s1x > e2x))) 
				head = split_one_wire(temp, head, start);
			
			if (e1x == s2x && s2x == e2x && ((e1y > s2y && e1y < e2y) || (e1y < s2y && e1y > e2y))) 
				head = split_one_wire(temp, head, end);
				
			if (e1y == s2y && s2y == e2y && ((e1x > s2x && e1x < e2x) || (e1x < s2x && e1x > e2x))) 
				head = split_one_wire(temp, head, end);
		}		
		temp = temp->next;	
	}	
	return head;
}




component *split_one_wire(component *comp, component *head, int new_location) {
	
	skip_placement_check = 1;
	head = update_circuit((Wire_base_id + new_location), comp->location, head, 2, 0);
	skip_placement_check = 0;
	add_action(MOVEx, comp->id, new_location, comp->location, comp->flags, 1, 1);
	add_action(ADDx, Wire_base_id + new_location, comp->location, comp->location, comp->flags, 1, 1);
	comp->location = new_location;
	cout << "Existing wire was split to make the connection." << endl;
	return head;
}





int draw_CLINE(int position) {

	point c1, c2; 
	int box;
		
	c1 = get_center(position);
	
	if (print_text)
		cout << "Click to end the wire." << endl;
	
	do {
		box = get_scrolled_window_click();
		if (box == CANCEL) {
			cout << "\nCANCEL" << endl;
			return 0;
		}
	} while (!is_in_window(box));	

	c2 = get_center(box);
	
	draw_WIRE(c1, c2);
	
	return (Wire_base_id + box);
}





void get_info(component *head) {

	int position, ID, order;
	IC ic;
	
	if (head == 0) {
		beep();
		cout << "\nThere are no components in the circuit to get information on." << endl;
		return;
	}
	cout << "\nClick on the component you would like to get information on." << endl;	
	position = get_valid_position();
	
	if (position == -1)
		return;
	
	while (head) {	
		if (head->location == position) {
			ID = head->id;

			if (has_value(ID)) {				
				if (hasValue[ID] == 2 || head->value > 0 || (ID == DIODE_ID && head->value == 0) || (head->value != 0 && (ID == BATTERY_ID || ID == V_SOURCE_ID || ID == I_SOURCE_ID))) {
					cout << range_value(head->value);
					print_ch(get_units(head->id, head->value, 0));
					cout << " ";
				}
				else if (ID == DIODE_ID && head->value < 0) {
					cout << range_value(diode_vdrop);
					print_ch(get_units(head->id, diode_vdrop, 0));
					cout << endl;
				}
				else {
					cout << "Variable ";
					head->value = 0;
				}
			}
			if (ID == GROUND_ID) {
				out(ground);
				cout << "V ";
			}
			else if (ID == POWER_ID) {
				out(supply_voltage);
				cout << "V ";	
			}	
			cout << get_name(head->id, 1) << endl;
			
			draw_status_bar();
			draw_status(get_name(head->id, 0));
			wait(.1);

			order = get_order(head->id);
			
			if (order == transistorx) { // transistor
				transistor tr;
				tr = Qnum_find(tr, (head->id - Q_base_id));
				display_info(&tr);
			}
			else if (order == icx) { // ic
				find_ic(ic, (head->id - IC_base_id));
				ic.name[0] = ' ';
				ic.description = add_spaces(ic.description, ic.description.length());
				specs(ic.name, ic.pins, ic.description);
			}
		}
		else if (get_order(head->id) == wirex && head->id - Wire_base_id == position)
			cout << "Wire" << endl;
				
		head = head->next;
	}
	wait(.5);
}





connections get_connections(int location, int component_id, char flags) {
	
	int cclass, signc, order;
	connections x;
	
	set_all_connections(x, 0);

	if (flags & ROTATED)
		signc = -1;
	else
		signc = 1;

	if (get_order(component_id) == wirex) {
		x.in2  = location;
		x.out2 = component_id - Wire_base_id;
		return x;
	}	
	if (component_id <= MAX_USED_ID) {
		cclass = conn_class[component_id];
		
		switch (cclass) {	
			case 1: // buf, inv
				x.in2  = location - cScale*(signc + signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 2: // battery, vs, cs
				if (flags & ROTATED) {
					x.in2  = location + cScale*(x_blocks + x_blocks);
					x.out2 = location - cScale*(x_blocks + x_blocks);	
				}
				else {
					x.in2  = location - cScale - cScale;
					x.out2 = location + cScale + cScale;
				}
				return x;
				
			case 3: // rlc
				if (flags & ROTATED) {
					x.in2  = location - cScale*(x_blocks + x_blocks);
					x.out2 = location + cScale*(x_blocks + x_blocks);
				}
				else {
					x.in2  = location - cScale - cScale;
					x.out2 = location + cScale + cScale;
				}
				return x;
				
			case 4: // 2-in gates
				x.in1  = location - cScale*(x_blocks + signc + signc);
				x.in3  = location + cScale*(x_blocks - signc - signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 5: // op amp
				x.in1  = location - cScale*(signc*x_blocks + signc + signc);
				x.in3  = location + cScale*(signc*x_blocks - signc - signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 6: // transistor
				x.in2  = location - cScale*(signc + signc);
				x.out1 = location + cScale*(-signc*x_blocks + signc + signc);
				x.out3 = location + cScale*(signc*x_blocks + signc + signc);
				return x;
			
			case 7: // input, power, ground
				x.out2 = location;
				return x;
		
			case 8:  // output
				x.in2 = location;
				return x;
				
			case 9: // 3-in gates
				x.in1  = location - cScale*(x_blocks + signc + signc);
				x.in2  = location - cScale*(signc + signc);
				x.in3  = location + cScale*(x_blocks - signc - signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 10: // flipflops
				x.in1  = location - cScale*(x_blocks + signc + signc);
				x.in2  = location - cScale*(signc + signc);
				x.in3  = location + cScale*(x_blocks + signc + signc);
				x.out1 = location + cScale*(-x_blocks + signc + signc);
				x.out2 = location + cScale*(signc + signc);
				x.out3 = location + cScale*(x_blocks + signc + signc); 
				return x;
				
			case 11: // dependent sources
				if (flags & ROTATED) {
					x.in1  = location - cScale*(x_blocks + x_blocks + 1); 
					x.in2  = location + cScale*(x_blocks + x_blocks); 
					x.out1 = location + cScale*(x_blocks + x_blocks - 1);
					x.out2 = location - cScale*(x_blocks + x_blocks);
				}
				else {
					x.in1  = location - cScale*(x_blocks + 2); 
					x.in2  = location - cScale*2; 
					x.out1 = location + cScale*(-x_blocks + 2);
					x.out2 = location + cScale*2;
				}		
				return x;
				
			case 12: // tri-state buffer/inverter
				x.in1  = location - cScale*(2*signc*x_blocks + signc + signc);
				x.in2  = location - cScale*(signc + signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 13: // mux
				x.in1  = location - cScale*(2*signc*x_blocks + signc + signc);
				x.in2  = location - cScale*(x_blocks + signc + signc);
				x.in3  = location + cScale*(x_blocks - signc - signc);
				x.out2 = location + cScale*(signc + signc);
				return x;
			
			case 14: // adder
				x.in2  = location - cScale*(2*signc*x_blocks + signc + signc); // Cin
				x.in1  = location - cScale*(x_blocks + signc + signc);
				x.in3  = location + cScale*(x_blocks - signc - signc);
				x.out2 = location + cScale*(signc + signc);
				x.out3 = location + cScale*(signc + signc + 2*signc*x_blocks); // Cout
				return x;
				
			case 15: // transformer
				if (flags & ROTATED) {
					x.in1  = location + cScale*(x_blocks - 2);
					x.in3  = location + cScale*(x_blocks + 2);
					x.out1 = location - cScale*(x_blocks + 2);
					x.out3 = location - cScale*(x_blocks - 2);
				}
				else {
					x.in1  = location - cScale*(x_blocks + x_blocks + 1);
					x.in3  = location + cScale*(x_blocks + x_blocks - 1);
					x.out1 = location - cScale*(x_blocks + x_blocks - 1);
					x.out3 = location + cScale*(x_blocks + x_blocks + 1);
				}	
				return x;
				
			case 16: // 4-input gates
				x.in1  = location - cScale*(x_blocks + signc + signc);
				x.in2  = location - cScale*(signc + signc);
				x.in3  = location + cScale*(x_blocks - signc - signc);
				x.out1 = location - cScale*(2*signc*x_blocks + signc + signc); // in 4
				x.out2 = location + cScale*(signc + signc);
				return x;
				
			case 17: // delay element
				if (flags & ROTATED) {
					x.in2  = location - cScale*x_blocks;
					x.out2 = location + cScale*x_blocks;
				}
				else {
					x.in2  = location - cScale*(signc);
					x.out2 = location + cScale*(signc);	
				}
				return x;
		}
	}
	order = get_order(component_id);

	if (order == flipflopx) { // flipflop
		x.in1  = location - x_blocks - signc - signc;
		x.out1 = location - x_blocks + signc + signc;
		x.out3 = location + x_blocks + signc + signc;
		
		if (component_id != PRSL_ID && component_id != NRSL_ID)
			x.in2 = location - signc - signc;		
		if (component_id != PDFF_ID && component_id != NDFF_ID && component_id != PTDFF_ID && component_id != NTDFF_ID && component_id != PTFF_ID && component_id != NTFF_ID)
			x.in3 = location + x_blocks - signc - signc;	
		if (component_id > NTFF_ID)
			x.out2 = location + signc + signc;		
		return x;
	}	
	if (order == transistorx) { // transistor
		x.in2  = location - signc - signc;
		x.out1 = location - signc*x_blocks + signc + signc;
		x.out3 = location + signc*x_blocks + signc + signc;
		return x;
	}
	return x;
}




void set_all_connections(connections &x, int val) {

	x.in1 = x.in2 = x.in3 = x.out1 = x.out2 = x.out3 = val;
}
	
	



string get_name(int component_id, int show_name) {

	int order;

	if (component_id <= MAX_USED_ID) {
		component_name = comp_name[component_id];
	}
	else {
		order = get_order(component_id);

		if (order == flipflopx) // flipflop
			component_name = get_name_of_ff(component_id - FF_base_id);
	
		else if (order == transistorx) { // transistor
			transistor tr;
			tr = Qnum_find(tr, component_id - Q_base_id);
			if (show_name)
				cout << add_spaces(tr.name, tr.name.length());
			component_name = " Transistor";
		}
		else if (order == icx) { // ic
			IC ic;
			find_ic(ic, (component_id - IC_base_id));
			if (show_name)
				cout << add_spaces(ic.name, ic.name.length());
			component_name = " IC";
		}
		else if (order == wirex)
			component_name = "Wire";	
		else
			component_name = "Unknown";
	}
	return component_name;
}





int get_order(int component_id) {
	
	if (component_id >= Wire_base_id && component_id < Wire_max_id) // custom wire
		return wirex;
	
	if (component_id <= 0)
		return -1;
	
	if (component_id < INPUT_ID || (component_id > CUSWIRE_ID && component_id < FF_base_id)) // standard components
		return 3;
		
	if ((component_id >= INPUT_ID && component_id <= CUSWIRE_ID) || component_id == ST_SEL_ID) // I/O
		return 8;
		
	if (component_id >= FF_base_id && component_id <= FF_max_id) // flipflop
		return flipflopx;
		
	if (component_id > Q_base_id && component_id <= Q_max_id) // transistor
		return transistorx;
		
	if (component_id >= IC_base_id && component_id <= IC_max_id) // ic
		return icx;
	
	return -1;
}





unsigned char *get_units(int id, double value, int for_drawing) {
			
	units[1] = units[2] = units[3] = units[4] = units[5] = 0;	
	units[0] = ' ';

	if (!has_value(id))
		return get_units_multiplier(value, units, for_drawing);
	
	if ((id == PFET_ID || id == NFET_ID) && mos_delay_not_wl) {
		comp_units = "  D";
	}
	else if (id >= 0 && id <= MAX_USED_ID) {
		comp_units = cunits[id];
	}
	else {
		comp_units = " ";
	}
	if (comp_units == " ") {
		return get_units_multiplier(value, units, for_drawing);
	}
	#ifdef xWIN32
	if (comp_units == "  Ohm" && !for_drawing)
		strcpy((char *)units, OHM_TEXT);
	else
	#endif
		for (unsigned int i = 0; i < comp_units.length() && i < 6; ++i) {
			units[i] = comp_units[i];
		}
	return get_units_multiplier(value, units, for_drawing);
}





unsigned char *get_units_multiplier(double value, unsigned char *units, int for_drawing) {

	double scaled;
		
	units[1] = get_units_and_scale(value, scaled);
	
	#ifdef xWIN32
	if (for_drawing && units[1] == 230)
		units[1] = 'u';
	#endif

	return units;
}




void init_and_draw_circuit(component *& head, string name, int pins, string description, int organize) {

	if (is_ic) {
		x_screen = x_blocks;
		y_screen = y_blocks;
	}
	else {
		x_screen = x_screen_real;
		y_screen = y_screen_real;
	}
	init_buttons(1);
	draw_circuit(head, name, pins, description, organize);

	x_screen = x_blocks;
	y_screen = y_blocks;
	needs_redraw = 0;
}




void draw_circuit(component *& head, string name, int pins, string description, int organize) {
	
	int loc, offset, top_limit, bot_limit;
	component *temp;

	text_size = 12;
	
	if (organize)
		head = organize_components(head);
		
	if (is_ic)
		draw_IC_ON_SCREEN(name, pins, description);

	line_weight = LINE_THICKNESS;
	top_limit   = x_blocks*(y_blocks + window_overlap);
	bot_limit   = -window_overlap*x_blocks;
	o_size      = (int)(sign(o_size)*6*object_size/sqrt((double)x_blocks*y_blocks));
	offset      = x_blocks*sch_origin_y + sch_origin_x;
	temp        = head;
	
	while (temp != 0) {
		loc = temp->location - offset;
		
		if (get_order(temp->id) == wirex || (loc >= bot_limit && (show_hierarchy || (loc < top_limit && (temp->flags & BASE_CIRCUIT))))) {
			temp = draw_individual_component(temp, temp->id, temp->location, temp->value, temp->flags);
			if (show_values_on_screen == 1) {
				SetDrawingColorx(BLACK); // this needs to be here to make the font look right (for some uknown reason)
				show_values(temp);
			}
		}
		temp = temp->next;
	}
	if (show_comp_numbers)
		draw_component_numbers(head);

	line_weight = 1;
}




void draw_component_numbers(component *head) {
	
	int ID, show, offset, one_unit, number[NUM_COMP_NUMS+1], loc, loc_offset, order;
	unsigned i;
	char display[MAX_SAVE_CHARACTERS+1] = {0};
	
	drawing_color     = DKGREY;
	sch_origin_grid_x = sch_origin_x;
	sch_origin_grid_y = sch_origin_y;
	loc_offset        = x_blocks*sch_origin_y + sch_origin_x;
	
	for (i = 0; i <= NUM_COMP_NUMS; ++i) {
		number[i] = 1;
	}
	while (head != 0) {
		loc = head->location - loc_offset;

		if ((show_hierarchy && loc >= x_blocks) || (is_in_window(loc) && (head->flags & BASE_CIRCUIT))) {	
			show = offset = 1;
			ID   = head->id;
			
			if (head->flags & ROTATED)
				one_unit = 1;
			else
				one_unit = x_blocks;
			
			if (head->flags & BASE_CIRCUIT) {
				if (IS_LOGIC_GATE(ID)) {
					sprintf(display, "G%i", number[14]++);
					one_unit = 0;
					show     = 2;
				}
				else {
					switch (ID) {					
						case RES_ID:
							sprintf(display, "R%i", number[0]++);
							break;
							
						case CAP_ID:
							sprintf(display, "C%i", number[1]++);
							break;
							
						case INDUCTOR_ID:
							sprintf(display, "L%i", number[2]++);
							if (!(head->flags & ROTATED))
								++offset;
							break;
							
						case XFMR_ID:
							sprintf(display, "T%i", number[3]++);
							++offset;
							break;
							
						case V_SOURCE_ID:
							sprintf(display, "V%i", number[4]++);
							++offset;
							break;
							
						case I_SOURCE_ID:
							sprintf(display, "I%i", number[5]++);
							++offset;
							break;
							
						case BATTERY_ID:
							sprintf(display, "B%i", number[6]++);
							++offset;
							break;
							
						case VCVS_ID:
							sprintf(display, "VCVS%i", number[7]++);
							++offset;
							break;
							
						case VCCS_ID:
							sprintf(display, "VCCS%i", number[8]++);
							++offset;
							break;
							
						case CCVS_ID:
							sprintf(display, "CCVS%i", number[9]++);
							++offset;
							if (head->flags & ROTATED)
								++offset;
							break;
							
						case CCCS_ID:
							sprintf(display, "CCCS%i", number[10]++);
							++offset;
							if (head->flags & ROTATED)
								++offset;
							break;
							
						case OPAMP_ID:
							sprintf(display, "OA%i", number[11]++);
							one_unit = x_blocks - 1;
							if (head->flags & ROTATED)
								one_unit += 2;
							break;
							
						case NFET_ID:
						case PFET_ID:
							sprintf(display, "M%i", number[12]++);
							one_unit = x_blocks + 1;
							if (head->flags & ROTATED)
								one_unit -= 2;
							break;
							
						case DIODE_ID:
							sprintf(display, "D%i", number[13]++);
							break;
							
						case T_ST_BUF_ID:
						case T_ST_INV_ID:
							sprintf(display, "G%i", number[14]++);
							one_unit = 0;
							break;
							
						case MUX_ID:
							sprintf(display, "MUX%i", number[15]++);
							one_unit = x_blocks - 2;
							if (head->flags & ROTATED)
								one_unit += 4;
							break;
							
						case ADDER_ID:
							sprintf(display, "AD%i", number[16]);
							++number[16];
							one_unit = x_blocks - 1;
							if (head->flags & ROTATED)
								one_unit += 3;
							break;
							
						case DELAY_ID:
							sprintf(display, "GD%i", number[20]++);
							break;

						case TLINE_ID:
							sprintf(display, "TL%i", number[21]++);
							if (head->flags & ROTATED)
								++one_unit;
							break;
							
						default:
							order = get_order(ID);

							if (order == transistorx) {
								sprintf(display, "Q%i", number[17]++);
								one_unit = x_blocks + 1;
								if (head->flags & ROTATED)
									one_unit += 2;
							}
							else if (order == icx) {
								sprintf(display, "U%i", number[18]++);
								one_unit = x_blocks;
							}
							else if (order == flipflopx) {
								sprintf(display, "FF%i", number[19]++);
								one_unit = x_blocks;
								++offset;
							}
							else if (show == 1)
								show = 0;
					}
				}
				if (show) {	
					draw_text(shift_chars(display), -(head->location - one_unit*offset), 9);
				}
			}
		}	
		head = head->next;
	}
	drawing_color = BLACK;
	sch_origin_grid_x = 0;
	sch_origin_grid_y = 0;
}





void show_values(component *head) {
	
	int offset(x_blocks), ID(head->id), rot_offset;
	double Value(head->value);
	char display[MAX_CHARACTERS + 1] = {0}, display2[MAX_CHARACTERS + 1] = {0};
	
	sch_origin_grid_x = sch_origin_x;
	sch_origin_grid_y = sch_origin_y;
	
	if (has_value(ID) && ID != DIODE_ID) {	
		if (ID == DELAY_ID) {
			if (head->flags & ROTATED)
				offset = 1;
		}
		else {
			if (GV) {
				if (head->flags & ROTATED)
					rot_offset = -1;
				else
					rot_offset = 1;
				
				if (IS_LOGIC_GATE(ID)) 
					offset = rot_offset;
				else if (ID == MUX_ID || ID == ADDER_ID)
					offset = x_blocks + rot_offset;
			}
			if (head->value == 0) {
				draw_VARIABLE(head->location);
				return;
			}
			if (ID == CAP_ID || ID == RES_ID || ID == INDUCTOR_ID) {
				if (head->value < 0) {
					head->value = 0;
					draw_VARIABLE(head->location);
					return;
				}
				else if (head->flags & ROTATED) 
					offset = 2;		
			}
			else if (ID == XFMR_ID) {
				if (head->value < 0) {
					head->value = 0;
					draw_VARIABLE(head->location);
					return;
				}
				else if (head->flags & ROTATED) 
					offset = 3;
				else
					offset = 3*x_blocks;
			}
			else if (ID == BATTERY_ID || ID == V_SOURCE_ID || ID == I_SOURCE_ID) {
				// *** voltage source should be allowed to be 0V ***
				if (head->flags & ROTATED)
					offset = 3 - (ID == BATTERY_ID);
				else
					offset = x_blocks + x_blocks;
					
				Value = fabs(head->value);
			}
			else if (ID >= VCVS_ID && ID <= CCCS_ID) {
				if (head->flags & ROTATED)
					offset = 2;
				else
					offset = x_blocks;
					
				Value = fabs(head->value);
			}
			else if (ID == NFET_ID || ID == PFET_ID) {
				offset = x_blocks + x_blocks;
				Value = fabs(head->value);
			}
			else if (ID == TLINE_ID) {
				if (head->flags & ROTATED)
					offset = 1;
				else
					offset = -x_blocks + 2;
			}
		}
		sprintf(display, "%f", range_value(Value));
		
		if (head->value > 0)
			display[6] = 0;
		else
			display[7] = 0;
			
		strcat(display, (char*)get_units(head->id, head->value, 1));
		shift_and_remove_zeros(display, display2);
		draw_text(display2, -(head->location + offset), VALUE_TEXT_SIZE);
	}
	sch_origin_grid_x = 0;
	sch_origin_grid_y = 0;
}





component *update_circuit(int id, int location, component *head, double state, double value) {
	
	int max_loc;
	char flags = BASE_CIRCUIT;

	if (id < 0) {
		if (IS_ROT90(id)) {
			flags |= ROT90;
		}
		else {
			flags |= ROT180;
		}
		id = -id;
	}
	if (id == CUSWIRE_ID || id == NO_ID) 
		return head;
		
	if (location < 1) {
		cerr << "Error: Invalid position: " << location << "." << endl;
		return head;
	}
	max_loc = max(max_boxes, new_num.h_boxes*new_num.v_boxes);
		
	if (is_ic)
		max_loc *= max_ic_windows; // max_total_boxes
	else
		max_loc *= max_main_circuit_windows;
		
	if (location >= max_loc || (get_order(id) == wirex && id >= Wire_base_id + max_loc)) {
		if (!surpress_update_error) {
			beep();
			cerr << "\nWarning: Not enough allocated circuit space to update circuit or the circuit is invalid. Component not entered in circuit." << endl; 
			cout << "\nLocation = " << location << ", max location = " << max_loc << ", max_total_boxes = " << max_total_boxes;
			if (is_ic)
				cout << ", max_ic_windows = " << max_ic_windows << endl;
			else
				cout << ", max_cir_windows = " << max_main_circuit_windows << endl;				
		}
		++update_error;
		if (update_error > MAX_ERROR_MESSAGES)
			surpress_update_error = 1;
		return head;
	} 	
	if (!check_placement(head, id, location, state)) {		
		if (num_iterations == 0 && value == 0) {
			value = get_value(id);
		}
		if (component *new_component = new component) {				
			new_component->id       = id;
			new_component->location = location;
			new_component->state    = (char)state;
			new_component->value    = value;
			new_component->flags    = flags;
			new_component->next     = head;
			
			if (get_order(new_component->id) == transistorx) {
				new_component->value = get_q_type(new_component->id);
			}
			if (!surpress_show_values) {
				show_values(new_component);
			}			
			return new_component;
		}
		else {
			out_of_memory();
		}
	}
	return head;
}





int check_placement(component *head, int id, int location, double state) {
	
	int errorA, errorB;
	
	if (num_iterations == 0 && !skip_placement_check)
		Error_check(head, id, location, state, errorA, errorB);
	else 
		return 0;
	
	if (errorA == 1)  
		conflict = 1;
		
	else if (num_iterations == 0 && errorB == 1) {
		conflict = 2;
		#ifdef xMAC_OS
			draw_char('X', (location + x_blocks - 1), 1, 60); // Mac
		#endif
		#ifdef xWIN32
			draw_char('X', (location - 2*x_blocks - 1), 1, 60); // Win32
		#else
			draw_char('X', (location + x_blocks - 1), 1, 60); // XWindows ?
		#endif
		cout << "\nComponent cannot be placed here!" << endl;
	}
	return (errorA || errorB);	
}





void Error_check(component *temp, int id, int location, double state, int &errorA, int &errorB) {
	
	int tid, order, torder, order_exp, id_in_range, id_m_wbid, loc_p_wbid, same_ends;
		
	errorA = errorB = 0;
	order = get_order(id);
	id_m_wbid = id - Wire_base_id;
	loc_p_wbid = location + Wire_base_id;
	same_ends = (location == id_m_wbid);
	order_exp = (order != 0 && order != wirex && order != 8);
	id_in_range = (id <= FF_base_id && id >= INPUT_ID);
	
	if (order == wirex && same_ends) 
		errorB = 1;
	
	while (temp != 0) {
		if (temp->flags & BASE_CIRCUIT) {
			tid = temp->id;
			torder = get_order(tid);
			
			if (temp->location == location) {
				if (!errorB && ((tid == id && (id_in_range || (torder == wirex && order == wirex))) || ((id == POWER_ID && tid == GROUND_ID) || (id == GROUND_ID && tid == POWER_ID)) || (((tid == id && temp->state != state) || (order_exp && torder != 0 && torder != 8 && torder != wirex))))) 
					errorB = 1;
			
				if (!errorA && temp->state != 1000 && temp->state != state && ((tid == INPUT_ID || tid == POWER_ID || tid == GROUND_ID)))
					errorA = 1;
			}
			if (!errorB && (order == wirex && ((loc_p_wbid == tid && temp->location == id_m_wbid) || (same_ends && temp->location == (tid - Wire_base_id)))))
				errorB = 1;
			
			if (errorA && errorB)
				return;
		}
		temp = temp->next;
	}
}



// for future use in testing and debugging
int circuit_check(component *head, int show_error, int is_top_level) {

	int status(1), errorA, errorB, id, loc, max_loc;
	
	if (head == 0)
		return -1;
	
	max_loc = max(max_boxes, new_num.h_boxes*new_num.v_boxes);
		
	if (is_ic)
		max_loc *= max_ic_windows; // max_total_boxes
	else
		max_loc *= max_main_circuit_windows;
	
	while (head != 0) {
		errorA = errorB = 0;
		id = head->id;
		loc = head->location;
		if (loc < 1 || loc >= max_loc || (get_order(id) == wirex && id >= Wire_base_id + max_loc)) {
			if (show_error)
				cerr << "Location Error: id = " << id << ", loc = " << loc << endl;
			status = 0;
		}
		if (id > Wire_max_id) {
			if (show_error)
				cerr << "Id Error: id = " << id << ", loc = " << loc << endl;
			status = 0;
		}
		if (is_top_level && !(head->flags & BASE_CIRCUIT)) {
			if (show_error)
				cerr << "Circuit Id Error: id = " << id << ", loc = " << loc << ", flags = " << (unsigned)(head->flags) << endl;
			status = 0;
		}
		Error_check(head->next, id, loc, head->state, errorA, errorB);
		if (errorA || errorB) {
			if (show_error)
				cerr << "Placement Error: id = " << id << ", loc = " << loc<< ", state = " << head->state << endl;
			status = 0;
		}
		head = head->next;
	}
	return status;
}






double get_value(int id) {

	double value;	
	
	if (!has_value(id))
		return 0;
	
	if (GV && (IS_LOGIC_GATE(id) || id == ADDER_ID || id == MUX_ID)) {
		cout << "\nEnter the delay of this " << get_name(id, 0) << ":  ";
		value = ind();
		if (value < 0)
			value = 0;
		cout << value << endl;
		return value;
	}	
	switch(id) {
	
		case CAP_ID:
			cout << "\nEnter the value of this Capacitor in Farads(-# or 0 for variable):  ";
			break;
			
		case RES_ID:
			cout << "\nEnter the value of this Resistor in Ohms(-# or 0 for variable):  ";
			break;
			
		case DIODE_ID:
			cout << "\nEnter the voltage drop VDon for this diode(0 to use default of " << diode_vdrop << "V, -# to reverse orientation):  ";
			break;
			
		case BATTERY_ID:
			cout << "\nEnter the value of this Battery in Volts(0 for variable):  ";
			break;
			
		case INDUCTOR_ID:
			cout << "\nEnter the value of this Inductor in Henrys(-# or 0 for variable):  ";
			break;
			
		case XFMR_ID:
			cout << "\nEnter the value of this Transformer's mutual inductance in Henrys(-# or 0 for variable):  ";
			break;
			
		case V_SOURCE_ID:
			cout << "\nEnter the value of this Voltage Source in Volts(0 for variable):  ";
			break;
			
		case I_SOURCE_ID:
			cout << "\nEnter the value of this Current Source in Amps(0 for variable):  ";
			break;
			
		case VCVS_ID:
		case CCVS_ID:
		case VCCS_ID:
		case CCCS_ID:
			cout << "\nEnter the Multiplier(0 for variable):  ";
			break;
			
		case NFET_ID:
		case PFET_ID:
			if (mos_delay_not_wl) {
				cout << "\nEnter the delay of this MOSFET:  ";
			}
			else {
				cout << "\nEnter the W/L ratio of this MOSFET(-# or 0 for variable):  ";
			}
			break;
			
		case DELAY_ID:
			cout << "\nEnter the delay value:  ";
			break;

		case TLINE_ID:
			cout << "\nEnter the width of this Transmission Line(0 for variable):  ";
			break;
	}
	value = ind();
		
	if (value > 0 || (value != 0 && (id == BATTERY_ID || id == V_SOURCE_ID || id == I_SOURCE_ID || id == VCVS_ID || id == VCCS_ID || id == CCVS_ID || id == CCCS_ID))) {	
		cout << range_value(value);
		print_ch(get_units(id, value, 0));
		cout << endl;
	}
	else if (id == DIODE_ID && value == 0) {
		cout << range_value(diode_vdrop);
		print_ch(get_units(id, diode_vdrop, 0));
		cout << endl;
		return diode_vdrop;
	}
	else if (id != DIODE_ID) {
		return 0;
	}
	return value;
}





double range_value(double value1) {
	
	int count;
	
	for(count = 0; fabs(value1) > 999.999 && count < 3; value1 /= 1000) {
		++count;
	}
	for(count = 0; fabs(value1) < .999999 && count < 5; value1 *= 1000) {
		++count;
	}
	return value1;
}





int has_value(int id) {
	
	return (abs(id) <= MAX_USED_ID && (has_val[abs(id)] == 1 || (GV && has_val[abs(id)] == 2)));
}




int can_be_rotated(int id) {

	int order = get_order(id);

	return (order != flipflopx && order != icx && order != wirex && id != INPUT_ID && id != OUTPUT_ID && id != CUSWIRE_ID);
}





void set_circuit_prefs() {  
	
	int position;
	
	if (freq == -1)
		freq = 0;
	
	do {
		draw_circuit_pref_window();
		
		position = box_clicked();
						
		switch (position) {
		
			case 126:
				if (grid_color == 8)
					grid_color = 9;
				else
					grid_color = 8;
				break;
			
			case 232:
				freq = !freq;
				break;
				
			case 338:
				if (DEMO) {
					demo_error();
					save_pref = 0;
				}
				save_pref = !save_pref;
				break;
				
			case 444:
				delete_pref = !delete_pref;
				break;
				
			case 550:
				need_node = !need_node;
				break;
				
			case 153:
				GV = !GV;
				break;
				
			case 259:
				useDELAY = !useDELAY;
				break;
				
			case 365:
				if (recalculate == 1)
					recalculate = 2;
				else if (recalculate == 2 || recalculate == 0)
					recalculate = 1;
				break;
				
			case 471:
				if (recalculate == 0)
					recalculate = 2;
				else if (recalculate == 2 || recalculate == 1)
					recalculate = 0;
				break;
				
			case 577:
				force_init_states = !force_init_states;
				break;
				
			case 683:
				dont_print_tt = !dont_print_tt;
				break;
				
			case 789:
				use_rubberbanding = !use_rubberbanding;
				break;
				
			// 895	
				
			case 645:
			case 646:
			case 647:
			case 648:
				cout << "\nEnter new supply voltage:  ";
 				supply_voltage = ind();
 				if (supply_voltage < ground)
 					cout << "Warning: Supply voltage is less than ground but will represent a logic high anyway." << endl;
 				mark_all_changed();
				break;
				
			case 750:
			case 751:
			case 752:
			case 753:
				cout << "\nEnter new ground:  ";
 				ground = ind();
 				if (supply_voltage < ground)
 					cout << "Warning: Ground is greater than supply voltage but will represent a logic low anyway." << endl;
 				mark_all_changed();
				break;
		
			case 896:
			case 897:
			case 898:
			case 949:
			case 950:
			case 951:
			case 1002:
			case 1003:
			case 1004:
				position = -1;
				break;
		}
		
	} while (position != -1);
}





void draw_circuit_pref_window() { 

	static int const bullet_offset = 20;
	static int const column1 = 1;
	static int const column2 = 28;	
	point xy0, xy1, xy2, xy3;
	
	init_menu(1000, WHITE, 0);
	text_size = 0;
	
	draw_text(" Show Circuit Grid ---------------------------", -(2*x_screen  + column1), -12);
	draw_text(" Display frequency in rad/sec instead of Hz --", -(4*x_screen  + column1), -12);
	draw_text(" Ask to Save on Close ------------------------", -(6*x_screen  + column1), -12);
	draw_text(" Ask Before Deleting Components --------------", -(8*x_screen  + column1), -12);
	draw_text(" Show Circuit Nodes --------------------------", -(10*x_screen + column1), -12);
	draw_text(" (+)Power in Volts:", -(12*x_screen + 1), -12);
	draw_text(" Ground in Volts:", -(14*x_screen + 1), -12);
	
	// logic engine stuff
	draw_text(" Logic Gates Have Delay Values ---------------", -(2*x_screen  + column2), -12);
	draw_text(" Calculate Gate Delays for Logic (slower) ----", -(4*x_screen  + column2), -12);
	draw_text(" Initial States are Read Only ----------------", -(10*x_screen + column2), -12);
	draw_text(" Don't Print Truth Table on Screen -----------", -(12*x_screen + column2), -12);
	
	draw_text(" Allow Rubberbanding When Moving Components --", -(14*x_screen + column2), -12);
	
	// recalculate stuff
	draw_text(" Always Recalculate Circuit ------------------", -(6*x_screen  + column2), -12);
	draw_text(" Never Recalculate Circuit -------------------", -(8*x_screen  + column2), -12);
	
	draw_text(" OK", -(16*x_screen + 48), -16);
	
	xy0 = get_center((15*x_screen  + 47));
	xy1 = get_center((16*x_screen  + 47));
	xy2 = get_center((17*x_screen  + 50));
	xy3 = get_center((18*x_screen  + 50));
	
	SetDrawingColorx(BLACK);
	RectangleFrame(xy1.x, (xy0.y + xy1.y)/2, xy2.x, (xy2.y + xy3.y)/2);
	
	dont_whiteout_double = 1;
	draw_double(supply_voltage, 'V', -(12*x_screen + 9)); 
	draw_double(ground, 'V', -(14*x_screen + 8));
	dont_whiteout_double = 0;
	
	if (grid_color == 8)
		draw_FBULLET(2*x_screen + bullet_offset);
	else
		draw_BULLET(2*x_screen  + bullet_offset);
	
	if (freq == 1)
		draw_FBULLET(4*x_screen + bullet_offset);
	else
		draw_BULLET(4*x_screen  + bullet_offset);
		
	if (save_pref)
		draw_FBULLET(6*x_screen + bullet_offset);
	else
		draw_BULLET(6*x_screen  + bullet_offset);
		
	if (delete_pref)
		draw_FBULLET(8*x_screen + bullet_offset);
	else
		draw_BULLET(8*x_screen  + bullet_offset);
		
	if (need_node)
		draw_FBULLET(10*x_screen + bullet_offset);
	else
		draw_BULLET(10*x_screen  + bullet_offset);
		
	// logic engine stuff
	if (GV)
		draw_FBULLET(2*x_screen + (column2 - column1) + bullet_offset);
	else
		draw_BULLET(2*x_screen  + (column2 - column1) + bullet_offset);
	
	if (useDELAY)
		draw_FBULLET(4*x_screen + (column2 - column1) + bullet_offset);
	else
		draw_BULLET(4*x_screen  + (column2 - column1) + bullet_offset);
		
	if (force_init_states)
		draw_FBULLET(10*x_screen + (column2 - column1) + bullet_offset);
	else
		draw_BULLET(10*x_screen  + (column2 - column1) + bullet_offset);
	
	if (dont_print_tt)
		draw_FBULLET(12*x_screen + (column2 - column1) + bullet_offset);
	else
		draw_BULLET(12*x_screen  + (column2 - column1) + bullet_offset);
	
	if (use_rubberbanding)
		draw_FBULLET(14*x_screen + (column2 - column1) + bullet_offset);
	else
		draw_BULLET(14*x_screen  + (column2 - column1) + bullet_offset);
		
	// recalculate stuff	
	if (recalculate == 0) {
		draw_BULLET(6*x_screen  + (column2 - column1) + bullet_offset);
		draw_FBULLET(8*x_screen + (column2 - column1) + bullet_offset);
	}
	else if (recalculate == 1) {
		draw_FBULLET(6*x_screen + (column2 - column1) + bullet_offset);
		draw_BULLET(8*x_screen  + (column2 - column1) + bullet_offset);
	}
	else {
		draw_BULLET(6*x_screen + (column2 - column1) + bullet_offset);
		draw_BULLET(8*x_screen + (column2 - column1) + bullet_offset);
	}	
	text_size = 12;
}





void set_AC_circuit_prefs() {

	draw_AC_circuit_pref_window();
}





void draw_AC_circuit_pref_window() {

	prefs();
}




void print_circuit_info(component *head, int flag) { // for debugging

	cout << "\nCircuit (" << flag << ") is as follows: " << endl;
	cout << "name, id, location, value" << endl;

	while (head != 0) {
		cout << get_name(head->id, 0) << " ";
		if (head->flags & ROTATED)
			cout << "-";
		cout << head->id << " " << head->location << " " << head->value << endl;
		head = head->next;
	}
}




component *revert(component *head) {

	if (!circuit_has_changed()) {
		cout << "\nCircuit has not been changed since the last save." << endl;
		return head;
	}
	if (is_ic)
		string_to_char(NAME, ic_name, MAX_SAVE_CHARACTERS);
	else if (current_name[0] != '.' || current_name[1] != 0)
		strcpy(NAME, current_name);
	else {
		cout << "\nThere is no saved file to revert to." << endl;
		return head;
	}
	if (head != 0) {
		cout << "\nAre you sure you want to revert to the last saved version of this circuit?  ";
		if (!decision())
			return head;
	}
	OPEN = 1;
	head = open_circuit(head);
	needs_redraw = 1;
	changed = changed2 = 0;
	
	return head;
}





void save_circuit(component *& head) {
		
	while (!save_cir(head)) {
		cout << "\nSave circuit?  ";
		if (!decision())
			return;
	}
}





int save_cir(component *& head) {

	ofstream outfile;
	char sc_name[MAX_SAVE_CHARACTERS + 1] = {0};
	char window_title[MAX_SAVE_CHARACTERS + def_win_title_len] = {0};
	int save_as_ic(0), name_flag(0);
	component *temp;
			
	if (DEMO) {
		demo_error();
		return 1;
	}
	if (head == 0) {
		beep();
		cout << "\nThere is no circuit to save!" << endl;
		return 1;
	}	
	if (is_ic && (ic_name == "_NEW" || ic_name == " NEW" || ic_name == "_FIND" || ic_name == " FIND" || ic_name == "_Unknown" || ic_name == " Unknown")) {
		SelectConsoleWindow();
		cout << "\nEnter name for IC:  ";
		cin >> ws;
		ic_name = xin(MAX_SAVE_CHARACTERS);
		ic_name = shift_chars(ic_name);
		ic_name = remove_spaces(ic_name, ic_name.length());
	}
	if (!SAVE || (!is_ic && (current_name[0] == '.' && current_name[1] == 0))) {		
		SelectConsoleWindow();
		cout << "\n\nSave circuit as(max 31 characters";
	
		if (is_ic) {
			ic_name[0] = '_';
			cout << ", enter \".\" to save as " << ic_name;
		}
		else {
			cout << ", enter \"IC\" to save as an IC";
	
			if (current_name[0] != '.' || current_name[1] != 0) {
				name_flag = 1;
				cout << ", enter \".\" to save as ";
				print_ch((unsigned char *)current_name);
			}
		}		
		cout << ", enter \"Netlist\" to export as a netlist, or enter \"Text\" to write as text graphics):  ";
		cin >> ws;					
		strcpy(sc_name, xin(MAX_SAVE_CHARACTERS));
		
		SAVE = 0;
	}
	if (SAVE || (name_flag && sc_name[0] == '.' && sc_name[1] == 0)) {
		strcpy(sc_name, current_name);
		name_flag = 2;
	}
	if (is_ic) {
		if (SAVE || (sc_name[0] == '.' && sc_name[1] == 0)) 
			string_to_char(sc_name, ic_name, MAX_SAVE_CHARACTERS);
			
		ic_name[0] = ' ';
	}
	else if (strcmp(sc_name, "netlist") == 0 || strcmp(sc_name, "Netlist") == 0 || strcmp(sc_name, "NETLIST") == 0) {
		export_netlist(head);
		return 1;
	}
	else if (strcmp(sc_name, "text") == 0 || strcmp(sc_name, "Text") == 0 || strcmp(sc_name, "TEXT") == 0) {
		write_circuit_as_text(head);
		return 1;
	}
	else if (strcmp(sc_name, "ic") == 0 || strcmp(sc_name, "IC") == 0) {
		save_as_ic = 1;
		SelectConsoleWindow();
		cout << "\n\nSave IC as(max 31 characters):  ";
		cin >> ws;					
		strcpy(sc_name, xin(MAX_SAVE_CHARACTERS));
		ic_name = shift_chars(sc_name);
		ic_name = remove_spaces(ic_name, ic_name.length()); 
	}	
	if (name_flag != 2 && (strcmp(current_name, sc_name) != 0) && !overwrite_file(sc_name)) 		
		return 0;
		
	if (is_ic && sc_name[0] == '_') {
		if (!outfile_file(outfile, sc_name, 0)) {
			beep();
			cerr << "\nError: Circuit could not be saved as " << sc_name << "!" << endl;		
			return 0;
		}
	}
	else {	
		outfile.open(sc_name, ios::out | ios::trunc);
	
		if (outfile.fail() || !filestream_check(outfile)) {
			beep();
			cerr << "\nError: Circuit could not be saved as " << sc_name << "!" << endl;		
			return 0;
		}
	}
	show_clock();
	
	if (!is_ic && !save_as_ic)	{
		if (GV)	
			outfile << "Circuit_Solver_Saved_Circuit_GV" << endl << "xxx" << endl;
		else
			outfile << "Circuit_Solver_Saved_Circuit" << endl << "xxx" << endl;
		outfile << supply_voltage << " " << ground << endl;			
	}
	else {
		ic_name[0] = '_';
		if (GV)	{
			if (looks_like_IC)
				outfile << "Circuit_Solver_Saved_IC_GV";
			else
				outfile << "Circuit_Solver_Saved_Subcircuit_GV";
		}
		else {
			if (looks_like_IC)
				outfile << "Circuit_Solver_Saved_IC";
			else
				outfile << "Circuit_Solver_Saved_Subcircuit";
		}	
		outfile << endl << ic_name << endl;
		ic_name[0] = ' ';
	}
	if (saveBOXES)
		outfile << x_blocks << " " << -y_blocks << endl;

	temp = head;
	
	strcpy(window_title, init_window_title);
	set_DW_title(strcat(window_title, sc_name));
	title_set = 1;
	
	if (!is_ic)
		strcpy(current_name, sc_name);

	if (!filestream_check(outfile)) {
		return 0;
	}
	while (temp != 0) {			
		if (temp->location >= 0 && (allow_save_off_screen || is_in_window(temp->location))) {
			if (temp->flags & ROTATED)
				outfile << "-";
			
			outfile << temp->id << " " << temp->location;
			
			if (has_value(temp->id))
				outfile << " " << temp->value;
				
			outfile << endl;
		
			if (!outfile.good()) {
				cerr << "\nError: Not enough space to save entire circuit!" << endl;
				outfile.close();
				return 0;
			}
		}
		temp = temp->next;
	}			
	outfile.close();
	reset_cursor();
	
	changed = changed2 = 0;
	cout << "\nThe current Circuit Solver circuit has been saved as: " << sc_name << "." << endl;
	
	return 1;		
}




component *open_circuit(component *head) {

	ifstream infile;
	char oc_name[MAX_SAVE_CHARACTERS + 1] = {0};
	char window_title[MAX_SAVE_CHARACTERS + def_win_title_len] = {0};
	string cname_id;
	int ID, LOCATION, input_loc, saved_type(0);
	double value(0);
	
	if (OPEN != 1) {
		SelectConsoleWindow();
		cout << "\n\nOpen circuit named:  ";
		cin  >> ws;
		strcpy(oc_name, xin(MAX_SAVE_CHARACTERS));
	}
	else
		strcpy(oc_name, NAME);
		
	OPEN = 0;
	
	input_loc = infile_file(infile, oc_name);
	
	if (!input_loc) {
		beep();
		cerr << "\nThe Circuit Solver circuit " << oc_name << " could not be opened!" << endl;
		return head;
	}
	new_num.h_boxes = new_num.v_boxes = 0;
	
	if (!filestream_check(infile))
		return head;
	
	infile >> cname_id;

	if (cname_id == "Circuit_Solver_Saved_Circuit")
		saved_type = 1;
	if (cname_id == "Circuit_Solver_Saved_Circuit_GV")
		saved_type = 2;
	if (cname_id == "Circuit_Solver_Saved_IC")
		saved_type = 3;
	if (cname_id == "Circuit_Solver_Saved_IC_GV")
		saved_type = 4;
	if (cname_id == "Circuit_Solver_Saved_Subcircuit")
		saved_type = 5;
	if (cname_id == "Circuit_Solver_Saved_Subcircuit_GV")
		saved_type = 6;
	if (cname_id == "Circuit")
		saved_type = 7;

	switch (saved_type) {
	case 1:
	case 2:
		if (!filestream_check(infile))
			return head;
		
		infile >> cname_id;
		if (!filestream_check(infile))
			return head;
		
		infile >> supply_voltage >> ground;
		if (saveBOXES) {
			if (!filestream_check(infile))
				return head;
			
			infile >> new_num.h_boxes >> new_num.v_boxes;
		}
		GV    = saved_type - 1;
		is_ic = 0;
		break;

	case 3:
	case 4:
	case 5:
	case 6:
		if (!filestream_check(infile))
			return head;
		
		infile >> ic_name;
		switch (get_ic_type(ic_name)) {
			case 1:
			case 3:
			case 1000:
				supply_voltage = cmos_supply;
				break;
			case 2: 
				supply_voltage = TTL_supply;
				break;
			default:
				supply_voltage = default_voltage;
		}
		if (saveBOXES) {
			if (!filestream_check(infile)) 
				return head;
			
			infile >> new_num.h_boxes >> new_num.v_boxes;
		}
		GV            = (saved_type == 4 || saved_type == 6);	
		looks_like_IC = (saved_type == 3 || saved_type == 4);
		ground        = 0;
		is_ic         = 1;
		break;

	case 7:
	default:
		if (saved_type == 7 || (cname_id != "" && (cname_id[0] == '#' || cname_id[0] == '*'))) {
			cout << "\nReading netlist. Components will be auto-placed in the drawing area." << endl;

			if (saveBOXES) {
				init_buttons(1);
				new_num.h_boxes = x_blocks;
				new_num.v_boxes = y_blocks;
				whiteout();
			}
			infile.close();
			infile_file(infile, oc_name); // reset file
			show_clock();
			StringTable net_names(0);
			head = import_netlist(head, infile, net_names, cname_id);
			reset_cursor();
			
			if (head != 0 && graph_enable) {
				strcpy(window_title, init_window_title);
				set_DW_title(strcat(window_title, oc_name)); 
				title_set = 1;
				strcpy(current_name, oc_name);
			}
			Opened = changed4 = 1;
			surpress_update_error = update_error = num_iterations = 0;
			
			return head;
		}
		else {
	 		beep();
			cerr << "\nThe file " << oc_name << " is not a Circuit Solver circuit!" << endl;
			infile.close();
			return head;
		}	
	}
	num_iterations        = 1;
	surpress_update_error = update_error = 0;
	
	destroyL(head);
	head = 0;
		
	if (new_num.v_boxes > 0) {	// actual component - old version without saved box numbers	
		if (has_value(new_num.h_boxes)) {
			if (!filestream_check(infile)) {
				return head;
			}
			infile >> value;
		}
		else {
			value = 0.0;
		}
		ID              = new_num.h_boxes;
		LOCATION        = new_num.v_boxes;
		new_num.h_boxes = 0;
		new_num.v_boxes = 0;
		head = update_circuit(ID, LOCATION, head, 2, value);
	}
	else if (new_num.v_boxes < 0) {
		new_num.v_boxes = -new_num.v_boxes;
	}
	show_clock();
	
	while (infile.good() && infile >> ID >> LOCATION) {		
		if (has_value(ID)) {
			infile >> value;
		}
		else {
			value = 0.0;
		}
		head = update_circuit(ID, LOCATION, head, 2, value);
		
		if (update_error) 
			surpress_update_error = 1;
	}
	reset_cursor();
	
	if (update_error == 1)
		cout << "1 invalid component." << endl;
	else if (update_error > 1)
		cout << update_error << " invalid components." << endl;
		
	if (head != 0)	
		cout << "\nThe circuit has been opened successfully!" << endl;
	else
		cerr << "\nError: The circuit contains no data, or the data is corrupted!" << endl;
	
	infile.close();
	reset_actions();
	
	if (graph_enable) {
		strcpy(window_title, init_window_title);
		if (input_loc == 2) 
			strcat(window_title, data_folder);
		else if (input_loc == 3)
			strcat(window_title, circuit_folder);
			
		set_DW_title(strcat(window_title, oc_name));
		title_set = 1;
	}
	if (!is_ic) {
		strcpy(current_name, "");
		if (input_loc == 2) 
			strcat(current_name, data_folder);
		else if (input_loc == 3)
			strcat(current_name, circuit_folder);
			
		strcat(current_name, oc_name);
	}	
	Opened = changed4 = 1;
	surpress_update_error = update_error = num_iterations = 0;
					
	return head;
}











