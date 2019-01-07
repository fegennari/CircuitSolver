#include "CircuitSolver.h"
#include "Logic.h"
#include "StringTable.h"
#include "Netlist.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LogicFunctions.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Functions
// By Frank Gennari
#define NUM_LF_VARS 26 // one for each letter of the alphabet


char global_function[MAX_ENTERED_CHARACTERS] = {0};


void create_logic_function();
void read_logic_function(string input);
int input_logic_function(char *function, int num_vars);
int parse_logic_function(char *function, char *function2, int num_vars);
component *add_logic_function(component *head, StringTable &net_names, connections &netnums, int &curr_netnum, int &curr_pos, int &num_components);
component *add_gate(component *head, connections &netnums, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components);
component *add_gate_and_netnums(component *head, comp_nets *& cnets, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits, int decomp_xor);
int get_netnum_from_char(StringTable &net_names, char c, int &curr_netnum);
int get_id_from_char(char c);
component *split_large_gate(component *head, comp_nets *& cnets, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits, int fanin_limit, int decomp_xor);
component *gen_2in_gate(component *head, comp_nets *& cnets, int *nets3, int *nets4, int ID, int &curr_pos, int &curr_netnum, int &num_components, int &num_splits, int decomp_xor);
component *decompose_xor(component *head, comp_nets *& cnets, int *nets, int ID, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits);
component *xor_and_outputs(component *head, comp_nets *& cnets, int *out1, int *out2, int &curr_netnum, int &curr_pos, int &num_components, int &output_net, int num_outputs, int fanin_limit, int decomp_xor, int add_outputs);
component *add_io_to_circuit(component *head, comp_nets *& cnets, int net, int id, int &curr_pos, int &num_components);


void add_new_nets(comp_nets *& cnets, connections &netnums);
int get_netnum_from_string(StringTable &net_names, string &netname, int &curr_netnum);

void input_equation(char *equation);

int calc_next_pos(int &curr_pos, int ID);




void create_logic_function() {

	int num_vars(4);
	char function[MAX_ENTERED_CHARACTERS] = {0};
	
	if (input_logic_function(function, num_vars)) {
		cout << "\nFunction is: ";
		print_ch((unsigned char *)function);
		cout << endl << endl;
	}
}



void read_logic_function(string input) {

	unsigned int i;
	char function2[MAX_ENTERED_CHARACTERS] = {0};

	for (i = 0; i < input.length() && i < (unsigned)MAX_ENTERED_CHARACTERS; ++i)
		function2[i] = input[i];
		
	function2[i] = 0;
		
	parse_logic_function(global_function, function2, NUM_LF_VARS); // have to fix for arbitrary number of variables...
	
	cout << "Function is: ";
	print_ch((unsigned char *)global_function);
	cout << endl;
}




int input_logic_function(char *function, int num_vars) {

	char function2[MAX_ENTERED_CHARACTERS] = {0};
	
	if (num_vars <= 0)
		return 0;

	num_vars = min(num_vars, NUM_LF_VARS);
	
	cout << "\nEnter a logic function of the input variables A - " << (char)('A' + num_vars-1) << ": F = ";
	input_equation(function2);
	
	return parse_logic_function(function, function2, num_vars);
}




