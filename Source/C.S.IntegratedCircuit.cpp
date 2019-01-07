#include "CircuitSolver.h"
#include "Logic.h"
#include "Draw.h"
#include "StringTable.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.IntegratedCircuits.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Integrated Circuits
// By Frank Gennari
int const useICStringTable = 1;

string const unknown1 = " ????";
string const unknown2 = " Unknown";

int countdown(0), timer(0), ICdata_size(0), ICdata_capacity(0);
box ic_num;
StringTable IC_names(0);


extern int o_size, object_size, num_buttons, is_ic, Opened, changed, changed2, num_iterations, saveBOXES, storeDATAB, force_begin_in_schematic_window, must_exit, drawing_color, DEMO, surpress_update_error, update_error, GV, looks_like_IC, title_set;
extern int x_blocks, y_blocks, x_screen, y_screen;
extern double supply_voltage, ground;
extern box new_num;
extern string ic_name;
extern IC *ICdata;



special show_ic(special &comp, component *&head, string name, string description, int pins, string old_name, int is_opened);
IC *draw_ic_window(IC *data, int &last_screen, int &last_ic, int screen, int &count, int &count2);
void show_ic_selection();
int fill_ic_window(IC *data, int count);
void draw_ics(IC *data, int count);
component *pinout(component *head, string name, string description, int pins);
component *modify(component *head, string name, int pins, string description);
void specs(string name, int pins, string description);
IC *add_ic(string name, string description, int pins, IC *data, IC *&tail);
void add_ic_to_datafile(string name, string description, int pins);
IC *read_ic_data(int screen, int &last_screen, int &last_ic);
IC ic_search(IC &data, string name1, int use_status_bar);
component *find_ic_circuit(component *head, string name);
int get_ic_pins(int comp_id);
int count_ics();
int count_data(IC *data);
void new_ic();
void no_datafile(void);

void reset_actions();






component *pick_ic(component *head, int opened, string name, int pins, string description) {

	int ic, count, count2, count3(1), screen(1), last_screen, last_ic;
	component *temp = NULL;
	IC *data = NULL, *temp2;
	IC DATA;
	special comp;
	string find_name;
	
	comp.selection = -1;
			
	if (!opened) {
		do {	
			data = draw_ic_window(data, last_screen, last_ic, screen, count, count2);
						
			do {
				cout << "\nChoose an Integrated Circuit." << endl;
	
				ic = box_clicked();
			
				if (ic == 0) { // <-
				
					if (screen == 1) {
						destroyL(data);
						return head;
					}
					else
					--screen;
				}
				else if (ic == (count + 1)) { // ->
				
					if (count == (MAX_BOXES - 2))
						++ screen;
					else {
						destroyL(data);
						return head;						
					}
				}
				else if (ic == 1 && screen == 1) { // new
					if (DEMO) 
						demo_error();
					else {
						cout << "\nNEW" << endl;
						new_ic();
						data = draw_ic_window(data, last_screen, last_ic, screen, count, count2);
					}	
				}
				else if (ic == 2 && screen == 1) { // find
					cout << "\nEnter the name of the IC(case sensitive):  ";
					find_name = xin(MAX_SAVE_CHARACTERS);
					find_name = shift_chars(find_name);
					find_name = remove_spaces(find_name, find_name.length());
					DATA = ic_search(DATA, find_name, 1);
					if (DATA.name != unknown2) {
						comp = show_ic(comp, temp, DATA.name, DATA.description, DATA.pins, name, 0);
						if (comp.selection == 1) {
							destroyL(data);
							if (screen == 1)
								ic -= 2;
							return draw_component_at_click((IC_base_id + countdown + 1), head, name, pins, description);
						}
					}
					else
						cout << "\nThe IC " << find_name << " was not found." << endl;
							
					count2 = fill_ic_window(data, count);
					
					ic = 1;
				}
			} while (ic > (count + 1) || (ic == (count + 1) && count < (MAX_BOXES - 2)) || (ic == 1 && screen == 1));
	
			if (ic != 0 && ic != (count + 1)) {
		
				temp2 = data;
				count3 = 1;
			
				while (count3 < ic && temp2->next != 0) {
			
					++count3;
					temp2 = temp2->next;
				}
				comp = show_ic(comp, temp, temp2->name, temp2->description, temp2->pins, name, 0);
				ic_name = name;
			}	
		} while (comp.selection == 0 || comp.selection == -1);
		
		if (comp.selection == 1) {
			destroyL(data);
			if (screen == 1)
				ic -= 2;
			return draw_component_at_click((IC_base_id + (screen - 1)*(MAX_BOXES - 2) + ic), head, name, pins, description);
		}
	}
	else {
		DATA = ic_search(DATA, ic_name, 1);		
		comp = show_ic(comp, head, DATA.name, DATA.description, DATA.pins, name, 1);
	}
	is_ic = 0;
	
	destroyL(data);
	
	return head;
}




