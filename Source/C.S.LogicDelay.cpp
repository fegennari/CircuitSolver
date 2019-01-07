#include "CircuitSolver.h"
#include "Logic.h"
#include "Delay.h"
#include "CompArray.h"
#include "Equation.h"
#include "GridArray.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LogicDelay.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Gate Delays for Logic Engine
// By Frank Gennari
double default_delay(0);


extern int changed3, GV, needs_redraw, x_screen, y_screen;
extern const char *hasValue;
extern GridArray NODENUMBER;


void set_gate_delays(component *head, comp_sim *comps, int nodes, int num_comps, int mode);
int  delay_needs_set1(component *head);
int  delay_needs_set2(comp_sim *comps, int num_comps);
void delay_from_equation(cs_equation &eq, double *gate_base_delay);
void get_gate_delay_params(double *gate_base_delay);
void count_fanouts(comp_sim *comps, unsigned short *foctr, int nodes, int num_comps);
void gen_delays(cs_equation &eq, comp_sim *comps, double *gate_base_delay, unsigned short *foctr, int num_comps, int change_all, int fo_used);
void back_annotate_delays(component *head, comp_sim *comps, int num_comps);




void set_gate_delays(component *head, comp_sim *comps, int nodes, int num_comps, int mode) {
	
	int change_all(1), counter(0), fo_used;
	unsigned short *foctr = NULL;

	static double gate_base_delay[MAX_NEW_USED_ID] = {0};
	static cs_equation eq(MAX_ENTERED_CHARACTERS);

	if (!GV) {
		return;
	}
	if (mode == 0) {
		if (!delay_needs_set1(head))
			return;
	}
	else {
		if (!delay_needs_set2(comps, num_comps))
			return;
	}
	if (default_delay == 0) {
		cout << "At least one gate in the circuit has no delay value and must be changed." << endl;
		if (mode != 0) {
			cout << "Do you want to change the delay values of all gates at once?  ";
			change_all   = decision();
			needs_redraw = 1;
		}
		delay_from_equation(eq, gate_base_delay);
		
		if (mode == 0 && !changed3) {
			cout << "Circuit must now be recalculated." << endl;
		}
		changed3 = 1;
	}
	if (mode == 0 || comps == NULL || nodes <= 0 || num_comps <= 0) {
		return;
	}
	fo_used = eq.is_var_used("fo");

	if (fo_used) {
		foctr = memAlloc(foctr, nodes);
		count_fanouts(comps, foctr, nodes, num_comps);
	}
	gen_delays(eq, comps, gate_base_delay, foctr, num_comps, change_all, fo_used);

	if (fo_used) {
		delete [] foctr;
	}
	back_annotate_delays(head, comps, num_comps);
}




int delay_needs_set1(component *head) {

	while (head != NULL) {
		if (head->id < MAX_NEW_USED_ID && hasValue[head->id] == 2 && head->value == 0) {
			return 1;
		}
		head = head->next;
	}
	return 0;
}




int delay_needs_set2(comp_sim *comps, int num_comps) {

	for (unsigned i = 0; (int)i < num_comps; ++i) {
		if (comps[i].id < MAX_NEW_USED_ID && hasValue[comps[i].id] == 2 && comps[i].value == 0) {
			return 1;
		}
	}
	return 0;
}




void delay_from_equation(cs_equation &eq, double *gate_base_delay) {
	
	int  status, new_eq(0);
	char *var_names[3];
	char name1[5] = "orig", name2[3] = "fi", name3[3] = "fo";  
	
	var_names[0]  = name1;
	var_names[1]  = name2;
	var_names[2]  = name3;
	default_delay = 0;
			 
	if (!eq.add_vars(var_names, NULL, 3, 0)) {
		cout << "An equation had been previously entered. Do you want to enter a new one?  ";
		if(decision()) {
			new_eq = 1;
		}
		else {
			cout << "Using previously entered equation." << endl;
			return;
		}
	}
	status = eq.status();
	if (status != EQ_OK && status != EQ_UNINIT && !(new_eq && status == EQ_NAME_ERROR)) {
		internal_error();
		cerr << "Equation variable error code " << status << "." << endl;
		return;
	}
	cout << "\n'x'    = the constant specific to gate type"
		 << "\n'orig' = the original delay value of the gate"
		 << "\n'fi'   = the number of fanins/inputs to the gate"
		 << "\n'fo'   = the number of fanouts from the gate"
		 << "\nEnter the delay equation of a logic gate in terms of the above parameters: " << endl;
	
	eq.input("Error in delay expression, reenter: ");
	
	if (eq.var_with_x()) {
		get_gate_delay_params(gate_base_delay);
	}
}




