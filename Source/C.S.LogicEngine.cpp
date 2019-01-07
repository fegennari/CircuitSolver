#include "CircuitSolver.h"
#include "Logic.h"
#include "GridArray.h"
#include "StringTable.h"
#include "NetPointerSet.h"
#include "Propagations.h"
#include "PropagationFunctions.h"

#include "C.S.LogicComponents.cpp"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LogicEngine.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Engine
// By Frank Gennari
int const test_show_nets        = 0; // debugging - user can click on screen and get net numbers
int const allow_cyclic_circuits = 1; // slows down simulations of invalid circuits though
int const show_iterations       = 0;
int const check_dup_conflicts   = 1;
int const use_fast_eval         = 0;
int const lazy_eval             = 1; // *** make this a user option later ***

int const check_interval_time = 1000; // in ms


#define TRUTH_TABLE_FILE "TruthTable.txt"
#define FF_DEF_DELAY     0
#define TRANS_DEF_DELAY  0
#define RES_DEF_DELAY    0
#define DIODE_DEF_DELAY  0


int PREVIOUS_CLOCK(0), EDGE(1), AUTOTRIGGER(0), LOOP(0), NOCLOCK, AC1DC0(0), FAST(0), Q_POLARITY(0), no_inputs(0), force_init_states(0), dont_print_tt(1), allow_mult_loops(allow_cyclic_circuits); 
int loop_num, ncomps(0), conflict, num_iterations(0), unknown_state, illegal_input(0), illegal_FF(0), useDELAY(1), read_only_error(0), Q_warning(0), iteration_error, nivs(0); // mostly for error checking
char INITIAL_OUTPUT(0), PRESET(0), CLEAR(0), CARRY_OUT(0), last_known_value(0);
double supply_voltage(default_voltage), ground(0), max_gate_delay(0);
IC temp_ic;


clist *conflict_list;

extern int o_size, changed, changed3, Opened, useDIGITAL, useHASHTABLE, GV, NODES, useCOMPLEX, x_blocks, y_blocks, mos_delay_not_wl;
extern const char *hasValue;
extern GridArray NODENUMBER;


component *evaluate_logic(component *head);
int check_for_errors(int num_outputs, int &error, BINX& OUTx, int is_last);
void display_error_messages(BINX& OUTx, int num_inputs, int num_ff); 
component *delete_all(char *NODEVALUE, string *str_arr, int *permute_output, unsigned *input_nets, unsigned *output_nets, double *net_delay, char *read_only, char *initially_set, component *head, component *unsim, wire *wiresave, BINX *input_vectors, comp_sim *cs_arr, int temp_x_blocks, int temp_y_blocks);
void create_comp_sims(component *head, comp_sim *cs_arr, int num_components);
void set_initial_states(BINX &init_Q_states, BINX &init_O_states, int num_outputs, int num_ff);
int open_tt_for_write(ofstream &outfile, int num_inputs, int num_outputs, int num_ivs);
int write_tt_line(ofstream &outfile, BINX &INx, BINX &OUTx, double logic_delay, int num_inputs, int num_outputs);
int display_saved_table();
void outfile_bit(char bit, ofstream &outfile);
void draw_truth_table(comp_sim *comps, BINX &init_Q_states, string *str_arr, unsigned *input_nets, unsigned *output_nets, int num_inputs, int num_outputs);
void print_table(BINX& INx, int num_inputs);
void write_outputs(int num_outputs, BINX& OUTx);
void print_outputs(BINX& OUTx, int num_outputs);
void out_calc(BINX &OUTx, comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, int nodes, int num_outputs, int num_comps);
void out_calc_graph(BINX &OUTx, comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, int nodes, int num_outputs, int num_comps);
void final_check(comp_sim *comps, BINX &OUTx, int num_comps);
char propagate_once(comp_sim &comp, int cid, int &outputs, char Value, char *&NODEVALUE, char *read_only, char *initially_set, double *net_delay, int *permute_output, BINX &OUTx, char &making_progress, int &out_set_ctr); 
void set_state(comp_sim &comp, char *NODEVALUE, char *read_only, char *initially_set, double *net_delay, double this_gate_delay, int nnumber, char Value, int index, int outpos, char &making_progress);
void calc_delay(comp_sim &comp, char *NODEVALUE, int nnumber, double *net_delay, double this_gate_delay);
int is_controlling_value(int ID, int value);
void state_machine(comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *input_nets, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int num_inputs, int num_outputs, int num_flipflops, int nodes, int num_ivs, BINX *input_vectors, int num_comps);
void set_output_state(BINX& OUTx, comp_sim &comp, int outputs, char *NODEVALUE, int &out_set_ctr);
void count_loop(int dec, int last_val, int loop_num);
int count_io(component *head);
void find_inputs(comp_sim *comps, string *str_arr, BINX &INPUT, unsigned *input_nets, int num_inputs, char *NODEVALUE, int show_inputs); 
void find_outputs(comp_sim *comps, string *str_arr, int *permute_output, unsigned *output_nets, int num_outputs);
int reset_circuit2(comp_sim *comps, char *NODEVALUE, double *net_delay, char *read_only, char *initially_set, int nodes, int num_comps);
void reset_states(char *NODEVALUE, double *net_delay, int nodes, int init);
void show_conflicts(comp_sim *comps, int num_comps);
void add_conflict(int location, int type);
void label_io(int location, int ID, int use_alpha, char name_char);
void add_gate_to_netlist(ofstream &outfile, int id, int location);


int open_netlist_file(ifstream &infile, string &input_buffer);
comp_sim *import_netlist_to_comp_sim(ifstream &infile, StringTable &net_names, string *&str_arr, string &input_buffer, int &num_comps, int &num_nets);

int read_or_generate_ivs(BINX *&input_vectors, int &num_ivs, int &num_inputs, int &mode, unsigned &num_permutations);
void random_permutation(BINX &iv, int &last_permute, unsigned num_permutations);

comp_sim *topo_sort(comp_sim *comps, int *&permute_output, unsigned *&input_nets, unsigned *&output_nets, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int nodes, int &num_inputs, int &num_outputs, int &num_ff, int num_comps);

void set_gate_delays(component *head, comp_sim *comps, int nodes, int num_comps, int mode);

component *simulation_setup(component *head, wire *&wires, int &num_wires);
component *init_data_structures(component *head, wire *&wiresave, component *&unsim, wire *wires, int num_wires, int &nodes, int &error, int is_digital, int &comp_count);
component *reset_circuit(component *head, component *unsim, wire *wiresave);

component *link_components(component *head);

void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);

void draw_text(string text, int box_num, int t_size);
void draw_char(char x, int box_num, int color, int size);
void draw_RED_CIRCLE(int box_num);
void draw_blue_x(int location);
void draw_SQUAREWAVE(int box_num);

void get_DC_waveform_cursor();

point get_center(int box_num);
int get_scrolled_window_click();

