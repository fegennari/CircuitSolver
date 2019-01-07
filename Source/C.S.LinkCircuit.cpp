#include "CircuitSolver.h"
#include "Logic.h"

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LinkCircuit.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Link Circuit 
// By Frank Gennari
#define ICx1 19
#define ICx2 23
#define ICy1 3
#define ICy2 15
#define ICsy 2


int window_size(default_num_buttons), circuit_error(0), main_window_size(1), max_location(0), main_window_offset;
component *memory_block = NULL; // large block of memory to allocate components from

extern box new_num;
extern int num_iterations, max_total_boxes, storeDATAB, max_circuits, ICdata_size, GV, saveBOXES, useCOMPLEX;
extern int x_blocks, y_blocks, x_screen, y_screen;
extern IC *ICdata;



struct subcircuit {
	
	int id, pins, num_components, num_wires;
	char cname[MAX_SAVE_CHARACTERS + 1];
	component *comp;
};



component *link_components(component *head, wire *&wires, int &num_wires);
component *flat_build(component *head, wire *&wires, int num_wires, int main_window_size);
subcircuit read_subcircuit(int id, int max_subcircuit_types);
int get_links(component *head, subcircuit *circuit, int level, int &filecount, int max_subcircuit_types, int &num_comps, int &num_wires, char *seen_subcircuit, int parent_id);
component *link_subcircuits(component *head, component *& tail, component *list, subcircuit *circuit, wire *wires, int &wire_index, int &num_wires, int &filenum, int &mem_block_index, int mem_size);
int merge_circuits(component *head, wire *wires, int &wire_index, int num_wires, int filenum, component *& tail, int &mem_block_index, int mem_size);
int connect_subcircuit(wire *wires, int &wire_index, int &num_wires, int numpins, int filenum, int parent_filenum, int position);
int add_wire(wire *wires, int &wire_index, int num_wires, int filenum, int parent_filenum, int start, int end);
component *add_new_component(component *subcircuit, const component& comp, int& error);
int count_components(component *head, component *& tail, int &subcircuit_count, int &max_position, int &nwires);
component *evaluate_ic(component *head);
int is_wire(int ID);
void create_wires(component *head, wire *wires, int &wire_index, int nwires);


void resize_node_array(int num_circuits, int main_window_size, int init_size);

component *clear(component *head);

int count_ics();

double get_q_type(int id);

void draw_status_bar();
void draw_status(const char *status);





component *link_components(component *head, wire *&wires, int &num_wires) {
	
	unsigned int i;
	int level(0), filecount(0), filenum(0), max_subcircuit_types, subcircuit_count, mem_block_index(0), wire_index(0);
	int num_components, max_position, main_window_size, num_total_comps(0), num_new_comps, nwires;
	component *tail = NULL;
	subcircuit *circuit = NULL;
	char *seen_subcircuit = NULL;
	
	show_clock();
	num_wires     = 0;
	circuit_error = 0;
	memory_block  = NULL;
		
	num_components = count_components(head, tail, subcircuit_count, max_position, nwires);	
	main_window_size = min(max_main_circuit_windows, (int)(max_position/max_boxes + 1));
		
	if (subcircuit_count == 0) {
		cout << "\n";
		num_wires = nwires;
		return flat_build(head, wires, num_wires, main_window_size);
	}	
	window_size = x_blocks*y_blocks*max_ic_windows; 
	
	reset_timer(1);
	show_elapsed_time();
	cout << "Linking Circuit." << endl;
	draw_status_bar();	
	draw_status(" Linking Circuit");
	
	max_subcircuit_types = count_ics();
	
	if (max_subcircuit_types == -1) {
		cout << "\nNo IC/subcircuit files found." << endl;
		num_wires = nwires;
		return flat_build(head, wires, num_wires, main_window_size);
	}
	seen_subcircuit = memAlloc_init(seen_subcircuit, max_subcircuit_types+1, (char)0);
	circuit         = memAlloc(circuit, max_subcircuit_types+1);
	
	for (i = 0; (int)i <= max_subcircuit_types; ++i) {
		circuit[i].id             = -1;
		circuit[i].pins           = 0;
		circuit[i].num_wires      = 0;
		circuit[i].num_components = 0;
	}
	main_window_offset = window_size*(main_window_size - 1);
	max_location       = max_boxes*max_circuits;
	
	level = get_links(head, circuit, level, filecount, max_subcircuit_types, num_total_comps, num_wires, seen_subcircuit, -2);	
	num_new_comps = num_total_comps - num_components;
	
	//cout << "Number of components = " << num_total_comps << ", New = " << num_new_comps << ", Number of wires = " << num_wires << endl;
	
	memory_block = memAlloc(memory_block, num_new_comps);
	wires        = memAlloc(wires,        num_wires);

	create_wires(head, wires, wire_index, nwires);

	if (filecount != 0) {
		show_elapsed_time();
		cout << filecount << " subcircuit type";
		if (filecount > 1)
			cout << "s." << endl;
		else
			cout << "." << endl;
	}
	show_clock();
	
	if (level >= 0 && filecount != 0) {
		show_elapsed_time();
		cout << "Building Circuit. Please wait." << endl;
		draw_status_bar();	
		draw_status(" Building Circuit");
	
		head = link_subcircuits(head, tail, head, circuit, wires, wire_index, num_wires, filenum, mem_block_index, num_new_comps);
		resize_node_array(filenum, main_window_size, num_total_comps);
	}
	else {
		resize_node_array(0, main_window_size, 0);
	}
	if (mem_block_index != num_new_comps) {
		internal_error();
		cout << "Warning: Extra memory allocated and not used for components in Link Circuits: " << num_new_comps << " allocated, " << mem_block_index << " used." << endl;
	}
	if (wire_index != num_wires) {
		internal_error();
		cout << "Warning: Extra memory allocated and not used for wires in Link Circuits: " << num_wires << " allocated, " << wire_index << " used." << endl;
		for (i = wire_index; i < (unsigned)num_wires; ++i) {
			wires[i].start = wires[i].end = 0;
		}
	}
	for (i = 0; (int)i <= max_subcircuit_types; ++i) {
		if (circuit[i].id >= 0)
			destroyL(circuit[i].comp);
	}
	delete [] circuit;
	delete [] seen_subcircuit;
	
	show_elapsed_time();
	cout << "done." << endl;

	return head;
}




