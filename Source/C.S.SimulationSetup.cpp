#include "CircuitSolver.h"
#include "Logic.h"
#include "GridArray.h"
#include "CompArray.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.SimulationSetup.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Simulation Setup - Initialization and Reset
// By Frank Gennari
int const NO_SORT            = 0;
int const CHECK_SORTED_COMPS = 1;

// 0 = never simulate, 1 = only digital, 2 = only analog, 3 = always simulate
char const sim_val[FF_max_id+1] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			 1, 3, 0, 3, 3, 3, 3, 1, 1, 3, 3, 0, 3, 2, 1, 2, 2, 2, 2, 2, 2,
			 1, 1, 1, 1, 3, 3, 2, 1, 1, 1, 1, 1, 1, 1, 3, 0, 0, 0, 0, 0, 0, 0, 
			 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
			 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};


extern component *memory_block;
extern int DEMO;
extern GridArray NODENUMBER;


component *simulation_setup(component *head, wire *&wires, int &num_wires);
component *init_data_structures(component *head, wire *&wiresave, component *&unsim, wire *wires, int num_wires, int &nodes, int &error, int is_digital, int &comp_count);
component *reset_circuit(component *head, component *unsim, wire *wiresave);
component *sort_list(component **parr, int sizex, int startx, int endx);
component *remove_wires(component *head, wire *&wiresave);
component *return_wires(component *head, wire *&wiresave);
component *simulation_cleanup(component *head);
component *clear(component *head);

void split_unsimulatable(int is_digital, component *&head, component *&unsim, int &comp_count);
void merge_unsimulatable(component *&head, component *&unsim);


component *link_components(component *head, wire *&wires, int &num_wires);

void resize_node_array(int num_circuits, int main_window_size, int init_size);

int number_nodes(component *head, wire *wires, int num_wires); // use one or the other

void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);




component *simulation_setup(component *head, wire *&wires, int &num_wires) {

	reset_timer(1);
	show_elapsed_time();
	cout << "Starting Comonent Organization..." << endl;
	head = organize_components(head);
	show_elapsed_time(); 
	cout << "done." << endl;
	head = link_components(head, wires, num_wires);
	
	return head;
}




component *init_data_structures(component *head, wire *&wiresave, component *&unsim, wire *wires, int num_wires, int &nodes, int &error, int is_digital, int &comp_count) {

	show_clock();
	draw_status_bar();
	draw_status(" Finding Nets");
	nodes = number_nodes(head, wires, num_wires);

	if (!nodes) {
		beep();
		cerr << "\nThis circuit contains no nets or otherwise cannot be simulated." << endl;
		error = 1;
		return head;
	}
	if (DEMO && nodes > default_demo_nets) {
		beep();
		cerr << "\nThe maximum nets allowed in the DEMO version of Circuit Solver is " << default_demo_nets << ". Aborting simulation." << endl;
		error = 1;
		return head;		
	}
	reset_timer(1);
	show_elapsed_time();
	cout << "Splitting components..." << endl;

	head = remove_wires(head, wiresave);
	split_unsimulatable(is_digital, head, unsim, comp_count);

	show_elapsed_time();
	cout << "done." << endl;

	if (head == 0) {
		beep();
		cerr << "\nError: There are no simulatable components." << endl;
		error = 1;
		return reset_circuit(head, unsim, wiresave);
	}
	error = 0;
	
	return head;
}




component *reset_circuit(component *head, component *unsim, wire *wiresave) {

	reset_timer(1);
	show_elapsed_time();
	cout << "Resetting circuit..." << endl;

	merge_unsimulatable(head, unsim);
	head = return_wires(head, wiresave);
	//head = organize_components(head);
	reset_cursor();

	show_elapsed_time();
	cout << "done." << endl;

	return head;
}