void get_gate_delay_params(double *gate_base_delay) {

	unsigned int i;
	int draw_id, selection;
	double base_delay;
			
	for (i = 0; (int)i < MAX_NEW_USED_ID; ++i) {
		gate_base_delay[i] = default_delay;
	}
	do {
		init_menu(NUM_GD_TYPES+2, 10, 0);	
		draw_text(" DONE ", reserved_pos_value, 12);
		draw_text(" APPLY TO ALL ", -1, 12);
	
		for (i = 0; (int)i < NUM_GD_TYPES; ++i) {
			draw_id = gate_delay_types[i];
			base_delay = gate_base_delay[draw_id];
			draw_double(base_delay, 'X', -(int)(i+2));
			if (draw_id == FF_ID) {
				draw_FF(i+2, -100);
				draw_CK(i+2);
				draw_CK0(i+2);
			}
			else {
				draw_comp[draw_id](i+2);
			}
		}
		selection = box_clicked();
		if (selection == 1) {
			cout << "Enter the base delay of all logic components: ";
			base_delay = ind(); // use equation
			for (i = 0; (int)i < NUM_GD_TYPES; ++i) {
				gate_base_delay[gate_delay_types[i]] = base_delay;
			}
		}
		else if (selection > 1 && selection < NUM_GD_TYPES+2) {
			cout << "Enter the delay of a(n) " << get_name(gate_delay_types[selection-2], 0) << ": ";
			base_delay = ind(); // use equation
			gate_base_delay[gate_delay_types[selection-2]] = base_delay;
		}
	} while (selection != 0);
	
	x_screen = y_screen = 0; // *** might screw up zoom ***
	init_buttons(1);

	/*if (head != NULL) {
		draw_circuit(head, "", 0, "", 0); // kind of strange without wires
	}*/
}




void count_fanouts(comp_sim *comps, unsigned short *foctr, int nodes, int num_comps) {
	
	unsigned int i;
	int *conn, ID;
	
	if (foctr == NULL || comps == NULL || num_comps <= 0 || nodes <= 0) {
		return;
	}
	memset(foctr, 0, nodes*sizeof(unsigned short));

	for(i = 0; (int)i < num_comps; ++i) {
		ID   = comps[i].id;
		conn = comps[i].conn;

		if (ID <= OPAMP_ID || (ID >= T_ST_BUF_ID && ID <= ADDER_ID) || (ID >= QAND_ID && ID <= DELAY_ID) || get_order(ID) == flipflopx) {		
			if (conn[0] >= 0)
				++foctr[conn[0]];		
			if (conn[1] >= 0)
				++foctr[conn[1]];	
			if (conn[2] >= 0)
				++foctr[conn[2]];
			if ((ID >= QAND_ID && ID <= QXNOR_ID) && conn[3] > 0)
				++foctr[conn[3]];
		}
	}
}


void gen_delays(cs_equation &eq, comp_sim *comps, double *gate_base_delay, unsigned short *foctr, int num_comps, int change_all, int fo_used) {

	unsigned i, fanouts = 0;
	int *conn, ID;
	double params[3];

	for (i = 0; (int)i < num_comps; ++i) {
		ID = comps[i].id;

		if (ID < MAX_NEW_USED_ID && hasValue[ID] == 2 && (change_all || comps[i].value == 0)) {
			if (fo_used) {
				conn = comps[i].conn;

				if (get_order(ID) == flipflopx)
					fanouts = (foctr[conn[3]] + foctr[conn[5]]);
				else if (comps[i].id == ADDER_ID) // sum fanouts for FF and adder
					fanouts = (foctr[conn[4]] + foctr[conn[5]]);
				else
					fanouts = foctr[conn[4]]; // standard logic gates
			}
			params[0] = comps[i].value;
			params[1] = comp_fanin[ID];
			params[2] = fanouts;
			
			eq.set_vars(params);
			comps[i].value = eq.eval(gate_base_delay[gate_delay_mapping[ID]]);
		}
	}
}


void back_annotate_delays(component *head, comp_sim *comps, int num_comps) {

	for (int i = 0; i < num_comps; ++i) {
		if (head == NULL) {
			return;
		}
		head->value = comps[i].value;
		head = head->next;
	}
}