int is_in_window(int location);

void click_and_show_nets();




component *evaluate_logic(component *head) {
	
	int num_inputs(0), num_outputs(0), num_ff(0), num_ivs(0), curr_iv(0), iv_cntr(0);
	int error(1), init_error(0), iteration_time, check_interval(1), iv_mode(0), last_permute(-1);
	int nodes(0), errors, shown_input_error(0), num_comps(0), num_wires(0), tt_open, tt_good(1);
	int temp_x_blocks(x_blocks), temp_y_blocks(y_blocks);
	int *permute_output = NULL;
	char *NODEVALUE = NULL;
	unsigned *input_nets = NULL, *output_nets = NULL, num_permutations(NUM_PERMUTATIONS_PER_IV);
	double *net_delay = NULL, max_delay(0), logic_delay(0);
	char *read_only = NULL, *initially_set = NULL;
	string *str_arr = NULL, input_buffer;
	BINX *input_vectors = NULL, OUTx, init_O_states, init_Q_states;
	wire *wiresave = NULL, *hier_wires = NULL;
	component *unsim = NULL;
	comp_sim *cs_arr = NULL;
	NetPointerSet fanin(0), fanout(0), bidir(0);
	ifstream infile;
	ofstream tt_file;
	
	LOOP = NOCLOCK   = loop_num = last_known_value = unknown_state = illegal_FF = read_only_error = iteration_error = no_inputs = 0;
	useHASHTABLE     = -1;
	useDIGITAL       = 1;
	allow_mult_loops = allow_cyclic_circuits;

	if (head == NULL) { // try to open netlist
		if (!open_netlist_file(infile, input_buffer)) {
			return head;
		}
		StringTable net_names(0); // now we can use the StringTable
		nodes    = 1;
		cs_arr   = import_netlist_to_comp_sim(infile, net_names, str_arr, input_buffer, num_comps, nodes);
		x_blocks = y_blocks = 0;
	}
	else { // only if circuit already open or opening non-netlist		
		if (changed3 || Opened) {
			show_clock();
			useCOMPLEX = 0;
			head       = simulation_setup(head, hier_wires, num_wires);
		}
		else {
			if (!display_saved_table()) {
				cerr << "\nError displaying saved truth table." << endl;
			}
			return head;
		}
		head = init_data_structures(head, wiresave, unsim, hier_wires, num_wires, nodes, init_error, 1, num_comps);	
		if (init_error) {
			return head;
		}
		cs_arr = memAlloc(cs_arr, num_comps);	
		create_comp_sims(head, cs_arr, num_comps);
	}
	if (useDELAY && GV)	{
		net_delay = memAlloc(net_delay, nodes+1);
	}
	set_gate_delays(head, cs_arr, nodes, num_comps, 1);
	cs_arr = topo_sort(cs_arr, permute_output, input_nets, output_nets, fanin, fanout, bidir, str_arr, nodes, num_inputs, num_outputs, num_ff, num_comps);

	reset_timer(1);

	NODEVALUE     = memAlloc(NODEVALUE,     nodes+1);
	read_only     = memAlloc(read_only,     nodes+1);
	initially_set = memAlloc(initially_set, nodes+1);

	if (!read_or_generate_ivs(input_vectors, num_ivs, num_inputs, iv_mode, num_permutations)) {
		return delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);
	}
	ncomps = reset_circuit2(cs_arr, NODEVALUE, net_delay, read_only, initially_set, nodes, num_comps);	
	find_inputs(cs_arr, str_arr, input_vectors[0], input_nets, num_inputs, NODEVALUE, 1);
	find_outputs(cs_arr, str_arr, permute_output, output_nets, num_outputs);

	if (num_inputs == 0) {
		no_inputs = 1;
	}
	if (illegal_input) {
		cerr << "\nError: Illegal input location! State at input node is already set." << endl;
		return delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);
	}
	if (num_ff > 0) {
		if (num_inputs == 0 && num_outputs > 0) { // Must num_inputs == 0?
			state_machine(cs_arr, NODEVALUE, net_delay, permute_output, input_nets, output_nets, read_only, initially_set, fanin, fanout, bidir, str_arr, num_inputs, num_outputs, num_ff, nodes, num_ivs, input_vectors, num_comps);
			return delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);
		}
		set_initial_states(init_Q_states, init_O_states, num_outputs, num_ff);
	}
	OUTx.init(num_outputs);
	OUTx.setall(-1);
	show_elapsed_time();

	tt_open = open_tt_for_write(tt_file, num_inputs, num_outputs, num_ivs);
	draw_truth_table(cs_arr, init_Q_states, str_arr, input_nets, output_nets, num_inputs, num_outputs);

	// *** ADD NET NAMES TO OUTPUT ***

	reset_timer(1);
	draw_status_bar();
	draw_status(" Evaluating");

	for (iv_cntr = 0; iv_cntr < num_ivs; ++iv_cntr) {
		if (iv_cntr != 0) {
			if (iv_mode == 1) {
				input_vectors[curr_iv].increment(); // exhaustive simulation, curr_iv remains at 0
			}
			else if (iv_mode == 2) {
				random_permutation(input_vectors[curr_iv], last_permute, num_permutations); // random gray code
			}
		}
		if (iv_cntr > 0) {
			find_inputs(cs_arr, str_arr, input_vectors[curr_iv], input_nets, num_inputs, NODEVALUE, 0);

			if (illegal_input && !shown_input_error) {
				stop_timer();
				beep();
				cout << "\nWarning: Attempted forced state of input. Inputs may be shorted together or to power or ground. Continue?  ";
				if (!decision()) {
					if (tt_open)
						tt_file.close();
					return delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);
				}
				else
					shown_input_error = 1;
					
				start_timer();
			}
		}
		out_calc(OUTx, cs_arr, NODEVALUE, net_delay, permute_output, output_nets, read_only, initially_set, fanin, fanout, bidir, nodes, num_outputs, num_comps);

		if (iv_cntr == 0) {
			iteration_time = max(1, elapsed_time()); // in ms
			check_interval = max(1, check_interval_time/iteration_time); // check for user cancel only every so many interations based on timing
		}
		if (GV) {
			max_delay = max(max_delay, max_gate_delay);
		}
		get_DC_waveform_cursor();
		print_table(input_vectors[curr_iv], num_inputs);

		if (GV) {
			logic_delay = max_gate_delay;
		}
		if (tt_open && tt_good) {
			tt_good = write_tt_line(tt_file, input_vectors[curr_iv], OUTx, logic_delay, num_inputs, num_outputs);
		}
		errors = check_for_errors(num_outputs, error, OUTx, (num_inputs == 0 || iv_cntr == (num_ivs - 1)));
				
		if (errors == 1) {
			reset_cursor();
			if (tt_open)
				tt_file.close();
			return delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);
		}
		else if (errors == 2) {
			break;
		}
		if (iv_cntr%check_interval == 0 && interrupt()) {
			break;
		}
		if (iv_mode == 0) {
			++curr_iv;
		}
	}
	if (tt_open) {
		tt_file.close();
	}
	if (use_fast_eval) {
		final_check(cs_arr, OUTx, num_comps);
	}
	cout << "\nSimulation time:";	
	show_elapsed_time();
	cout << endl;
	draw_status_bar();

	if (iv_cntr >= num_ivs) 
		draw_status(" Finished Evaluating");
	else
		draw_status(" Interrupted");
			
	if (useDELAY && GV && OUTx.get_bit(0) != 2 && max_delay >= 0) {
		cout << "\nMax gate delay to outputs = " << max_delay << ".";
	}
	display_error_messages(OUTx, num_inputs, num_ff);
	
	if (test_show_nets) { // Net Testing
		click_and_show_nets();
	}
	head   = delete_all(NODEVALUE, str_arr, permute_output, input_nets, output_nets, net_delay, read_only, initially_set, head, unsim, wiresave, input_vectors, cs_arr, temp_x_blocks, temp_y_blocks);			
	Opened = changed3 = 0;
	nivs   = iv_cntr;
	reset_cursor();
	
	return head;
}