int parse_logic_function(char *function, char *function2, int num_vars) {

	int level = 0, saw_var, last_was_var = 0;
	unsigned int i, j;
		
	for (i = 0, j = 0; (int)i < MAX_ENTERED_CHARACTERS && function2[i] != 0; ++i) {
		saw_var = 0;
	
		switch (function2[i]) {
		
			case '(':
				++level;
				function[j++] = '(';
				break;
				
			case ')':
				--level;
				function[j++] = ')';
				break;
					
			case '\'':
				if (j == 0) {
					cout << "\nError: Expression starting with '" << endl;
					return 0;
				}
				function[j] = function[j-1];
				function[j-1] = '-';
				++j;
				saw_var = 1;
				break;
			
			case '-':
			case '!':
			case '~':
				if (last_was_var)
					function[j++] = '*';
				
				function[j++] = '-';
				break;
				
			case '+':
			case '|':
				function[j++] = '+';
				break;
				
			case '*':
			case '&':
				function[j++] = '*';
				break;
				
			case '^':
				function[j++] = '^';
				break;
				
			case '=':
				function[j++] = '=';
				break;
				
			case ' ':
			case '@':
				break;
				
			default:
				if (function2[i] >= 'a' && function2[i] <= ('a' + num_vars-1))
					function2[i] += ('A' - 'a');
				else if (function2[i] < 'A' || function2[i] > ('A' + num_vars-1)) {
					cerr << "\nError: Undefined symbol: " << function2[i] << endl;
					return 0;
				}
				if (last_was_var)
					function[j++] = '*';
				
				function[j++] = function2[i];
				saw_var = 1;
		}
		function[j] = 0;
				
		if (level < 0) {
			cerr << "\nError: Too many closing parentheses." << endl;
			return 0;
		}
		if (saw_var)
			last_was_var = 1;
		else
			last_was_var = 0;
	}
	if (level > 0) {
		cerr << "\nError: Missing " << level << " closing parentheses." << endl;
		return 0;
	}	
	return 1;
}




component *add_logic_function(component *head, StringTable &net_names, comp_nets *& cnets, int &curr_netnum, int &curr_pos, int &num_components) {
	
	unsigned int i;
	//char name[MAX_ENTERED_CHARACTERS] = {0};
	int nets[5] = {-1, -1, -1, -1, -1}, ID, inputs; // CHANGE?
	connections netnums;
			
	if (global_function[0] != 0 && global_function[1] != 0) {
		if (global_function[1] != '=') {
			cerr << "\nError in expression: expecting '=' but got " << global_function[1] << "." << endl;
			return head;
		}
		nets[0] = get_netnum_from_char(net_names, global_function[0], curr_netnum);
	}
	else {
		cerr << "\nError: expression too short." << endl;
		return head;	
	}	
	// use global_function in loop
	
	for (i = 2; (int)i < MAX_ENTERED_CHARACTERS && global_function[i] != 0; ++i) {
		if (global_function[i] >= 'A' && global_function[i] <= 'Z') {
			nets[1] = get_netnum_from_char(net_names, global_function[i], curr_netnum);
		}
		else {
			ID     = get_id_from_char(global_function[i]);
			inputs = 2;
			
			switch (global_function[i]) {
				case '-':
					inputs = 1;
				case '+':
				case '*':
				case '^':
					if (global_function[i+1] == 0) return head;
					nets[inputs] = get_netnum_from_char(net_names, global_function[i+1], curr_netnum);
					head         = add_gate(head, netnums, nets, ID, inputs, curr_netnum, curr_pos, num_components);
					add_new_nets(cnets, netnums);
					nets[1]      = nets[0];
					++i;
					// do something with nets
					
					break;
					
				case '(':
					//recursive call
					//head = add_logic_function(head, net_names, cnets, curr_netnum, curr_pos, num_components);
					break;
					
				case ')':
				case '\0':
					return head;
					
				default:
					cerr << "\nIllegal token in expression: " << global_function[i] << "." << endl;
			}
		}
	}		
	return head;
}




component *add_gate(component *head, connections &netnums, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components) {

	head = update_circuit(ID, calc_next_pos(curr_pos, ID), head, 2, 0);

	set_all_connections(netnums, -1);
		
	switch (num_inputs) {
	case 4:
		if (nets[4] == -1) nets[4] = curr_netnum;
		netnums.out1 = nets[4];
	case 3:
		if (nets[3] == -1) nets[3] = curr_netnum;
		netnums.in2 = nets[3];
	case 2:
		if (nets[2] == -1) nets[2] = curr_netnum;
		netnums.in3 = nets[2];
		if (nets[1] == -1) nets[1] = curr_netnum;
		netnums.in1 = nets[1];
		break;

	case 1:
		if (nets[1] == -1) nets[1] = curr_netnum;
		netnums.in2 = nets[1];
	}
	if (nets[0] == -1) nets[0] = curr_netnum;
	netnums.out2 = nets[0];
		
	++num_components;
	
	return head;
}




