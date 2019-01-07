#include "CircuitSolver.h"
#include "Logic.h"
#include "NetPointerSet.h"



// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.TopoSort.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Topological Sort for Logic Engine
// By Frank Gennari
#define WAS_PROC_FLAG (char)0x01
#define IS_CONST_FLAG (char)0x02
#define IS_STSEL_FLAG (char)0x04

int const test_show_topo_sort = 0;


extern int num_iterations, force_init_states, allow_mult_loops, global_has_gnd;


comp_sim *topo_sort(comp_sim *comps, int *&permute_output, unsigned *&input_nets, unsigned *&output_nets, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int nodes, int &num_inputs, int &num_outputs, int &num_ff, int num_comps);
void get_fanin_pointers(NetPointerSet &fanin, int *conn, char *dep_count, unsigned i);
char add_io_to_set(NetPointerSet &ios, int conn, unsigned i);
void check_io_status_recur(NetPointerSet &bidir, NetPointerSet &fanin, NetPointerSet &fanout, comp_sim *comps, char *dep_count, unsigned i, unsigned curr_net_id);
void move_bidir_to_io(NetPointerSet &bidir, NetPointerSet &ios, unsigned i);
unsigned move_bidir_2port(NetPointerSet &bidir, NetPointerSet &ios1, NetPointerSet &ios2, comp_sim *comps, unsigned i);
unsigned get_bidir_other_net(comp_sim &comp, unsigned i);
void add_net_to_queue(int *nets_to_proc, NetPointerSet &fanin, char *dep_count, char *net_flag, unsigned net_id, unsigned &ntp_curr);

void copy_comp_sim(comp_sim &comp2, comp_sim &comp1);
void check_print_num_message(int number, int mtype, const char *obj_type);




// ************************************************
// New Topo Sort
// ************************************************


