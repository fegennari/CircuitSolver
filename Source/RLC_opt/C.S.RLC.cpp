#include "CircuitSolver.h"
#include "Logic.h"
#include "GridArray.h"

#include "RLC_Tree.h"
#include "Oracle.h"
#include "Optimizer.h"


// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.RLC.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver RLC circuit analysis
// EE219A final project
// Began 10/23/02
// by Frank Gennari
int const TEST_NODES        = 0;
int const SHOW_BRANCHES     = 0;
int const ALLOW_CLICK_NODES = 0;
int const SHOW_RESULTS_DATA = 0;


extern int useCOMPLEX, useDIGITAL, no_ref_needed, needs_redraw;
extern double supply_voltage;
extern GridArray NODENUMBER;


int rlc_eval(component *&head);
int circuit_is_rlc(component *head, int allow_l);
void build_graph(component *head, RLC_tree &RLCT);
void show_RLC_results(RLC_tree &RLCT);
int get_tech_data(RLC_tree &RLCT);
int get_tech_data_from_file(tech_data &tdata);
void get_tech_data_from_user(tech_data &tdata);

// external functions
component *simulation_setup(component *head, wire *&wires, int &num_wires);
component *init_data_structures(component *head, wire *&wiresave, component *&unsim, wire *wires, int num_wires, int &nodes, int &error, int is_digital, int &comp_count);
component *reset_circuit(component *head, component *unsim, wire *wiresave);

void click_and_show_nets();

int get_scrolled_window_click();



int rlc_eval(component *&head) {

	int num_wires(0), nodes, comp_count, init_error, circuit_type, critical_leaf(-1), delay_model, iopt;
	double max_delay, area, alpha, delay_constraint, area_constraint;
	component *unsim = NULL;
	wire *wiresave, *hier_wires;
	RLC_tree RLCT;
	oracle Oracle(&RLCT);
	optimizer opt(&Oracle);

	delay_model = TYPE_RC_DELAY;
	//delay_model = TYPE_TPDI;
	//delay_model = TYPE_TRI;

	alpha            = 0.0;
	delay_constraint = 0.0;
	area_constraint  = 0.0;

	Oracle.set_delay_model(delay_model); // use better RLC delay?
	Oracle.set_objective(alpha); // objective = D*A^alpha

	if (!Oracle.set_constraint(TYPE_DELAY, delay_constraint)) {
		return 0;
	}
	if (!Oracle.set_constraint(TYPE_AREA, area_constraint)) {
		return 0;
	}
	// *** use net names and SPICE netlist import/export ***

	useDIGITAL   = 0;
	useCOMPLEX   = 1;
	head         = simulation_setup(head, hier_wires, num_wires);
	reset_timer(1);
	circuit_type = circuit_is_rlc(head, 1);

	if (circuit_type == 0) {
		cout << "This is not a pure RLC circuit. Ignoring non-RLC components." << endl;
	}
	if (circuit_type == 1) {
		cout << "No variables to optimize." << endl;
	}
	//no_ref_needed = 1; // looks like we need a ground node 0 for now
	head = init_data_structures(head, wiresave, unsim, hier_wires, num_wires, nodes, init_error, 2, comp_count);
	//no_ref_needed = 0;

	if (init_error) {
		cerr << "Error initializing circuit data structures." << endl;
		return 0;
	}
	if (nodes == 0) {
		cerr << "Error: Circuit has no nodes." << endl;
		head = reset_circuit(head, unsim, wiresave);
		return 0;
	}
	RLCT.alloc_nodes(nodes);
	stop_timer();

	if (circuit_type != 1 && !get_tech_data(RLCT)) { // tech file
		cerr << "Error reading technology data." << endl;
		head = reset_circuit(head, unsim, wiresave);
		return 0;
	}
	start_timer();
	build_graph(head, RLCT);

	if (RLCT.is_tree()) {
		cout << "This circuit is a tree." << endl;	
	}
	else {
		cout << "This circuit is not a tree (2)." << endl;
	}
	cout << "Num_params = " << RLCT.get_num_params() << "." << endl;
	RLCT.build_rc_lc_terms(1);

	if (!RLCT.is_sizing_monotonic()) {
		cout << "Warning: Sizing is non-monotonic and probably suboptimal." << endl;
	}
	if (TEST_NODES) {
		stop_timer();
		RLCT.print(SHOW_BRANCHES);
		click_and_show_nets();
		start_timer();
	}
	max_delay = RLCT.get_max_delay(critical_leaf, delay_model);
	area      = RLCT.get_area(MOS_PLUS_WIRE_AREA);
	cout << "Max delay of " << max_delay << " at node " << critical_leaf << " with area " << area << "." << endl;

	show_RLC_results(RLCT);

	if (circuit_type != 1) {
		if (alpha == 0.0 && area_constraint == 0.0) {
			iopt = RLCT.optimize_independent_params(TYPE_DELAY);
		}
		else {
			iopt = RLCT.optimize_independent_params(TYPE_AREA_AND_DELAY);
		}
		cout << "Init time:";
		show_elapsed_time();
		cout << endl;

		if (iopt == 2) {
			cout << "There are no non-trivial parameters left to optimize." << endl;
			cout << "Do you want to write the results back to the circuit?  ";

			if (decision()) {
				RLCT.write_params_to_circuit();
				needs_redraw = 1;
			}
		}
		else {
			stop_timer();
			cout << "Do you want to optimize this circuit?  ";

			if (decision()) {
				start_timer();

				if (opt.run(1)) {
					if (SHOW_RESULTS_DATA) {
						opt.print_data(2);
					}
					RLCT.print_leaf_delays();

					if (opt.check_optimal_results(0.0)) {
						cout << "Results seem to be optimal." << endl;
					}
					else {
						cout << "Results are suboptimal!" << endl;
					}
					show_RLC_results(RLCT);
					stop_timer();
					cout << "Do you want to write the results back to the circuit?  ";

					if (decision()) {
						RLCT.write_params_to_circuit();
						needs_redraw = 1;
					}
					start_timer();
				}
				cout << "Optimization time:";
				show_elapsed_time();
				cout << endl;
			}
		}
	}
	head = reset_circuit(head, unsim, wiresave);
	useCOMPLEX = 0;

	return 1;
}