component *flat_build(component *head, wire *&wires, int num_wires, int main_window_size) {

	int wire_index(0);

	resize_node_array(0, main_window_size, 0);
	wires = memAlloc(wires, num_wires);
	create_wires(head, wires, wire_index, num_wires);

	return head;
}




subcircuit read_subcircuit(int id, int max_subcircuit_types) {

	subcircuit circuit;
	component new_comp;	
	ifstream infile;
	IC ic;
	char cname_id[MAX_SAVE_CHARACTERS + 1] = {0};
	int Count(1), oldGV(GV), error(0);
	
	id -= IC_base_id;
	
	circuit.id             = id;
	circuit.comp           = NULL;
	circuit.num_components = 0;
	circuit.num_wires      = 0;
	strcpy(circuit.cname, " Unknown");
	
	
	if (storeDATAB) {
		if (ICdata_size == 0)
			return circuit;
		if (id > 0 && id <= ICdata_size && id <= max_subcircuit_types) {
			ic.name = ICdata[id-1].name;
			ic.description = ICdata[id-1].description;
			ic.pins = ICdata[id-1].pins;
			Count = id;
		}
		else
			Count = -1;
			
		string_to_char(cname_id, ic.name, MAX_SAVE_CHARACTERS + 1);
	}
	else {
		if (!infile_file(infile, "C.S.IC Data")) 
			return circuit;
			
		if (saveBOXES) {
			if (!filestream_check(infile))
				return circuit;
			infile >> new_num.h_boxes >> new_num.v_boxes;
		}		
		while (infile.good() && infile >> cname_id >> ic.description >> ic.pins && Count < id) 
			++Count;
	}
	if (Count == id) {
		circuit.pins = ic.pins;
		strcpy(circuit.cname, remove_spaces(cname_id, sizeof(cname_id)));
	}
	else {
		cerr << "Cannot locate subcircuit (IC) " << add_spaces(circuit.cname, strlen(circuit.cname)) << " in datafile." << endl;
		circuit.pins = 0;
		return circuit;
	}
	if (!storeDATAB) {
		infile.close();
		infile.clear();
	}
	if (!infile_file(infile, circuit.cname) || !filestream_check(infile)) 
		return circuit;
	
	infile >> cname_id;
	
	if (!filestream_check(infile))
		return circuit;
	
	if (strcmp(cname_id, "Circuit_Solver_Saved_IC") == 0 || strcmp(cname_id, "Circuit_Solver_Saved_Subcircuit") == 0) {
		infile >> cname_id;
		GV = 0;
	}
	else if (strcmp(cname_id, "Circuit_Solver_Saved_IC_GV") == 0 || strcmp(cname_id, "Circuit_Solver_Saved_Subcircuit_GV") == 0) {
		infile >> cname_id;
		GV = 1;
	}
	else {
		cerr << "The subcircuit (IC) " << add_spaces(circuit.cname, strlen(circuit.cname)) << " is of incorrect file type." << endl;
		return circuit;
	}	
	if (strcmp(cname_id, circuit.cname) != 0)
		cerr << "Filename " << add_spaces(circuit.cname, strlen(circuit.cname)) << " does not match circuit title " << cname_id << ". Will continue anyway." << endl;
	
	if (saveBOXES) {
		if (!filestream_check(infile))
			return circuit;
		infile >> new_num.h_boxes >> new_num.v_boxes;
	}				
	while (!error && infile.good() && infile >> new_comp.id >> new_comp.location) {	
		if (has_value(new_comp.id))
			infile >> new_comp.value;
		else
			new_comp.value = 0;
		
		circuit.comp = add_new_component(circuit.comp, new_comp, error);
	}	
	infile.close();	
	GV = oldGV;
	
	return circuit;
}