int check_for_errors(int num_outputs, int &error, BINX& OUTx, int is_last) {
	
	int flag(0);

	stop_timer();
	
	if (OUTx.get_bit(0) == -1) {
		beep();
		cout << "?\nError: No output found! Please designate an output and try again." << endl;
		reset_cursor();
		return 2;
	}
	write_outputs(num_outputs, OUTx);

	if (error == 1) {
		if (OUTx.any_equal_to(2)) {		
			cout << "\nError: Unknown input of logic gate! One or more output(s) cannot be calculated for this state. ";
			
			if (last_known_value != 0 && is_in_window(last_known_value)) {
				draw_char('S', last_known_value, 1, 20);
				cout << "The last evaluated component is marked with a red S. ";
			}
			flag = 1;
		}
		else if (conflict == 1) {
			if (read_only_error)
				cerr << "\nWarning: Attempted force of fixed state node (input, power, or ground shorted).";
				
			cerr << "\nWarning: Conflicting logic states found at location(s) circled red. Results may be incorrect. ";
			flag = 1;
		}
		if (flag) {
			beep();
			if (!is_last) {
				cout << "Continue?  ";
				if (!decision()) { 
					cout << endl;
					return 1;
				}
			}
			else
				cout << endl;
									
			error = 0;
		}
	}
	start_timer();
		
	return 0;
}





void display_error_messages(BINX& OUTx, int num_inputs, int num_ff) {
	
	if (num_inputs > 8) { // 8???
		get_continue();
	}
	#ifdef CAN_SCROLL // Temporary fix since Win32 console currently doesn't scroll, must fix that later
	if (num_inputs > 4 || (num_inputs > 3 && (read_only_error || conflict || illegal_FF || unknown_state || (num_ff > 0 && NOCLOCK))))
		cout << "\n\nClick on Cancel (X) to scroll the console window and then type any character and press Return to continue." << endl;
	else
		cout << "\n";
	#else
	cout << "\n";
	#endif
	
	if (read_only_error) {
		cout << "\nWarning: Attempted force of fixed state node (input, power, or ground shorted). Results may be incorrect.";
	}	
	if (Q_warning) {
		cout << "\nWarning: " << (supply_voltage - ground) << "V across B-E junction of transistor may exceed maximum value." << endl;
	}
	if (conflict) {
		cout << "\nWarning: Conflicting logic states found at location(s) marked in red. Results may be incorrect.";
	}
	if (illegal_FF) {
		cout << "\nWarning: Illegal input to RS Latch/FF - R and S = 1." << endl;		
	}
	if (unknown_state) {
		if (OUTx.get_bit(0) != 2)
			cout << "\nUnknown states are marked with red '?'s." << endl;
		else if (OUTx.get_bit(0) != -1) 
			cout << "\nNote: The output was found, but some other states were unknown(marked with '?'). Possible problems are multiple paths or incorrectly connected components." << endl;
	}
	if (num_ff > 0 && NOCLOCK) {
		cout << "\nWarning: No clock input was found, so the clock could not be updated and remained at the initial state." << endl;
		NOCLOCK = 0;
	} 	
	if (iteration_error == -1) {
		cout << "\nWarning: Outputs marked with * may not be correct." << endl;
	}
}





component *delete_all(char *NODEVALUE, string *str_arr, int *permute_output, unsigned *input_nets, unsigned *output_nets, double *net_delay, char *read_only, char *initially_set, component *head, component *unsim, wire *wiresave, BINX *input_vectors, comp_sim *cs_arr, int temp_x_blocks, int temp_y_blocks) {

	if (NODEVALUE != NULL)      delete [] NODEVALUE;
	if (permute_output != NULL) delete [] permute_output;
	if (input_nets != NULL)     delete [] input_nets;
	if (output_nets != NULL)    delete [] output_nets;
	if (read_only != NULL)      delete [] read_only;	
	if (initially_set != NULL)  delete [] initially_set;	
	if (input_vectors != NULL)  delete [] input_vectors;	
	if (cs_arr != NULL)         delete [] cs_arr;
	if (str_arr != NULL)        delete [] str_arr;

	if (useDELAY && GV && net_delay != NULL) {
		delete [] net_delay;
	}
	x_blocks = temp_x_blocks;
	y_blocks = temp_y_blocks;
		
	return reset_circuit(head, unsim, wiresave);
}




void create_comp_sims(component *head, comp_sim *cs_arr, int num_components) {

	int *conn, counter(0);
	component *temp = head;
	connections x;
		
	while(counter < num_components) {
		if (temp == NULL) {
			internal_error();
			cerr << "Error: NULL component pointer in create_comp_sims." << endl;
			return;
		}
		x    = get_connections(temp->location, temp->id, temp->flags);
		conn = cs_arr[counter].conn;
		
		cs_arr[counter].id       = temp->id;
		cs_arr[counter].value    = temp->value;
		cs_arr[counter].state    = temp->state;
		cs_arr[counter].location = temp->location;
		cs_arr[counter].flags    = temp->flags;
		
		if(x.in1  > 0) conn[0] = NODENUMBER[x.in1];  else conn[0] = -1;
		if(x.in2  > 0) conn[1] = NODENUMBER[x.in2];  else conn[1] = -1;
		if(x.in3  > 0) conn[2] = NODENUMBER[x.in3];  else conn[2] = -1;
		if(x.out1 > 0) conn[3] = NODENUMBER[x.out1]; else conn[3] = -1;
		if(x.out2 > 0) conn[4] = NODENUMBER[x.out2]; else conn[4] = -1;
		if(x.out3 > 0) conn[5] = NODENUMBER[x.out3]; else conn[5] = -1;

		if (temp->location == x.in2) {
			cs_arr[counter].conn_loc = conn[1];
		}
		else if (temp->location == x.out2) {
			cs_arr[counter].conn_loc = conn[4];
		}
		else {
			cs_arr[counter].conn_loc = -1;
		}
		++counter;
		temp = temp->next;
	}
	if (temp != NULL) {
		internal_error();
		cerr << "Error: Component pointer should be NULL in create_comp_sims." << endl;
	}
}




