#include "CircuitSolver.h"
#include "Netlist.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Netlist.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Netlisting (SPICE and PODEM compatible)
// By Frank Gennari
int const nl_verbose = 0;
int const use_ICs    = 0;


#define DEF_SPICE_LEVEL 3
#define FANIN_LIMIT     4
#define DECOMP_XOR      1

// 0, 1, 2, 9 - for netlist compatibility (1 = PODEM, 2 = SPICE)
int compatibility(2), has_ground_net, has_vdd_net, max_width, last_length, last_max_width, entered_x;
double Xval(0);


extern char input[];
extern int useCOMPLEX, useDIGITAL, countdown, no_ref_needed, skip_placement_check, cScale, surpress_show_values;
extern int x_blocks, y_blocks;
extern GridArray NODENUMBER;


// import
int open_netlist_file(ifstream &infile, string &input_buffer);
component *import_netlist(component *head, ifstream &infile, StringTable &net_names, string &input_buffer);
comp_sim *import_netlist_to_comp_sim(ifstream &infile, StringTable &net_names, string *&str_arr, string &input_buffer, int &num_comps, int &num_nets);
void free_ic_net_list(ICNet_list *ic_net_list);
component *read_parse_netlist(component *head, ifstream &infile, StringTable &net_names, string input_buffer, ICNet_list *& ic_net_list, comp_nets *& cnets, int &num_components, int &curr_netnum, int nl_flags);
comp_sim *create_comp_sims_from_netlist(component *head, comp_nets *cnets, int num_components);
void add_new_nets(comp_nets *& cnets, connections &netnums);
void read_comp(ifstream &infile, StringTable &net_names, int &curr_netnum, int pins, int init_i, string &input_buffer, int &comp_number, int *&nets, unsigned &nets_cap);
int calc_next_pos(int &curr_pos, int ID);
int do_assignment(string &lhs, string &rhs, StringTable &net_names, int &curr_netnum);
void get_real_name(string &out, string &in);
double read_number(char *input, cs_equation &eq);
double read_two_port(ifstream &infile, StringTable &net_names, cs_equation &eq, connections &netnums, int &curr_netnum, string &input_buffer, int &comp_number, int *&nets, unsigned &nets_cap);
double extract_param(string &param_str, cs_equation &eq);
int extract_type(ifstream &infile, string &input_buffer, int &buf_pos);
int get_input_nodes(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int &buf_pos, int *&nets, unsigned &nets_cap);
int get_input_node(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int &buf_pos, int &node, int nnum);
int get_num_in_parens(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int pos);
int get_netnum_from_string(StringTable &net_names, string &netname, int &curr_netnum);
int map_output_one_to_one(StringTable &net_names, StringTable &out_table, string &str, int *out1, int *out2, char *match_found, int &output_ctr, int &curr_netnum, int &error_code);
int string_match_one_to_one(StringTable &s_table, string &str, char *match_found, char ntype, int &error_code);

// export
void export_netlist(component *& head);
int check_comp_types(component *head);
int exp_netlist(component *& head);
void write_ios(ofstream &outfile, component *head, int flags, int &status);
void write_gates(ofstream &outfile, component *head, int &status);
void add_gate_to_netlist(ofstream &outfile, int id, int location, char flags);
void write_analog(ofstream &outfile, component *head, int flags, int add_sc, int &status);
void write_subcircuits(ofstream &outfile, component *head, int add_sc, int &status);
void write_models(ofstream &outfile, component *head, int &status);
int create_random_netlist(unsigned nin, unsigned nout, unsigned size, const char *name);


component *simulation_setup(component *head, wire *&wires, int &num_wires);
component *simulation_cleanup(component *head);

component *autoplace_components(component *head, ICNet_list *ic_net_list, comp_nets *cnets, int num_components, int num_nets);

int count_components(component *head, component *& tail, int &subcircuit_count, int &max_position, int &nwires);

int number_nodes(component *head, wire *wires, int num_wires);

transistor Qnum_find(transistor &tr, int num);
int Qsearch(transistor& Model, string Q_select, int use_status_bar);

IC ic_search(IC &data, string name1, int use_status_bar);
int count_ics();

void read_logic_function(string input);
component *add_logic_function(component *head, StringTable &net_names, comp_nets *& cnets, int &curr_netnum, int &curr_pos, int &num_components);
component *split_large_gate(component *head, comp_nets *& cnets, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits, int fanin_limit, int decomp_xor);
component *xor_and_outputs(component *head, comp_nets *& cnets, int *out1, int *out2, int &curr_netnum, int &curr_pos, int &num_components, int &output_net, int num_outputs, int fanin_limit, int decomp_xor, int add_outputs);
component *add_io_to_circuit(component *head, comp_nets *& cnets, int net, int id, int &curr_pos, int &num_components);

double calc_one_y(char *equation, double *number, double x, int &i, int &j, int size, int &error, int op_order);
int recode(char *equation, char *recoded_equation, double *number);
void help();


// ****** Import ******



int open_netlist_file(ifstream &infile, string &input_buffer) {

	int status(0);
	char nl_name[MAX_SAVE_CHARACTERS + 1] = {0};

	while (status == 0) {
		cout << "\nEnter the name of the input netlist file:  ";
		cin  >> ws;
		strcpy(nl_name, xin(MAX_SAVE_CHARACTERS));
	
		if (!infile_file(infile, nl_name)) {
			beep();
			cerr << "\nThe Circuit Solver netlist " << nl_name << " could not be opened!" << endl;
			status = 0;
		}
		else if (!filestream_check(infile)) {
			status = 0;
		}
		else {
			infile >> input_buffer;
		
			if (input_buffer == "Circuit" || (input_buffer != "" && (input_buffer[0] == '#' || input_buffer[0] == '*'))) {
				status = 1;
			}
			else {
				cerr << "The file " << nl_name << " is not a circuit solver compatible netlist." << endl;
				infile.close();
				infile.clear();
				status = 0;
			}
		}
		if (status == 0) {
			cout << "Try again?  ";
			if (!decision()) {
				return 0;
			}
		}
	}
	return 1;
}




component *import_netlist(component *head, ifstream &infile, StringTable &net_names, string &input_buffer) {

	ICNet_list *ic_net_list = NULL;
	comp_nets *cnets = NULL;
	int num_components(0), curr_netnum(1);

	head = read_parse_netlist(head, infile, net_names, input_buffer, ic_net_list, cnets, num_components, curr_netnum, 0);
	head = autoplace_components(head, ic_net_list, cnets, num_components, curr_netnum);

	free_ic_net_list(ic_net_list);
	destroyL(cnets);

	return head;
}



// if no graphical circuit is required and ic_net_list == NULL, build network without wires and locations
comp_sim *import_netlist_to_comp_sim(ifstream &infile, StringTable &net_names, string *&str_arr, string &input_buffer, int &num_comps, int &num_nets) {

	unsigned str_arr_size;
	ICNet_list *ic_net_list = NULL;
	comp_nets *cnets = NULL;
	comp_sim *cs_arr;
	component *head = NULL;
	connections netnums;
	//StringTable net_names(0);

	set_all_connections(netnums, -1);

	head = read_parse_netlist(head, infile, net_names, input_buffer, ic_net_list, cnets, num_comps, num_nets, 0);

	if (has_ground_net) {
		head = update_circuit(GROUND_ID, GROUND_LOC,   head, 0, 0);
		netnums.out2 = 0;
		add_new_nets(cnets, netnums);
		++num_comps;
	}
	if (has_vdd_net) {
		head = update_circuit(POWER_ID,  GROUND_LOC+1, head, 0, 0);
		netnums.out2 = has_vdd_net;
		add_new_nets(cnets, netnums);
		++num_comps;
	}
	str_arr_size = (unsigned)num_nets;
	cs_arr       = create_comp_sims_from_netlist(head, cnets, num_comps);
	str_arr      = net_names.make_string_array(str_arr_size);

	// DO SOMETHING WITH ic_net_list

	free_ic_net_list(ic_net_list);
	destroyL(cnets);
	destroyL(head);

	return cs_arr;
}




void free_ic_net_list(ICNet_list *ic_net_list) {

	ICNet_list *temp_net_list = ic_net_list;

	while (temp_net_list != NULL) {
		destroyL(temp_net_list->nets);
		temp_net_list = temp_net_list->next;
	}
	destroyL(ic_net_list);
}



// *** use flags ***