IC *draw_ic_window(IC *data, int &last_screen, int &last_ic, int screen, int &count, int &count2) {

	last_ic = 0;
	last_screen = 1;		
	destroyL(data);
	data = read_ic_data(screen, last_screen, last_ic);
	count = count_data(data);		
	count2 = fill_ic_window(data, count);
	
	return data;
}





int fill_ic_window(IC *data, int count) {

	IC *temp = data;
	int count2(1);
	
	draw_selection_window(count);
			
	while (temp != 0) {	
		draw_ics(temp, count2);		
		++count2;
		temp = temp->next;
	}
	return count2;
}





special show_ic(special &comp, component *&head, string name, string description, int pins, string old_name, int is_opened) {
	
	int needs_saved;
	
	/*ic_name[0] = */old_name[0] = name[0] = ' ';
	
	do {
		init_menu(6, 10, 0);
		
		show_ic_selection();
		
		draw_LARROW(0);
		
		draw_text(name, 1, 12);
		draw_text(description, 2, 12);
		
		cout << "\nClick on a square." << endl;
				
		comp.selection = box_clicked();
				
		switch (comp.selection) {
			
			case 0: // back
				if (is_opened) 
					return comp;
				break;
			
			case 1: // use IC
				if (is_opened) {
					beep();
					cout << "\nThere is no open circuit to add this IC to!" << endl;
					comp.selection = -1;
				}
				else if (name == ic_name) {
					beep();
					cout << "\nAn IC cannot be added to itself!" << endl;
					comp.selection = -1;
				}
				break;
				
			case 2: // pinout
				head = pinout(head, name, description, pins);
				break;
			
			case 3:	// specs
				specs(name, pins, description);
				break;	
			
			case 4:	 // modify
				if (DEMO) {
					demo_error();
					break;	
				}
				if (ic_name == name && !Opened) {
					beep();
					cout << "\nThis IC is already being modified!" << endl;
				}
				else {
					head = modify(head, name, pins, description);
					ic_name = old_name;
				}
				break;
			
			case 5: // new
				if (DEMO) {
					demo_error();
					break;	
				}
				cout << "\nNEW" << endl;
				new_ic();
				break;
		}
		if (!is_ic)
			needs_saved = 1;
		else
			needs_saved = 0;
		
	} while (comp.selection != 0 && comp.selection != 1);
	
	if (!needs_saved)
		changed = 0;
	
	if (comp.selection == 0) {
 		destroyL(head);
 		head = 0;
		comp.head = 0;
	}
	return comp;
}





void show_ic_selection() {

	draw_text(" BACK", reserved_pos_value, 12);
	draw_text(" USE IC", -1, 12);
	draw_text(" PINOUT", -2, 12);
	draw_text(" SPECIFICATIONS", -3, 12);
	draw_text(" MODIFY", -4, 12);
	draw_text(" NEW", -5, 12);
}




void draw_ics(IC *data, int count) {

	draw_LARGE_IC(count, o_size, data->pins);
	draw_IC_PINS(count, o_size, data->pins, 0);
	draw_text(data->name, -count, 12);
}