void set_initial_states(BINX &init_Q_states, BINX &init_O_states, int num_outputs, int num_ff) {

	char state;
	int cont(0);

	init_O_states.init(num_outputs);
	init_Q_states.init(num_ff);
	
	do {											
		cout << "\nEnter the Initial Clock State (0, 1";
		if (!LOOP)
			cout << ", or (A)utotrigger";
		cout << ")  :";
						 					
		state = inc();
		
		switch (state) {
		
			case '0':
				PREVIOUS_CLOCK = 0;
				if (LOOP) {
					CLEAR = 1;
					PRESET = 0;
				} 
				break;
				
			case '1':
				PREVIOUS_CLOCK = 1;
				if (LOOP) {
					PRESET = 1;
					CLEAR = 0;
				}
				break;
				
			case 'a':
			case 'A':
				if (!LOOP) {
					AUTOTRIGGER = 1;
					break;
				}
			default:
				beep();
				cout << "Invalid selection! Please enter only 0 or 1." << endl;
				if (!LOOP)
					cout << " or A";
				cout << "." << endl;
		}				
	} while (state != '0' && state != '1' && ((!comp_char(state, 'a')) || LOOP));
	
	if (LOOP) {
		cout << "\nSet up to " << num_outputs << " outputs to initial states(if connected directly to Q of flipflops)?  ";
		if (decision()) {
			cout << "\nEnter the initial output states in 1s and 0s:  ";
			inb(init_O_states, num_outputs);
		}
		else {
			cout << "\nSet Q states of up to " << num_ff << " flipflops to initial values?  ";
			if (decision()) {
				cout << "\nEnter the initial Q states in 1s and 0s:  ";
				inb(init_Q_states, num_ff); 
			}
			else
				cont = 1;	
		}
	}
	if (!LOOP || cont) do {											
		cout << "\nEnter the Initial Output (Q) of the Flipflop(s) (0 or 1):  ";
						 					
		state = inc();
						
		if (state == '0')
			INITIAL_OUTPUT = 0;
			 						
		else if (state == '1')
			INITIAL_OUTPUT = 1;
							
		else {
			beep();
			cout << "Invalid selection! Please enter only 0 or 1." << endl;
		}
	} while (state != '0' && state != '1');
}




int open_tt_for_write(ofstream &outfile, int num_inputs, int num_outputs, int num_ivs) {
	
	cout << "Creating Truth Table...";
	
	outfile.open(TRUTH_TABLE_FILE, ios::out | ios::trunc);

	if (outfile.fail() || !filestream_check(outfile)) {
		beep();
		cerr << "\nError: Could not write truth table file." << endl;
		return 0;
	}
	if (!(outfile << num_ivs << " " << num_inputs << " " << num_outputs << endl)) {
		beep();
		cerr << "\nError writing truth table header." << endl;
		return 0;
	}
	return 1;
}




int write_tt_line(ofstream &outfile, BINX &INx, BINX &OUTx, double logic_delay, int num_inputs, int num_outputs) {

	unsigned int i;
			
	for (i = 0; (int)i < num_inputs; ++i) {
		outfile << (int)INx.get_bit(i);
	}
	outfile << "  ";
		
	for (i = 0; (int)i < num_outputs; ++i) {
		outfile_bit(OUTx.get_bit(i), outfile);
	}
	if (useDELAY && GV && max_gate_delay >= 0) {
		outfile << " delay=" << logic_delay;
	}
	outfile << endl;

	if (!filestream_check(outfile)) {
		return 0;
	}
	return 1;
}



// just read truth table from disk
int display_saved_table() {

	ifstream infile;
	char c;

	cout << "Reading Truth Table..." << endl << endl;
	
	infile.open(TRUTH_TABLE_FILE, ios::in);

	if (infile.fail() || !filestream_check(infile)) {
		beep();
		cerr << "\nError: Could not read truth table file." << endl;
		return 0;
	}
	while ((c = infile.get()) != EOF) {
		cout << c;
	}
	infile.close();

	cout << endl;

	return 1;
}




inline void outfile_bit(char bit, ofstream &outfile) {
	
	if (known(bit))
		outfile << (int)bit << " ";
	else if (bit == 2)	
		outfile << "X ";
	else if (bit == -2)
		outfile << "Z ";		
	else if (bit != -1)
		outfile << "? ";
}





void draw_truth_table(comp_sim *comps, BINX &init_Q_states, string *str_arr, unsigned *input_nets, unsigned *output_nets, int num_inputs, int num_outputs) {
	
	unsigned int i;
	char ch;
	
	if (LOOP)
		ch = 'Q';
	else
		ch = 'F';

	cout << endl << endl;
	
	if (num_inputs == 0 && LOOP)
		cout << "Finite State Machine: ";
	else
		cout << "Truth Table: ";
		
	cout << ncomps << " component";
	
	if (ncomps != 1)
		cout << "s";
	
	if (changed3 || Opened)	
		cout << ". Click on the 'X' or 'Cancel' to cancel.";

	cout << endl;
	
	if (dont_print_tt || num_inputs < 0 || num_outputs < 0) {
		return;
	}
	cout << endl;

	if (num_inputs > 0)
		cout << " ";
	else if (LOOP)
		cout << "STATE   CK";
	else
		cout << "   ";
	
	for (i = 0; (int)i < num_inputs; ++i) {
		if (str_arr != NULL) {
			cout << str_arr[comps[input_nets[i]].conn_loc] << "   ";
		}
		else if (num_inputs <= MAX_INPUT_LETTER) {
			cout << (char)('A' + (i%MAX_INPUT_LETTER)) << "   ";
		}
		else {
			cout << "X" << i << " ";
			if (i < 10) {
				cout << " ";
			}
		}
	}
	if ((num_inputs >= 0 || LOOP) && num_outputs > 0)	
		cout << "    ";
			
	for (i = 0; (int)i < num_outputs; ++i) {
		if (str_arr != NULL) {
			cout << str_arr[comps[output_nets[i]].conn_loc] << "   ";
		}
		else {
			if (i < 10)
				cout << " ";
			cout << " " << ch << i; // columns won't align with >99 inputs, but in that case it won't fit on one column anyway
		}
	}
	cout << "\n_______________________________________________________________________________";
	
	if (LOOP) {
		cout << "\nINITIAL   " << PREVIOUS_CLOCK << "         ";
		
		for (i = 0; (int)i < num_outputs; ++i) {
			cout << (int)init_Q_states.get_bit(i) << "	 ";
		}
	}
}