// Much faster - runtime independent of critical path, but uses slightly more memory
comp_sim *topo_sort(comp_sim *comps, int *&permute_output, unsigned *&input_nets, unsigned *&output_nets, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int nodes, int &num_inputs, int &num_outputs, int &num_ff, int num_comps) {

	unsigned i, nnum, net_id, other_bidir, not_end, ntp_curr(0), level, level2, index_checkpoint, input_cntr(0);
	int ID, counter(0), outputs(0), add_this_one, last_output_added(0), comps_left(0), phase(1);
	int undriven_nets(0), multi_fo_nets(0), unused_nets(0), floating_nets(0), const_driven_nets(0);
	int more_outputs, total_outputs(0), warning(0), temp_counter, found_ground(0), found_power(0), has_st_sel(0);
	int unreachable_outs, unreachable_comps, vdd_net(-1);
	int *conn, *nets_to_proc = NULL;
	char *comp_added = NULL, *dep_count = NULL, *net_flag = NULL;
	comp_sim *comps2 = NULL;

	fanin.set_size(nodes+1);
	fanout.set_size(nodes+1);
	bidir.set_size(nodes+1);

	reset_timer(1);
	show_elapsed_time();
	cout << "Finding circuit topology..." << endl;
	
	num_inputs = num_outputs = num_ff = num_iterations = 0;

	// USE input_nets, output_nets
	
	comp_added   = memAlloc_init(comp_added, num_comps+1, (char)0);
	net_flag     = memAlloc_init(net_flag,   nodes+1,     (char)0);
	nets_to_proc = memAlloc(nets_to_proc,    nodes+1);
	comps2       = memAlloc(comps2,          num_comps+1);
	dep_count    = memAlloc(dep_count,       num_comps+1);

	// fill graph fanin/fanout pointers
	for (i = 0; (int)i < num_comps; ++i) { 
		ID   = comps[i].id;
		conn = comps[i].conn;

		if ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || ID == OPAMP_ID) { // standard logic gates + opamp + other logic							
			get_fanin_pointers(fanin, conn, dep_count, i);
			add_io_to_set(fanout, conn[4], i);
			
			if (ID >= QAND_ID && ID <= QXNOR_ID) { // 4-input gates
				add_io_to_set(fanin, conn[3], i);
				++dep_count[i];
			}
			else if (ID == ADDER_ID) { // adder has two outputs
				add_io_to_set(fanout, conn[5], i);
			}
		}
		else if (ID >= PRSL_ID && ID <= NTFF_ID) { // flipflops
			get_fanin_pointers(fanin, conn, dep_count, i);
			add_io_to_set(fanout, conn[3], i);
			add_io_to_set(fanout, conn[5], i);
		}
		else if (get_order(ID) == transistorx || ID == NFET_ID || ID == PFET_ID) { // transistors
			add_io_to_set(fanin, conn[1], i);
			add_io_to_set(bidir, conn[3], i);
			add_io_to_set(bidir, conn[5], i);
			dep_count[i] = 2;
		}
		else if (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) { // sources
			add_io_to_set(bidir, conn[1], i);
			add_io_to_set(bidir, conn[4], i);
			dep_count[i] = 1;
		}		
		else if (get_order(ID) != icx) { // ignore ICs
			switch(ID) {
						
				case OUTPUT_ID:
					++num_outputs;
					add_io_to_set(fanin, comps[i].conn_loc, i);
					dep_count[i] = 1;
					break;
						
				case BATTERY_ID: // ???
				case DIODE_ID: // change?					
				case RES_ID:
				case CAP_ID: // ignore for now ???							
				case INDUCTOR_ID: // treated as wire
					add_io_to_set(bidir, conn[1], i);
					add_io_to_set(bidir, conn[4], i);
					dep_count[i] = 1;
					break;

				case POWER_ID:
					if (found_power) { // duplicate VDD
						dep_count[i] = 0;
						break;
					}
					if (global_has_gnd && vdd_net == 0) {
						cerr << "Error in TopoSort: VDD shorted to GND." << endl;
					}
					add_io_to_set(fanout, comps[i].conn_loc, i);
					net_flag[comps[i].conn_loc] |= IS_CONST_FLAG;
					dep_count[i] = 0;
					found_power  = 1;
					vdd_net      = comps[i].conn_loc;
					break;

				case GROUND_ID:
					if (found_ground) { // duplicate GND
						dep_count[i] = 0;
						break;
					}
					if (comps[i].conn_loc != 0) {
						cerr << "Error in TopoSort: GND at net " << comps[i].conn_loc << "." << endl;
					}
					add_io_to_set(fanout, comps[i].conn_loc, i);
					net_flag[comps[i].conn_loc] |= IS_CONST_FLAG;
					dep_count[i] = 0;
					found_ground = 1;
					break;
					
				case INPUT_ID:
					++num_inputs;
					if (global_has_gnd && comps[i].conn_loc == 0) {
						cerr << "Error in TopoSort: Input shorted to GND." << endl;
					}
					if (comps[i].conn_loc == vdd_net) {
						cerr << "Error in TopoSort: Input shorted to VDD." << endl;
					}
					add_io_to_set(fanout, comps[i].conn_loc, i);
					net_flag[comps[i].conn_loc] |= IS_CONST_FLAG;
					dep_count[i] = 0;
					break;

				case ST_SEL_ID: // handled as non-const input
					add_io_to_set(fanout, comps[i].conn_loc, i);
					net_flag[comps[i].conn_loc] |= IS_STSEL_FLAG;
					dep_count[i] = 0;
					has_st_sel   = 1;
					break;
							
				default:
					cerr << "\nError: Unidentified component ID: " << ID << endl; 
			}
		}
	}
	found_power = found_ground = 0;
	input_nets  = memAlloc(input_nets,  num_inputs);
	output_nets = memAlloc(output_nets, num_outputs);

	// check for constant nets implying direction of bidir pins
	for (i = 0; i < (unsigned)nodes; ++i) {
		if (net_flag[i] & IS_CONST_FLAG) {
			if (!bidir.net_empty(i)) {
				not_end = bidir.set_iterator_net(i);
				while (not_end) {
					other_bidir = move_bidir_2port(bidir, fanin, fanout, comps, i); // bidir must be input			
					not_end     = bidir.advance_iterator(); // I/O status of nets that have been moved is checked in next loop
				}
			}
			if (!fanout.net_single(i) && !(net_flag[i] & IS_STSEL_FLAG)) {
				++const_driven_nets; // component fanouts to constant net
			}
		}
	}

	// check for odd fanout connections and implied bidirectional pin directions
	for (i = 0; i < (unsigned)nodes; ++i) {
		if (fanout.net_empty(i)) {
			if (bidir.net_empty(i)) {
				if (!fanout.net_empty(i)) { // otherwise it is floating
					++undriven_nets;
				}
			}
			else if (bidir.net_single(i)) {
				bidir.set_iterator_net(i);
				other_bidir = move_bidir_2port(bidir, fanout, fanin, comps, i); // assume bidir is really output
				if (other_bidir < i) { // if net already checked
					check_io_status_recur(bidir, fanin, fanout, comps, dep_count, other_bidir, i); // need to check I/O status of nets that have been moved
				}
			}
			// else need to iterate to determine dataflow direction in out_calc
		}
		else {
			if (fanout.net_multi(i) && !(net_flag[i] & IS_STSEL_FLAG)) {
				++multi_fo_nets;
				//if (str_arr != NULL) cout << "multiply driven net " << str_arr[i] << endl; // TESTING
			}
			if (!bidir.net_empty(i)) { // *** Careful with tri-state outputs ***
				not_end = bidir.set_iterator_net(i);
				while (not_end) {
					other_bidir = move_bidir_2port(bidir, fanin, fanout, comps, i); // assume bidir is really input
					if (other_bidir < i) { // if net already checked
						check_io_status_recur(bidir, fanin, fanout, comps, dep_count, other_bidir, i); // need to check I/O status of nets that have been moved
					}
					not_end = bidir.advance_iterator();
				}
			}
		}
		if (fanin.net_empty(i)) {
			if (fanout.net_empty(i)) {
				++floating_nets;
			}
			else if (i != 0 && i != vdd_net) { // ignore VDD and GND
				++unused_nets;
			}
		}
	}

	// find inputs and outputs
	for (i = 0; (int)i < num_comps; ++i) {
		ID = comps[i].id;
				
		if (ID == OUTPUT_ID) {
			comps[i].value = ++total_outputs;
		}	
		else if (ID == INPUT_ID) {
			input_nets[input_cntr++] = counter;
			comp_added[i] = 1;
		}
		else if (ID == ST_SEL_ID) {
			comp_added[i] = 1;
		}
		else if (ID == POWER_ID && !found_power) {
			comp_added[i] = 1;
			found_power   = 1;
		}
		else if (ID == GROUND_ID && !found_ground) {	
			comp_added[i] = 1;
			found_ground  = 1;
		}
		else if (get_order(ID) == flipflopx) {
			++num_ff;
		}
		if (comp_added[i] == 1) {
			copy_comp_sim(comps2[counter++], comps[i]);

			if (ID != ST_SEL_ID || force_init_states || num_inputs == 0 || (fanout.net_single(comps[i].conn_loc) && bidir.net_empty(comps[i].conn_loc))) {
				add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, comps[i].conn_loc, ntp_curr);
			}
		}
	}
	permute_output   = memAlloc_init(permute_output, total_outputs, (int)0);
	index_checkpoint = ntp_curr;
	level = level2   = 1;

	// add components in tolpological (levellized) order
	for (nnum = 0; nnum < ntp_curr; ++nnum) {
		net_id  = nets_to_proc[nnum];
		not_end = fanin.set_iterator_net(net_id);

		if (nnum >= index_checkpoint) {
			index_checkpoint = ntp_curr;
			if (phase == 1) {
				++level;
			}
			else {
				++level2;
			}
		}
		while (not_end) {
			i = fanin.get_curr();

			if (!comp_added[i]) {
				ID   = comps[i].id;
				conn = comps[i].conn;

				if (dep_count[i] == 0) {
					add_this_one = 0;

					if ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || ID == OPAMP_ID) { // standard logic gates + opamp + other logic							
						add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[4], ntp_curr);
						add_this_one = 1;
							
						if (ID == ADDER_ID) { // adder has two outputs
							add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[5], ntp_curr);
						}
					}
					else if (ID >= PRSL_ID && ID <= NTFF_ID) { // flipflops
						add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[3], ntp_curr);
						add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[5], ntp_curr);
						add_this_one = 1;
					} // transistors
					else if (get_order(ID) == transistorx || ID == NFET_ID || ID == PFET_ID) {
						add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[3], ntp_curr); // only one will actually be added
						add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[5], ntp_curr);
						add_this_one = 1;
					}
					else if (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) { // sources - do nothing
						// nothing
					}
					else if (get_order(ID) != icx) { // ignore ICs
						switch(ID) {
							
							case OUTPUT_ID:
								if (comps[i].value != 0) {
									output_nets[(int)comps[i].value-1] = counter;
									permute_output[outputs++]          = (int)comps[i].value;
									comps[i].value = 0;
									add_this_one   = 1;
									if (outputs == total_outputs) {
										last_output_added = 1;
									}
								}
								break;
							
							case BATTERY_ID:	
							case DIODE_ID: // change?					
							case RES_ID:
								add_this_one = 1; // only second iteration?
								add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[1], ntp_curr); // only one will actually be added
								add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, conn[4], ntp_curr);
								break;
								
							case CAP_ID: // ignore for now							
							case INDUCTOR_ID: // treated as wire							
							case ST_SEL_ID: // handled as input
							case POWER_ID: // power and ground - ignore?
							case GROUND_ID:
								break;
								
							default:
								cerr << "\nError: Unidentified component ID: " << ID << endl; 
						}
					}
					if (add_this_one) {
						copy_comp_sim(comps2[counter++], comps[i]);
						comp_added[i] = 1;				
						if (last_output_added && ID != OUTPUT_ID) {
							warning = 1;
							comps_left = num_comps - counter;
						}
						//fanin.remove_curr(); // remove link - optional
					}
				} // end if dep_count[i] == 0
			} // end if comp_added[i]
			not_end = fanin.advance_iterator();
		} // end while not_end
		if (nnum == ntp_curr && counter < num_comps && has_st_sel && phase == 1) { // at end of net queue with some components not processed, init states exist, and in phase 1
			phase = 2; // switch to phase 2 - initial states define ordering
			show_elapsed_time();	
			cout << "Switching to phase 2." << endl;

			for (i = 0; i < (unsigned)nodes; ++i) {
				if ((net_flag[i] & IS_STSEL_FLAG) && !(net_flag[i] & IS_CONST_FLAG)) {
					add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, i, ntp_curr);
				}
			}
			index_checkpoint = ntp_curr;
		}
	} // end for net_id
	
	level        = max(level, level2);
	temp_counter = counter;
	more_outputs = outputs;
	
	// add the rest of the components
	for (i = 0; (int)i < num_comps && counter < num_comps; ++i) {
		if (comp_added[i] == 0) {
			copy_comp_sim(comps2[counter++], comps[i]);
			
			if (comps[i].id == OUTPUT_ID) {
				if (comps[i].value != 0) {
					output_nets[(int)comps[i].value-1] = counter-1;
					permute_output[more_outputs++]     = (int)comps[i].value;
					comps[i].value = 0;
				}
				++temp_counter;	
			}
			else if (get_order(comps[i].id) == icx) {
				++temp_counter;
			}
		}	
	}

	// cleanup
	delete [] comp_added;
	delete [] net_flag;
	delete [] nets_to_proc;
	delete [] comps;
	delete [] dep_count;

	show_elapsed_time();	
	cout << "done." << endl;
	reset_timer(1);
	show_elapsed_time();
	cout << num_comps << " total component";
	get_print_plural(num_comps);
	cout << "." << endl;

	unreachable_outs  = total_outputs - outputs;
	unreachable_comps = counter - temp_counter;
	
	cout << "Maximum topological critical path is " << (level - 1) << " gate delays." << endl;

	// report warnings
	if (warning)
		cout << "Note: It appears that the output values can be calculated without evaluating " << comps_left << " of the gates/components." << endl;
	
	check_print_num_message(const_driven_nets, 1, "constant driven net");
	check_print_num_message(undriven_nets,     1, "undriven net");
	check_print_num_message(floating_nets,     1, "floating net");
	check_print_num_message(multi_fo_nets,     2, "multiply driven net");
	check_print_num_message(unused_nets,       2, "unused net");
	check_print_num_message(unreachable_outs,  1, "unreachable output");
	check_print_num_message(unreachable_comps, 1, "unreachable component");

	// DEBUGGING
	if (test_show_topo_sort) {
		cout << "Components topologically sorted are: " << endl;
		for(i = 0; (int)i < num_comps; ++i) {
			cout << get_name(comps2[i].id, 0) << ", ";
		}
		cout << "Click to continue." << endl;
		get_window_click();
	}
	return comps2;
}