// N+ N- NC+ NC-, voltage Source - ES<k>, VCCS - G<k>, CCVS - FS<k>
component *read_parse_netlist(component *head, ifstream &infile, StringTable &net_names, string input_buffer, ICNet_list *& ic_net_list, comp_nets *& cnets, int &num_components, int &curr_netnum, int nl_flags) {

	char ch, number[NUMBER_LENGTH + 1] = {0}, ICname[MAX_SAVE_CHARACTERS] = {0};
	char *input_ptr[1] = {NULL};
	unsigned int i, j(0), ns_pos, nets_cap(5); // start with 4 inputs + output
	int linenum(0), exit_loop(0), comp_number, comp_number2, base_net;
	int id(0), buf_pos(0), curr_pos(x_blocks), ic_netcount(0), ii, num_splits, num_inputs;
	int *nets = NULL;
	double value, l_val(0), w_val(0);
	string l_param, w_param, lhs, num_in;
	connections netnums;
	transistor Model;
	IC ic;
	ICNet *ic_nets = NULL, *temp_net, *last_net = NULL;
	ICNet_list *temp_net_list;
	cs_equation eq(MAX_ENTERED_CHARACTERS), *new_eq;

	has_ground_net = has_vdd_net = max_width = last_length = last_max_width = entered_x = 0;
	nets = memAlloc(nets, nets_cap);

	if (net_names.get_num_strings() == 0) {
		net_names.insert(ground_string, 0);
	}
	if (!(nl_flags & APPEND_MODE)) {
		destroyL(head);
		head = NULL;
	}
	reset_timer(1);
	show_elapsed_time();
	cout << "Beginning netlist import." << endl;
	
	infile >> ws;
	
	// Read in first (comment) line and ignore
	if (!infile.getline(input, MAX_ENTERED_CHARACTERS)) { 
		beep();
		cerr << "\nWarning: Empty or incomplete netlist was read!" << endl;
		return head;
	}
	skip_placement_check = 1; // components are auto-placed, don't need placement check
	surpress_show_values = 1;
	
	while (!exit_loop && infile.good() && infile >> input_buffer) {
		set_all_connections(netnums, -1);
		id = NO_ID;
				
		switch (input_buffer[0]) {
			
			/**** COMMENTS ****/
			case '/':  // comment?
				if (input_buffer.length() > 1) {
					if (input_buffer[1] == '*') {
						while (infile.good() && infile >> input_buffer) {
							if (input_buffer == "*/")
								break;
						}
						break;
					}
					else if (input_buffer[1] != '/')
						cerr << "Invalid line # " << linenum << " will be ignored: " << input_buffer << "." << endl;
				}
				else
					cerr << "Skipping unrecognized token '/' on line " << linenum << "." << endl;
			case '#':  // comment
			case '*':  // comment
			case '\n': // new line
			case '\0': // empty	
				if (!infile.getline(input, MAX_ENTERED_CHARACTERS))
					exit_loop = 1;
				break;
				
			case '.':  // commands and models (e.g. .options)
				if (input_buffer == ".end" || input_buffer == ".END" || !infile.getline(input, MAX_ENTERED_CHARACTERS))
					exit_loop = 1;
				break;
			
			case '$':
				if (input_buffer.length() == 1) {
					if (!(infile.good() && infile >> input_buffer)) {
						cerr << "Error: Expecting net name/number but reached end of file on line " << linenum << "." << endl;
						break;
					}
				}
			/**** NUMBERS ****/
			case '0':	
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				if (!(infile >> ch)) {
					exit_loop = 1;
					break;
				}
				if (ch != '=') 
					cerr << "Invalid token on line " << linenum << ": " << input_buffer << endl;
					
				lhs = input_buffer;
				if (!(infile >> input_buffer) || input_buffer.length() == 0) {
					exit_loop = 1;
					break;
				}	
				// <x> = <BUF, NOT, AND, OR, XOR, NAND, NOR, XNOR> (<x1>,<x2>,...,<xn>)
				
				if (do_assignment(lhs, input_buffer, net_names, curr_netnum)) {
					break;
				}
				base_net = get_netnum_from_string(net_names, lhs, curr_netnum);
				if (base_net == 0) {
					cerr << "Error: assignment to ground on line " << linenum << "." << endl;
					break;	
				}
				if (base_net == has_vdd_net) {
					cerr << "Error: assignment to vdd on line " << linenum << "." << endl;
					break;	
				}
				id         = extract_type(infile, input_buffer, buf_pos);
				num_inputs = get_input_nodes(infile, net_names, curr_netnum, input_buffer, buf_pos, nets, nets_cap);
				
				if (nl_verbose) {
					cout << "GATE: " << get_name(id, 0) << " to net " << base_net << " with " << num_inputs << " input";
					if (num_inputs > 1)
						cout << "s";
					cout << " ";

					for (i = 0; i < (unsigned)num_inputs; ++i) {
						if (i > 0)
							cout << ", ";
						cout << nets[i+1];
					}
					cout << endl;
				}
				if (num_inputs > FANIN_LIMIT || (DECOMP_XOR && (id == XOR_ID || id == XNOR_ID))) {
					num_splits = 0;
					nets[0]    = base_net;
					head       = split_large_gate(head, cnets, nets, id, num_inputs, curr_netnum, curr_pos, num_components, num_splits, FANIN_LIMIT, DECOMP_XOR);
					
					if (nl_verbose)
						cout << num_splits << " splits were made for " << num_inputs << " input component with ID " << id << " (" << get_name(id, 0) << ")." << endl;
					break;
				}
				if (num_inputs == 0) {
					cerr << "Error: Gate has no inputs!" << endl;
					break;
				}
				if (id == NOT_ID && num_inputs == 1 && !has_vdd_net) {
					has_vdd_net = base_net;
					if (nl_verbose)
						cout << "Creating VDD." << endl;
					break;
				}
				if (num_inputs == 3)
					id += T_GATE_OFFSET;
				else if (num_inputs == 4)
					id += Q_GATE_OFFSET;

				if (base_net >= 0) { // should always be true
					netnums.out2 = base_net;
				}
				switch (num_inputs) {
				case 4:
					netnums.out1 = nets[4];
				case 3:
					netnums.in2  = nets[3];
				case 2:
					netnums.in3  = nets[2];
				case 1:
					if (id == BUFFER_ID || id == NOT_ID)
						netnums.in2 = nets[1];
					else
						netnums.in1 = nets[1];
				}				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, 2, 0);
				break;
				
			// INPUT OUTPUT C R D L V I FVCVS FVCCS FCCVS FCCCS Q M OPAMP IC 
			
			/**** INPUT/IC ****/
			case 'I':
			case 'i':
				if (compare_token_ncs(input_buffer.c_str(), 1, "nput")) {
					if (nl_flags & SEPARATE_IO) {
						// *** ADD ***
					}
					buf_pos    = 6;
					num_inputs = get_input_nodes(infile, net_names, curr_netnum, input_buffer, buf_pos, nets, nets_cap);
					
					if (nl_verbose)
						cout << "INPUT net(s) ";

					for (i = 1; i <= (unsigned)num_inputs; ++i) { // note: offset by 1
						if (nets[i] < 0) {
							exit_loop = 1;
							break;
						}
						if (nl_verbose)
							cout << nets[i] << " ";

						head = add_io_to_circuit(head, cnets, nets[i], INPUT_ID, curr_pos, num_components);
					}
					if (nl_verbose)
						cout << endl;
					break;	
				}
				if (!compare_token_ncs(input_buffer.c_str(), 1, "c")) { // current source (not IC)
					value = read_two_port(infile, net_names, eq, netnums, curr_netnum, input_buffer, comp_number, nets, nets_cap);
					
					if (nl_verbose)
						cout << "I" << comp_number << " from net " << nets[0] << " to net " << nets[1] << " of value " << value << endl;
					
					head = update_circuit(I_SOURCE_ID, calc_next_pos(curr_pos, I_SOURCE_ID), head, comp_number, value);
					break;
				}
				// IC# n<1> n<2> n<3> ... n<x> <MODEL_NAME>
				if (nl_verbose) {
					cout << "IC"; // same as subcircuit (next case)
					j = 2;
				}
			/**** SUBCIRCUIT ****/
			case 'X':
			case 'x': // subcircuit (same as IC)
				if (comp_char(input_buffer[0], 'x') == 1) {
					if (input_buffer.length() == 2 && input_buffer[1] == '=') {
						if (!(infile >> input)) {
							cerr << "Error: Expecting value, got EOF at line " << linenum << "." << endl;
							break;
						}
						Xval = read_number(input, eq);
						entered_x = 1;
						break;
					}
					if (nl_verbose)
						cout << "Subcircuit";
					j = 1;
				}					
				for (i = j; i < (unsigned)NUMBER_LENGTH && i < input_buffer.length() && is_number(input_buffer[i]); ++i) 
					number[i-j] = input_buffer[i];
					
				number[i-j] = 0;
				comp_number = atoi(number);
				
				if (nl_verbose)
					cout << comp_number << " ";
				
				infile.getline(input, MAX_ENTERED_CHARACTERS);
				ic_netcount = 0;
				ic_nets = NULL;
				ns_pos = 0;
				
				// i = line length
				for (i = 0; i < (unsigned)MAX_ENTERED_CHARACTERS && input[i] != 0; ++i) {};
				
				// find location of subcircuit name
				for (ii = i; ii >= 0; --ii) {
					if (input[ii] != ' ')
						ns_pos = 1;
					else if (ns_pos == 1 && input[ii] == ' ') {
						ns_pos = ii + 1;
						break;
					}
				}
				for (i = 0; i < (unsigned)MAX_ENTERED_CHARACTERS && input[i] != 0; ++i) {
					if (input[i] != ' ') {
						if ((use_ICs && comp_char(input[i], 'n') == 0) || i >= ns_pos) {
							for(j = i; (j-i) < (unsigned)MAX_SAVE_CHARACTERS && j < (unsigned)MAX_ENTERED_CHARACTERS && input[j] != 0; ++j) {
								ICname[j-i] = input[j];
							}
							if (nl_verbose)
								cout << " " << ICname << " ";	
							break;
						}
						for (j = i+1; (j-i-1) < (unsigned)NUMBER_LENGTH && input[j] != 0 && is_number(input[j]); ++j) 
							number[j-i-1] = input[j];
							
						number[j-i-1] = 0;
						num_in = (string)number;
						comp_number2 = get_netnum_from_string(net_names, num_in, curr_netnum);
						++ic_netcount;
						i = j;
						
						if (nl_verbose)
							cout << comp_number2 << " ";
						
						temp_net = new ICNet;
						temp_net->netnum = comp_number2;
						temp_net->next = NULL;
						if(ic_nets == NULL)
							ic_nets = temp_net;
						else if (last_net != NULL)
							last_net->next = temp_net;
						last_net = temp_net;
					}
				}
				input_buffer = shift_chars(ICname);
				input_buffer = remove_spaces(input_buffer, input_buffer.length());
				ic = ic_search(ic, input_buffer, 0);
				id = IC_base_id + countdown + 1;
				
				if (nl_verbose)
					cout << "IC " << ic.name << endl;
					
				curr_pos += 2;
				head = update_circuit(id, calc_next_pos(curr_pos, IC_ID), head, 2, 0);
				curr_pos += 2;
				++num_components;
				
				temp_net_list       = new ICNet_list;
				temp_net_list->nets = ic_nets;
				temp_net_list->next = ic_net_list;
				ic_net_list         = temp_net_list;
				break;

			/**** OUTPUT ****/
			case 'O':
			case 'o':
				if (compare_token_ncs(input_buffer.c_str(), 1, "utput")) {
					if (nl_flags & SEPARATE_IO) {
						// *** ADD ***
					}
					buf_pos    = 7;
					num_inputs = get_input_nodes(infile, net_names, curr_netnum, input_buffer, buf_pos, nets, nets_cap);
					
					if (nl_verbose)
						cout << "OUTPUT net(s) ";
					
					for (i = 1; i <= (unsigned)num_inputs; ++i) { // these are actually n_outputs, but offset by 1
						if (nets[i] < 0) {
							exit_loop = 1;
							break;
						}
						if (nl_verbose)
							cout << nets[i] << " ";

						head = add_io_to_circuit(head, cnets, nets[i], OUTPUT_ID, curr_pos, num_components);
					}
					if (nl_verbose)
						cout << endl;
					break;
				}
				if (input_buffer == "OPAMP" || input_buffer == "opamp") {
					read_comp(infile, net_names, curr_netnum, 3, 1, input_buffer, comp_number, nets, nets_cap);
					
					if (!(infile >> nets[0] >> nets[1] >> nets[2])) {
						exit_loop = 1;
					}
					netnums.in1  = nets[0];
					netnums.in3  = nets[1];
					netnums.out2 = nets[2];
						
					if (nl_verbose)
						cout << "OPAMP net " << nets[0] << ", " << nets[1] << ", " << nets[2] << endl;

					// don't know value - use default
					value = A;
					
					head = update_circuit(OPAMP_ID, calc_next_pos(curr_pos, OPAMP_ID), head, comp_number, value);	
					break;
				}
				// else assume SPICE lossy transmission line
				// *** Note: This is the same code as in case T, but I'm duplicating it because these two should read different models eventually ***
				read_comp(infile, net_names, curr_netnum, 4, 1, input_buffer, comp_number, nets, nets_cap);
				id = TLINE_ID;

				if (nl_verbose) {
					cout << "Transmission Line " << comp_number << " from nets " << nets[0] << ", " << nets[1] << " to nets " << nets[2] << ", " << nets[3] << " with Z0 = " << value << "." << endl;
				}
				if (nets[0] == 0 && nets[2] == 0 && nets[1] != 0 && nets[3] != 0) {
					swap(nets[0], nets[1]);
					swap(nets[2], nets[3]);
				}
				else {
					if (nets[1] != 0) {
						cout << "Circuit Solver only supports grounded transmission lines, so T" << comp_number << " will be connected to ground instead of net " << nets[1] << "." << endl;
					}
					if (nets[3] != 0) {
						cout << "Circuit Solver only supports grounded transmission lines, so T" << comp_number << " will be connected to ground instead of net " << nets[3] << "." << endl;
					}
				}
				if (nets[1] != nets[3]) {
					cout << "Circuit Solver only supports transmission lines where the return pins are connected to the same nets." << endl;
				}
				value        = read_number(input, eq); // *** value is Z0 in SPICE and width in Circuit Solver ***
				netnums.in2  = nets[0];
				netnums.out2 = nets[2];
				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);
				break;

			/**** RLC ****/
			case 'L':
			case 'l': // check for ff (flipflop) also?
				if (compare_token_ncs(input, 1, "atch")) { // Latch
					cout << "Latches are currently not supported." << endl; // *** CHANGE ***
					break;
				}
				id = INDUCTOR_ID;
				ch = 'L';
			case 'R':
			case 'r':
				if (id == NO_ID) {
					id = RES_ID;
					ch = 'R';
				}
			case 'C':
			case 'c':
				if (id == NO_ID) {
					id = CAP_ID;
					ch = 'C';
				}
				value = read_two_port(infile, net_names, eq, netnums, curr_netnum, input_buffer, comp_number, nets, nets_cap);
				
				if (nl_verbose)
					cout << ch << comp_number << " from net " << nets[0] << " to net " << nets[1] << " of value " << value << endl;
				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);
				break;
			
			/**** V SOURCE ****/
			case 'V':
			case 'v':
			//case 'E':
			//case 'e':
				read_comp(infile, net_names, curr_netnum, 2, 1, input_buffer, comp_number, nets, nets_cap);

				if (input_buffer[0] == 'v' || input_buffer[0] == 'V') {
					if (compare_token_ncs(input, 0, "ac")) {
						id = V_SOURCE_ID;
					}
					else if (compare_token_ncs(input, 0, "dc")) {
						id = BATTERY_ID;
					}
					else {
						id = 0; // might want to use a default
					}
				}
				else {
					if (compare_token_ncs(input, 0, "s")) {
						id = BATTERY_ID; // AC or DC?
					}
					else {
						id = 0;
					}
				}
				if (id == 0) {
					cerr << "Invalid token: ";
					print_err_ch((unsigned char *)input);
					cerr << endl;
					id = V_SOURCE_ID;
				}
				if (!(infile >> input))
					exit_loop = 1;
					
				value        = read_number(input, eq);
				netnums.in2  = nets[0];
				netnums.out2 = nets[1];
				
				if (nl_verbose)
					cout << "V" << comp_number << " from net " << nets[0] << " to net " << nets[1] << " of value " << value << endl;
				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);
				break;

			/**** DEPENDENT SOURCES ****/
			// VCVS = E, CCVS = H, VCCS = G, CCCS = F,
			case 'E':
			case 'e':
			case 'F':
			case 'f':
			case 'G':
			case 'g':
			case 'H':
			case 'h': // controlled sources
				if (input_buffer == "FVCVS" || input_buffer == "fvcvs" || input_buffer == "e" || input_buffer == "E") {
					id = VCVS_ID;				
					if (nl_verbose)
						cout << "VCVS" << endl;
				}
				else if (input_buffer == "FVCCS" || input_buffer == "fvccs" || input_buffer == "g" || input_buffer == "G") {
					id = VCCS_ID;				
					if (nl_verbose)
						cout << "VCCS" << endl;
				}
				else if (input_buffer == "FCCVS" || input_buffer == "fccvs" || input_buffer == "fs" || input_buffer == "FS" || input_buffer == "h" || input_buffer == "H") {
					id = CCVS_ID;			
					if (nl_verbose)
						cout << "CCVS" << endl;
				}
				else if (input_buffer == "FCCCS" || input_buffer == "fcccs" || input_buffer == "f" || input_buffer == "F") {
					id = CCCS_ID;		
					if (nl_verbose)
						cout << "CCCS" << endl;
				}
				else { // undefined
					cerr << "Invalid token on line " << linenum << ": " << input_buffer << endl;
					break;
				}
				// *** cccs and ccvs might use name of 0V voltage source ***
				read_comp(infile, net_names, curr_netnum, 4, 5, input_buffer, comp_number, nets, nets_cap);
				value = read_number(input, eq);

				if (nl_verbose)
					cout << comp_number << " net " << nets[0] << ", " << nets[1] << ", " << nets[2] << ", " << nets[3] << " of value " << value << endl;

				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);

				netnums.in1  = nets[0];
				netnums.in2  = nets[1];
				netnums.out1 = nets[2];
				netnums.out2 = nets[3];
				break;

			/**** DIODE ****/
			case 'D':
			case 'd':
				read_comp(infile, net_names, curr_netnum, 2, 1, input_buffer, comp_number, nets, nets_cap);
				netnums.in2  = nets[0];
				netnums.out2 = nets[1];
				value        = diode_vdrop;
				
				if (nl_verbose) {
					cout << "D" << comp_number << " from net " << nets[0] << " to net " << nets[1] << " of type ";
					print_ch((unsigned char *)input);
					cout << " default value of " << diode_vdrop << endl;
				}
				head = update_circuit(DIODE_ID, calc_next_pos(curr_pos, DIODE_ID), head, comp_number, value);
				break;
			
			/**** MOSFET/JFET ****/
			case 'J':
			case 'j':
				cout << "JFETs are not supported in Circuit Solver and will be replaced by MOSFETs." << endl;
			case 'M':
			case 'm': // D G S
				read_comp(infile, net_names, curr_netnum, 4, 1, input_buffer, comp_number, nets, nets_cap);
				if (input[0] != 0) {
					if (comp_char(input[0], 'n')) // model
						id = NFET_ID;
					else if (comp_char(input[0], 'p'))
						id = PFET_ID;
					else
						id = FET_ID;
				}
				else
					id = FET_ID;

				if (infile >> l_param >> w_param) {
					l_val = extract_param(l_param, eq);
					w_val = extract_param(w_param, eq);
					value = w_val/l_val;
				}
				else {
					value = 1;
					exit_loop = 1;
				} 
				netnums.in2  = nets[0];
				netnums.out1 = nets[1];
				netnums.out3 = nets[2];
				
				if (nl_verbose)
					cout << "M" << comp_number << " net " << nets[0] << ", " << nets[1] << ", " << nets[2] << ", " << nets[3] << " w/l = " << value << endl;
				
				head = update_circuit(id, calc_next_pos(curr_pos, FET_ID), head, comp_number, value);
				break;
			
			/**** BJT ****/
			case 'Q':
			case 'q': // C B E
				read_comp(infile, net_names, curr_netnum, 3, 1, input_buffer, comp_number, nets, nets_cap);
				id = Qsearch(Model, (string)input, 0) + Q_base_id;
				
				if (id < Q_base_id || id > Q_max_id)
					id = TRANSISTOR_ID;
				
				netnums.in2  = nets[0];
				netnums.out1 = nets[1];
				netnums.out3 = nets[2];
					
				if (nl_verbose) {
					cout << "Q" << comp_number << " net " << nets[0] << ", " << nets[1] << ", " << nets[2] << " of model ";
					print_ch((unsigned char *)input);
					cout << " id " << id << endl;
				}
				head = update_circuit(id, calc_next_pos(curr_pos, TRANSISTOR_ID), head, comp_number, 0);
				break;

			/**** TRANSMISSION LINE ****/
			case 'T':
			case 't': // tline
				read_comp(infile, net_names, curr_netnum, 4, 1, input_buffer, comp_number, nets, nets_cap);
				id = TLINE_ID;

				if (nl_verbose) {
					cout << "Transmission Line " << comp_number << " from nets " << nets[0] << ", " << nets[1] << " to nets " << nets[2] << ", " << nets[3] << " with Z0 = " << value << "." << endl;
				}
				if (nets[0] == 0 && nets[2] == 0 && nets[1] != 0 && nets[3] != 0) {
					swap(nets[0], nets[1]);
					swap(nets[2], nets[3]);
				}
				else {
					if (nets[1] != 0) {
						cout << "Circuit Solver only supports grounded transmission lines, so T" << comp_number << " will be connected to ground instead of net " << nets[1] << "." << endl;
					}
					if (nets[3] != 0) {
						cout << "Circuit Solver only supports grounded transmission lines, so T" << comp_number << " will be connected to ground instead of net " << nets[3] << "." << endl;
					}
				}
				if (nets[1] != nets[3]) {
					cout << "Circuit Solver only supports transmission lines where the return pins are connected to the same nets." << endl;
				}
				value        = read_number(input, eq); // *** value is Z0 in SPICE and width in Circuit Solver ***
				netnums.in2  = nets[0];
				netnums.out2 = nets[2];
				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);
				break;

			case 'U':
			case 'u': // uniform distributed lossy RC transmission line (similar to T above)
				read_comp(infile, net_names, curr_netnum, 3, 1, input_buffer, comp_number, nets, nets_cap);
				id = TLINE_ID;

				if (nl_verbose) {
					cout << "Transmission Line " << comp_number << " from net " << nets[0] << " to net " << nets[1] << " with capacitances connected to net " << nets[2] << " with length = " << value << "." << endl;
				}
				if (nets[2] != 0) {
					cout << "Circuit Solver only supports grounded capacitor transmission lines, so U" << comp_number << " will be connected to ground instead of net " << nets[2] << "." << endl;
				}
				value        = read_number(input, eq); // *** value is length in SPICE and width in Circuit Solver, but we need length as well ***
				netnums.in2  = nets[0];
				netnums.out2 = nets[1];
				
				head = update_circuit(id, calc_next_pos(curr_pos, id), head, comp_number, value);
				break;
			
			/**** TRANSFORMER ****/
			case 'K':
			case 'k': // (Part of) Transformer
				//id = XFMR_ID;
				for (i = 1; i < (unsigned)NUMBER_LENGTH && i < input_buffer.length() && is_number(input_buffer[i]); ++i) 
					number[i-1] = input_buffer[i];
					
				comp_number = atoi(number);
				if (i < input_buffer.length() && input_buffer[i] == ' ') {
					++i;
				}
				if (i >= input_buffer.length() || comp_char(input_buffer[i], 'l') == 0) {
					cerr << "Invalid token on line " << linenum << ": " << input_buffer << ", character " << input_buffer[i] << endl;
					break;
				}
				for (; i < (unsigned)NUMBER_LENGTH && i < input_buffer.length() && is_number(input_buffer[i]); ++i) {
					number[i-1] = input_buffer[i];
				}					
				comp_number2 = atoi(number);
				value = read_number(input, eq); // value = 1 in most cases, Maybe change later
				break;
				
			/**** LOGIC FUNCTION ****/
			case '@': // boolean function
				if (input_buffer.length() > 1 || (infile.good() && infile >> input_buffer)) {
					read_logic_function(input_buffer);
					head = add_logic_function(head, net_names, cnets, curr_netnum, curr_pos, num_components);
				}
				else
					cerr << "Error: Expecting function after '@' on line " << linenum << "." << endl;
				break;
			
			/**** EQUATION ****/
			case '(': // equation
				if (input_buffer[input_buffer.length()-1] != ')') {
					cerr << "Error: Equations beginning with '(' must end with ')' (line " << linenum << ")." << endl;
					break;
				}
				input_buffer[input_buffer.length()-1] = 0;
				
				for (i = 1; i < input_buffer.length()-1 && is_token_char(input_buffer[i]); ++i) {
					input[i] = input_buffer[i];
				}
				input[i] = 0;
				if (input_buffer[i] != '=') {
					cerr << "Error: Expecting '=' after variable name in equation on line " << linenum << "." << endl;
					break;
				}
				new_eq = new cs_equation(MAX_ENTERED_CHARACTERS);
				new_eq->set_owner(&eq, 1);
				new_eq->create_from_expr((input_buffer.c_str() + i + 1));
				
				if (!new_eq->var_with_x()) { // equation is not dependent on x - constant
					value = new_eq->eval(0);
					input_ptr[0] = input;
					if(!eq.add_const(input_ptr, &value, 1, 1)) {
						cerr << "Error adding constant equation on line " << linenum << "." << endl;
					}
					delete new_eq;
					break;
				}
				if (!eq.add_equation(input, new_eq, 1)) {
					cerr << "Error adding equation on line " << linenum << "." << endl;
				}
				break;
				
			default: // undefined
				cerr << "Invalid token on line " << linenum << ": " << input_buffer << endl; 
		}
		++linenum;
		if (netnums.in1 + netnums.in2 + netnums.in3 + netnums.out1 + netnums.out2 + netnums.out3 > -6) {			
			add_new_nets(cnets, netnums);
			++num_components;
		}
	} // while (infile)

	infile.close();
	surpress_show_values = 0;

	if (nets != NULL) {
		delete [] nets;
	}
	if (nl_verbose)
		cout << endl;

	show_elapsed_time();
	cout << "Read " << num_components << " component";
	if (num_components != 1)
		cout << "s";
	cout << " and " << curr_netnum << " net";
	if (curr_netnum != 1)
		cout << "s";	
	cout << "." << endl;

	return head;
}