component *pinout(component *head, string name, string description, int pins) {
	
	int num_boxes(default_num_buttons), was_ic(is_ic), tempcolor, destroy_head(0);
	double temp_vcc(supply_voltage), temp_gnd(ground);
	box temp_num(new_num);
		
	if (head == NULL) {
		head = find_ic_circuit(head, name);
		new_num = ic_num;
		destroy_head = 1;
	}
	else
		ic_num = new_num;
		
	if (ic_num.v_boxes > 0) {
		num_boxes = ic_num.h_boxes*ic_num.v_boxes;
		if (!(ic_num.v_boxes%2))
			num_boxes += ic_num.h_boxes;
	}
	if (num_boxes == default_num_buttons)
		num_boxes -= 7; // Simple fix for when I changed default_num_buttons from 1000 to 1007.
	
	init_menu(num_boxes, 9, 1);
			
	o_size = (int)(30*object_size/sqrt(num_boxes + 4.0));
	
	draw_LARGE_IC(num_boxes/2, (int)(1.25*o_size), pins);
	draw_IC_PINS(num_boxes/2, (int)(1.25*o_size), -pins, 0);
	
	o_size = o_size/30;
	
	draw_text(" Pinout", 1, 12);
	draw_text(description, (int)(.75*x_blocks), 12);
	if (num_boxes != default_num_buttons - 7)
		num_boxes -= 7;
	if (!(pins%4))
		num_boxes -= 7;
		
	draw_text(name, num_boxes/2, 12);
	
	is_ic = 0;
	draw_circuit(head, " ", pins, " ", 1);
	is_ic = was_ic;
	new_num = temp_num;
		
	if (head == 0) {
		tempcolor = drawing_color;
		drawing_color = RED;
		draw_text(" No Model", (int)((y_blocks/2 + 4.5)*x_blocks), 12);
		drawing_color = tempcolor;
	}
	cout << "\nClick to continue." << endl;
	
	wait_for_click();
	
	supply_voltage = temp_vcc;
	ground = temp_gnd;
	
	if (destroy_head) {
		destroyL(head);
		head = 0;
	}
	return head;
}






component *modify(component *head, string name, int pins, string description) {
	
	int Decision, temp_Opened, temp_c, temp_c2, temp_num_buttons(num_buttons), destroy_head(0);
	double temp_vcc, temp_gnd;
	box temp_num = new_num;
	char window_title[MAX_SAVE_CHARACTERS + def_win_title_len] = {0}, char_name[MAX_SAVE_CHARACTERS] = {0};
			
	ic_name = name;
	
	temp_vcc = supply_voltage;
	temp_gnd = ground;
	temp_Opened = Opened;
	temp_c = changed;
	temp_c2 = changed2;
	
	if (head == NULL) {
		head = find_ic_circuit(head, name);
		new_num = ic_num;
		destroy_head = 1;
	}
	else
		ic_num = new_num;
	
	string_to_char(char_name, ic_name, ic_name.length());
	strcpy(window_title, init_window_title);
	set_DW_title(strcat(window_title, char_name));
	title_set = 1;
	
	reset_actions();
	num_buttons = ic_num.h_boxes*ic_num.v_boxes;
	init_menu(num_buttons, 9, 1);
				
	++is_ic;
	changed2 = 0;
	force_begin_in_schematic_window = 1;

	do {	
		head = select_component(head, name, pins, description);
		supply_voltage = default_voltage;
		Decision = must_exit = 0;
		
		num_buttons = temp_num_buttons;
		y_blocks    = (int)ceil(sqrt(num_buttons/window_ratio));
		x_blocks    = (int)ceil((double)num_buttons/(double)y_blocks);
		
		if (head != 0 && changed2) {
			cout << "\nSave modifications to IC?  "; 
		
			Decision = xdecision(1);
		
			if (Decision == 1) 
				save_circuit(head);
			else if (Decision == 2)
				force_begin_in_schematic_window = 1;
		}
 	} while (Decision == 2);
	
	reset_actions();
	
	--is_ic;
	
	new_num = temp_num;
	
	supply_voltage = temp_vcc;
	ground = temp_gnd;
	Opened = temp_Opened;
	changed = temp_c;
	changed2 = temp_c2;
	
	if (destroy_head) {
		destroyL(head);
		head = 0;
	}	
	return head;
}





void specs(string name, int pins, string description) {
		
	cout << "\n" << name << endl << description;
	
	switch (get_ic_type(name)) {
	
		case 1: 
			cout << "\n Type: CMOS \n Vcc = 3-14v (" << cmos_supply << "v typical) \n " << pins <<" Pin DIP" << endl;
			break;
	
		case 2: 
			cout << "\n Type: TTL \n Vcc = " << TTL_supply << "v (4.75v-5.25v) \n " << pins << " Pin DIP" << endl;
			break;
	
		case 3: 
			cout << "\n Type: Linear \n " << pins << " Pin DIP" << endl;
			break;
		
		case 1000:
			cout << "\n Type: Timer \n " << pins << " Pin DIP" << endl;
			break; 
		
		case 0:
		default:
			cout << "\n Specifications for this component could not be found." << endl;
	}
}





