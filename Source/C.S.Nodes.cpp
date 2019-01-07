#include "CircuitSolver.h"
#include "Logic.h"
#include "GridArray.h"
#include "NetPointerSet.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Nodes.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Nodes (Nets)
// by Frank Gennari
#define DEF_POWER_NODE  -4
#define END_FLAG        -3
#define DEF_NVAL        -1
#define NO_LOC          -1
#define MEM_BLOCK_SIZE  1000


int const use_graph_nodes = 0;

int NODES(0), max_total_boxes(default_max_total_boxes), useHASHTABLE(-1), max_speed(0), no_ref_needed(0), global_has_gnd(0);
																			// ^ set in prefs
GridArray NODENUMBER; // Hash Table/Array Class


extern int findr, groundp, useDIGITAL, useCOMPLEX, max_circuits, drawing_color;


int  number_nodes(component *head, wire *wires, int num_wires);
void recopy_nodenumbers(int *NODETEMP, int start, int end);
void assign_nodenumber(int start, int end, int &numnodes);
void merge_tree_nodes(int *NODETEMP, int nn_start, int nn_end);

int  number_nodes_graph_based(component *head, wire *xwires, int num_wires);
void add_wire_nodenumbers(wire *wires, int numwires);
void build_wire_graph(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts);
void propagate_constants(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts) ;
void propagate_all(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts, int &numnodes);
void propagate_net_ids(NetPointerSet &netptr, wire *wires, unsigned i, unsigned loc, int nodenum, unsigned numconsts);

int  set_other_points_to_nodes(int numnodes, component *temp);
int  is_valid_node(component *head, int position);
int  set_ground(component *head, int &numnodes, int &power_node, int &has_ground);
int  error_check_nodes(int location, int id, int power_node, int has_ground);
void click_and_show_nets();

void create_node_array(int num_circuits, int main_window_size);
void resize_node_array(int num_circuits, int main_window_size, int init_size);
int  is_wire(int ID);

void draw_status_bar();
void draw_status(const char *status);
void draw_GROUND(int box_num);

int get_scrolled_window_click();




// ************************************************
// Old Netlister
// ************************************************