int circuit_is_rlc(component *head, int allow_l) {

	int ID, has_mos_tl = 0;

	while (head != NULL) {
		ID = head->id;

		if (ID == NFET_ID || ID == PFET_ID || ID == TLINE_ID) {
			has_mos_tl = 1;
		}
		else if (ID != RES_ID && ID != CAP_ID && (!allow_l || ID != INDUCTOR_ID) && ID != NOT_ID && ID != POWER_ID && ID != GROUND_ID && ID != V_SOURCE_ID && ID != INPUT_ID /*&& ID != BATTERY_ID*/ && get_order(ID) != wirex) {
			cout << "Found non-RLC component: " << get_name(ID, 0) << "." << endl;
			return 0;
		}
		head = head->next;
	}
	return 1 + has_mos_tl;
}


void build_graph(component *head, RLC_tree &RLCT) {

	int ID, index;
	double length;
	connections x;

	while (head != NULL) {
		ID = head->id;

		switch (ID) {
		case V_SOURCE_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_vsource(NODENUMBER[x.in2], NODENUMBER[x.out2], head->value);
			break;
		
		case POWER_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_vsource(NODENUMBER[x.out2], 0, supply_voltage);
			break;

		case INPUT_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_vsource(NODENUMBER[x.out2], 0, supply_voltage);
			break;
		
		case RES_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_resistor(NODENUMBER[x.in2], NODENUMBER[x.out2], TYPE_RESISTOR, index, head->value);
			break;
		
		case INDUCTOR_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_inductor(NODENUMBER[x.in2], NODENUMBER[x.out2], head->value);
			break;
		
		case CAP_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_capacitor(NODENUMBER[x.in2], NODENUMBER[x.out2], head->value, 1);
			break;

		case NFET_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_mosfet(NODENUMBER[x.in2], NODENUMBER[x.out3], NODENUMBER[x.out1], 0, 0, &(head->value));
			break;

		case PFET_ID:
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_mosfet(NODENUMBER[x.in2], NODENUMBER[x.out1], NODENUMBER[x.out3], 0, 1, &(head->value));
			break;

		case TLINE_ID:
			// *** need to deal with length ***
			length = 1.0;
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_tline(NODENUMBER[x.in2], NODENUMBER[x.out2], length, &(head->value));
			break;

		case NOT_ID: // What about size? Use delay?
			x = get_connections(head->location, ID, head->flags);
			RLCT.add_inverter(NODENUMBER[x.in2], NODENUMBER[x.out2], &(head->value));
			break;

		case GROUND_ID: // ignore
			break;

		default:
			cout << "Unsupported component: " << get_name(ID, 0) << endl;
		}
		head = head->next;
	}
}