int get_links(component *head, subcircuit *circuit, int level, int &filecount, int max_subcircuit_types, int &num_comps, int &num_wires, char *seen_subcircuit, int parent_id) {
	
	char add_level(false);
	int ID, ncomps, nwires, wval, is_level0;

	is_level0 = (filecount == 0);
	
	while (head != 0) {
		if (get_order(head->id) == icx) {
			ID = head->id - IC_base_id;

			if (ID <= max_subcircuit_types) {
				if (circuit[ID].id == parent_id) {
					beep();
					cerr << "\nError: Recursive circuit found: IC/Subcircuit has been added to itself: " << add_spaces(circuit[ID].cname, strlen(circuit[ID].cname)) << ". Process terminated." << endl << endl;
					return -1;
				}
				if (seen_subcircuit[ID] == 1) {
					beep();
					cerr << "\nError: Recursive circuit found: " << add_spaces(circuit[ID].cname, strlen(circuit[ID].cname)) << ". Process terminated." << endl << endl;
					return -1;
				}
				if (circuit[ID].id == -1) {	
					circuit[ID] = read_subcircuit(head->id, max_subcircuit_types);

					if (circuit[ID].comp == 0) {
						cout << "The subcircuit (IC) " << add_spaces(circuit[ID].cname, strlen(circuit[ID].cname)) << " was not found or was empty and will be ignored." << endl;
						circuit[ID].num_components = 0;
					}
					else {
						if (filecount < max_subcircuit_types)
							++filecount;
						else { 
							beep();
							cerr << "\nError: Subcircuit filecount overflow. This circuit has too many unique subcircuits to fit into the current drawing environment. Process terminated after " << filecount << " loop(s)." << endl;
							return -1;
						}
						if (!add_level) {
							if (level < MAX_LEVEL)
								++level;
							else {
								beep();
								cerr << "\nError: Subcircuit loop overflow. This circuit has too many levels of hierarchy to simulate. Process terminated after " << filecount << " loop(s)." << endl;
								return -1;
							}
						}
						add_level = true;
						ncomps = nwires = 0;
						seen_subcircuit[ID] = 1;		
						level = get_links(circuit[ID].comp, circuit, level, filecount, max_subcircuit_types, ncomps, nwires, seen_subcircuit, circuit[ID].id);
						seen_subcircuit[ID] = 0;
						circuit[ID].num_components = ncomps;
						circuit[ID].num_wires      = nwires;
						num_comps += ncomps; // connections in subcircuit
						num_wires += nwires;						
						if (level < 0)
							return -1;
					}
				}
				else {
					num_comps += circuit[ID].num_components;
					num_wires += circuit[ID].num_wires;
				}
				num_wires += circuit[ID].pins; // add connection for each pin
			}
		}
		if ((wval = is_wire(head->id)) > 0) {
			num_wires += wval;
			if (is_level0)
				++num_comps;
		}
		else {
			++num_comps;
		}
		head = head->next;	
	}
	return level;
}