// Complex graph exploration type algorithm, uses tree array, close to linear time
// Union-Find
int number_nodes(component *head, wire *wires, int num_wires) {

	if (use_graph_nodes) {
		return number_nodes_graph_based(head, wires, num_wires);
	}

	int i, *nextnn, s_nextnn, numnodes(1), maximum(0), other_max(0), offset;
	int power_node(DEF_POWER_NODE), has_ground, error(0), ground, index;
	int *NODETEMP = NULL, *NODEOFFSET = NULL;
	char *exists = NULL;

	global_has_gnd = 0;

	if (head == NULL) {
		if (wires != NULL) delete [] wires;
		return 0;
	}
	if (num_wires > 0 && wires == NULL) {
		cerr << "Error: NULL wires in number_nodes." << endl;
		return 0;
	}
	if (num_wires == 0 && wires != NULL) {
		cerr << "Error: Wires should be NULL in number_nodes." << endl;
		delete [] wires;
	}
	reset_timer(1);
	show_elapsed_time();
	cout << "Initializing Netlister. " << endl;
	draw_status_bar();	
	draw_status(" Initializing Netlister");
	
	NODENUMBER.setall(DEF_NVAL);			
	ground         = set_ground(head, numnodes, power_node, has_ground);
	global_has_gnd = has_ground;
			
	if (!ground) {
		if (no_ref_needed) {
			--numnodes;
		}
		else {
			if (wires != NULL) delete [] wires;
			return 0;
		}
	}
	for (i = 0; i < num_wires; ++i) {
		assign_nodenumber(wires[i].start, wires[i].end, numnodes);
	}
    NODETEMP   = memAlloc(NODETEMP, numnodes+1);
    NODEOFFSET = memAlloc_init(NODEOFFSET, numnodes+1, 0);
    exists     = memAlloc_init(exists, numnodes+1, (char)0);
         
	for (i = 0; i <= numnodes; ++i) {	
		NODETEMP[i] = i;
	}
	show_elapsed_time();
	cout << "Finding Nets. " << endl;
	draw_status_bar();	
	draw_status(" Finding Nets");

	for (i = 0; i < num_wires; ++i) {
		merge_tree_nodes(NODETEMP, NODENUMBER[wires[i].start], NODENUMBER[wires[i].end]);
	}
	for (i = 1; i < numnodes; ++i) {
		if (NODETEMP[i] != i && NODETEMP[i] != NODETEMP[NODETEMP[i]]) {
			NODETEMP[i] = NODETEMP[NODETEMP[i]];
		}
	}
	show_elapsed_time();
	cout << "Combining... " << endl;
	
	if (power_node > 0)
		exists[power_node] = 1;

	for (i = 0; i < num_wires; ++i) {
		exists[NODENUMBER[wires[i].start]] = 1;
		exists[NODENUMBER[wires[i].end]]   = 1;
	}	
	for (i = 1; i < numnodes; ++i) {
		if (!exists[i])
			NODETEMP[i] = 0;
	}
	for (i = 1; i < numnodes; ++i) {
		if (NODETEMP[i] > other_max) {
			other_max = NODETEMP[i];
			++maximum;
			NODEOFFSET[i] = NODETEMP[i] - maximum;
		}
		else 
			NODETEMP[i] = NODETEMP[NODETEMP[i]];		
	}
	offset = numnodes - maximum;

	for (i = 0; i < num_wires; ++i) {
		recopy_nodenumbers(NODETEMP, wires[i].start, wires[i].end);
	}		
	while (head != 0) {		
		if (!is_wire(head->id)) {
			if (!error) {
				error = error_check_nodes(head->location, head->id, power_node, has_ground);
				if (error) {
					cout << "Continue?  ";
					if (!decision()) {
						delete [] NODETEMP;
						delete [] NODEOFFSET;
						delete [] exists;
						if (wires != NULL) delete [] wires;
						return 0;
					}
				}
			}
			numnodes = set_other_points_to_nodes(numnodes, head);
		}		
		head = head->next;
	}	
	NODENUMBER.resetnext(); 
	nextnn = &NODENUMBER.getnext(END_FLAG, index); 
	s_nextnn = *nextnn;

	while (s_nextnn != END_FLAG) {
		if (s_nextnn >= 0 && s_nextnn <= numnodes) {
			if (s_nextnn <= other_max && NODEOFFSET[s_nextnn] != 0)
				*nextnn = s_nextnn - NODEOFFSET[s_nextnn];
			if (*nextnn > other_max) 
				*nextnn = *nextnn - offset;
		}
		nextnn = &NODENUMBER.getnext(END_FLAG, index);
		s_nextnn = *nextnn;
	}
	delete [] NODETEMP;
	delete [] NODEOFFSET;
	delete [] exists;
	if (wires != NULL) delete [] wires;
		
	NODES = numnodes - offset + 1;
	NODENUMBER.setmax(NODES - 1);
	
	show_elapsed_time();
	cout << "Found " << NODES << " nets." << endl;
		
	//NODENUMBER.print();
		
	return NODES;
}




void recopy_nodenumbers(int *NODETEMP, int start, int end) {

	NODENUMBER.remap(NODETEMP, start);
	NODENUMBER.remap(NODETEMP, end);
}




void assign_nodenumber(int start, int end, int &numnodes) {

	int nn_start(NODENUMBER[start]), nn_end(NODENUMBER[end]);

	if (nn_start == DEF_NVAL) {
		if (nn_end == DEF_NVAL) {
			NODENUMBER[start] = numnodes;
			NODENUMBER[end]   = numnodes;
			++numnodes;
		}
		else {
			NODENUMBER[start] = nn_end;
		}
	}
	else if (nn_end == DEF_NVAL) {
		NODENUMBER[end] = nn_start;
	}
	else {
		if (nn_start > nn_end)
			NODENUMBER[start] = nn_end;
		else if (nn_end > nn_start)
			NODENUMBER[end]   = nn_start;
	}
}