// 100K = 0.3s on 750MHz Athlon (log2(N) ~ 16-17)
component *organize_components(component *head) {
	
	unsigned int sorted(1), reversed(1), lsize(0), i(0);
	int last(0);
	component *head2 = NULL, *temp = head, **circuit = NULL;

	if (NO_SORT || head == 0 || head->next == 0) {
		return head;
	}
	last = head->location;
			
	while (temp != 0) { // check if sorted, reverse sorted, or unsorted and count elements
		if (temp->location < last) {
			sorted   = 0;
		}
		else if (temp->location > last) {
			reversed = 0;
		}
		last = temp->location;
		temp = temp->next;
		++lsize;
	}
	if (sorted) // already sorted 
		return head;
		
	if (reversed) { // (un)reverse the list (Reading from a file reverses the list for efficiency.)
		head2 = 0;	
		while (head != 0) {
			temp       = head->next;
			head->next = head2;
			head2      = head;
			head       = temp;
		}
		return head2;
	}
	circuit = memAlloc(circuit, lsize+1); // pointer array
	if (circuit == NULL) {
		return head;
	}
	temp = head;
	
	while (temp != 0 && i < lsize) { // fill pointer array
		circuit[i++] = temp;
		temp         = temp->next;
	}
	head2 = sort_list(circuit, lsize-1, 0, lsize-1); // nlogn sort (below)
	
	/*for (i = 0; i < lsize - 1; ++i) // This will undo the sort operation, returning to the original component list
		circuit[i]->next = circuit[i + 1];			
	circuit[lsize - 1]->next = 0;
	head2 = circuit[0];*/
	
	delete [] circuit;

	return head2;
}



// used in organize_components() above, my own creation, something like mergesort
component *sort_list(component **parr, int sizex, int startx, int endx) {
	
	int midx, last(0);
	component *first, *second, *temp, *sorted;
		
	if (endx - startx < 2) {
		if (startx == endx) {
			return parr[startx];
		}
		if (parr[startx]->location <= parr[endx]->location) {
			parr[startx]->next = parr[endx];
			return parr[startx];
		}
		parr[endx]->next = parr[startx];
		return parr[endx];
	}
	if (startx == 0 && endx == sizex) {
		last = 1;
	}
	midx = (startx + endx)/2;
		
	first  = sort_list(parr, sizex, startx, midx);
	second = sort_list(parr, sizex, midx+1, endx);
	
	if (first->location <= second->location)  {
		sorted = first;
		first  = first->next;
		++startx;
	}	
	else {
		sorted = second;
		second = second->next;
		--endx;
	}
	temp = sorted;
	
	while (startx <= midx && midx < endx) {
		if (first->location <= second->location)  {
			temp->next = first;
			first      = first->next;
			++startx;
		}	
		else {
			temp->next = second;
			second     = second->next;
			--endx;
		}
		temp = temp->next;
	}
	while (startx <= midx) {
		temp->next = first;
		first      = first->next;
		temp       = temp->next;
		++startx;
	}
	while (midx < endx) {
		temp->next = second;
		second     = second->next;
		temp       = temp->next;
		--endx;
	}	
	if (last) {
		temp->next = 0;
	}
	return sorted;
}




component *remove_wires(component *head, wire *&wiresave) {
		
	component *temp, *temp2;
	int wire_count(0);
				
	temp = head;
	while (temp!= 0) {
		if ((temp->flags & BASE_CIRCUIT) && get_order(temp->id) == wirex) {
			++wire_count;
		}
		temp = temp->next;
	}
	wiresave          = memAlloc(wiresave, wire_count+1);	
	wiresave[0].start = wiresave[0].end = wire_count;
	wire_count        = 1;
		
	while (head && get_order(head->id) == wirex) {
		if (head->flags & BASE_CIRCUIT) {
			wiresave[wire_count].start = head->location;
			wiresave[wire_count].end   = head->id - Wire_base_id;
			++wire_count;
			temp = head;
			head = head->next;
			delete temp;
		}
		else {
			temp = head;
			head = head->next; // others deleted later
		}
	}
	if(!head) {
		return 0;
	}
	temp = head;
	temp2 = temp->next;
	
	while(temp2 != 0){
		if(get_order(temp2->id) == wirex){
			temp->next = temp2->next;
			if (temp2->flags & BASE_CIRCUIT) {
				wiresave[wire_count].start = temp2->location;
				wiresave[wire_count].end   = temp2->id - Wire_base_id;
				++wire_count;
				delete temp2; // others deleted later
			}
			temp2 = temp->next;
		}
		else{
			temp  = temp->next;
			temp2 = temp->next;
		}
	}	
	return head;
}




