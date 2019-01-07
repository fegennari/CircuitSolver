
comp_sim *topo_sort(comp_sim *comps, int *&permute_output, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int nodes, int &num_inputs, int &num_ff, int num_comps);
int propagate(comp_sim &comp, int *can_propagate, int prop_num);


// ************************************************
// Old Topo Sort
// ************************************************


// Topological sort based on component connections - speeds up analysis
comp_sim *topo_sort(comp_sim *comps, int *&permute_output, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int nodes, int &num_inputs, int &num_ff, int num_comps) {
	
	if (use_graph_topo_sort) {
		return topo_sort_graph_based(comps, permute_output, fanin, fanout, bidir, str_arr, nodes, num_inputs, num_ff, num_comps);
	}

	unsigned int i;
	int *conn, *can_propagate = NULL, ID, counter(0), comp_num, outputs(0), add_this_one, comps_left(0);
	int more_outputs, total_outputs(0), warning(0), temp_counter, found_ground(0), found_power(0);
	int unreachable_outs, unreachable_comps;
	char has_changed, *comp_added = NULL;
	comp_sim *comps2 = NULL;
		
	reset_timer(1);
	show_elapsed_time();	
	cout << "Finding circuit topology..." << endl;
	
	num_inputs     = 0;
	num_ff         = 0;
	num_iterations = 0;

	// *** USE input_nets, output_nets ***
	
	can_propagate = memAlloc_init(can_propagate, nodes+1,  0);
	comp_added    = memAlloc_init(comp_added, num_comps+1, (char)0);
	comps2        = memAlloc(comps2, num_comps+1);

	// find inputs and outputs
	for(i = 0; (int)i < num_comps; ++i) { // find inputs and outputs
		ID = comps[i].id;
				
		if (ID == OUTPUT_ID) {
			comps[i].value = ++total_outputs;
		}	
		else if (ID == INPUT_ID) {
			++num_inputs;
			comp_added[i] = 1;
		}
		else if (ID == ST_SEL_ID) {
			comp_added[i] = 1;
		}
		else if (ID == POWER_ID && !found_power) {
			comp_added[i] = 1;
			found_power = 1;
		}
		else if (ID == GROUND_ID && !found_ground) {	
			comp_added[i] = 1;
			found_ground = 1;
		}
		else if (get_order(ID) == flipflopx) {
			++num_ff;
		}
		if (comp_added[i] == 1) {
			copy_comp_sim(comps2[counter++], comps[i]);
			if (ID != ST_SEL_ID || force_init_states || no_inputs) {
				can_propagate[comps[i].conn_loc] = -1;
			}
		}
	}
	permute_output = memAlloc_init(permute_output, total_outputs, (int)0);
	
	// topologically order components
	do {		
		has_changed = false;
		comp_num = 0;		
	
		for(i = 0; (int)i < num_comps; ++i) {
			ID   = comps[i].id;
			conn = comps[i].conn;				
							
			if (comp_added[comp_num] == 0 && propagate(comps[i], can_propagate, num_iterations + 1)) {
				add_this_one = 0;
															
				if ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || ID == OPAMP_ID) { // standard logic gates + opamp + other logic
					if (propagate(comps[i], can_propagate, 0)) {							
						can_propagate[conn[4]] = num_iterations + 1;
						add_this_one = 1;
						
						if (ID == ADDER_ID) { // adder has two outputs
							can_propagate[conn[5]] = num_iterations + 1;
						}	
					}
				}
				else if (ID >= PRSL_ID && ID <= NTFF_ID) { // flipflops
					if (propagate(comps[i], can_propagate, 0)) {							
						can_propagate[conn[3]] = num_iterations + 1;
						add_this_one = 1;
					}
					if (propagate(comps[i], can_propagate, 0)) {							
						can_propagate[conn[5]] = num_iterations + 1;
						add_this_one = 1;	
					}
				} // transistors
				else if (get_order(ID) == transistorx || ID == NFET_ID || ID == PFET_ID) {
					if (can_propagate[conn[1]] != 0) {
						if (can_propagate[conn[3]] != 0 && can_propagate[conn[5]] == 0) {
							can_propagate[conn[5]] = num_iterations + 1;
							allow_mult_loops = 1;	
							add_this_one = 1;
						}
						else if (can_propagate[conn[5]] != 0 && can_propagate[conn[3]] == 0) {
							can_propagate[conn[3]] = num_iterations + 1;
							allow_mult_loops = 1;
							add_this_one = 1;
						}
					}
				}
				else if (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) // sources - do nothing
					add_this_one = 0;
				
				else if (get_order(ID) != icx) { // ignore ICs
					switch(ID) {
						
						case OUTPUT_ID:
							if (comps[i].value != 0) {
								if (can_propagate[comps[i].conn_loc] != num_iterations + 1 && can_propagate[comps[i].conn_loc] != 0) {
									permute_output[outputs++] = (int)comps[i].value;
									comps[i].value = 0;
									add_this_one = 1;
								}
							}
							break;
						
						case BATTERY_ID:	
						case DIODE_ID: // change?					
						case RES_ID: 
							if (can_propagate[conn[1]] != 0 && can_propagate[conn[4]] == 0) {
								can_propagate[conn[4]] = num_iterations + 1;	
								add_this_one = 1; // only second iteration?
							}
							else if (can_propagate[conn[4]] != 0 && can_propagate[conn[1]] == 0) {
								can_propagate[conn[1]] = num_iterations + 1;	
								add_this_one = 1; // only second iteration?
							}
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
					comp_added[comp_num] = 1;					
					has_changed = true;
				}
			}
			++comp_num;										
		}		
		++num_iterations;
		
		if (outputs == total_outputs && has_changed) {
			if (warning == 0) {
				warning = 1;
				comps_left = num_comps - counter;
			}
			else
				warning = 2;
		}
				
	} while (num_iterations <= max(nodes, num_comps) && counter < num_comps && has_changed);
		
	temp_counter = counter;
	more_outputs = outputs;
	
	// add the rest of the components
	for (i = 0; (int)i < num_comps && counter < num_comps; ++i) {
		if (comp_added[i] == 0) {
			copy_comp_sim(comps2[counter++], comps[i]);
			
			if (comps[i].id == OUTPUT_ID && comps[i].value != 0) {
				permute_output[more_outputs++] = (int)comps[i].value;
				comps[i].value = 0;
			}
			if (get_order(comps[i].id) == icx || comps[i].id == OUTPUT_ID)
				++temp_counter;	
		}	
	}				
	delete [] can_propagate;
	delete [] comp_added;
	delete [] comps;		
	
	show_elapsed_time();	
	cout << "done.\n" << num_comps << " total component";
	if (num_comps != 1)
		cout << "s";
	cout << "." << endl;
	
	reset_timer(1);
	show_elapsed_time();	
	cout << "Maximum topological critical path is " << (num_iterations - 1) << " gate delays." << endl;
	
	num_iterations    = 0;
	unreachable_outs  = total_outputs - outputs;
	unreachable_comps = counter - temp_counter;
	
	// report warnings
	if (warning == 2)
		cout << "Note: It appears that the output values can be calculated without evaluating " << comps_left << " of the gates/components." << endl;
	
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




int propagate(comp_sim &comp, int *can_propagate, int prop_num) {
	
	int *conn = comp.conn;

	if (conn[0] >= 0 && can_propagate[conn[0]] == prop_num)
		return 0;
		
	if (conn[1] >= 0 && can_propagate[conn[1]] == prop_num)
		return 0;
		
	if (conn[2] >= 0 && can_propagate[conn[2]] == prop_num)
		return 0;
	
	if (comp.id >= QAND_ID && comp.id <= QXNOR_ID && conn[3] >= 0 && can_propagate[conn[3]] == prop_num)
		return 0;
		
	return 1;
}