component *add_gate_and_netnums(component *head, comp_nets *& cnets, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits, int decomp_xor) {

	connections netnums;

	if (num_inputs == 2 && decomp_xor) {
		return decompose_xor(head, cnets, nets, ID, curr_netnum, curr_pos, num_components, num_splits);
	}
	head = add_gate(head, netnums, nets, ID, num_inputs, curr_netnum, curr_pos, num_components);	
	add_new_nets(cnets, netnums);

	return head;
}




int get_netnum_from_char(StringTable &net_names, char c, int &curr_netnum) {

	char name[2];
	string netname;

	name[0] = c;
	name[1] = 0;
	netname = (string)name;
	return get_netnum_from_string(net_names, netname, curr_netnum);
}




int get_id_from_char(char c) {

	switch (c) {
		case '-':
			return NOT_ID;
			break;
		case '*':
			return AND_ID;
			break;
		case '+':
			return OR_ID;
			break;
		case '^':
			return XOR_ID;
			break;
	}
	return NO_ID;
}



// nets should contain |num_inputs| input nets plus one output net = size num_inputs+1
component *split_large_gate(component *head, comp_nets *& cnets, int *nets, int ID, int num_inputs, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits, int fanin_limit, int decomp_xor) {

	unsigned i, j, nsplits, nshort, inputs, nin, nnum;
	int ID2(ID), *nets2 = NULL, *nets3 = nets, nets4[4];

	if (fanin_limit < 2 || fanin_limit > 4) {
		cerr << "Error: Fanin limit must between 2 and 4 (inclusive): " << fanin_limit << "." << endl;
		return head;
	}
	if (decomp_xor) {
		if (ID != XOR_ID && ID != XNOR_ID) {
			cout << "ID = " << ID << endl;
			decomp_xor = 0; // gate is not XOR/XNOR
		}
		else if (fanin_limit > 2) {
			fanin_limit = 2; // reduce to 2-input XOR gates (with inverted output for XNOR)
		}
	}
	if ((ID - AND_ID)%2 == 1) { // inverted gate
		--ID; // only output gate is inverted, internal tree is non-inverting
	}

	if (ID >= AND_ID || ID <= XNOR_ID) {
		switch (num_inputs) {
			case 0:
				cerr << "Error: Illegal logic gate: " << get_name(ID, 0) << " with no inputs." << endl;
				return head;

			case 1:
				cerr << "Error: Illegal logic gate: " << get_name(ID, 0) << " with only one input." << endl;
				return head;

			case 2:
				return add_gate_and_netnums(head, cnets, nets, ID2, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor); // 2 input - standard 2-in gate OK

			case 3:
				if (fanin_limit == 2) { // split 3-in into two 2-ins
					head = gen_2in_gate(head, cnets, nets3, nets4, ID, curr_pos, curr_netnum, num_components, num_splits, decomp_xor);
					nets4[0] = nets[0];
					nets4[1] = nets[3];
					nets4[2] = curr_netnum;
					head = add_gate_and_netnums(head, cnets, nets4, ID2, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);	
					++curr_netnum;
					return head;
				}
				return add_gate_and_netnums(head, cnets, nets, (ID2+T_GATE_OFFSET), 3, curr_netnum, curr_pos, num_components, num_splits, decomp_xor); // make 3-in gate

			case 4:
				if (fanin_limit < 4) { // split 4-in into a 3-in and a 2-in
					head = gen_2in_gate(head, cnets, nets3, nets4, ID, curr_pos, curr_netnum, num_components, num_splits, decomp_xor);
					nets4[1] = nets[3];
					nets4[2] = nets[4];

					if (fanin_limit == 3) {
						nets4[0] = nets[0];
						nets4[3] = curr_netnum;
						head = add_gate_and_netnums(head, cnets, nets4, (ID2+T_GATE_OFFSET), 3, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);
					}
					else {
						++curr_netnum;
						nets4[0] = curr_netnum;
						head = add_gate_and_netnums(head, cnets, nets4, ID, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);	
						nets4[0] = nets[0];
						nets4[1] = curr_netnum-1;
						nets4[2] = curr_netnum;
						head = add_gate_and_netnums(head, cnets, nets4, ID2, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);
						++num_splits;
					}						
					++curr_netnum;
					return head;
				}
				return add_gate_and_netnums(head, cnets, nets, (ID2+Q_GATE_OFFSET), 4, curr_netnum, curr_pos, num_components, num_splits, decomp_xor); // make 4-in gate	

			default: // split into smaller gates
				if (num_inputs < 0) {
					cerr << "Error: num_inputs = " << num_inputs << " is negative." << endl;
					return head;
				}
				if (num_inputs > 16) {
					nsplits = 4;
				}
				else {
					nsplits = (unsigned)ceil(((double)num_inputs)/4.0); // fanin of next stage gate
				}
				nshort = (4 - (num_inputs%4))%4; // number of inputs short of full 4-in gate tree
				inputs = (unsigned)ceil(((double)num_inputs)/((double)nsplits)); // max inputs per branch
				inputs = max(inputs, (unsigned)4);
				nets2  = memAlloc(nets2, inputs+1);  // intermediate nets
				nets3  = memAlloc(nets3, nsplits+1); // next stage nets
				nnum   = 0; // input net counter
			
				for (i = 0; i < nsplits; ++i) {				
					if (nshort > 0) {
						if (nshort > nsplits) { // 2-in gate
							nshort -= 2;
							nin = inputs - 2;
							nets2[inputs] = nets2[inputs-1] = -1;
						}
						else { // 3-in gate
							--nshort;
							nin = inputs - 1;
							nets2[inputs] = -1;
						}
					}
					else { // 4-in gate
						nin = inputs;
					}
					for (j = 0; j < nin; ++j) {
						nets2[j+1] = nets[++nnum]; // copy net numbers from all inputs to each branch
					}
					nets2[0]   = curr_netnum; // output of branching gate
					nets3[i+1] = curr_netnum; // connected to input to final gate
					++curr_netnum; // advance to next available net ID
					++num_splits;
					head = split_large_gate(head, cnets, nets2, ID, nin, curr_netnum, curr_pos, num_components, num_splits, fanin_limit, decomp_xor);
				}
				nets3[0] = nets[0]; // connect output of large gate to output of final gate
				head = split_large_gate(head, cnets, nets3, ID2, nsplits, curr_netnum, curr_pos, num_components, num_splits, fanin_limit, decomp_xor);
				delete [] nets2;
				delete [] nets3;
				return head;
		}
	}
	return head;
}