component *link_subcircuits(component *head, component *& tail, component *list, subcircuit *circuit, wire *wires, int &wire_index, int &num_wires, int &filenum, int &mem_block_index, int mem_size) {
	
	int parent_filenum(filenum), IC_id;
		
	if (circuit_error)
		return head;
	
	while (list != 0) {			
		if (filenum == 0 && !(list->flags & BASE_CIRCUIT))
			return head;
				
		if (get_order(list->id) == icx) {
			IC_id = list->id - IC_base_id;
			if (circuit[IC_id].id == IC_id) {
				if (filenum > max_circuits) {
					cerr << "\nError: Link Loop Overflow. Circuit may be recursive, so only the first level will be used. Process terminated after " << filenum << " loop(s)." << endl << endl;
					circuit_error = 1;
					beep();
					return clear(head);
				}
				++filenum;						
				if (!merge_circuits(circuit[IC_id].comp, wires, wire_index, num_wires, filenum, tail, mem_block_index, mem_size)) {
					return head;
				}
				if (!connect_subcircuit(wires, wire_index, num_wires, circuit[IC_id].pins, filenum, parent_filenum, list->location))
					cerr << "Error connecting subcircuit." << endl;
			}					
			else
				cerr << "Error: Subcircuit (IC) was not found in data folder." << endl;
		}
		list = list->next;
	}
	return head;
}




int merge_circuits(component *head, wire *wires, int &wire_index, int num_wires, int filenum, component *& tail, int &mem_block_index, int mem_size) { 
	
	int position_offset(window_size*filenum + main_window_offset), ID;
	connections x;
				
	while (head != 0) {
		if (head->location + position_offset >= max_location) {
			cerr << "\nThere is not enough allocated circuit space in memory to merge the circuits. The remaining components will not be merged." << endl;
			return 0;
		}
		ID = head->id;

		if (ID < Wire_base_id + max_location) {
			if (get_order(ID) == wirex) {
				if (wire_index >= num_wires || wire_index < 0) {
					internal_error();
					cerr << "\nError: Access to illegal wire index in merge_circuits(): " << wire_index << ". Wire array size is " << num_wires << "." << endl;
					return 0;
				}
				wires[wire_index].start = head->location    + position_offset;
				wires[wire_index++].end = ID - Wire_base_id + position_offset;
			}
			else if (!useCOMPLEX && ID == INDUCTOR_ID) {
				if (wire_index >= num_wires || wire_index < 0) {
					internal_error();
					cerr << "\nError: Access to illegal wire index in merge_circuits(): " << wire_index << ". Wire array size is " << num_wires << "." << endl;
					return 0;
				}
				x = get_connections(head->location, ID, head->flags);
				wires[wire_index].start = x.in2  + position_offset;
				wires[wire_index++].end = x.out2 + position_offset;
			}
			else if (!useCOMPLEX && ID == XFMR_ID) {
				if ((wire_index+1) >= num_wires || wire_index < 0) {
					internal_error();
					cerr << "\nError: Access to illegal wire index in merge_circuits(): " << wire_index << ". Wire array size is " << num_wires << "." << endl;
					return 0;
				}
				x = get_connections(head->location, ID, head->flags);
				wires[wire_index].start = x.in1  + position_offset;
				wires[wire_index++].end = x.in3  + position_offset;
				wires[wire_index].start = x.out1 + position_offset;
				wires[wire_index++].end = x.out3 + position_offset;
			}
			else {
				if (mem_block_index >= mem_size || mem_block_index < 0) {
					internal_error();
					cerr << "\nError: Access to illegal memory block index in merge_circuits(): " << mem_block_index << ". Memory block size is " << mem_size << "." << endl;
					return 0;
				}
				component *new_component = &memory_block[mem_block_index++];
					
				new_component->id       = ID;				
				new_component->location = head->location + position_offset;
				new_component->state    = head->state + filenum;
				new_component->value    = head->value;
				new_component->flags    = 0;
				new_component->next     = 0;	
				tail->next = new_component;
				tail       = new_component;
			}
		}	
		head = head->next;
	}
	return 1;
}





int connect_subcircuit(wire *wires, int &wire_index, int &num_wires, int numpins, int filenum, int parent_filenum, int position) {
	
	int start, end, const1, const2, const3, const4;
	
	numpins = min(numpins, max_pins);
		
	if (numpins%2 == 1)
		++numpins;

	const1 =  ICy2*x_blocks -   numpins + ICx1;
	const2 =  ICy1*x_blocks + 3*numpins + ICx2;
	const3 =  ICsy*x_blocks -  (numpins >> 2) + position - 1;
	const4 = -ICsy*x_blocks +   numpins - (numpins >> 2) + position;

	if (numpins%4 != 0) {
		const1 += 2;
		const2 += 2;
	}	
	for (int i = 1; i <= numpins; ++i) {	
		if (i <= (numpins >> 1)) {
			start = const1 + (i << 2);				
			end   = const3 + i;
		}
		else {
			start = const2 - (i << 2);
			end   = const4 - i;
		}
		if(start < 0 || end < 0) {
			cerr << "Error: Cannot process subcircuit pins extending outside of drawing area." << endl;
			--num_wires;
		}
		else if (!add_wire(wires, wire_index, num_wires, filenum, parent_filenum, start, end)) {
			return 0;
		}
	}
	return 1;
}			
	