comp_sim *create_comp_sims_from_netlist(component *head, comp_nets *cnets, int num_components) {

	int *conn, i;
	comp_sim *cs_arr = NULL;

	if (num_components <= 0)
		return NULL;

	show_elapsed_time();
	cout << "Building component network from netlist." << endl;

	cs_arr = memAlloc(cs_arr, num_components);

	// need to build comp_sim backwards because update_circuit adds components to the beginning of the list
	// and the sorting/reversing phase is skipped in this case
	for (i = num_components - 1; i >= 0; --i) {
		if (head == NULL) {
			internal_error();
			cerr << "Error: Null pointer head in create_comp_sims_from_netlist at index " << (num_components-i-1) << "." << endl;
		}
		if (cnets == NULL) {
			internal_error();
			cerr << "Error: Null pointer cnets in create_comp_sims_from_netlist at index " << (num_components-i-1) << "." << endl;
		}
		if (head == NULL || cnets == NULL) {
			return cs_arr;
		}

		// copy data
		conn = cs_arr[i].conn;

		cs_arr[i].flags    = head->flags;
		cs_arr[i].id       = head->id;
		cs_arr[i].location = head->location;     // should be unimportant
		cs_arr[i].state    = head->state; // should be 'X'
		cs_arr[i].value    = head->value;

		conn[0] = cnets->nets.in1;
		conn[1] = cnets->nets.in2;
		conn[2] = cnets->nets.in3;
		conn[3] = cnets->nets.out1;
		conn[4] = cnets->nets.out2;
		conn[5] = cnets->nets.out3;

		if (cnets->nets.in2 >= 0) {
			cs_arr[i].conn_loc = cnets->nets.in2;
		}
		else {
			cs_arr[i].conn_loc = cnets->nets.out2;
		}
		head  = head->next;
		cnets = cnets->next;
	}
	if (head != NULL) {
		internal_error();
		cerr << "Error: End of component linked list has not been reached in create_comp_sims_from_netlist with num_components = " << num_components << "." << endl;
	}
	if (cnets != NULL) {
		internal_error();
		cerr << "Error: End of net linked list has not been reached in create_comp_sims_from_netlist with num_components = " << num_components << "." << endl;
	}
	return cs_arr;
}