void print_table(BINX& INx, int num_inputs) {
	
	unsigned i;
	
	if (!dont_print_tt)
		cout << "\n";
			
	if (num_inputs == 0) {
		cout << "No Inputs:			 ";                   
		return;
	}
	if (dont_print_tt)
		return;

	cout << " ";
		
	for (i = 0; (int)i < num_inputs; ++i) {
		cout << (int)INx.get_bit(i) << "   ";
	}
	cout << "      ";	
}




void write_outputs(int num_outputs, BINX& OUTx) {
	
	if (dont_print_tt)
		return;
	
	print_outputs(OUTx, num_outputs);
		
	if (useDELAY && GV && OUTx.get_bit(0) != 2 && max_gate_delay >= 0) {
		cout << "delay=" << max_gate_delay;
	}
}




void print_outputs(BINX& OUTx, int num_outputs) {
	
	char bit;
	unsigned i;

	for (i = 0; (int)i < num_outputs; ++i) {
		bit = OUTx.get_bit(i);

		switch (bit) {
		case -1:
			cout << "?   ";
			break;
		case 2:
		case 9:
			cout << "X   ";
			break;
		case -2:
			cout << "Z   ";
			break;
		case 0:
			cout << "0   ";
			break;
		case 1:
			cout << "1   ";
			break;
		default:
			cout << "<" << (int)bit << "> ";
			break;
		}
	}
}




void out_calc(BINX &OUTx, comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, int nodes, int num_outputs, int num_comps) {

	if (use_fast_eval) {
		out_calc_graph(OUTx, comps, NODEVALUE, net_delay, permute_output, output_nets, read_only, initially_set, fanin, fanout, bidir, nodes, num_outputs, num_comps);
	}

	int *conn, i, ID, outputs(0), progress_this_loop(0), num_restarts(0), out_set_ctr(0);
	char making_progress(0), Value(no_val);
	double this_gate_delay(0);
	unsigned index;

	// *** need net_has_changed array ***
		
	OUTx.setall(-1);
	num_iterations = conflict = 0;
	max_gate_delay = 0;
	FAST = 0;
	
	for(i = 0; i < num_comps; ++i) {
		/*if (lazy_eval) {
			none_changed = 1;
			for (j = 0; j < num_inputs; ++j) {
				if (net_has_changed[each input] & 0x01) {
					none_changed = 0;
					break;
				}
			}
			if (none_changed) {
				continue;
			}
		}*/
		ID    = comps[i].id;
		conn  = comps[i].conn; // moce up?
		Value = propagate_once(comps[i], i, outputs, Value, NODEVALUE, read_only, initially_set, net_delay, permute_output, OUTx, making_progress, out_set_ctr);

		if ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || ID == OPAMP_ID) { // standard logic gates + opamp + other logic
			if (GV && ID != DELAY_ID) {
				this_gate_delay = comps[i].value;
			}
			set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, this_gate_delay, conn[4], Value, i, 2, making_progress);
		}
		else if (ID >= PRSL_ID && ID <= NTFF_ID) { // flipflops
			set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, FF_DEF_DELAY, conn[3], Value, i, 1, making_progress);
			Value = BooleanNot(NODEVALUE[conn[0]]);
			set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, FF_DEF_DELAY, conn[5], Value, i, 3, making_progress);
			if (Value == 3) {
				NODEVALUE[conn[2]] = comps[i].state = NODEVALUE[conn[0]] = 0; // bad
				illegal_FF = 1;
			}
		}
		if (making_progress) {
			progress_this_loop = 1;
		}
		if (comps[i].state == 2 && ID <= Component_max_id && (!allow_mult_loops || !progress_this_loop) && OUTx.get_bit(0) != -1 && ID != NFET_ID && ID != PFET_ID && ID != DIODE_ID && ID != CUSWIRE_ID && ID != INDUCTOR_ID && ID != ST_SEL_ID/* && get_order(ID) != wirex && get_order(ID) != transistorx && get_order(ID) != icx*/) {
			if ((comps[i].flags & BASE_CIRCUIT) && is_in_window(comps[i].location)) {
				draw_char('?', comps[i].location, 1, 12);
			}
			unknown_state = 1;
		}
		++num_iterations;

		if (out_set_ctr >= num_outputs) {
			break; // all outputs have been assigned to
		}		
		if (i == (num_comps-1) && num_restarts < num_comps && allow_mult_loops && progress_this_loop && OUTx.any_equal_to(2)) {
			FAST    = 1;  // ???
			i       = -1; // continue (slower, but supports transistors)
			outputs = 0;
			progress_this_loop = 0;
			++num_restarts;
		}				
		making_progress = 0;
	}
	// last resort - check if output was updated after processing		
	if (out_set_ctr < num_outputs && OUTx.any_equal_to(2)) { 
		for(i = 0; i < num_outputs; ++i) {
			index = output_nets[i];
			set_output_state(OUTx, comps[index], permute_output[i], NODEVALUE, out_set_ctr);
			
			if (GV && useDELAY) {
				max_gate_delay = max(max_gate_delay, net_delay[comps[index].conn_loc]);
			}
			if (out_set_ctr >= num_outputs)
				break;
		}
	}
	if (show_iterations) {
		cout << "Num iterations is " << num_iterations << ", num restarts = " << num_restarts << endl;
	}
	reset_states(NODEVALUE, net_delay, nodes, 0);
	show_conflicts(comps, num_comps);
	num_iterations = 0;
}



// new version
void out_calc_graph(BINX &OUTx, comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, int nodes, int num_outputs, int num_comps) {

	int *conn, *nets_to_proc = NULL, ID, outputs(0), out_set_ctr(0);
	double this_gate_delay(0);
	char making_progress(0), Value(no_val);
	unsigned i, nnum, net_id, not_end, ntp_curr(0);

	conflict = 0;
	max_gate_delay = 0;
	FAST = 0;

	nets_to_proc = memAlloc(nets_to_proc, nodes+1);

	for (i = 0; (int)i < num_comps; ++i) {
		ID = comps[i].id;

		switch (ID) {
		case INPUT_ID:
		case POWER_ID:
		case GROUND_ID:
			//add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, comps[i].conn_loc, ntp_curr);
			break;
			
		case ST_SEL_ID:
			if (force_init_states || no_inputs || (fanout.net_single(comps[i].conn_loc) && bidir.net_empty(comps[i].conn_loc))) {
				//add_net_to_queue(nets_to_proc, fanin, dep_count, net_flag, comps[i].conn_loc, ntp_curr);
			}
			break;

		default:
			i = (unsigned)num_comps; // force exit of for loop
		}
	}
	for (nnum = 0; nnum < ntp_curr; ++nnum) {
		net_id  = nets_to_proc[nnum];
		not_end = fanin.set_iterator_net(net_id);

		while (not_end) {
			i    = fanin.get_curr();
			ID   = comps[i].id;
			conn = comps[i].conn;
			// ADD

			Value = propagate_once(comps[i], i, outputs, Value, NODEVALUE, read_only, initially_set, net_delay, permute_output, OUTx, making_progress, out_set_ctr);

			if ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || ID == OPAMP_ID) { // standard logic gates + opamp + other logic
				if (GV && ID != DELAY_ID) {
					this_gate_delay = comps[i].value;
				}
				set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, this_gate_delay, conn[4], Value, i, 2, making_progress);
			}
			else if (ID >= PRSL_ID && ID <= NTFF_ID) { // flipflops
				set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, FF_DEF_DELAY, conn[3], Value, i, 1, making_progress);
				Value = BooleanNot(NODEVALUE[conn[0]]);
				set_state(comps[i], NODEVALUE, read_only, initially_set, net_delay, FF_DEF_DELAY, conn[5], Value, i, 3, making_progress);
				if (Value == 3) {
					NODEVALUE[conn[2]] = comps[i].state = NODEVALUE[conn[0]] = 0; // bad
					illegal_FF = 1;
				}
			}
			if (out_set_ctr >= num_outputs) {
				break; // all outputs have been assigned to
			}
		}
	}
	show_conflicts(comps, num_comps);

	delete [] nets_to_proc;
}