// path compression for Union-Find
// This looks slow but is actually almost constant time since each loop is only
// executed once or twice, except for rare occasions.
void merge_tree_nodes(int *NODETEMP, int nn_start, int nn_end) {

	int start_root, end_root, new_root, nr;

	if (nn_start >= 0 && nn_end >= 0 && nn_start != nn_end) {	
		start_root = nn_start;
		end_root = nn_end;
		nr = NODETEMP[start_root];

		while (start_root != nr && nr != NODETEMP[nr]) {
			start_root = nr; // find root of first branch
			nr = NODETEMP[start_root];
		}
		start_root = nr; // root IDs
		nr = NODETEMP[end_root];

		while (end_root != nr && nr != NODETEMP[nr]) {
			end_root = nr; // find root of second branch
			nr = NODETEMP[end_root];
		}
		end_root = nr;

		if (end_root > start_root) // new root is lesser of the two
			new_root = NODETEMP[end_root] = start_root; 
		else if (start_root > end_root)
			new_root = NODETEMP[start_root] = end_root;
		else
			new_root = start_root;
		
		start_root = nn_start;
		
		// connect all nodes along paths to new root	
		while (NODETEMP[start_root] != new_root) {
			end_root = NODETEMP[start_root];
			NODETEMP[start_root] = new_root;
			start_root = end_root;
		}
		end_root = nn_end;
		
		while (NODETEMP[end_root] != new_root) {
			start_root = NODETEMP[end_root];
			NODETEMP[end_root] = new_root;
			end_root = start_root;
		}
	}
}




// ************************************************
// New Netlister
// ************************************************


// Graph-based version
int number_nodes_graph_based(component *head, wire *wires, int numwires) {

	int numnodes(1), power_node(DEF_POWER_NODE), has_ground, ground, error(0), max_index;
	unsigned nnsize, const_size, numconsts;
	char *wire_consts = NULL;

	if (head == NULL) {
		if (wires != NULL) delete [] wires;
		return 0;
	}
	if (numwires > 0 && wires == NULL) {
		cerr << "Error: NULL wires in number_nodes." << endl;
		return 0;
	}
	if (numwires == 0 && wires != NULL) {
		cerr << "Error: Wires should be NULL in number_nodes." << endl;
		delete [] wires;
	}
	reset_timer(1);
	show_elapsed_time();
	cout << "Initializing Netlister. " << endl;
	draw_status_bar();	
	draw_status(" Initializing Netlister");
	
	NODENUMBER.setall(DEF_NVAL);			
	ground = set_ground(head, numnodes, power_node, has_ground);
	const_size = NODENUMBER.size();
	
	if (!ground) {
		if (no_ref_needed) {
			--numnodes;
		}
		else {
			if (wires != NULL) delete [] wires;
			return 0;
		}
	}
	wire_consts = memAlloc(wire_consts, numwires);
	numconsts   = numnodes; // must be less than 256

	add_wire_nodenumbers(wires, numwires);

	nnsize    = NODENUMBER.size() - const_size + numconsts;
	NetPointerSet netptr(nnsize);

	max_index = NODENUMBER.assign_incremental_indices(numconsts);

	if (max_index+1 != nnsize) {
		internal_error();
		cerr << "Error: Assigned " << max_index+1 << " NODENUMBER indices, but NODENUMBER size is " << nnsize << "." << endl;
	}
	build_wire_graph(netptr, wires, wire_consts, numwires, numconsts);

	show_elapsed_time();
	cout << "Finding Nets. " << endl;
	draw_status_bar();	
	draw_status(" Finding Nets");

	propagate_constants(netptr, wires, wire_consts, numwires, numconsts);
	propagate_all(netptr,       wires, wire_consts, numwires, numconsts, numnodes);

	--numnodes; // overstepped in previous iteration

	while (head != 0) {
		if (!is_wire(head->id)) {
			if (!error) {
				error = error_check_nodes(head->location, head->id, power_node, has_ground);
				if (error) {
					cout << "Continue?  ";
					if (!decision()) {
						if (wires       != NULL) delete [] wires;
						if (wire_consts != NULL) delete [] wire_consts;
						return 0;
					}
				}
			}
			numnodes = set_other_points_to_nodes(numnodes, head);
		}		
		head = head->next;
	}
	NODES = numnodes+1;
	if (wires       != NULL) delete [] wires;
	if (wire_consts != NULL) delete [] wire_consts;

	show_elapsed_time();
	cout << "Found " << NODES << " nets." << endl;
		
	//NODENUMBER.print(); // debugging
		
	return NODES;
}