void add_new_nets(comp_nets *& cnets, connections &netnums) {

	comp_nets *temp_cnets = new comp_nets;
	temp_cnets->nets      = netnums;
	temp_cnets->next      = cnets;	
	cnets                 = temp_cnets;
}




int calc_next_pos(int &curr_pos, int ID) {
	
	int length = cScale*comp_lengths[ID], width = cScale*comp_widths[ID];
	int dwidth = (int)ceil((width + 1)/2.0);

	if (x_blocks == 0 || y_blocks == 0)
		return 1; // arbitrary number > 0 to avoid divide by zero error
		
	if (curr_pos%x_blocks >= (x_blocks - MAX_LENGTH - ceil(length/2.0) - 1)) {
		curr_pos = (int)((curr_pos/x_blocks + max_width)*x_blocks + floor(length/2.0));
		max_width = 0;
	}
	else {
		if (dwidth > last_max_width) {
			max_width += (dwidth - last_max_width);
			curr_pos += x_blocks*(dwidth - last_max_width);
			last_max_width = dwidth;
		}
		max_width = max(width, max_width);
		curr_pos += (int)(ceil((length + last_length)/2.0));
	}
	last_length = length;
	
	return curr_pos;
}




int do_assignment(string &lhs, string &rhs, StringTable &net_names, int &curr_netnum) {

	int node1, node2;

	if (lhs.length() > 0 && rhs.length() > 0 && (rhs[0] == '$' || rhs[0] == '"' || is_number(rhs[0]))) {
		get_real_name(lhs, lhs);
		get_real_name(rhs, rhs);
		
		if (nl_verbose)
			cout << "Found assignment from " << lhs << " to " << rhs << endl;
		
		node1 = net_names.search(lhs);
		node2 = net_names.search(rhs);
				
		if (node1 == default_st_val) {
			if (node2 == default_st_val) {
				net_names.insert(lhs, curr_netnum);
				net_names.insert(rhs, curr_netnum);
				++curr_netnum;
				return 1;
			}
			net_names.insert(lhs, node2);
			return 1;
		}
		if (node2 == default_st_val) {
			net_names.insert(rhs, node1);
			return 1;
		}
		cerr << "Error: Both net " << lhs << " and net " << rhs << " are already assigned." << endl;
		return 1;
	}
	return 0;
}