int get_ic_type(string icname) {

	if (icname[1] == '4' && (icname[2] == '0' || icname[2] == '5'))
		return 1;
		
	else if (icname[1] == '7' && icname[2] == '4')
		return 2;
		
	else if (comp_char(icname[1], 'l') && comp_char(icname[2], 'm'))
		return 3;
		
	else if (icname[1] == '5' && icname[2] == '5' && icname[3] == '5')
		return 1000;
		
	return 0;
}





IC *add_ic(string name, string description, int pins, IC *data, IC *&tail) {
	
	description = add_spaces(description, description.length());
	
	name[0] = ' ';
	
	if (IC *new_ic = new IC) {		
		new_ic->name = name;
		new_ic->description = description;
		new_ic->pins = pins;
		new_ic->next = 0;
		
		if (data == 0) {
			tail = new_ic;
			return new_ic;
		}	
		tail->next = new_ic;
		tail = new_ic;
		return data;
	}
	else {
		cerr << "\nError: IC data could not be read because Circuit Solver is out of memory." << endl
			 << "To continue, allocate more memory to Circuit Solver.";
				 
		out_of_memory();
	}
	return data;
}




void add_ic_to_datafile(string name, string description, int pins) {
	
	int i, ret_val;
		
	if (ICdata_size < ICdata_capacity) {
		ICdata[ICdata_size].name = name;
		ICdata[ICdata_size].description = description;
		ICdata[ICdata_size].pins = pins;
		ICdata[ICdata_size].next = &ICdata[ICdata_size-1];
	}
	else { // double array size
		IC *old_data = ICdata;
		
		if (ICdata_capacity == 0)
			ICdata_capacity = 1;
			
		ICdata_capacity *= 2;
		ICdata = memAlloc(ICdata, ICdata_capacity);
		
		for (i = 0; i < ICdata_size; ++i) { 
			ICdata[i] = old_data[i];
			ICdata[i].next = &ICdata[i-1];
		}			
		for (; i < ICdata_capacity; ++i) {
			ICdata[i].name = unknown2;
			ICdata[i].description = unknown2;
			ICdata[i].pins = DEFAULT_NUM_PINS;
			ICdata[i].next = &ICdata[i-1];
		}	
		ICdata[ICdata_size].name = name;
		ICdata[ICdata_size].description = description;
		ICdata[ICdata_size].pins = pins;
		
		if (ICdata_size == 0)
			ICdata[ICdata_size].next = 0;
		else
			ICdata[ICdata_size].next = &ICdata[ICdata_size-1];
			
		delete [] old_data;
	}
	ret_val = IC_names.insert(name, ICdata_size);
	
	if (ret_val == 0)
		cerr << "Error: Duplicate name found in IC datafile: " << name << endl;
		
	++ICdata_size;
}





void find_ic(IC &ic, int id) {
	
	ifstream infile;
	int count(1);
	
	ic.name = ic.description = unknown1;
	ic.pins = DEFAULT_NUM_PINS;
	
	if (storeDATAB) {
		if (ICdata_size == 0) {
			no_datafile();
			return;
		}
		if (id < 1 || id > ICdata_size)
			return;
			
		ic.name = ICdata[id-1].name;
		ic.description = ICdata[id-1].description;
		ic.pins = ICdata[id-1].pins;
	}
	else {
		if (!infile_file(infile, ic_datafile)) {
			no_datafile();
			return;
		}
		while (infile.good() && infile >> ic.name >> ic.description >> ic.pins && count < id) {
			++count;
		}
		infile.close();
			
		if (count != id) {		
			ic.name = ic.description = unknown1;
			ic.pins = DEFAULT_NUM_PINS;
		}
	}	
	return;
}





int count_ics() {

	ifstream infile;
	IC ic;
	int count(0);
		
	if (storeDATAB) {
		if (ICdata_size == 0) {
			no_datafile();
			return -1;
		}
		else
			return ICdata_size;
	}
	else {
		if (!infile_file(infile, ic_datafile)) {
			no_datafile();
			return -1;
		}	
		while (infile.good() && infile >> ic.name >> ic.description >> ic.pins) {
			++count;
		}
		infile.close();
	}	
	return count;
}	