void get_fanin_pointers(NetPointerSet &fanin, int *conn, char *dep_count, unsigned i) {

	dep_count[i] = add_io_to_set(fanin, conn[0], i) 
		         + add_io_to_set(fanin, conn[1], i) 
		         + add_io_to_set(fanin, conn[2], i);
}




char add_io_to_set(NetPointerSet &ios, int conn, unsigned i) {

	if (conn >= 0) {
		ios.insert(conn, i);
		return 1;
	}
	return 0;
}




void check_io_status_recur(NetPointerSet &bidir, NetPointerSet &fanin, NetPointerSet &fanout, comp_sim *comps, char *dep_count, unsigned i, unsigned curr_net_id) {

	int not_end;
	unsigned other_bidir;
	net_it_state state;

	if (fanout.net_empty(i)) {
		if (bidir.net_single(i)) {
			state = bidir.it_get_state();
			bidir.set_iterator_net(i);
			other_bidir = move_bidir_2port(bidir, fanout, fanin, comps, i); // assume bidir is really output
			if (other_bidir < curr_net_id) { // if net already checked
				check_io_status_recur(bidir, fanin, fanout, comps, dep_count, other_bidir, curr_net_id); // need to check I/O status of nets that have been moved
			}
			bidir.net_set_state(state);
		}
		// else need to iterate to determine dataflow direction in out_calc
	}
	else {
		if (!bidir.net_empty(i)) {
			state = bidir.it_get_state();
			not_end = bidir.set_iterator_net(i);
			while (not_end) {
				other_bidir = move_bidir_2port(bidir, fanin, fanout, comps, i); // assume bidir is really input
				if (other_bidir < curr_net_id) { // if net already checked
					check_io_status_recur(bidir, fanin, fanout, comps, dep_count, other_bidir, curr_net_id); // need to check I/O status of nets that have been moved
				}
				not_end = bidir.advance_iterator();
			}
			bidir.net_set_state(state);
		}
	}
}