void get_real_name(string &out, string &in) {
	
	if (in.length() == 0) {
		out = in;
		return;
	}
	if (in[0] == '$')
		out = (string)(in.c_str() + 1);
	else
		out = in;
	
	if (out == "\"0\"")
		out = ground_string;
			
	if (out == ground_string)
		has_ground_net = 1;
}




void read_comp(ifstream &infile, StringTable &net_names, int &curr_netnum, int pins, int init_i, string &input_buffer, int &comp_number, int *&nets, unsigned &nets_cap) {
	
	unsigned i;
	char number[NUMBER_LENGTH + 1] = {0};
	string net_str;

	for (i = init_i; i < (unsigned)NUMBER_LENGTH && i < input_buffer.length() && is_number(input_buffer[i]); ++i) {
		number[i-init_i] = input_buffer[i];
	}
	comp_number = atoi(number);
	
	if (!infile.good()) {
		return;
	}
	if (pins >= (int)nets_cap) { // resize
		delete [] nets;
		nets_cap = (unsigned)(1.5*pins);
		nets     = memAlloc(nets, nets_cap);
	}
	for (i = 0; i < (unsigned)pins; ++i) {
		if (!(infile >> net_str)) {
			return;
		}
		else {
			nets[i] = get_netnum_from_string(net_names, net_str, curr_netnum);
		}
	}
	infile >> input;
}




double read_number(char *input, cs_equation &eq) {

	int cont(1);
	double num;
	
	if (input[0] == 0)
		return 0;
		
	if (input[0] == '(') {
		if (!eq.create_from_expr(input)) {
			cerr << "Invalid expression: " << input << ". Setting to 0." << endl;
			return 0;
		}
		if (eq.var_with_x() && !entered_x) {
			cout << "Enter the parameter x:  ";
			Xval = ind();
			entered_x = 1;	
		}
		num = eq.eval(Xval);
		
		if (!valid_number(num)) {
			cerr << "Expression evaluates to invalid number: " << input << ". Setting to 0." << endl;
			return 0;
		}
		return num;	
	}
	num = atof(input);
	
	for (unsigned int i = 0; i < (unsigned)MAX_ENTERED_CHARACTERS && input[i] != 0 && cont; ++i) {
		if (is_number(input[i])) {
			cont = 1;
			continue;
		}
		cont = 0;

		switch (input[i]) {
			case '.':
			case '-':
			case 'e':
			case 'E':
				cont = 1; // still part of the number
				break;
					
			case 'a':
			case 'A':
				num *= 1e-18;
				break;
			
			case 'f':
			case 'F':
				num *= 1e-15;
				break;
					
			case 'p':
				num *= 1e-12;
				break;
					
			case 'n':
			case 'N':
				num *= 1e-9;
				break;
					
			case 'u':
			case 'U':
				num *= 1e-6;
				break;
					
			case 'm':
				if (compare_token_ncs(input, i+1, "eg"))
					num *= 1e6;	
				else
					num *= 1e-3;
				break;
					
			case 'k':
			case 'K':
				num *= 1e3;
				break;
					
			case 'M':
				num *= 1e6;
				break;
					
			case 'g':
			case 'G':
				num *= 1e9;
				break;
					
			case 't':
			case 'T':
				num *= 1e12;
				break;
					
			case 'P':
				num *= 1e15;
				break;
					
			default:
				cerr << "Invalid token: ";
				print_err_ch((unsigned char *)input);
				cerr << endl;
		}
	}
	return num;
}