component *gen_2in_gate(component *head, comp_nets *& cnets, int *nets3, int *nets4, int ID, int &curr_pos, int &curr_netnum, int &num_components, int &num_splits, int decomp_xor) {

	nets4[0] = curr_netnum;
	nets4[1] = nets3[1];
	nets4[2] = nets3[2];
	++num_splits;

	return add_gate_and_netnums(head, cnets, nets4, ID, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);
}




component *decompose_xor(component *head, comp_nets *& cnets, int *nets, int ID, int &curr_netnum, int &curr_pos, int &num_components, int &num_splits) {

	int nets2[3], net_a, net_b, a_not, b_not, net_c, net_d;

	if (ID != XOR_ID && ID != XNOR_ID) {
		return head;
	}
	++num_splits; // counts as one split

	net_a = nets[1];
	a_not = curr_netnum++;
	nets2[0] = a_not;
	nets2[1] = net_a;
	head = add_gate_and_netnums(head, cnets, nets2, NOT_ID, 1, curr_netnum, curr_pos, num_components, num_splits, 0);

	net_b = nets[2];
	b_not = curr_netnum++;
	nets2[0] = b_not;
	nets2[1] = net_b;
	head = add_gate_and_netnums(head, cnets, nets2, NOT_ID, 1, curr_netnum, curr_pos, num_components, num_splits, 0);

	if (ID == XNOR_ID) {
		swap(net_b, b_not);
	}
	net_c = curr_netnum++;
	nets2[0] = net_c;
	nets2[1] = net_a;
	nets2[2] = b_not;
	head = add_gate_and_netnums(head, cnets, nets2, AND_ID, 2, curr_netnum, curr_pos, num_components, num_splits, 0);

	net_d = curr_netnum++;
	nets2[0] = net_d;
	nets2[1] = a_not;
	nets2[2] = net_b;
	head = add_gate_and_netnums(head, cnets, nets2, AND_ID, 2, curr_netnum, curr_pos, num_components, num_splits, 0);

	nets2[0] = nets[0]; // output
	nets2[1] = net_c;
	nets2[2] = net_d;
	return add_gate_and_netnums(head, cnets, nets2, OR_ID, 2, curr_netnum, curr_pos, num_components, num_splits, 0);
}



 // out1 and out2 are of size num_outputs