void add_wire_nodenumbers(wire *wires, int numwires) {

	for (unsigned i = 0; (int)i < numwires; ++i) {
		NODENUMBER.replace_if_default(wires[i].start, TEMP_GA_VALUE);
		NODENUMBER.replace_if_default(wires[i].end,   TEMP_GA_VALUE);
	}
}




void build_wire_graph(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts) {

	int start_n, end_n, status1, status2;

	for (unsigned i = 0; (int)i < numwires; ++i) {
		start_n = NODENUMBER[wires[i].start];
		end_n   = NODENUMBER[wires[i].end];

		if (start_n < numconsts) {
			wire_consts[i] = (char)(start_n + 1);
			if (end_n < numconsts) {
				if (end_n != start_n) {
					cerr << "Error: Power shorted to ground!" << endl;
				}
			}
		}
		else if (end_n < numconsts) {
			wire_consts[i] = (char)(end_n + 1);
		}
		else {
			wire_consts[i] = 0;
		}
		status1 = netptr.insert(start_n, (i << 1));
		status2 = netptr.insert(end_n,  ((i << 1) + 1));

		if (status1 == 0 || status2 == 0) {
			internal_error();
			cerr << "Error inserting net ";
			if (status1 == 0) {
				cerr << start_n;
			}
			else {
				cerr << end_n;
			}
			cerr << " into NetPointerSet in build_wire_graph()." << endl;
		}
	}
}




void propagate_constants(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts) {

	char wci;

	for (unsigned i = 0; (int)i < numwires; ++i) {
		if (wires[i].start != NO_LOC) {
			wci = wire_consts[i];
			if (wci != 0) {
				--wci;
				propagate_net_ids(netptr, wires, i, wires[i].start, wci, numconsts);
				propagate_net_ids(netptr, wires, i, wires[i].end,   wci, numconsts);
			}
		}
	}
}




void propagate_all(NetPointerSet &netptr, wire *wires, char *wire_consts, int numwires, int numconsts, int &numnodes) {

	for (unsigned i = 0; (int)i < numwires; ++i) {
		if (wires[i].start != NO_LOC) {
			propagate_net_ids(netptr, wires, i, wires[i].start, numnodes, numconsts);
			propagate_net_ids(netptr, wires, i, wires[i].end,   numnodes, numconsts);
			++numnodes;
		}
	}
}




void propagate_net_ids(NetPointerSet &netptr, wire *wires, unsigned i, unsigned loc, int nodenum, unsigned numconsts) {

	int not_end, net_id, index, wire_end;
	net_it_state state;

	net_id = NODENUMBER.replace_value(loc, nodenum);
	wires[i].start = NO_LOC;

	if (net_id == DEF_NVAL) {
		internal_error();
		cerr << "Error: Access to illegal nodenumber " << net_id << " from location " << loc << " in propagate_net_ids()." << endl;
		return;
	}
	if (netptr.net_single(net_id)) // this is the only connected wire
		return;

	if (netptr.net_empty(net_id)) {
		internal_error();
		cerr << "Error: Encountered empty net " << net_id << " in search through fanout of wire at location " << loc << " in propagate_net_ids()." << endl;
		return;
	}
	state   = netptr.it_get_state();
	not_end = netptr.set_iterator_net(net_id);

	while (not_end) {
		index    = netptr.get_curr();
		wire_end = index%2; // extract start/end bit
		index  >>= 1;

		if (index != i && wires[index].start != NO_LOC) { // if a new wire
			if (wire_end) {
				propagate_net_ids(netptr, wires, index, wires[index].start, nodenum, numconsts);
			}
			else {
				propagate_net_ids(netptr, wires, index, wires[index].end,   nodenum, numconsts);
			}
		}
		not_end = netptr.advance_iterator();
	}
	netptr.net_set_state(state);
}