double read_two_port(ifstream &infile, StringTable &net_names, cs_equation &eq, connections &netnums, int &curr_netnum, string &input_buffer, int &comp_number, int *&nets, unsigned &nets_cap) {

	read_comp(infile, net_names, curr_netnum, 2, 1, input_buffer, comp_number, nets, nets_cap);
	netnums.in2  = nets[0];
	netnums.out2 = nets[1];
	
	return read_number(input, eq);
}




double extract_param(string &param_str, cs_equation &eq) {
	
	int found_num(-1);
	unsigned int i;
	
	for (i = 0; i < (unsigned)MAX_CHARACTERS && i < param_str.length(); ++i) {
		if (found_num == -1 && (param_str[i] == '.' || is_number(param_str[i]))) 
			found_num = i;
			
		if (found_num) 
			input[i-found_num] = param_str[i];
	}
	if (i - found_num >= 0)
		input[i-found_num] = 0;
	
	return read_number(input, eq);
}




int extract_type(ifstream &infile, string &input_buffer, int &buf_pos) {

	//BUF, NOT, AND, OR, XOR, NAND, NOR, XNOR
	int id(NO_ID);
	
	buf_pos = 0;
	
	switch(input_buffer[0]) {
	
		case 'a':
		case 'A':
			if (compare_token_ncs(input_buffer.c_str(), 1, "nd")) {
				id = AND_ID;
				buf_pos = 3;
			}
			break;
			
		case 'b':
		case 'B':
			if (compare_token_ncs(input_buffer.c_str(), 1, "uf")) {
				id = BUFFER_ID;
				buf_pos = 3;
			}
			break;
			
		case 'i':
		case 'I':
			if (compare_token_ncs(input_buffer.c_str(), 1, "nv")) {
				id = NOT_ID;
				buf_pos = 3;
			}
			break;
			
		case 'n':
		case 'N':
			if (compare_token_ncs(input_buffer.c_str(), 1, "and")) {
				id = NAND_ID;
				buf_pos = 4;
			}
			else if (compare_token_ncs(input_buffer.c_str(), 1, "or")) {
				id = NOR_ID;
				buf_pos = 3;
			}
			else if (compare_token_ncs(input_buffer.c_str(), 1, "ot")) {
				id = NOT_ID;
				buf_pos = 3;
			}
			break;
			
		case 'o':
		case 'O':
			if (compare_token_ncs(input_buffer.c_str(), 1, "r")) {
				id = OR_ID;
				buf_pos = 2;
			}
			break;
			
		case 'x':
		case 'X':
			if (compare_token_ncs(input_buffer.c_str(), 1, "or")) {
				id = XOR_ID;
				buf_pos = 3;
			}
			else if (compare_token_ncs(input_buffer.c_str(), 1, "nor")) {
				id = XNOR_ID;
				buf_pos = 4;
			}
			break;
	}
	if (id == NO_ID) {
		cerr << "Illegal gate specification: " << input_buffer << endl;
		return id;
	}
	if ((int)input_buffer.length() <= buf_pos) {
		if (!(infile >> input_buffer)) {
			return id;
		}
		if (input_buffer[0] != '(') {
			cerr << "Error: Expecting '(': " << input_buffer << endl;
			return id;
		}
		if (input_buffer.length() == 1) {
			if (!(infile >> input_buffer)) {
				return id;
			}
			buf_pos = 0;
		}
		else {
			buf_pos = 1;
		}
	}
	else {
		if (input_buffer[buf_pos] != '(') {
			cerr << "Error: Expecting '(': " << input_buffer << endl;
			return id;
		}
		if ((int)input_buffer.length() == buf_pos+1) {
			if (!(infile >> input_buffer)) {
				return id;
			}
			buf_pos = 0;
		}
		else {
			++buf_pos;
		}
	}
	return id;
}




int get_input_nodes(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int &buf_pos, int *&nets, unsigned &nets_cap) {

	int input_counter(0);

	do {
		++input_counter;

		if (input_counter >= (int)nets_cap) {
			nets = (int *)double_array_size(nets, nets_cap);
		}
	} while (get_input_node(infile, net_names, curr_netnum, input_buffer, buf_pos, nets[input_counter], input_counter));

	return input_counter;
}




int get_input_node(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int &buf_pos, int &node, int nnum) {
	
	unsigned int i, j;
	char nname[MAX_ENTERED_CHARACTERS] = {0};
	string name_in;
			
	for (i = buf_pos, j = 0; i < input_buffer.length() && input_buffer[i] != ' ' && input_buffer[i] != ',' && input_buffer[i] != ')'; ++i, ++j) {
		nname[j] = input_buffer[i];
	}
	if ((int)i == buf_pos) {
		cerr << "Error: Expecting net name/number";
		if (nnum != 1)
			cerr << " or ')'";
		cerr << ": " << input_buffer << endl;
		return 0;
	}	
	nname[j] = 0;
	name_in = (string)nname;
	node = get_netnum_from_string(net_names, name_in, curr_netnum);
	
	if (node < 0) {
		cerr << "Error: Invalid net number: " << node << endl;
		return 0;
	}
	if (i >= input_buffer.length() || ((i + 1) == input_buffer.length() && input_buffer[i] == ',')) {
		if (!(infile >> input_buffer)) {
			cerr << "Error: Expecting net name/number or ')': " << input_buffer << endl;
			return 0;
		}
		buf_pos = 0;	
	}
	else if (input_buffer[i] == ')') {
		return 0;
	}
	else {
		buf_pos = i + 1;
	}
	return 1;
}



// not used
int get_num_in_parens(ifstream &infile, StringTable &net_names, int &curr_netnum, string &input_buffer, int pos) {

	int saw_paren(0);
	unsigned int i(0), j;
	char nname[MAX_ENTERED_CHARACTERS] = {0};
	string name_in;

	if (pos >= (int)input_buffer.length()) {
		if (!(infile >> input_buffer)) {
			cerr << "Error: Expecting net name/number or '(': " << input_buffer << endl;
			return -1;
		}
	}
	else
		i = pos;
		
	if (input_buffer[i] == '(') {
		++i;
		saw_paren = 1;
	}		
	for (j = 0; i < input_buffer.length()  && input_buffer[i] != ' ' && input_buffer[i] != ',' && input_buffer[i] != ')'; ++i, ++j)
		nname[j] = input_buffer[i];
		
	nname[j] = 0;
		
	if (j == 0) {
		cerr << "Error: Expecting net name/number: " << input_buffer << endl;
		return -1;
	}
	if (saw_paren && i < input_buffer.length() && input_buffer[i] != ')')
		cerr << "Error: Expecting ')': " << input_buffer << endl;
				
	else if ((i + saw_paren) < input_buffer.length())
		cerr << "Error: Extra characters at end of line: " << input_buffer << endl;
	
	name_in = (string)nname;
	
	return get_netnum_from_string(net_names, name_in, curr_netnum);
}




int get_netnum_from_string(StringTable &net_names, string &netname, int &curr_netnum) {
	
	int netval;
	string name;

	if (netname.length() == 0) {
		cerr << "\nError: NULL string for netname." << endl;
		return -1;
	}				
	get_real_name(name, netname);

	netval = net_names.search_and_insert(name, curr_netnum);

	if (netval == curr_netnum) {
		if (name == "\"1\"") {
			has_vdd_net = curr_netnum;
		}
		++curr_netnum;

		return curr_netnum-1;
	}
	return netval;
}