void final_check(comp_sim *comps, BINX &OUTx, int num_comps) {

	int i, ID;

	for(i = 0; i < num_comps; ++i) {
		ID = comps[i].id;
		if (comps[i].state == 2 && ID <= Component_max_id && OUTx.get_bit(0) != -1 && ID != NFET_ID && ID != PFET_ID && ID != DIODE_ID && ID != CUSWIRE_ID && ID != INDUCTOR_ID && ID != ST_SEL_ID) {
			if ((comps[i].flags & BASE_CIRCUIT) && is_in_window(comps[i].location)) {
				draw_char('?', comps[i].location, 1, 12);
			}
			unknown_state = 1;
		}
	}
}





inline char propagate_once(comp_sim &comp, int cid, int &outputs, char Value, char *&NODEVALUE, char *read_only, char *initially_set, double *net_delay, int *permute_output, BINX &OUTx, char &making_progress, int &out_set_ctr) {
	
	int *conn;
	char vi2, vo2;
	double this_gate_delay, comp_val;
	
	if (comp.id <= FF_max_id) {
		Value = propValue[comp.id](NODEVALUE, comp);
	}
	else {
		Value = no_val;
	}
	if (Value == no_val) {
		conn = comp.conn;

		switch (comp.id) {		
			case OUTPUT_ID: // output
				set_output_state(OUTx, comp, permute_output[outputs], NODEVALUE, out_set_ctr);
				++outputs;
					
				if (GV && useDELAY) {
					max_gate_delay = max(max_gate_delay, net_delay[comp.conn_loc]);
				}
				break;
			
			case RES_ID: // Resistor
				if (known(NODEVALUE[conn[1]]))
					set_state(comp, NODEVALUE, read_only, initially_set, net_delay, RES_DEF_DELAY, conn[4], comp.state, cid, 2, making_progress);
				else if (known(NODEVALUE[conn[4]]))
					set_state(comp, NODEVALUE, read_only, initially_set, net_delay, RES_DEF_DELAY, conn[1], comp.state, cid, 4, making_progress);
				break;
			
			case DIODE_ID: // diode 
				vi2 = NODEVALUE[conn[1]];
				vo2 = NODEVALUE[conn[4]];
				if ((FAST && vi2 == -2) || known(vi2)) {
					if (comp.value < 0)
						Value = DIODE2(vi2);
					else
						Value = DIODE(vi2);
					set_state(comp, NODEVALUE, read_only, initially_set, net_delay, DIODE_DEF_DELAY, conn[4], Value, cid, 2, making_progress);
				}
				else if ((FAST && vo2 == -2) || known(vo2)) {
					if (comp.value < 0)
						Value = DIODE2(vo2);
					else
						Value = DIODE(vo2);
					set_state(comp, NODEVALUE, read_only, initially_set, net_delay, DIODE_DEF_DELAY, conn[1], Value, cid, 4, making_progress);
				}
				break;		
			
			case BATTERY_ID: // battery
			case VCVS_ID:
			case CCVS_ID:
			case VCCS_ID:
			case CCCS_ID:
				if (comp.id == BATTERY_ID) {
					comp_val = supply_voltage/2.0;
				}
				else {
					comp_val = 0;
				}
				if (comp.value >= comp_val) {
					if (known(NODEVALUE[conn[4]]) && initially_set[conn[4]] == false) {
						add_conflict(get_connections(comp.location, comp.id, comp.flags).out2, 0);
						conflict = 1;
					}
					comp.state = NODEVALUE[conn[4]] = 1;
					NODEVALUE[conn[1]] = 0;
				}
				else if (comp.value <= -comp_val) {
					if (known(NODEVALUE[conn[1]]) && initially_set[conn[1]] == false) {
						add_conflict(get_connections(comp.location, comp.id, comp.flags).in2, 0);
						conflict = 1;
					}
					comp.state = NODEVALUE[conn[1]] = 1;
					NODEVALUE[conn[4]] = 0;
				}
				else {
					if (known(NODEVALUE[conn[4]]) && !known(NODEVALUE[conn[1]])) {
						comp.state = NODEVALUE[conn[1]] = NODEVALUE[conn[4]];
					}
					else if (known(NODEVALUE[conn[1]]) && !known(NODEVALUE[conn[4]])) {
						comp.state = NODEVALUE[conn[4]] = NODEVALUE[conn[1]];
					}
				}
				break;
			
			default:
				if (comp.id == NFET_ID || comp.id == PFET_ID || get_order(comp.id) == transistorx) { // transistor 					
					if (known(NODEVALUE[conn[1]]) || known(NODEVALUE[conn[3]]) || known(NODEVALUE[conn[5]]) 
						|| (FAST && (NODEVALUE[conn[3]] == -2 || NODEVALUE[conn[1]] == -2 || NODEVALUE[conn[5]] == -2))) {
						
						if (mos_delay_not_wl && (comp.id == NFET_ID || comp.id == PFET_ID)) {
							this_gate_delay = comp.value;
						}
						else {
							this_gate_delay = TRANS_DEF_DELAY;
						}
						if (comp.id == NFET_ID) {
							Value = NFET(NODEVALUE[conn[1]], NODEVALUE[conn[3]], NODEVALUE[conn[5]]);
						}
						else if (comp.id == PFET_ID)	{	
							Value = PFET(NODEVALUE[conn[1]], NODEVALUE[conn[3]], NODEVALUE[conn[5]]);
						}													
						else if ((int)(comp.value) == 1) {
							Value = NTRANSISTOR(NODEVALUE[conn[1]], NODEVALUE[conn[3]], NODEVALUE[conn[5]]);
						}
						else if ((int)(comp.value) == 2) {
							Value = PTRANSISTOR(NODEVALUE[conn[1]], NODEVALUE[conn[3]], NODEVALUE[conn[5]]);
						}
						if (Q_POLARITY == 0 || Value == -2) {
							set_state(comp, NODEVALUE, read_only, initially_set, net_delay, this_gate_delay, conn[3], Value, -1, 0, making_progress);
						}
						if (Q_POLARITY == 1 || Value == -2) {
							set_state(comp, NODEVALUE, read_only, initially_set, net_delay, this_gate_delay, conn[5], Value, -1, 0, making_progress);
						}
					}
				}
			}
		}
	return Value;
}