// ************************************************
// Shared
// ************************************************



int set_other_points_to_nodes(int numnodes, component *temp) {

	connections x = get_connections(temp->location, temp->id, temp->flags);
	
	if (x.in1 != 0  && NODENUMBER.replace_if_default(x.in1,  numnodes+1))
		++numnodes;
		
	if (x.in2 != 0  && NODENUMBER.replace_if_default(x.in2,  numnodes+1))
		++numnodes;
		
	if (x.in3 != 0  && NODENUMBER.replace_if_default(x.in3,  numnodes+1))
		++numnodes;
		
	if (x.out1 != 0 && NODENUMBER.replace_if_default(x.out1, numnodes+1))
		++numnodes;
		
	if (x.out2 != 0 && NODENUMBER.replace_if_default(x.out2, numnodes+1))
		++numnodes;
		
	if (x.out3 != 0 && NODENUMBER.replace_if_default(x.out3, numnodes+1))
		++numnodes;

	return numnodes;
}




int is_valid_node(component *head, int position) {

	int ID, order;
	connections x;

	if (position < 0)
		return 0;

	while (head != 0) {
		order = get_order(head->id);
		ID    = head->id;

		if (order == wirex) {
			if (head->location == position || (ID - Wire_base_id) == position) {
				return 1;
			}
		}
		else {	
			if (order == transistorx || (ID >= TRANSISTOR_ID && ID <= DIODE_ID) || (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) || ID == BATTERY_ID || ID == INDUCTOR_ID || ID == NFET_ID || ID == PFET_ID || ID == XFMR_ID || ID == TLINE_ID) {
				x = get_connections(head->location, ID, head->flags);
				if (x.in1 == position || x.in2 == position || x.in3 == position || x.out1 == position || x.out2 == position || x.out3 == position) {
					return 1;
				}
			}
		}
		head = head->next;
	}
	return 0;
}




int set_ground(component *head, int &numnodes, int &power_node, int &has_ground) {
	
	int ground(0), position, good_net, temp_dc;		
	component *temp = head;
	
	if (head == 0)
		return 0;

	while (temp != 0) {		
		if (temp->id == GROUND_ID) {
			temp->state = 0;
			temp->value = 0;
			NODENUMBER[temp->location] = 0;
			++ground;
		}
		else if (temp->id == POWER_ID) {
			NODENUMBER[temp->location] = 1;
			power_node = 1;
		}
		temp = temp->next;
	}
	numnodes = 1 + (power_node == 1);
	has_ground = ground;

	if (!ground && useDIGITAL && head != 0) {
		NODENUMBER[head->location] = 0;
		++ground;
	}
	else if (!ground && !findr) {
		if (no_ref_needed) 
			cout << "Selecting arbitrary node 0." << endl;
			
		else {
			stop_timer();
			reset_cursor();
			
			cout << "\n\nNo ground or reference voltage was found in this circuit.";
#ifndef TEXT_ONLY			
			cout << " Would you like to pick a zero voltage(ground) point now?  ";
			if (decision()) {
				set_special_cursor(GROUND_CURSOR);
				cout << "\nPlease click on a point in the circuit that you would like to temporarily make zero volts." << endl;
				do {
					position = get_window_click();				
					if (!is_valid_node(head, position)) {
						beep();
						cerr << "\nInvalid point! You must select a point(node) in the circuit. \nWould you like to try again?  ";
						if (!decision()) {
							cout << "\nPlace a ground in the circuit and try again." << endl;
							reset_cursor();
							return 0;		
						}
						good_net = 0;
					}
					else {
						temp_dc = drawing_color;
						drawing_color = LTGREY;
						draw_GROUND(position);
						drawing_color = temp_dc;
						good_net = 1;
					}
				} while (!good_net);
				
				cout << "\nFinding Nets:";				
				show_clock();
				
				groundp = position;
				NODENUMBER[position] = 0;
				++ground;
			}
			else {
				cout << "\nPlace a ground in the circuit and try again." << endl;
				return 0;		
			}
#endif
			start_timer();
		}
	}
	else if (!ground && findr) {
		NODENUMBER[groundp] = 0;
		++ground;
	}		
	return ground;
}