int add_wire(wire *wires, int &wire_index, int num_wires, int filenum, int parent_filenum, int start, int end) {

	int wstart(window_size*filenum + main_window_offset + start);
	int wend(window_size*parent_filenum + main_window_offset + end);
	
	if (wstart >= max_location || wend >= max_location) {
		cerr << "\nThere is not enough allocated circuit space in memory to add all wires. Some connections will not be made." << endl;
		return 0;
	}
	if (wire_index >= num_wires || wire_index < 0) {
		internal_error();
		cerr << "\nError: Access to illegal wire index in add_wire(): " << wire_index << ". Wire array size is " << num_wires << "." << endl;
		return 0;
	}
	wires[wire_index].start = wstart;
	wires[wire_index++].end = wend;

	return 1;
}





component *add_new_component(component *subcircuit, const component& comp, int& error) {
	
	if (comp.location >= max_location || (get_order(comp.id) == wirex && comp.id >= Wire_base_id + max_location)) {
		cerr << "\nThere is not enough allocated circuit space in memory to add all components. Some components will be left out." << endl;
		error = 1;
		return subcircuit;
	}
	if (component *new_subcomponent = new component) {				
		new_subcomponent->location = comp.location;
		new_subcomponent->state    = subcircuit_flag;
		new_subcomponent->next     = subcircuit;

		if (comp.id < 0) {
			new_subcomponent->id    = -comp.id;
			new_subcomponent->flags = ROTATED;
		}
		else {
			new_subcomponent->id    = comp.id;
			new_subcomponent->flags = 0;
		}
		if (get_order(new_subcomponent->id) == transistorx) {
			new_subcomponent->value = get_q_type(new_subcomponent->id);
		}
		else {
			new_subcomponent->value = comp.value;
		}
		return new_subcomponent;
	}
	else {
		out_of_memory();
		error = 1;
	}
	return subcircuit;
}





int count_components(component *head, component *& tail, int &subcircuit_count, int &max_position, int &nwires) {

	int comp_count(0);
	
	subcircuit_count = max_position = nwires = 0;
	
	while (head != 0) {
		if (get_order(head->id) == icx) {
			++subcircuit_count;
		}
		else {
			nwires += is_wire(head->id);
		}
		++comp_count;
		max_position = max(max_position, head->location);
		tail = head;
		head = head->next;
	}
	return comp_count;
}




int is_wire(int ID) {

	if (get_order(ID) == wirex)
		return 1;

	if (useCOMPLEX)
		return 0;

	if (ID == INDUCTOR_ID)
		return 1;
	
	if (ID == XFMR_ID)
		return 2;
		
	return 0;
}




void create_wires(component *head, wire *wires, int &wire_index, int nwires) {

	int i, ID;
	connections x;

	while (head != 0) {
		ID = head->id;

		if (get_order(ID) == wirex) {
			if (wire_index >= nwires) {
				++wire_index;
				break;
			}
			wires[wire_index].start = head->location;
			wires[wire_index++].end = ID - Wire_base_id;
		}
		else if (!useCOMPLEX) {
			if (ID == INDUCTOR_ID) {
				if (wire_index >= nwires) {
					++wire_index;
					break;
				}
				x = get_connections(head->location, ID, head->flags);
				wires[wire_index].start = x.in2;
				wires[wire_index++].end = x.out2;
			}
			else if (ID == XFMR_ID) {
				if (wire_index+1 >= nwires) {
					wire_index += 2;
					break;
				}
				x = get_connections(head->location, ID, head->flags);
				wires[wire_index].start = x.in1;
				wires[wire_index++].end = x.in3;
				wires[wire_index].start = x.out1;
				wires[wire_index++].end = x.out3;
			}
		}
		head = head->next;
	}
	if (wire_index != nwires) {
		internal_error();
		cerr << "Error: Created " << wire_index << " wires, but " << nwires << " wires were allocated." << endl;
		for (i = wire_index; i < nwires; ++i) {
			wires[i].start = wires[i].end = 0;
		}
	}
}