component *xor_and_outputs(component *head, comp_nets *& cnets, int *out1, int *out2, int &curr_netnum, int &curr_pos, int &num_components, int &output_net, int num_outputs, int fanin_limit, int decomp_xor, int add_outputs) {

	unsigned i, ctr;
	int nets[3], *nets2 = NULL, num_splits(0);

	if (out1 == NULL || out2 == NULL || num_outputs < 1) {
		output_net = -1;
		return head;
	}
	nets2 = memAlloc(nets2, num_outputs+1);

	for (i = 0, ctr = 0; (int)i < num_outputs; ++i) {
		if (out1[i] != out2[i]) {
			if (add_outputs == 2) {
				head = add_io_to_circuit(head, cnets, curr_netnum, OUTPUT_ID, curr_pos, num_components);
			}
			nets2[++ctr] = curr_netnum;
			nets[0]      = curr_netnum++;
			nets[1]      = out1[i];
			nets[2]      = out2[i];
			head         = add_gate_and_netnums(head, cnets, nets, XOR_ID, 2, curr_netnum, curr_pos, num_components, num_splits, decomp_xor);
		}
	}
	if (ctr == 0) {
		output_net = -1; // should return VDD net, but it might not be created yet or known to the parent function
	}
	else {
		if (ctr > 1) {
			if (add_outputs) {
				head = add_io_to_circuit(head, cnets, curr_netnum, OUTPUT_ID, curr_pos, num_components);
			}
			output_net = curr_netnum;
			nets2[0]   = curr_netnum++;
			head       = split_large_gate(head, cnets, nets2, AND_ID, (int)ctr, curr_netnum, curr_pos, num_components, num_splits, fanin_limit, decomp_xor);
		}
		else {
			output_net = curr_netnum-1;
		}
	}
	delete [] nets2;

	return head;
}




component *add_io_to_circuit(component *head, comp_nets *& cnets, int net, int id, int &curr_pos, int &num_components) {

	connections netnums;

	set_all_connections(netnums, -1);

	if (id == INPUT_ID) {
		netnums.out2 = net;
	}
	else if (id == OUTPUT_ID) {
		netnums.in2  = net;
	}
	else {
		return head;
	}
	head = update_circuit(id, calc_next_pos(curr_pos, id), head, 2, 0);
	add_new_nets(cnets, netnums);
	++num_components;

	return head;
}