int error_check_nodes(int location, int id, int power_node, int has_ground) {
		
	if (has_ground && id == POWER_ID && NODENUMBER[location] == 0) {
		cerr << "\nError: Power is shorted to ground!" << endl;
		beep();
		return 1;
	}
	else if (has_ground && id == INPUT_ID && NODENUMBER[location] == 0) {
		cerr << "\nError: Input is shorted to ground!" << endl;
		beep();
		return 1;
	}
	else if (id == INPUT_ID && NODENUMBER[location] == power_node) {
		cerr << "\nError: Input is shorted to power!" << endl;
		beep();
		return 1;
	}
	return 0;
}




// ************************************************
// Node Array
// ************************************************



void create_node_array(int num_circuits, int main_window_size) {
	
	char enough_memory(1);
	
	num_circuits = min(num_circuits, max_circuits);
	
	if (num_circuits < 0)
		max_total_boxes = default_max_total_boxes;
	else
		max_total_boxes = max_boxes*(num_circuits + main_window_size);
	
	if (max_total_boxes/max_boxes > 1) {
		show_elapsed_time();
	}
	cout << "Allocating space for " << max_total_boxes/max_boxes << " circuit";
	if (max_total_boxes/max_boxes > 1)
		cout << "s." << endl;
	else
		cout << "." << endl;
			
	if (!(NODENUMBER.create(max_total_boxes+1, CS_DECIDES, DEF_NVAL, MEM_BLOCK_SIZE)))
		enough_memory = 0;
				
	while (!enough_memory) { // Should not need to do this, but...
		
		NODENUMBER.destroy();
		max_total_boxes /= 2;
		
		cerr << "Not enough memory, decreasing to " << max_total_boxes/max_boxes << " circuits";
		if (max_total_boxes/max_boxes > 1)
			cout << "s." << endl;
		else
			cout << "." << endl;
		
		if (!(NODENUMBER.create(max_total_boxes+1, CS_DECIDES, DEF_NVAL, MEM_BLOCK_SIZE)))
			enough_memory = 0;
	}	
	NODENUMBER.setall(0);
	
	if (max_total_boxes/max_boxes > 1) {
		show_elapsed_time();
		cout << "Circuit space allocated successfully." << endl;
	}
}




void resize_node_array(int num_circuits, int main_window_size, int init_size) {
					
	NODENUMBER.destroy();
						
	if (!(num_circuits == 0 && main_window_size == 0)) {
		create_node_array(num_circuits, main_window_size);
	}				
	if (!NODENUMBER.set_ht_size(init_size)) {
		out_of_memory();
	}
}




void click_and_show_nets() {

	int net(1);

	cout << "\nBeginning Net Testing. Click on Simulate to end." << endl;

	while (net != 0) {
		net = get_scrolled_window_click();

		if (net >= 0) {
			cout << "Net " << NODENUMBER[net] << ", location = " << net << endl;
			//cout << ", net_delay = " << net_delay[NODENUMBER[net]] << endl;
		}
	}
	cout << "\nEnding Net Testing." << endl;
}