component *return_wires(component *head, wire *&wiresave) {
	
	if (wiresave == 0)
		return head;
									
	for (unsigned int i = 1; (int)i <= wiresave[0].start; ++i) {			
		if (component *replaced_wire = new component) {	
			replaced_wire->id       = wiresave[i].end + Wire_base_id;
			replaced_wire->location = wiresave[i].start;
			replaced_wire->state    = 2;
			replaced_wire->value    = 0;
			replaced_wire->flags    = BASE_CIRCUIT;
			replaced_wire->next     = head;
			head                    = replaced_wire;
		}
		else {
			out_of_memory();
		}
	}	
	delete [] wiresave;
	wiresave = NULL;
		
	return head;
}




component *simulation_cleanup(component *head) {

	reset_timer(1);
	cout << "Cleaning up..." << endl;

	resize_node_array(0, 0, 0);	
	head = clear(head);
	head = organize_components(head);

	show_elapsed_time();
	cout << "done." << endl;

	return head;
}




component *clear(component *head) {
	
	component *temp, *temp2;
				
	while (head && !(head->flags & BASE_CIRCUIT)) {
		temp = head;
		head = head->next;
		//delete temp; // this is part of memory_block, which is deleted later
	}
	if(!head) {
		return 0;
	}
	temp  = head;
	temp2 = temp->next;
	
	while(temp2 != 0) {
		if(!(temp2->flags & BASE_CIRCUIT)) {
			temp->next = temp2->next;
			//delete temp2; // same here
			temp2 = temp->next;
		}
		else{
			temp  = temp->next;
			temp2 = temp->next;
		}
	}
	if (memory_block != NULL) {
		delete [] memory_block;
		memory_block = NULL;
	}
	return head;
}




void split_unsimulatable(int is_digital, component *&head, component *&unsim, int &comp_count) {

	int ID, remove, saw_power(0), saw_ground(0), sim_value, shorted(0), cc;
	component *temp = head, *temp2, *last = head;
	connections x;
	
	unsim = NULL;
	comp_count = 0;
	
	if (head == 0)
		return;
	
	while (temp != 0) {
		ID     = temp->id;
		remove = 0;	
		if (ID == POWER_ID) {
			if (saw_power)
				remove = 1;
			else
				saw_power = 1;
		}
		else if (ID == GROUND_ID) {
			if (saw_ground)
				remove = 1;
			else
				saw_ground = 1;
		}
		else if (get_order(ID) == icx || get_order(ID) == wirex) {
			remove = 1;
		}
		else if (ID <= FF_max_id) {
			if (CHECK_SORTED_COMPS) {
				cc = conn_class[ID];

				if (cc == 1 || cc == 2 || cc == 3) { // 2-port
					x = get_connections(temp->location, ID, temp->flags);

					if (NODENUMBER[x.in2] == NODENUMBER[x.out2]) { // shorted component
						remove = 1;
						++shorted;
					}
				}
			}
			if (!remove) {
				sim_value = sim_val[ID];
				if (sim_value == 0 || (sim_value == 2 && is_digital == 1) || (sim_value == 1 && is_digital == 0)) {
					remove = 1;
				}
			}
		}		
		if (remove) {
			if (temp == head) {
				last = head = temp->next;
			}
			else {
				last->next = temp->next;
			}
			temp2 = unsim;
			unsim = temp;
			temp  = temp->next;
			unsim->next = temp2;
		}
		else {
			last = temp;
			temp = temp->next;
			++comp_count;
		}
	}
	if (shorted > 0) {
		cout << "Warning: There are " << shorted << " shorted components." << endl;
	}
}




void merge_unsimulatable(component *&head, component *&unsim) {

	component *temp = head;
	
	if (unsim == 0) {
		return;
	}
	if (head == 0) {
		head  = unsim;
		unsim = NULL;
		return;
	}			
	while (temp->next != 0)	{	
		temp = temp->next;
	}
	temp->next = unsim;	
	unsim      = NULL;
}