void show_RLC_results(RLC_tree &RLCT) {
	
	int position;

	if (!ALLOW_CLICK_NODES) {
		return;
	}
	stop_timer();
	reset_cursor();
	set_special_cursor(PROBEDC_CURSOR);

	cout << "Click on a node for info." << endl;
	cout << "Click on Cancel or another button to exit probe mode and continue." << endl;
	
	do {
		position = get_scrolled_window_click();

		if (position > num_menu_boxes) {
			RLCT.print_node_info(NODENUMBER[position]);
			cout << endl;
		}
	} while (position > num_menu_boxes);

	start_timer();
}



int get_tech_data(RLC_tree &RLCT) {

	tech_data tdata;

	cout << "Would you like to use the default technology data values (all 1.0)?  ";
	if (decision()) { // yes
		return RLCT.get_tech_data().is_valid();
	}
	if (!get_tech_data_from_file(tdata)) {
		get_tech_data_from_user(tdata);
	}
	return RLCT.set_tech_data(tdata);
}



int get_tech_data_from_file(tech_data &tdata) {

	ifstream infile;
	string str; // dummy variable
	char filename[MAX_SAVE_CHARACTERS + 1] = {0};

	while (1) {
		cout << "Would you like to use a saved technology data file?  ";
		if (!decision()) { // no
			return 0;
		}
		cout << "\nOpen tech data file named:  ";
		cin  >> ws;
		strcpy(filename, xin(MAX_SAVE_CHARACTERS));

		if (!infile_file(infile, filename)) {
			beep();
			cerr << "\nThe tech data file " << filename << " could not be opened!" << endl;
			infile.clear();
			continue;
		}
		if (!filestream_check(infile)) {
			infile.close();
			infile.clear();
			cerr << "Error reading tech data file " << filename << "." << endl;
			continue;
		}
		if (!(infile >> str >> tdata.tline_seg >> str >> tdata.is_distributed >> str >> tdata.mos_rdsn >> str >> tdata.mos_rdsp >> str >> tdata.mos_cg   >> str >> tdata.mos_cd >> str >> tdata.mos_cs
				     >> str >> tdata.tline_r0  >> str >> tdata.tline_l0       >> str >> tdata.tline_c0
					 >> str >> tdata.mos_min   >> str >> tdata.mos_max        >> str >> tdata.wire_min >> str >> tdata.wire_max >> str >> tdata.mw_area_r)) {
			cerr << "Error reading numbers from tech data file " << filename << "." << endl;
			infile.close();
			infile.clear();
			continue;
		}
		infile.close();

		return 1;
	};
	return 0;
}



void get_tech_data_from_user(tech_data &tdata) {

	cout << "All tech data parameters must be greater than zero." << endl;
	cout << "Enter number of segments per transmission line:  ";
	tdata.tline_seg = inpg0();
	cout << "Use a distributed transmission line model?  ";
	tdata.is_distributed = decision();
	cout << "Enter parameter mos_rdsn:  ";
	tdata.mos_rdsn  = ind();
	cout << "Enter parameter mos_rdsp:  ";
	tdata.mos_rdsp  = ind();
	cout << "Enter parameter mos_cg:  ";
	tdata.mos_cg    = ind();
	cout << "Enter parameter mos_cd:  ";
	tdata.mos_cd    = ind();
	cout << "Enter parameter mos_cs:  ";
	tdata.mos_cs    = ind();
	cout << "Enter parameter tline_r0:  ";
	tdata.tline_r0  = ind();
	cout << "Enter parameter tline_l0:  ";
	tdata.tline_l0  = ind();
	cout << "Enter parameter tline_c0:  ";
	tdata.tline_c0  = ind();

	cout << "Enter min mos size:  ";
	tdata.mos_min   = ind();
	cout << "Enter max mos size:  ";
	tdata.mos_max   = ind();
	cout << "Enter min wire size:  ";
	tdata.wire_min  = ind();
	cout << "Enter max wire size:  ";
	tdata.wire_max  = ind();
	cout << "Enter the MOS area to wire area ratio:  ";
	tdata.mw_area_r = ind();
}