unsigned move_bidir_2port(NetPointerSet &bidir, NetPointerSet &ios1, NetPointerSet &ios2, comp_sim *comps, unsigned i) {

	int not_end;
	unsigned comp_id, other_bidir;
	net_it_state state;

	comp_id     = bidir.get_curr();
	other_bidir = get_bidir_other_net(comps[comp_id], i);

	if (other_bidir == i) {
		return other_bidir;
	}
	move_bidir_to_io(bidir, ios1, i);

	state   = bidir.it_get_state();
	not_end = bidir.set_iterator_net(other_bidir);

	while (not_end) {
		if (bidir.get_curr() == comp_id) {
			move_bidir_to_io(bidir, ios2, other_bidir);
			bidir.net_set_state(state);
			return other_bidir;
		}
		not_end = bidir.advance_iterator();
	}
	bidir.net_set_state(state);

	internal_error();
	cerr << "Error: Invalid net<=>comp pointers in move_bidir_2port." << endl;

	return other_bidir;
}




unsigned get_bidir_other_net(comp_sim &comp, unsigned i) {

	int ID(comp.id);

	if (get_order(ID) == transistorx || ID == NFET_ID || ID == PFET_ID) {
		if (comp.conn[3] != i) {
			return comp.conn[3];
		}
		if (comp.conn[5] == i) {
			cout << "Warning: Transistor S/D or E/C pins shorted together." << endl;
		}
		return comp.conn[5];
	}
	else if ((ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) || ID == BATTERY_ID || ID == DIODE_ID || ID == RES_ID || ID == CAP_ID || ID == INDUCTOR_ID) {
		if (comp.conn[1] != i) {
			return comp.conn[1];
		}
		if (comp.conn[4] == i) {
			cout << "Warning: Component I/O pins shorted together." << endl;
		}
		return comp.conn[4];
	}
	internal_error();
	cerr << "Error: Bidirectional net points to illegal component type: " << ID << "." << endl;

	return 0;
}