/*
Valid Logic States:
	-2	Z - high impdeance
	-1	uninitialized (should not occur)
	0	logic low
	1	logic high
	2	X - unknown or don't care
	3	flipflop/latch error - set and reset both asserted
	9	conflicting logic state or race condition
	
	10	D_NOT (fault testing, new, not used yet)
	11	D     (fault testing, new, not used yet)
	
	0 and 1 are known and opposites
*/
inline void set_state(comp_sim &comp, char *NODEVALUE, char *read_only, char *initially_set, double *net_delay, double this_gate_delay, int nnumber, char Value, int index, int outpos, char &making_progress) {
	
	char nodevalue(NODEVALUE[nnumber]);

	//if (lazy_eval) {}
		
	if ((FAST && Value == nodevalue) || (known(nodevalue) && unknown(Value))) {
		return;
	}
	if (!lazy_eval && opposite(Value, nodevalue) && initially_set[nnumber] == false) {
		if (index >= 0) {
			add_conflict(index, outpos);
		}
		Value = nodevalue = 9;	// Conflicting State Error, 9 = undefined logic level
		iteration_error = 1;
	}					
	if (known(Value) || (Value == -2 && nodevalue == 2)) {
		if (Value == -2 || read_only[nnumber] == false) {
			if (Value != nodevalue && !known(nodevalue) && nodevalue != 9) { 
				making_progress = true;
				if (GV && useDELAY && known(Value)) {
					calc_delay(comp, NODEVALUE, nnumber, net_delay, this_gate_delay);
				}
			}
			nodevalue = Value;
			if (comp.flags & BASE_CIRCUIT) {
				last_known_value = comp.location;
			}
			if (outpos != 3) {
				comp.state = Value;
			}
		}
		else if (Value != nodevalue) {
			if (index >= 0) {
				add_conflict(index, outpos);
			}
			nodevalue       = 9; // Attempted Force of Fixed State Net Error
			read_only_error = 1;
		}
	}	
	NODEVALUE[nnumber] = nodevalue;
}




inline void calc_delay(comp_sim &comp, char *NODEVALUE, int nnumber, double *net_delay, double this_gate_delay) {

	double gdelay;
	
	if (comp.id <= FF_max_id) {
		gdelay = propDelay[comp.id](NODEVALUE, comp.conn, net_delay);
	}
	else {
		gdelay = 0;
	}
	net_delay[nnumber] = max(net_delay[nnumber], gdelay + this_gate_delay);

	//cout << "net_delay[" << nnumber << "] = " << net_delay[nnumber] << ", gdelay = " << gdelay << ", this_gate_delay = " << this_gate_delay << endl;
}



// not used yet
inline int is_controlling_value(int ID, int value) {

	if (ID > MAX_USED_ID) {
		return 0;
	}
	switch (isControllingValue[ID]) {
	case cvAND:
		return (value == 0);
	case cvOR:
		return (value == 1);
	case cvNO:
		return 0;
	default:
		return 0;
	}
	return 0;
}




// This needs to be fixed eventually.
void state_machine(comp_sim *comps, char *NODEVALUE, double *net_delay, int *permute_output, unsigned *input_nets, unsigned *output_nets, char *read_only, char *initially_set, NetPointerSet &fanin, NetPointerSet &fanout, NetPointerSet &bidir, string *str_arr, int num_inputs, int num_outputs, int num_flipflops, int nodes, int num_ivs, BINX *input_vectors, int num_comps) {
	
	unsigned int i;
	int ID, loop_iterations, curr_iv(0), clock, out_counter(0), tt_open, tt_good(1), dec;
	char val;
	double logic_delay(0);
	BINX OUTx(num_outputs), init_O_states, init_Q_states;
	ofstream tt_file;

	if (num_flipflops > MAX_FLIPFLOPS) { 
		cout << "\nThe Circuit can only have up to " << MAX_FLIPFLOPS << " flipflops to evaluate it as a finite state machine. Please remove some and simulate again." << endl;
		return;
	}	
	cout << "\nEnter the number of iterations of the finite state machine:  ";
	loop_iterations = inp();
	if (loop_iterations == 0) {
		return;
	}
	LOOP = 2*loop_iterations;
	
	set_initial_states(init_Q_states, init_O_states, num_outputs, num_flipflops);	
	draw_truth_table(comps, init_Q_states, str_arr, input_nets, output_nets, 0, num_outputs);
	
	for(i = 0; (int)i < num_comps; ++i) {	
		if (get_order(comps[i].id) == flipflopx) {
			ID  = comps[i].id; 
			val = init_Q_states.get_bit(num_outputs);
			NODEVALUE[comps[i].conn[3]] = val; // Q
			NODEVALUE[comps[i].conn[5]] = BooleanNot(val); // !Q
			
			if (num_flipflops < MAX_LABELLED_IO) {
				draw_char(('1' + (char)num_flipflops), (comps[i].location + 1), 10, 12);
			}
			if (ID != RSL_ID && ID != PRSL_ID && ID != NRSL_ID && ID != (num_flipflops + RSL_ID)) {
				clock = get_connections(comps[i].location, ID, comps[i].flags).in2;
			}
			// *** use clock ***
		}
		else if (comps[i].id == OUTPUT_ID) {
			NODEVALUE[comps[i].conn_loc] = init_O_states.get_bit(permute_output[num_outputs]);
			++out_counter;
		}
	}
	find_outputs(comps, str_arr, permute_output, output_nets, num_outputs);
	tt_open = open_tt_for_write(tt_file, num_inputs, num_outputs, num_ivs);

	for (curr_iv = 0; curr_iv < num_ivs; ++curr_iv) {
		// need to set logic_delay = ?

		out_calc(OUTx, comps, NODEVALUE, net_delay, permute_output, output_nets, read_only, initially_set, fanin, fanout, bidir, nodes, num_outputs, num_comps);	
		
		dec = input_vectors[curr_iv].conv_to_dec();
		count_loop(dec, LOOP, curr_iv);
		print_outputs(OUTx, num_outputs);

		if (tt_open && tt_good) {
			tt_good = write_tt_line(tt_file, input_vectors[curr_iv], OUTx, logic_delay, num_inputs, num_outputs);
		}
		PREVIOUS_CLOCK = BooleanNot(PREVIOUS_CLOCK);
		PRESET = CLEAR = 0;

		if (dec >= LOOP) {
			break;
		}
	}	
	cout << "\n";	
	LOOP = 0;
	reset_states(NODEVALUE, net_delay, nodes, 0);
	if (tt_open) {
		tt_file.close();
	}
}