int map_output_one_to_one(StringTable &net_names, StringTable &out_table, string &str, int *out1, int *out2, char *match_found, int &output_ctr, int &curr_netnum, int &error_code) {

	int match_net, out_net;

	if (out1 == NULL || out2 == NULL || match_found == NULL || output_ctr < 0 || curr_netnum < 0) {
		error_code = BAD_VALUES;
		return 0;
	}
	match_net = string_match_one_to_one(out_table, str, match_found, OUTPUT_TYPE, error_code);

	if (error_code != 0) {
		return match_net;
	}
	out_net = net_names.search_and_insert(str, curr_netnum); // first reference to this net name?

	if (out_net == curr_netnum) {
		++curr_netnum; // create new net
	}
	out1[output_ctr]   = match_net; // old net currently in out_table
	out2[output_ctr++] = out_net;   // new net
	error_code         = 0;

	return out_net;
}




int string_match_one_to_one(StringTable &s_table, string &str, char *match_found, char ntype, int &error_code) {

	int match_net;

	if (match_found == NULL) {
		error_code = BAD_VALUES;
		return 0;
	}
	if (s_table.get_num_strings() == 0) {
		error_code = TABLE_EMPTY | NO_MATCH;
		return 0;
	}
	match_net = s_table.search(str);

	if (match_net == default_st_val) {
		error_code = NO_MATCH;
		return 0;
	}
	if (match_found[match_net] & ntype) {
		error_code = REPEAT_MATCH;
		return match_net;
	}
	match_found[match_net] |= ntype;
	error_code = 0;

	//s_table.remove(str); // ???

	return match_net;
}




// ****** Export ******




void export_netlist(component *& head) {

	while (!exp_netlist(head)) {	
		cout << "\nSave netlist?  ";
		if (!decision())
			return;
	}
}



int exp_netlist(component *& head) {

	ofstream outfile;
	char netlist_name[MAX_SAVE_CHARACTERS + 1] = {0};
	int complex, subcircuit_count, nwires, status(1), flags(0);
	int add_sc(0), max_position, nodes, num_wires(0);
	component *tail = NULL;
	wire *hier_wires = NULL;
	
	if (head == 0 || count_components(head, tail, subcircuit_count, max_position, nwires) == 0) {
		beep();
		cout << "\nThere is no netlist to save!" << endl;
		return 1;
	}
	cout << "\nSave netlist as(max 31 characters):  ";
	
	cin >> ws;					
	strcpy(netlist_name, xin(MAX_SAVE_CHARACTERS));
	
	if (!overwrite_file(netlist_name)) {
		return 0;
	}
	if (subcircuit_count > 0) {	
		cout << "\nAdd components included in subcircuits (ICs) to netlist?  ";
		if (decision())
			add_sc = 1;
	}
	complex    = useCOMPLEX;
	useCOMPLEX = 1;
	useDIGITAL = 0;
	
	show_clock();	
	outfile.open(netlist_name, ios::out | ios::trunc);
	
	if (outfile.fail() || !filestream_check(outfile)) {
		beep();
		cerr << "\nError: Netlist could not be saved as " << netlist_name << "!" << endl;
		return 0;
	}
	if (add_sc) {
		head = simulation_setup(head, hier_wires, num_wires);
	}
	no_ref_needed = 1;
	nodes = number_nodes(head, hier_wires, num_wires);
	no_ref_needed = 0;
	
	if (!nodes) {
		if (add_sc) {
			head = simulation_cleanup(head);
		}
		return 1;
	}
	flags = check_comp_types(head);	

	if (compatibility == 1)
		outfile << "# ";
	else if (compatibility == 2 || compatibility == 9)
		outfile << "* ";
		
	outfile << "Circuit - " << netlist_name << endl << endl;

	if (!filestream_check(outfile)) {
		status = 0;
	}
	write_ios(outfile, head, flags, status); // I/Os are added first.
	write_gates(outfile, head, status); // Logic gates are added before analog components.
	write_analog(outfile, head, flags, add_sc, status);

	if ((flags & HAS_ICS) && !add_sc) {
		write_subcircuits(outfile, head, add_sc, status); // ICs
	}
	if (flags & NEEDS_MODELS) {
		write_models(outfile, head, status); // write model definitions
	}
	if (status) {
		if (compatibility != 1) {
			outfile << endl << ".OPTIONS" << endl << ".END" << endl;
		}
		if (!filestream_check(outfile)) {
			status = 0;
		}
	}
	outfile.close();
	reset_cursor();
	
	useCOMPLEX = complex;
	
	if (add_sc) {
		head = simulation_cleanup(head);
	}
	if (!status) {
		cerr << "\nError: Not enough space to save entire netlist!" << endl;
		return 0;
	}
	cout << "\nThe current Circuit Solver netlist has been saved as: " << netlist_name << "." << endl;

	return 1;
}




int check_comp_types(component *head) {

	int flags(0), ID, order;

	compatibility = 2;
	
	while (head != 0) {
		ID    = head->id;
		order = get_order(ID);

		switch (order) {
		case wirex:
		case flipflopx:
			break;

		case icx:
			flags |= HAS_ICS;
			break;

		case transistorx:
			flags |= NEEDS_MODELS;
			break;

		default:
			switch (ID) {
			case CAP_ID:
			case INDUCTOR_ID:
				flags |= USE_AC_SRC;
				break;

			case INPUT_ID:
				flags |= HAS_INPUTS;
				compatibility = 1;
				break;

			case OUTPUT_ID:
				flags |= HAS_OUTPUTS;
				compatibility = 1;
				break;

			case NFET_ID:
			case PFET_ID:
				flags |= NEEDS_MODELS;
				break;

			default:
				if ((ID >= BUFFER_ID && ID <= FF_ID) || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || (ID >= QAND_ID && ID <= DELAY_ID)) {
					compatibility = 1;
				}
			}
		}
		head = head->next;
	}
	return flags;
}




void write_ios(ofstream &outfile, component *head, int flags, int &status) {

	component *temp = head;

	if (status == 0) {
		return;
	}
	if (flags & HAS_INPUTS) {
		while (temp != 0) {
			if (temp->id == INPUT_ID) {
				if (compatibility == 0)
					outfile << "INPUT " << NODENUMBER[temp->location] << endl;
				else
					outfile << "INPUT(" << (NODENUMBER[temp->location] + nof) << ")" << endl;

				if (!outfile.good()) {
					status = 0;
					return;
				}
			}
			temp = temp->next;
		}
		temp = head;
	}
	if (flags & HAS_OUTPUTS) {
		while (temp != 0) {
			if (temp->id == OUTPUT_ID) {	
				if (compatibility == 0)
					outfile << "OUTPUT " << NODENUMBER[temp->location]  << endl;
				else
					outfile << "OUTPUT(" << (NODENUMBER[temp->location] + nof) << ")" << endl;

				if (!outfile.good()) {
					status = 0;
					return;
				}
			}
			temp = temp->next;
		}
	}
}




void write_gates(ofstream &outfile, component *head, int &status) {

	int ID;

	if (status == 0) {
		return;
	}
	while (head != 0) {	
		ID = head->id;	
		if ((ID > NO_ID && ID <= TXNOR_ID) || (ID >= QAND_ID && ID <= QXNOR_ID)) {
			add_gate_to_netlist(outfile, ID, head->location, head->flags);

			if (!outfile.good()) {
				status = 0;
				return;
			}
		}
		head = head->next;
	}
}




void add_gate_to_netlist(ofstream &outfile, int id, int location, char flags) {

	connections x;
	
	x = get_connections(location, id, flags);
	
	if (compatibility == 0)
		outfile << NODENUMBER[x.out2] << " " << comp_symbol[id] << " ";
	else
		outfile << NODENUMBER[x.out2] + nof << " = " << comp_symbol[id] << "(";

	switch (id) {

		case BUFFER_ID:
		case NOT_ID:
		case DELAY_ID:
			outfile << NODENUMBER[x.in2];
			break;
			
		case AND_ID:
		case NAND_ID:
		case OR_ID:
		case NOR_ID:
		case XOR_ID:
		case XNOR_ID:
			outfile << NODENUMBER[x.in1] << " " << NODENUMBER[x.in3];
			break;
			
		case TAND_ID:
		case TNAND_ID:
		case TOR_ID:
		case TNOR_ID:
		case TXOR_ID:
		case TXNOR_ID:
			outfile << NODENUMBER[x.in1] << " " << NODENUMBER[x.in2] << " " << NODENUMBER[x.in3];
			break;
			
		case QAND_ID:
		case QNAND_ID:
		case QOR_ID:
		case QNOR_ID:
		case QXOR_ID:
		case QXNOR_ID:
			outfile << NODENUMBER[x.in1] << " " << NODENUMBER[x.in2] << " " << NODENUMBER[x.in3] << " " << NODENUMBER[x.out1];
			break;
	}
	if (compatibility != 0) {
		outfile << ")";
	}
	outfile << endl;
}