void move_bidir_to_io(NetPointerSet &bidir, NetPointerSet &ios, unsigned i) {

	ios.insert(i, bidir.get_curr());
	bidir.remove_curr();
}




void add_net_to_queue(int *nets_to_proc, NetPointerSet &fanin, char *dep_count, char *net_flag, unsigned net_id, unsigned &ntp_curr) {

	int not_end;
	net_it_state state;

	if (net_flag[net_id] & WAS_PROC_FLAG) {
		return;
	}
	net_flag[net_id] |= WAS_PROC_FLAG;
	nets_to_proc[ntp_curr++] = net_id;

	state   = fanin.it_get_state();
	not_end = fanin.set_iterator_net(net_id);

	while (not_end) {
		if (dep_count[fanin.get_curr()] <= 0) {
			internal_error();
			cerr << "Error: Dependency counter of component #" << fanin.get_curr() << " is negative in add_net_to_queue()." << endl;
			break;
		}
		--dep_count[fanin.get_curr()];
		not_end = fanin.advance_iterator();
	}
	fanin.net_set_state(state);
}




void copy_comp_sim(comp_sim &comp2, comp_sim &comp1) {

	comp2.id       = comp1.id;
	comp2.value    = comp1.value;
	comp2.state    = comp1.state;
	comp2.location = comp1.location;
	comp2.conn_loc = comp1.conn_loc;
	comp2.flags    = comp1.flags;

	comp2.conn[0] = comp1.conn[0];
	comp2.conn[1] = comp1.conn[1];
	comp2.conn[2] = comp1.conn[2];
	comp2.conn[3] = comp1.conn[3];
	comp2.conn[4] = comp1.conn[4];
	comp2.conn[5] = comp1.conn[5];
}




void check_print_num_message(int number, int mtype, const char *obj_type) {

	if (number > 0) {
		switch (mtype) {
		case 1:
			cout << "Warning: ";
			break;
		case 2:
			cout << "Note: ";
			break;
		case 3:
		default:
			cout << "Error: ";
		}
		if (number == 1)
			cout << "There is " << number << " " << obj_type << "." << endl;
		else
			cout << "There are " << number << " " << obj_type << "s." << endl;
	}
}