inline void set_output_state(BINX& OUTx, comp_sim &comp, int outputs, char *NODEVALUE, int &out_set_ctr) {
	
	char bit, value;
	
	if (outputs == 0) {
		return; // error?
	}
	bit   = OUTx.get_bit(outputs-1);
	value = NODEVALUE[comp.conn_loc];

	if (!known(bit) && known(value) /*&& (net_has_changed[output_net] & 0x02)*/) {
		++out_set_ctr;
	}
	if ((unknown(bit) || LOOP) && bit != 9) {
		comp.state = bit = value;
	}
	else if (bit == -1) {
		if (known(value))
			comp.state = bit = value;
		else
			bit = 2;	
	}		
	else if (value == 9) {
		bit = 9;
	}
	OUTx.set_bit(outputs-1, bit);
}




void count_loop(int dec, int last_val, int loop_num) {

	cout << "\n";
	
	if (dec <= last_val) {
		if (loop_num%2 == 0)
			cout << "  " << (dec/2 + 1) << "       ";
		else
			cout << "          ";
	}	
	cout << PREVIOUS_CLOCK << "          ";
}





int count_io(component *head) {

	while (head != 0) {	
		if (head->id == INPUT_ID || head->id == OUTPUT_ID) {
			return 1;
		}
		head = head->next;
	}
	return 0;
}





void find_inputs(comp_sim *comps, string *str_arr, BINX &INPUT, unsigned *input_nets, int num_inputs, char *NODEVALUE, int show_inputs) {
	
	unsigned i, index;
	int state;
	
	illegal_input = 0;

	for(i = 0; (int)i < num_inputs; ++i) {
		index = input_nets[i];

		if (show_inputs && (comps[index].flags & BASE_CIRCUIT)) {
			if (str_arr != NULL) {
				SetDrawingColorx(GREEN);
				draw_text(str_arr[comps[index].conn_loc], comps[i].location-2+x_blocks, 9); // ???
			}
			if (i < MAX_LABELLED_IO) {
				SetDrawingColorx(BLACK);
				label_io((comps[index].location - 2), i, (num_inputs <= MAX_INPUT_LETTER), 'X');
			}	
		}
		state = INPUT.get_bit(i);
		comps[index].state = state;
			
		if (!lazy_eval && opposite(state, NODEVALUE[comps[index].conn_loc])) {
			illegal_input = 1;
		}
		else {
			NODEVALUE[comps[index].conn_loc] = state;
		}
	}
}





void find_outputs(comp_sim *comps, string *str_arr, int *permute_output, unsigned *output_nets, int num_outputs) {
	
	unsigned i, index;

	for(i = 0; (int)i < num_outputs; ++i) {
		index = output_nets[i];

		if (comps[index].flags & BASE_CIRCUIT) {
			if (str_arr != NULL) {
				SetDrawingColorx(GREEN);
				draw_text(str_arr[comps[index].conn_loc], comps[index].location+2+x_blocks, 9); // ???
			}
			if (i <= MAX_LABELLED_IO) {
				SetDrawingColorx(BLACK);
				label_io((comps[index].location + 2), permute_output[i]-1, 0, 'F');
			}
		}
	}
	SetDrawingColorx(BLACK);
}






int reset_circuit2(comp_sim *comps, char *NODEVALUE, double *net_delay, char *read_only, char *initially_set, int nodes, int num_comps) {
	
	unsigned int i;
	int count(0), ID, seen_power(0), seen_ground(0);
		
	if (!LOOP) {
		destroyL(conflict_list);
		conflict_list = NULL;

		reset_states(NODEVALUE, net_delay, nodes, 1);

		memset(read_only,     0, nodes*sizeof(char));
		memset(initially_set, 0, nodes*sizeof(char));
	}
	for(i = 0; (int)i < num_comps; ++i) {		
		comps[i].state = 2;
		ID             = comps[i].id;
			
		if ((ID < INPUT_ID || ID > BATTERY_ID) && ID < IC_base_id) {
			++count;
		}
		else {
			switch (ID) {
			case INPUT_ID:
				read_only[comps[i].conn_loc] = true;
				break;
			case POWER_ID:
				if (!seen_power) {
					read_only[comps[i].conn_loc] = true;
					seen_power = 1;
				}
				break;
			case GROUND_ID:
				if (!seen_ground) {
					read_only[comps[i].conn_loc] = true;
					seen_ground = 1;
				}
				break;
			case ST_SEL_ID:
				initially_set[comps[i].conn_loc] = true;
				break;
			}
		}
	}
	return count;
}




void reset_states(char *NODEVALUE, double *net_delay, int nodes, int init) {

	if (LOOP) {
		return;
	}
	if (useDELAY && GV) { // *** what about lazy eval??? ***
		memset(net_delay, 0, nodes*sizeof(double));
	}
	if (init || !lazy_eval) {
		memset(NODEVALUE, 2, nodes*sizeof(char));
	}
}





void show_conflicts(comp_sim *comps, int num_comps) {

	int old_osize(o_size), loc;
	connections x;
	clist *temp = conflict_list;
	
	while (temp != 0) {
		loc = 0;

		if (temp->conflict < 0)
			continue;

		if (temp->type != 0) {
			if (temp->conflict >= num_comps)
				continue;
			x = get_connections(comps[temp->conflict].location, comps[temp->conflict].id, comps[temp->conflict].flags);
		}
		switch (temp->type) {
		case 0:
			draw_RED_CIRCLE(temp->conflict);
			break;
		case 1:
			if (x.out1 > 0)
				loc = x.out1;
			break;
		case 2:
			if (x.out2 > 0)
				loc = x.out2;
			break;
		case 3:
			if (x.out3 > 0)
				loc = x.out3;
			break;
		case 4:
			if (x.in2 > 0)
				loc = x.in2;
			break;
		}
		if (loc != 0) {
			o_size   = o_size/4;
			draw_RED_CIRCLE(loc);
			conflict = 1;
			o_size   = old_osize;
		}
		temp = temp->next;
	}
}




void add_conflict(int location, int type) {

	clist *new_clist, *temp = conflict_list;
		
	if (location > max_boxes)
		return;
	
	if (check_dup_conflicts) {
		while (temp != 0) {
			if (temp->conflict == location && temp->type == type) // conflict already found, this may be slow
				return;
				
			temp = temp->next;
		}
	}
	new_clist = new clist;
	if (!new_clist) { 
		out_of_memory();
		return;
	}
	new_clist->conflict = location; // add new conflict
	new_clist->type     = type;
	new_clist->next     = conflict_list;
	conflict_list       = new_clist;
}