IC *read_ic_data(int screen, int &last_screen, int &last_ic) {

	ifstream infile;
	IC* data = NULL, *tail = NULL;
	string name, description;
	int pins, total(0), screenx(1);
	
	cout << "\nReading IC Data.";
	
	draw_text_on_window(" Reading Data");
	
	show_clock();
	
	if (screen == 1) {
		data = add_ic("_NEW", "_Custom_IC", DEFAULT_NUM_PINS, data, tail);
		++total;
		++last_ic; 
		data = add_ic("_FIND", "_FIND", DEFAULT_NUM_PINS, data, tail);
		++total;
		++last_ic;
	} 	
	if (storeDATAB) {
		if (ICdata_size == 0) {
			no_datafile();
			return data;
		}
		for (; last_ic < ICdata_size + 2*(screen == 1) && last_screen <= screen; ++last_ic) {
			if (screen == last_screen) {
				if (screen == 1)
					data = add_ic(ICdata[last_ic-2].name, ICdata[last_ic-2].description, ICdata[last_ic-2].pins, data, tail);
				else
					data = add_ic(ICdata[last_ic].name, ICdata[last_ic].description, ICdata[last_ic].pins, data, tail);
			}
			if ((last_ic+1)%(MAX_BOXES - 2) == 0) 
				++last_screen;
		
			if (!((last_ic+1)%search_value))  
				cout << ".";
		}
	}
	else {	
		if (!infile_file(infile, ic_datafile)) {
			no_datafile();
			return data;
		}
		while (screenx <= screen && infile.good() && infile >> name >> description >> pins) {				
			if (screen == screenx) {
				data = add_ic(name, description, pins, data, tail);
			}
			++total;
				
			if (total%(MAX_BOXES - 2) == 0 && total != 0) {
				++screenx;
			}
			if (!(total%search_value)) {
				cout << ".";
			}
		}	
		infile.close();	
	}
	cout << endl;
	
	reset_cursor();
	
	return data;
}





IC ic_search(IC &data, string name1, int use_status_bar) {
	
	ifstream infile;
	int match(0);
	
	countdown = 0;
		
	data.name = unknown2;
	data.description = unknown2;
	data.pins = DEFAULT_NUM_PINS;
	
	if (use_status_bar) {
		cout << "\nReading IC Data.";
	
		draw_text_on_window(" Reading Data");
	
		show_clock();
	}
	if (storeDATAB) {
		if (ICdata_size == 0) {
			no_datafile();
			return data;
		}
		if (useICStringTable) {
			countdown = IC_names.search(name1);
			if (countdown >= 0 && countdown < ICdata_size) {
				match = 1;
				data.name = ICdata[countdown].name;
				data.description = ICdata[countdown].description;
				data.pins = ICdata[countdown].pins;
			}
		}
		else {
			for (countdown = 0; countdown < ICdata_size && !match; ++countdown) {
				if (name1 == remove_spaces(ICdata[countdown].name, ICdata[countdown].name.length())) {
					match = 1;
					data.name = ICdata[countdown].name;
					data.description = ICdata[countdown].description;
					data.pins = ICdata[countdown].pins;
				}
			}
		}
	}
	else {
		if (!infile_file(infile, ic_datafile)) {
			no_datafile();
			return data;
		}
		while (!match && infile.good() && infile >> data.name >> data.description >> data. pins) {
			if (name1 == data.name)
				match = 1;
				
			++countdown;
		}
		infile.close();
	}
	if (match) {
		data.description = add_spaces(data.description, data.description.length());
		data.name = add_spaces(data.name, data.name.length());
	}
	else {
		data.name = unknown2;
		data.description = unknown2;
		data.pins = DEFAULT_NUM_PINS;
		countdown = 0;
	}				
	if (use_status_bar)
		reset_cursor();
		
	return data;
}