void write_analog(ofstream &outfile, component *head, int flags, int add_sc, int &status) {

	int j, node1, node2, node3, ID, order, pin, pin_offset, temp_nodenum(0), cs(0);
	int number[num_nl_id_types+1];
	unsigned i;
	connections x;
	transistor tr;
	IC ic;

	if (status == 0) {
		return;
	}
	for (i = 0; i < (unsigned)num_nl_id_types; ++i) {
		number[i] = 1;
	}
	while (head != 0) { // Analog components are added last.
		ID    = head->id;
		order = get_order(ID);

		switch (order) {
		case wirex:
			break;

		case transistorx:
			outfile << "Q" << number[10];
			++number[10];
			x     = get_connections(head->location, ID, head->flags);
			node1 = NODENUMBER[x.out1];
			node2 = NODENUMBER[x.in2];
			node3 = NODENUMBER[x.out3];
			
			tr = Qnum_find(tr, (ID - Q_base_id));
			
			if (comp_char(tr.z, 'p')) 
				outfile << " " << node3 << " " << node2 << " " << node1 << " ";
			else
				outfile << " " << node1 << " " << node2 << " " << node3 << " ";
				
			outfile << tr.name << endl;
			break;

		case icx:
			if (add_sc) {
				break;
			}
			find_ic(ic, (ID - IC_base_id));
					
			if (use_ICs)
				outfile << "IC";
			else
				outfile << "X";
						
			outfile << number[13] << " ";
					
			++number[13];
			pin_offset = head->location - ic.pins/4;
								
			for (j = 1; j <= ic.pins; ++j) {
				if (j <= ic.pins/2)
					pin = pin_offset - 1 + (j + 2*x_blocks);		
				else 
					pin = pin_offset + ic.pins - (j + 2*x_blocks);
						
				if (use_ICs)
					outfile << "n";
						
				if (NODENUMBER[pin] >= 0)
					outfile << NODENUMBER[pin] << " ";
				else
					outfile << temp_nodenum++ << " ";
					}
			ic.name[0] = ' ';
			outfile << ic.name << endl;
			break;

		default:
			ID = head->id;
			x  = get_connections(head->location, ID, head->flags);

			switch (ID) {
			case VCVS_ID:
			case CCVS_ID:
			case VCCS_ID:
			case CCCS_ID:
				cs = 1;
			case RES_ID:
			case CAP_ID:
			case BATTERY_ID:
			case INDUCTOR_ID:
			case V_SOURCE_ID:
			case I_SOURCE_ID:
				outfile << comp_symbol[ID] << number[nl_comp_id[ID]];
				++number[nl_comp_id[ID]];
				outfile << " " << NODENUMBER[x.in2] << " " << NODENUMBER[x.out2] << " ";

				if (cs) {
					// *** WRITE CONTROLLED SOURCE NETS ***
					outfile << " " << NODENUMBER[x.in1] << " " << NODENUMBER[x.out1] << " "; // ???
				}
				if (ID == BATTERY_ID || ID == V_SOURCE_ID) {
					if (flags & USE_AC_SRC)
						outfile << "AC ";
					else
						outfile << "DC ";	
				}
				write_units(outfile, head->value);
				cs = 0;
				break;

			case DIODE_ID:
				outfile << comp_symbol[ID] << number[nl_comp_id[ID]];
				++number[nl_comp_id[ID]];
				outfile << " " << NODENUMBER[x.in2] << " " << NODENUMBER[x.out2] << " " << "DIODE" << endl;
				break;

			case NFET_ID:
			case PFET_ID:
				outfile << "M" << number[11];
				++number[11];
				node1 = NODENUMBER[x.out1];
				node2 = NODENUMBER[x.in2];
				node3 = NODENUMBER[x.out3];
				
				if (ID == NFET_ID) // DGS
					outfile << " " << node1 << " " << node2 << " " << node3 << " " << node3 << " " << "n_level_3 ";					
				else 
					outfile << " " << node3 << " " << node2 << " " << node1 << " " << node1 << " " << "p_level_3 ";
				
				outfile << "l=2u w=" << 2*head->value << "u" << endl;
				break;

			case OPAMP_ID:
				outfile << "OPAMP" << number[12];
				++number[12];
				outfile << " " << NODENUMBER[x.in3] << " " << NODENUMBER[x.in1] << " " << NODENUMBER[x.out2] << " " << "OPAMP" << endl; // +, -, Out
				break;

			case XFMR_ID: // Must fix later
				outfile << "L" << number[14] << " " << NODENUMBER[x.in1] << " " << NODENUMBER[x.in2] << " ";
				write_units(outfile, head->value); // Should have two values, L1 and L2
				outfile << "\nL" << (number[14] + 1) << " " << NODENUMBER[x.out1] << " " << NODENUMBER[x.out2] << " ";
				write_units(outfile, head->value); // M = sqrt(L1*L2)
				outfile << "\nK" << (int)(number[14]/2) << "L" << number[14] << " L" << (number[14] + 1) << "1" << endl;
				number[14] += 2;
				break;

			case TLINE_ID:
				outfile << comp_symbol[ID] << number[nl_comp_id[ID]];
				++number[nl_comp_id[ID]];
				outfile << " " << NODENUMBER[x.in2] << " 0 " << NODENUMBER[x.out2] << " 0 ";
				//outfile << "Z0="; // ???
				write_units(outfile, head->value); // *** NOT ACTUALLY CORRECT VALUE *** - want Z0, not width

			default:
				head = head->next;
				continue;
			}
		}
		if (!outfile.good()) {
			status = 0;
			return;
		}
		head = head->next;
	}
	outfile << endl;

	return;
}




void write_subcircuits(ofstream &outfile, component *head, int add_sc, int &status) {

	int num_ics, ID;
	char *ics_written = NULL;
	IC ic;

	if (status == 0) {
		return;
	}
	num_ics     = count_ics();
	ics_written = memAlloc_init(ics_written, num_ics, (char)0);
		
	while (head != 0) {
		if (get_order(head->id) == icx && !add_sc) {
			ID = head->id - IC_base_id;
			if (ID < num_ics && ics_written[ID] == 0) {
				ics_written[ID] = 1;
				find_ic(ic, ID);				
				outfile << ".subckt";
				ic.name[0] = ' ';
				outfile << ic.name << endl << endl;
				// *** write subcircuit ***
				outfile << ".ends" << endl << endl;

				if (!outfile.good()) {
					status = 0;
					return;
				}
			}
		}
		head = head->next;
	}
	delete [] ics_written;

	outfile << endl;
}




void write_models(ofstream &outfile, component *head, int &status) {

	int ID, n_model(0), p_model(0);
	transistor tr;

	if (status == 0) {
		return;
	}
	while (head != 0) {
		ID = head->id;
		if (get_order(ID) == transistorx) {
			tr = Qnum_find(tr, (ID - Q_base_id));
				
			outfile << ".model " << tr.name;
				
			if (comp_char(tr.z, 'p')) 
				outfile << " pnp " << "bf=" << tr.q_beta << endl;
			else if (comp_char(tr.z, 'n'))
				outfile << " npn " << "bf=" << tr.q_beta << endl;	
		}
		else if (ID == NFET_ID && !n_model) {
			outfile << ".model n_level_" << DEF_SPICE_LEVEL << " level=" << DEF_SPICE_LEVEL << endl;
			n_model = 1;
		}
		else if (ID == PFET_ID && !p_model) {
			outfile << ".model p_level_" << DEF_SPICE_LEVEL << " level=" << DEF_SPICE_LEVEL << endl;
			p_model = 1;
		}
		else {
			head = head->next;
			continue;
		}
		if (!outfile.good()) {
			status = 0;
			return;
		}
		head = head->next;
	}
}




// ****** Random Netlist ******


int create_random_netlist(unsigned nin, unsigned nout, unsigned size, const char *name) {

	ofstream outfile;

	if (nin == 0 || nout == 0 || size == 0 || name == NULL) {
		return 0;
	}
	if (!outfile_file(outfile, name, 0) || !filestream_check(outfile)) {
		cerr << "Error opening netlist " << name << " for writing." << endl;
		return 0;
	}

	// *** WRITE ***

	outfile.close();

	return 1;
}