component *find_ic_circuit(component *head, string name) {

	ifstream infile;
	string text;
	int ID, LOCATION, ic_type, was_ic(is_ic), oldGV(GV);
	double value(0);
	char cname[MAX_SAVE_CHARACTERS + 1] = {0};
	box temp_num;
	
	string_to_char(cname, remove_spaces(name, name.length()), MAX_SAVE_CHARACTERS);
	
	cname[0] = '_';
		
	if (!infile_file(infile, cname) || !filestream_check(infile)) 
		return head;
	
	infile >> text;
		
	if (text == "Circuit_Solver_Saved_IC" || text == "Circuit_Solver_Saved_Subcircuit")
		GV = 0;
	else if (text == "Circuit_Solver_Saved_IC_GV" || text == "Circuit_Solver_Saved_Subcircuit") 
		GV = 1;
	else
		return head;
		
	if (text == "Circuit_Solver_Saved_IC" || text == "Circuit_Solver_Saved_IC_GV")
		looks_like_IC = 1;
	else
		looks_like_IC = 0;
	
	if (!filestream_check(infile))
		return head;
		
	infile >> text;
		
	if (text != cname) {
		GV = oldGV;
		return head;
	}
	ic_type = get_ic_type(cname);
	
	if (ic_type == 1) 
		supply_voltage = cmos_supply;
	else if (ic_type == 2) 
		supply_voltage = TTL_supply;
	
	surpress_update_error = update_error = 0;
	ground = 0;
	num_iterations = is_ic = 1;	
	
	if (saveBOXES) {
		if (!filestream_check(infile))
			return head;
		infile >> ic_num.h_boxes >> ic_num.v_boxes;
	}
	destroyL(head);
	head = 0;
	
	if (ic_num.v_boxes > 0) {		
		if (has_value(ic_num.h_boxes)) {
			if (!filestream_check(infile))
				return head;
			infile >> value;
		}
		else {
			value = 0.0;
		}
		head = update_circuit(ic_num.h_boxes, ic_num.v_boxes, head, 1000, value);
		
		ic_num.h_boxes = 0;
		ic_num.v_boxes = 0;
	}
	else if (ic_num.v_boxes < 0) 
		ic_num.v_boxes = -ic_num.v_boxes;
	
	temp_num = new_num;
	new_num  = ic_num;
		
	while (infile.good() && infile >> ID >> LOCATION) {	
		if (has_value(ID)) {
			infile >> value;
		}
		else {
			value = 0.0;
		}
		head = update_circuit(ID, LOCATION, head, 1000, value);

		if (update_error)
			surpress_update_error = 1;
	}
	infile.close();
	
	surpress_update_error = update_error = num_iterations = 0;
	is_ic = was_ic;
	GV = oldGV;
	new_num = temp_num;
			
	return head;
}




int get_ic_pins(int comp_id) {

	int numpins;
	IC  ic;
	
	find_ic(ic, (comp_id - IC_base_id));	
	numpins = min(ic.pins, max_pins);
	
	if (numpins%2 == 1)
		++numpins;
		
	return numpins;
}





int count_data(IC *data) {

	int count(0);

	while (data != 0) {
		++count;
		data = data->next;
	}
	return count;
}





void new_ic() {
	
	ofstream outfile;
	string name, description;
	int pins, type;
	IC DATA;
	
	do {
		cout << "\nEnter name for IC(max 32 characters):  ";
		name = xin(MAX_SAVE_CHARACTERS);
		
		DATA = ic_search(DATA, shift_chars(name), 0);
		
		if (countdown != 0) {
			beep();
			cout << "\nThat name is already taken. Please use another one." << endl;
		}	
	} while (countdown != 0);
	
	cout << "\nEnter description of IC:  ";
	description = xin(MAX_CHARACTERS);
	
	do {
		cout << "\nEnter the number of pins on IC:  ";
		pins = in();
	
		if (pins%2 != 0)
			++pins;
	
		if (pins < 0)
			pins = -pins;
		
		if (pins > max_pins) {
			beep();
			cout << "\nA maximum of " << max_pins << " pins is allowed on an IC." << endl;
		}
		else if (pins > max_fit_pins) {
			beep();
			cout << "\nA " << pins << " pin IC will not fit in the selection and drawing windows. Proceed anyway?  ";
		}
	} while (pins > max_pins || (pins > max_fit_pins && !decision()));
	
	cout << "Is this an Integrated Circuit?  ";
	type = decision();
		
	show_clock();
	
	cout << "\nCreating IC." << endl;
	
	if (storeDATAB) 
		add_ic_to_datafile(shift_chars(name), shift_chars(description), pins);
	
	if (!outfile_file(outfile, ic_datafile, 1)) {
		beep();
		cerr << "\nError: IC could not be created - May be out of space." << endl;
		reset_cursor();
		return;
	}
	name = remove_spaces(name, name.length());
	description = remove_spaces(description, description.length());
	
	outfile << "_" << name << " _" << description << " " << pins << " " << type << endl;

	if (!outfile.good())
		cerr << "Error writing subcircuit information to file." << endl;
	
	outfile.close();
	reset_cursor();
}




void no_datafile() {
	
	if (num_iterations != 0 || fabs((float)timer - (float)get_time()) < pause_time)
		return;
	
	timer = get_time();
	beep();	
	cerr << "\n\nError: " << ic_datafile << " could not be found! Please make sure it is in the folder/directory 'Circuit Solver.Data' or in the same "
		 << "folder/directory as Circuit Solver, or choose 'NEW' to create a new datafile." << endl;
	reset_cursor();
}





