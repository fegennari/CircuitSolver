#include "CircuitSolver.h"
#include "Logic.h"
#include "Complex.h"
#include "Matrix.h"
#include "GridArray.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.AnalogAnalysis.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Analog Analysis
// by Frank Gennari
#define END_FLAG -3

#define AC_COMP_ID(ID)     (ID == CAP_ID   || ID == INDUCTOR_ID || ID == XFMR_ID)
#define BASIC_COMP_ID(ID)  (ID == RES_ID   || AC_COMP_ID(ID))
#define CURR_IN_MATRIX(ID) (ID == POWER_ID || ID == V_SOURCE_ID || ID == BATTERY_ID || ID == VCVS_ID || ID == CCVS_ID || ID == CCCS_ID)

enum {CURR = 0, CURR_C, POW_C, PMAX_C}; // current, complex {current, average power, and max power}


int const showVALUES         = 0;
int const showNODEVALUES     = 0;
int const showNODENUMBERS    = 1;
int const always_recalculate = 1;

double const min_rel_error = 1e-4;

					
int findr(0), groundp, changed4(0), bad_error(0), useINV(0), useDIGITAL(0), useCOMPLEX, complex_form(0), needs_redraw(0), no_soultion, additional_size(0), graph_sincos(0), source_variable(0), DC_only(0), force_recreate(0), input_waveform(0), freq(-1);
double calculated_value(0), f(0), min_scale(1), f_phase = 0;
char function;


struct datasave {

	int useSAVE, size, v_pos;
	double last_value, *V, *J, **Y, **invY;
	complex *DWGRID, *Vc, *Jc, **Yc, **invYc;
	wire *wiresave;
	component *head;
};

datasave matrixdata;

extern GridArray NODENUMBER;

extern int NODES, useHASHTABLE, origin_set;

extern double supply_voltage, ground;
extern double F_storage, F_temp;
extern point *graph_data, origin;

extern double domain, range, x_scale, y_scale;
extern int x_limit, y_limit, changed, changed3, Opened, inconsistent, arbitrary, Count, num_points, num_buttons, useDIALOG, useLOG, using_entered_equation, has_menubar;
extern int compatibility;


component *analog_analysis_init(component *head, complex *DWGRID);
component *analog_analysis(component *head, double input_value, complex *DWGRID);
void fill_admittance_matrix(int id, double Value, int ID, int &size1, int &has_controlled_source, int node1, int node2, int node3, int node4, double **Y, double *J, complex **Yc, complex *Jc);
void fill_J_matrix(int v_pos, int ID, double Value, double *J, complex *Jc, int node1, int node2);
int get_nodes(int ID, int &node1, int &node2, int &node3, int &node4, int size, int size1, const connections& x);
void add_opamp_numbers(component *head, int nodes, double **Y, double *J, complex **Yc, complex *Jc, int size, int size1, int create_new_matrices);
void solve_simulation(component *head, double **&invY, double **Y, double *J, double *&V, complex **&invYc, complex **Yc, complex *Jc, complex *&Vc, complex *Ic, complex *DWGRID, int size, int nodes, int create_new_matrices, int interactive);
component *show_AC_results(component *head, complex *DWGRID, complex *Ic, int Ic_size, int complex_form, char function);
void show_DC_results(component *head, complex *DWGRID, complex *Ic, int Ic_size);
int calc_current(component *comp, complex *arr, complex *Ic, connections &x, double &val, double &current, int is_complex, unsigned cm_count);
complex calc_param(complex *arr, complex *Ic, connections &x, int ID, double val, int type, unsigned cm_count);
component *calc_DC_offset(component *head, complex *DWGRID);
void create_matrices(double **&Y, double *&V, double *&J, complex **&Yc, complex *&Vc, complex *&Jc, int size);
void reset_matrixdata();
void cleanup_simulation(double **Y, double *V, double *J, complex **Yc, complex *Vc, complex *Jc, complex *Ic, complex **invYc, double **invY, int size);
void delete_matrices(double **Y, double *V, double *J, complex **Yc, complex *Vc, complex *Jc, int size);
void get_additional_size(component *head);
double get_minR(int component_type);
double get_maxR(int component_type);
void find_component(component *head, int location, int id, int bode, complex *DWGRID);
double analyze_circuit(component *&head, double input, double R_value, int position, int find_var, double &comp_value, int position1, int position2, int ID, int mag_or_phase, complex *DWGRID);
double solve_for_value(double R_value, int position, int find_var, double comp_value, int position1, int position2, int ID, int mag_or_phase, complex *DWGRID, double ref, double offset, int bode);
double find_value(double R_value, int position, int find_var, double comp_value, int position1, int position2, complex *DWGRID);
complex find_valuec(double R_value, int position, int find_var, double comp_value, int position1, int position2, int ID, complex *DWGRID);
void show_ranges(double R_value, double known1, double known2);
void display_iterative_results(int ID, double comp_value);
void plot_circuit_variable(component *head, double Value, int position, int find_var, int position1, int position2, int id, int bode, complex *DWGRID);
void plot_circuit_waveform(component *head, int mag_or_phase, double end, int npoints, double Value, int position, int find_var, double input, int position1, int position2, int id, complex *DWGRID, double ref, double offset);
double get_ref_value(component *head, int iv_val);
void setup_analog_graph();
void graph_waveform(double r, double w, double o, char type);
void export_netlist(component *& head);
datasave save_matrices(datasave& matrixsave, int useSAVE, int size, double *V, double *J, double **Y, double **invY, complex *DWGRID, complex *Vc, complex *Jc, complex **Yc, complex **invYc, component *head);
void destroy(datasave &matrixdata);


int reset_voltages(component *head);
int set_ground(component *head, int wires);
void set_common_voltages(component *head, int wires);

void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);
void draw_char(char x, int box_num, int color, int size);
void draw_double(double num, char label, int box_num);
void draw_SINEWAVE(int box_num);

int get_scrolled_window_click();
int get_valid_position();

component *link_components(component *head);

component *simulation_setup(component *head, wire *&wires, int &num_wires);
component *init_data_structures(component *head, wire *&wiresave, component *&unsim, wire *wires, int num_wires, int &nodes, int &error, int is_digital, int &comp_count);
component *reset_circuit(component *head, component *unsim, wire *wiresave);

int count_components(component *head, component *& tail, int &subcircuit_count);

IC find_ic(int id);

transistor Qnum_find(int num);

void get_AC_waveform_cursor();

void graph_setup();
void axis(int draw_axes);
void set_graphing_prefs(int needs_redraw);
double calc_next_x(double Start, double End, double delta_x, double point_count, double numpoints, double last_x);

int get_equation();

void split_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old);
void merge_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old);
void fft(double xr[], double xi[], int nu,int ie);




// findr = 0
// initialization/single analysis version of analog_analysis()
component *analog_analysis_init(component *head, complex *DWGRID) {
	
	int i, ID, nodes, size, size1, node1, node2, node3, node4, has_controlled_source(0), init_error(0), num_wires(0), zero_vsource(0);
	int has_variable(0), has_power(0), new_v(1), v_sources(0), has_AC_source(0), do_freq_sweep(0), do_loop(0), comp_count, Ic_size(0);
	double **Y = NULL, **invY = NULL, *V = NULL, *J = NULL, Value;
	complex **Yc = NULL, **invYc = NULL, *Vc = NULL, *Jc = NULL, *Ic = NULL;
	component *temp = NULL, *variable = NULL, *unsim = NULL;
	connections x;
	wire *wiresave = NULL, *hier_wires = NULL;

	if (head == NULL)
		return NULL;
	
	useHASHTABLE = 1;	
	inconsistent = arbitrary = useDIGITAL = force_recreate = input_waveform = 0;		
	matrixdata.last_value = matrixdata.useSAVE = findr = 0;
		
	if (useCOMPLEX && f != 0 && !Opened) {
		cout << "\nUse same input waveform as before?  ";
		if (decision())
			new_v = 0;	
	}
	else
		f_phase = 0;
	
	// Check circuit to see if it can be simulated.	
	if (changed3 || Opened || new_v || always_recalculate) {	
		useCOMPLEX      = 1;
		source_variable = 0;		
		head            = simulation_setup(head, hier_wires, num_wires);		
		temp            = head;
		useCOMPLEX      = 0;
					 
		while (temp != 0) {
			ID = temp->id;
			if (ID >= V_SOURCE_ID && ID <= CCCS_ID) {
				has_power = has_AC_source = 1;
			}
			else if (ID == POWER_ID || ID == BATTERY_ID)
				has_power = 1;
			if (AC_COMP_ID(ID))
				useCOMPLEX = 1;
			if (source_variable != 2 && (temp->value == 0 && (ID == V_SOURCE_ID || ID == BATTERY_ID || ID == I_SOURCE_ID))) {
				if (ID == I_SOURCE_ID)
					source_variable = 1;
				else {
					source_variable = 1;
					++v_sources;
				}
			}
			if ((temp->value <= 0 && BASIC_COMP_ID(ID)) || temp->value == 0 && ID >= VCVS_ID && ID <= CCCS_ID)
				source_variable = 2;
			temp = temp->next;
		}		
		if (source_variable == 2 || v_sources > 1)
			source_variable = 0;
		
		useINV = source_variable;
		
		if (!has_AC_source || DC_only)
			useCOMPLEX = 0;
		
		if (!has_power) {
			beep();
			cout << "\nThis circuit has no source of power!" << endl;
			if (hier_wires != NULL) delete [] hier_wires;
			return head;
		}
		has_power = 0;
		
		if (useCOMPLEX) {
			if (freq == -1 && new_v) {
				cout << "\nFrequency is set to Hertz. Change to radians/second? (This can also be set in the preferences.):  ";
				if (decision())
					freq = 1;
				else
					freq = 0;
			}
			cout << "\nPlot frequency response for this circuit?  ";
			if (decision()) {
				do_freq_sweep = 1;
				f = f_phase = 0;
			}
			else {
				cout << "\nWould you like to enter a custom input waveform?  ";
				if (decision()) {
					do_freq_sweep = input_waveform = 1;
					f = f_phase = 0;
				}
				else { // Get the frequency and waveform.
					if (new_v) {
						cout << "\nEnter the frequency ";
						if (freq == 1)
							cout << "in radians/second:  ";
						else
							cout << "in Hertz:  ";
						
						f = fabs(ind());
					
						cout << "Enter the input phase in degrees:  ";
						f_phase = ind();
					}
					if (f == 0) 
						useCOMPLEX = 0;	
					else {
						if (freq == 1)
							f /= two_pi;	
						
						if (!complex_form)
							complex_form = get_complex_form();
						
						if (complex_form >= 4)
							if (new_v) 
								do {											
									cout << "\nUse (s)in or (c)os?  ";
									function = inc();
						
									if 	(!comp_char(function, 's') && !comp_char(function, 'c'))	{		
										beep();
										cout << "Invalid selection! Please enter only S or C." << endl;
									}
								} while (!comp_char(function, 's') && !comp_char(function, 'c'));
							
							cout << "\nWould you like to graph the sin/cos equation of the voltage/current at a node?  ";
							graph_sincos = decision();	
					}
				}
			}
		}
		F_temp = F_storage;
		F_storage = f;
			
		head = init_data_structures(head, wiresave, unsim, hier_wires, num_wires, nodes, init_error, 0, comp_count);
		if (init_error) {
			return head;
		}		
		if (!NODES || memory_warning(nodes)) {
			return reset_circuit(head, unsim, wiresave);
		}
		matrixdata.wiresave = wiresave;
		size1 = nodes - 1;
		size  = size1;
		
		// Create memory to hold all matrices.
		get_additional_size(head);
		
		reset_timer(1);
		show_elapsed_time();	
		draw_status_bar();
		draw_status(" Creating Matrix");
				
		DWGRID = memAlloc_complex_zero(DWGRID, nodes+2);
	
		size += additional_size;
		create_matrices(Y, V, J, Yc, Vc, Jc, size);
						
		if (do_freq_sweep) { // frequency sweep
			matrixdata = save_matrices(matrixdata, 1, size, V, J, Y, invY, DWGRID, Vc, Jc, Yc, invYc, head);
			find_component(head, 0, 0, 1, DWGRID);
			delete [] DWGRID;
			delete_matrices(Y, V, J, Yc, Vc, Jc, size);
			return reset_circuit(head, unsim, wiresave);
		}				
		temp = head;
		
		while (temp != 0) {			
			ID = temp->id;
			zero_vsource = 0;

			if (BASIC_COMP_ID(ID) || ID == POWER_ID || ID == OPAMP_ID || (ID >= BATTERY_ID && ID <= MAX_DEP_SRC_ID)) {
				
				// Get node numbers.
				x = get_connections(temp->location, ID, temp->flags);
				
				if (!get_nodes(ID, node1, node2, node3, node4, size, size1, x)) {
					beep(); 
					cerr << "\nMatrix Size Error!" << endl;
					bad_error = 1;
					
					delete [] DWGRID;
					delete_matrices(Y, V, J, Yc, Vc, Jc, size);
					return reset_circuit(head, unsim, wiresave);
				}
				if (!do_loop && ((temp->value == 0 && ID != POWER_ID && ID != OPAMP_ID) || (BASIC_COMP_ID(ID) && temp->value <= 0))) {
					stop_timer();
					reset_cursor();
					
					if (ID == V_SOURCE_ID || ID == BATTERY_ID) {
						cout << "\nDo you want to keep this source as 0V?  ";
						zero_vsource     = decision();
						matrixdata.v_pos = size1;
					}
					if (zero_vsource) {
						Value = 0.0;
					}
					else {
						// Deal with variable components.
						if (!changed4 && calculated_value != 0 && fabs(calculated_value) < bound && !no_soultion) {
							cout << "\nUse previously calculated value for the variable " << get_name(temp->id, 1) << "? ";
							if (decision())
								Value = calculated_value;
							else {
								calculated_value = 0;
								cout << "\nEnter the value of the variable " << get_name(temp->id, 1)
						 	 		 << " (Enter 0 to solve for this " << get_name(temp->id, 1) << "):  ";
			
								Value = ind();
							}
						}
						else {
							cout << "\nEnter the value of the variable " << get_name(temp->id, 1)
						 		 << " (Enter 0 to solve for this " << get_name(temp->id, 1) << "):  ";
			
							Value = ind();
						}
						show_clock();
						
						if (Value == 0 || (BASIC_COMP_ID(ID) && Value <= 0)) {
							do_loop  = 1;
							variable = temp;
						}
						has_variable = 1;
					}
					temp->state = (char)Value;
					start_timer();
				}
				else {
					Value = temp->value;
				}
			}
			if (ID == RES_ID && fabs(Value) < Rmin) {
				Value = Rmin;
			}
			if (Value != 0 || ID == POWER_ID || ID == OPAMP_ID || zero_vsource) {
				if (useCOMPLEX) {
					ID = -ID;
				}
				fill_admittance_matrix(temp->id, Value, ID, size1, has_controlled_source, node1, node2, node3, node4, Y, J, Yc, Jc);
			}
			temp = temp->next;
		} // while (temp != 0)
		if (do_loop) {
			// Calculate component value.
			matrixdata = save_matrices(matrixdata, 1, size, V, J, Y, invY, DWGRID, Vc, Jc, Yc, invYc, head);
			find_component(head, variable->location, variable->id, 0, DWGRID);
			delete [] DWGRID;
			delete_matrices(Y, V, J, Yc, Vc, Jc, size);
			return reset_circuit(head, unsim, wiresave);
		}
		add_opamp_numbers(head, nodes, Y, J, Yc, Jc, size, size1, 1);
			
		// Solve node equations.
		cout << "\n";
		show_elapsed_time();
		cout << (nodes - 1) << " node";
		if (nodes != 2)
			cout << "s";
		cout <<" plus ground" << endl;
		
		show_elapsed_time();
		draw_status_bar();
		draw_status(" Solving Nodal Equations");
		
		// Check for a power source.
		if (useCOMPLEX) {
			for (i = 0; i < size; ++i)
				if (Jc[i].Re != 0)
					has_power = 1;
		}
		else {
			for (i = 0; i < size; ++i)
				if (J[i] != 0)
					has_power = 1;
		}
		if (!has_power) {
			beep();
			cout << "\nThis circuit has no apparent source of power!" << endl;
			delete [] DWGRID;
			delete_matrices(Y, V, J, Yc, Vc, Jc, size);
			return reset_circuit(head, unsim, wiresave);
		}
		Ic_size = size - nodes + 1;
		if (Ic_size > 0) {
			Ic = memAlloc(Ic, Ic_size);
		}
		solve_simulation(head, invY, Y, J, V, invYc, Yc, Jc, Vc, Ic, DWGRID, size, nodes, 1, 1);	
		
		cout << "\nSimulation time:";
		show_elapsed_time();
		cout << endl;
		
		draw_status_bar();
		draw_status(" Finished");	
		reset_cursor();
	
		cout << "\nThe circuit has been evaluated. " << endl;
		if (inconsistent) {
			cerr << "\nSome of the node voltages were inconsistent or were not found and could be incorrect. This circuit may violate KVL or KCL." << endl;
			if (has_controlled_source)
				cout << "The controlled source in the circuit or in a component may be causing this error." << endl;
			else
				beep();
		}
		else if (arbitrary && !has_controlled_source) {
			beep();
			cout << "\nWarning: More than one solution is possible for this circuit, at least one of the voltages is unknown, or the solution parameters differ by many orders of magnitude." << endl;
		}
	}
		
	// Display circuit values when the user clicks on the screen.
	if (!graph_sincos) {
		cout << "\nClick on a node to find its voltage or a component to find the current through it(+ is L to R or Down) and power dissipated in it.";
		cout << "\nClick on Cancel or another button to exit analog circuit evaluation mode." << endl << endl;
	}
	else
		cout << "\nClick on a component or node to see and graph its equation." << endl;
		
	if (useCOMPLEX) 		
		head = show_AC_results(head, DWGRID, Ic, Ic_size, complex_form, function); 		
	else 	
		show_DC_results(head, DWGRID, Ic, Ic_size);
	
	source_variable = 0;
	reset_cursor();			
	cleanup_simulation(Y, V, J, Yc, Vc, Jc, Ic, invYc, invY, size);
	delete [] DWGRID;
	
	if (!has_variable) 
		changed3 = Opened = 0;
	
	return reset_circuit(head, unsim, wiresave);
}




// *******************************
// findr = 1 or 2
// iterative version of analog_analysis()
component *analog_analysis(component *head, double input_value, complex *DWGRID) {
	
	int ID, nodes, size, size1, node1, node2, node3, node4, has_controlled_source(0), create_new_matrices(0);
	double **Y = NULL, **invY = NULL, *V = NULL, *J = NULL, Value = 1;
	complex **Yc = NULL, **invYc = NULL, *Vc = NULL, *Jc = NULL;
	component *temp = head;
	connections x;
	wire *wiresave = NULL;

	if (head == NULL)
		return NULL;
		
	if (!findr)
		return analog_analysis_init(head, DWGRID);
	
	if (!NODES) {
		cerr << "\nError: No nodes found!" << endl;
		return reset_circuit(head, NULL, wiresave);
	}
	if (freq == 1)
		f /= two_pi;
	
	inconsistent = arbitrary = 0;	
	F_temp = F_storage;
	F_storage = f;	
	nodes = NODES;				
	size1 = nodes - 1;
	size = size1 + additional_size;
	
	if (matrixdata.useSAVE == 0 || findr != 1 || !source_variable) {
		create_new_matrices = 1;
		if (force_recreate) {
			create_matrices(Y, V, J, Yc, Vc, Jc, size);
		}
		else if (!source_variable) {
			reset_matrixdata();
		}
	}
	if (!force_recreate) {
		size     = matrixdata.size;
		V        = matrixdata.V;
		J        = matrixdata.J;
		Y        = matrixdata.Y;
		invY     = matrixdata.invY;
		Vc       = matrixdata.Vc;
		Jc       = matrixdata.Jc;
		Yc       = matrixdata.Yc;
		invYc    = matrixdata.invYc;
		wiresave = matrixdata.wiresave;
		head     = matrixdata.head;
	}	
	while (temp != 0) {	
		ID = temp->id;	
		
		if (BASIC_COMP_ID(ID) || ID == POWER_ID || ID == OPAMP_ID || (ID >= BATTERY_ID && ID <= MAX_DEP_SRC_ID)) {			
			// Get node numbers.
			x = get_connections(temp->location, ID, temp->flags);
			
			if (!get_nodes(ID, node1, node2, node3, node4, size, size1, x)) {
				beep(); 
				cerr << "\nMatrix Size Error!" << endl;
				bad_error = 1;
				
				if (create_new_matrices) {
					if (force_recreate)
						delete_matrices(Y, V, J, Yc, Vc, Jc, size);
					return reset_circuit(head, NULL, wiresave);
				}
				else {
					destroy(matrixdata);
					reset_cursor();
					return matrixdata.head;
				}
			}
			if ((temp->value == 0 && ID != POWER_ID && ID != OPAMP_ID) || (BASIC_COMP_ID(ID) && temp->value <= 0)) {
				if (findr == 2) {
					if (Count == 0)
						cout << "\nWarning: Variable components will be given a value of 1." << endl;
					Value = 1;
				}
				else {
					if (create_new_matrices) 
						Value = input_value;
					else 
						fill_J_matrix(matrixdata.v_pos, temp->id, (input_value - matrixdata.last_value), J, Jc, node1, node2); 
				}
			}
			else 
				Value = temp->value;
		}				
		if (create_new_matrices && fabs(Value) < Rmin && ID == RES_ID) 
			Value = Rmin;
		
		if (Value != 0 || ID == POWER_ID || ID == OPAMP_ID) {
			if (useCOMPLEX)
				ID = -ID;
			
			if (create_new_matrices)
				fill_admittance_matrix(temp->id, Value, ID, size1, has_controlled_source, node1, node2, node3, node4, Y, J, Yc, Jc);
		}
		temp = temp->next;
	}
	matrixdata.last_value = input_value;
	
	add_opamp_numbers(head, nodes, Y, J, Yc, Jc, size, size1, create_new_matrices);			
	solve_simulation(head, invY, Y, J, V, invYc, Yc, Jc, Vc, (complex *)NULL, DWGRID, size, nodes, create_new_matrices, 0);		
	
	if (force_recreate)
		cleanup_simulation(Y, V, J, Yc, Vc, Jc, (complex *)NULL, invYc, invY, size);
	else
		changed4 = 0;
			
	return head;
}





void fill_admittance_matrix(int id, double Value, int ID, int &size1, int &has_controlled_source, int node1, int node2, int node3, int node4, double **Y, double *J, complex **Yc, complex *Jc) {		
	
	int abs_id(abs(ID));
	double val;
	complex valc;

	if (fabs(Value) < UnknownV && abs_id != POWER_ID && abs_id != OPAMP_ID && abs_id != V_SOURCE_ID && abs_id != BATTERY_ID) {
		return; 
	}
	switch (ID) {
		case RES_ID: // DC resistor
			val = 1.0/Value;
			if (node1 >= 0)
				Y[node1][node1] += val;
			if (node2 >= 0)
				Y[node2][node2] += val;
			if (node1 >= 0 && node2 >= 0) {
				Y[node1][node2] -= val;
				Y[node2][node1] -= val;
			}
			break;
		
		case XFMR_ID: // needed as DC short
			if (node3 >= 0)
				Y[node3][node3] += Rmax;
			if (node4 >= 0)
				Y[node4][node4] += Rmax;
			if (node3 >= 0 && node4 >= 0) {
				Y[node3][node4] -= Rmax;
				Y[node4][node3] -= Rmax;
			}
			// continues into inductor	

		case INDUCTOR_ID: // needed as DC short
			if (node1 >= 0)
				Y[node1][node1] += Rmax;
			if (node2 >= 0)
				Y[node2][node2] += Rmax;
			if (node1 >= 0 && node2 >= 0) {
				Y[node1][node2] -= Rmax;
				Y[node2][node1] -= Rmax;
			}
			break;
						
		case POWER_ID: // DC power
			if (node2 >= 0) {
				Y[size1][node2] += 1.0;
				Y[node2][size1] += 1.0;
			}
			J[size1] += supply_voltage;
			++size1;
			break;
		
		case V_SOURCE_ID: // DC voltage source
			if (node1 >= 0) {
				Y[size1][node1] -= 1.0;
				Y[node1][size1] -= 1.0;
			}
			if (node2 >= 0) {
				Y[size1][node2] += 1.0;
				Y[node2][size1] += 1.0;
			}
			if (!DC_only)
				J[size1] += Value;
			++size1; 
			break;
			
		case BATTERY_ID: // DC battery	
			if (node1 >= 0) {
				Y[size1][node1] -= 1.0;
				Y[node1][size1] -= 1.0;
			}
			if (node2 >= 0) {
				Y[size1][node2] += 1.0;
				Y[node2][size1] += 1.0;
			}
			J[size1] += Value;
			++size1; 
			break;
				
		case I_SOURCE_ID: // DC current source
			if (node1 >= 0)
				J[node1] -= Value;
			if (node2 >= 0)
				J[node2] += Value;
			break;
						
		case VCVS_ID: // DC VCV source
			has_controlled_source = 1;
			
			if (node3 >= 0)
				Y[size1][node3] -= Value;
			if (node4 >= 0)
				Y[size1][node4] += Value;
						
			if (node1 >= 0) {
				Y[size1][node1] -= 1.0;
				Y[node1][size1] -= 1.0;
			}
			if (node2 >= 0) {
				Y[size1][node2] += 1.0;
				Y[node2][size1] += 1.0;
			}
			++size1;
			break;
			
		case VCCS_ID: // DC VCC source
			has_controlled_source = 1;
			
			if (node2 >= 0 && node3 >= 0)
				Y[node2][node3] -= Value;
			if (node1 >= 0 && node4 >= 0)
				Y[node1][node4] -= Value;
			if (node2 >= 0 && node4 >= 0)
				Y[node2][node4] += Value;
			if (node1 >= 0 && node3 >= 0)
				Y[node1][node3] += Value;
			break;
			
		case CCVS_ID: // DC CCV source
			has_controlled_source = 1;
			val = 1.0/Value;
			
			if (node4 >= 0 && node2 >= 0) 
				Y[node4][node2] -= val;
			if (node4 >= 0 && node1 >= 0) 
				Y[node4][node1] += val;
			if (node3 >= 0 && node2 >= 0) 
				Y[node3][node2] += val;
			if (node3 >= 0 && node1 >= 0) 
				Y[node3][node1] -= val;
				
			if (node1 >= 0) 
				Y[node1][size1] -= 1.0;
			if (node3 >= 0)	
				Y[size1][node3] -= 1.0;		
			if (node2 >= 0) 
				Y[node2][size1] += 1.0;
			if (node4 >= 0)	
				Y[size1][node4] += 1.0;
						
			++size1;// this is different from the SPICE MNA CCVS stamp
			break;
			
		case CCCS_ID: // DC CCC source
			has_controlled_source = 1;
			
			if (node3 >= 0) {
				Y[node3][size1] += 1.0;
				Y[size1][node3] += 1.0;
			}
			if (node4 >= 0) {
				Y[node4][size1] -= 1.0;
				Y[size1][node4] -= 1.0;
			}
			if (node1 >= 0)
				Y[node1][size1] += Value;
			if (node2 >= 0)
				Y[node2][size1] -= Value;
				
			++size1;
			break;
			
		case OPAMP_ID:  // DC OP AMP - not dealt with here
		case -OPAMP_ID: // AC OP AMP is the same
			has_controlled_source = 1;
			break;
			
		/*case TWO_PORT: // Two Port Devices (reciprocity) - antennas, etc.
			if (node1 >= 0)
				Y[][] = ;
			if (node2 >= 0)
				Y[][] = ;					
			if (node3 >= 0) 
				Y[][] = ;
			if (node4 >= 0) 
				Y[][] = ;
			break;*/
						
						
		// complex components
		case -XFMR_ID: // trransformer *** MUST CHANGE ***
			valc = admittance(id, f, Value);
			if (node3 >= 0)
				Yc[node3][node3] = Yc[node3][node3] + valc;
			if (node4 >= 0)
				Yc[node4][node4] = Yc[node4][node4] + valc;
			if (node3 >= 0 && node4 >= 0) {
				Yc[node3][node4] = Yc[node3][node4] - valc;
				Yc[node4][node3] = Yc[node4][node3] - valc;
			}
			// XFMR continues
		case -CAP_ID: // capacitor
		case -RES_ID: // resistor
		case -INDUCTOR_ID: // inductor
			valc = admittance(id, f, Value);
			if (node1 >= 0)
				Yc[node1][node1] = Yc[node1][node1] + valc;
			if (node2 >= 0)
				Yc[node2][node2] = Yc[node2][node2] + valc;
			if (node1 >= 0 && node2 >= 0) {
				Yc[node1][node2] = Yc[node1][node2] - valc;
				Yc[node2][node1] = Yc[node2][node1] - valc;
			}
			break;
						
		case -POWER_ID: // AC power 
			if (node2 >= 0) {
				Yc[size1][node2].Re = Yc[size1][node2].Re + 1.0;
				Yc[node2][size1].Re = Yc[node2][size1].Re + 1.0;
			}
			//Jc[size1].Re = Jc[size1].Re + supply_voltage;
			++size1;
			break;
			
		case -BATTERY_ID: // AC battery
		
			if (node1 >= 0) {
				Yc[size1][node1].Re = Yc[size1][node1].Re - 1.0;
				Yc[node1][size1].Re = Yc[node1][size1].Re - 1.0;
			}
			if (node2 >= 0) {
				Yc[size1][node2].Re = Yc[size1][node2].Re + 1.0;
				Yc[node2][size1].Re = Yc[node2][size1].Re + 1.0;
			}
			//Jc[size1].Re = Jc[size1].Re + Value;
			++size1;
			break;
		
		case -V_SOURCE_ID: // AC voltage source
			if (node1 >= 0) {
				Yc[size1][node1].Re = Yc[size1][node1].Re - 1.0;
				Yc[node1][size1].Re = Yc[node1][size1].Re - 1.0;
			}
			if (node2 >= 0) {
				Yc[size1][node2].Re = Yc[size1][node2].Re + 1.0;
				Yc[node2][size1].Re = Yc[node2][size1].Re + 1.0;
			}
			Jc[size1].Re = Jc[size1].Re + Value;
			++size1; 
			break;
				
		case -I_SOURCE_ID: // AC current source
			if (node1 >= 0)
				Jc[node1].Re = Jc[node1].Re - Value;
			if (node2 >= 0)
				Jc[node2].Re = Jc[node2].Re + Value;
			break;
						
		case -VCVS_ID: // AC VCV source
			has_controlled_source = 1;
		
			if (node3 >= 0)
				Yc[size1][node3].Re = Yc[size1][node3].Re - Value;
			if (node4 >= 0)
				Yc[size1][node4].Re = Yc[size1][node4].Re + Value;
						
			if (node1 >= 0) {
				Yc[size1][node1].Re = Yc[size1][node1].Re - 1.0;
				Yc[node1][size1].Re = Yc[node1][size1].Re - 1.0;
			}
			if (node2 >= 0) {
				Yc[size1][node2].Re = Yc[size1][node2].Re + 1.0;
				Yc[node2][size1].Re = Yc[node2][size1].Re + 1.0;
			}
			++size1;
			break;
			
		case -VCCS_ID: // AC VCC source
			has_controlled_source = 1;
			
			if (node2 >= 0 && node3 >= 0)
				Yc[node2][node3].Re -= Value;
			if (node1 >= 0 && node4 >= 0)
				Yc[node1][node4].Re -= Value;
			if (node2 >= 0 && node4 >= 0)
				Yc[node2][node4].Re += Value;
			if (node1 >= 0 && node3 >= 0)
				Yc[node1][node3].Re += Value;
			break;
			
		case -CCVS_ID: // AC CCV source
			has_controlled_source = 1;
			val = 1.0/Value;
			
			if (node4 >= 0 && node2 >= 0) 
				Yc[node4][node2].Re -= val;
			if (node4 >= 0 && node1 >= 0) 
				Yc[node4][node1].Re += val;
			if (node3 >= 0 && node2 >= 0) 
				Yc[node3][node2].Re += val;
			if (node3 >= 0 && node1 >= 0) 
				Yc[node3][node1].Re -= val;
				
			if (node1 >= 0) 
				Yc[node1][size1].Re -= 1.0;
			if (node3 >= 0)	
				Yc[size1][node3].Re -= 1.0;		
			if (node2 >= 0) 
				Yc[node2][size1].Re += 1.0;
			if (node4 >= 0)	
				Yc[size1][node4].Re += 1.0;
						
			++size1;
			break;
			
		case -CCCS_ID: // AC CCC source
			has_controlled_source = 1;
			
			if (node3 >= 0) {
				Yc[node3][size1].Re += 1.0;
				Yc[size1][node3].Re += 1.0;
			}
			if (node4 >= 0) {
				Yc[node4][size1].Re -= 1.0;
				Yc[size1][node4].Re -= 1.0;
			}
			if (node1 >= 0)
				Yc[node1][size1].Re += Value;
			if (node2 >= 0)
				Yc[node2][size1].Re -= Value;
				
			++size1;
			break;
	}
}





void fill_J_matrix(int v_pos, int ID, double Value, double *J, complex *Jc, int node1, int node2) {
	
	if (useCOMPLEX)
		ID = -ID;
			
	if (fabs(Value) < UnknownV || v_pos < 0)
		return; 
	
	switch (ID) {
		case BATTERY_ID: // DC battery	
		case V_SOURCE_ID: // DC voltage source
			J[v_pos] += Value;
			break;
				
		case I_SOURCE_ID: // DC current source
			if (node1 >= 0)
				J[node1] -= Value;
			if (node2 >= 0)
				J[node2] += Value;
			break;
			
		case -BATTERY_ID: // AC battery	
		case -V_SOURCE_ID: // AC voltage source
			Jc[v_pos].Re = Jc[v_pos].Re + Value;
			break;
				
		case -I_SOURCE_ID: // AC current source
			if (node1 >= 0)
				Jc[node1].Re = Jc[node1].Re - Value;
			if (node2 >= 0)
				Jc[node2].Re = Jc[node2].Re + Value;
			break;
	}
}





int get_nodes(int ID, int &node1, int &node2, int &node3, int &node4, int size, int size1, const connections& x) {
	
	if (ID == POWER_ID) {
		node1 = NODENUMBER[x.out2];
		node2 = node3 = node4 = 0;
	}
	else if (ID == OPAMP_ID) {
		node1 = NODENUMBER[x.in1] - 1; // Vin-
		node2 = NODENUMBER[x.in3] - 1; // Vin+
		node3 = NODENUMBER[x.out2] - 1; // Vout
		node4 = 0;
	}
	else if (ID == XFMR_ID) { // transformer
		node1 = NODENUMBER[x.in1] - 1;
		node2 = NODENUMBER[x.in3] - 1;
		node3 = NODENUMBER[x.out1] - 1;
		node4 = NODENUMBER[x.out3] - 1;
	}
	else { // two-port + controlled sources
		node1 = node2 = node3 = node4 = 0;
		if (x.in2 > 0)
			node1 = NODENUMBER[x.in2] - 1;
		if (x.out2 > 0)
			node2 = NODENUMBER[x.out2] - 1;
		if (x.in1 > 0)
			node3 = NODENUMBER[x.in1] - 1;
		if (x.out1 > 0)
			node4 = NODENUMBER[x.out1] - 1;
	}						
	if (node1 >= size || node2 >= size || node3 >= size || node4 >= size || size1 > size)
		return 0;
		
	return 1;
}





void add_opamp_numbers(component *head, int nodes, double **Y, double *J, complex **Yc, complex *Jc, int size, int size1, int create_new_matrices) {
	
	int i, node1, node2, node3, node4;
	connections x;
		
	while (head != 0) {
		
		if (head->id == OPAMP_ID) {
			x = get_connections(head->location, head->id, head->flags);
					
			if (!get_nodes(head->id, node1, node2, node3, node4, size, size1, x)) { 
				beep();
				cerr << "\nMatrix Size Error!" << endl;
				return;
			}			
			if (useCOMPLEX) {
				if (create_new_matrices) {
					for (i = 0; i < nodes; ++i)
						Yc[node3][i].Re = Yc[node3][i].Im = 0;
				
					if (node1 >= 0) 
						Yc[node3][node1].Re += 1.0;
					if (node2 >= 0) 
						Yc[node3][node2].Re -= 1.0;
				}	
				Jc[node3].Re = Jc[node3].Im = 0.0;
			}
			else {
				if (create_new_matrices) {
					for (i = 0; i < nodes; ++i)
						Y[node3][i] = 0;
				
					if (node1 >= 0) 
						Y[node3][node1] += 1.0;
					if (node2 >= 0) 
						Y[node3][node2] -= 1.0;
				}	
				J[node3] = 0;
			}
		}	
		head = head->next;
	}
}




void solve_simulation(component *head, double **&invY, double **Y, double *J, double *&V, complex **&invYc, complex **Yc, complex *Jc, complex *&Vc, complex *Ic, complex *DWGRID, int size, int nodes, int create_new_matrices, int interactive) {

	int i, nextnn, index;
	
	if (useCOMPLEX) {
		if (useINV) {
			if (create_new_matrices || invYc == NULL) {
				if (invYc == NULL) delete_matrix(invYc, size);
				invYc = inverse(Yc, size, size);
			}
			else {
				invYc = matrixdata.invYc;
			}
			multiply(invYc, Jc, Vc, size, size); 
		}
		else 
			Vc = gauss_solve(Yc, Jc, Vc, size, size);
		
		// Copy values back into drawing window array.
		NODENUMBER.resetnext();
		nextnn = NODENUMBER.getnext(END_FLAG, index);
		
		while (nextnn != END_FLAG) {
			if (nextnn > 0 && nextnn < nodes) {
				DWGRID[nextnn] = phase_shift(Vc[nextnn-1], f_phase);
				
				if (interactive && showVALUES && nextnn >= 0)
					cout << "Net: " << nextnn << " " << DWGRID[nextnn] << endl;
			}
			nextnn = NODENUMBER.getnext(END_FLAG, index);
		}
		if (Ic != NULL) {
			for (i = nodes-1; i < size; ++i) {
				Ic[i-nodes+1].Re = Vc[i].Re;
				Ic[i-nodes+1].Im = Vc[i].Im;
			}
		}
		if (interactive && showNODEVALUES) {
			cout << "Voltages:" << endl;
			for (i = 0; i < nodes-1; i++) {
				cout << Vc[i] << endl;
			}
			if (nodes < size) {
				cout << "Currents of voltage sources:" << endl;
				for (; i < size; i++) {
					cout << Vc[i] << endl;
				}
			}
		}
	}	
	else {
		if (useINV) {		
			if (create_new_matrices || invY == NULL) {
				if (invY == NULL) delete_matrix(invY, size);
				invY = inverse(Y, size, size);
			}
			else {
				invY = matrixdata.invY;
			}
			multiply(invY, J, V, size, size); 
		}
		else 
			V = gauss_solve(Y, J, V, size, size);

		// Copy values back into drawing window array.
		NODENUMBER.resetnext();
		nextnn = NODENUMBER.getnext(END_FLAG, index);
		
		while (nextnn != END_FLAG) {
			if (nextnn > 0 && nextnn < nodes) {
				DWGRID[nextnn].Re = V[nextnn-1];
				
				if (interactive && showVALUES && nextnn >= 0)
					cout << "Net: " << nextnn << " " << DWGRID[nextnn].Re << endl;					
			}
			nextnn = NODENUMBER.getnext(END_FLAG, index);
		}
		if (Ic != NULL) {
			for (i = nodes-1; i < size; ++i) {
				Ic[i-nodes+1].Re = V[i];
				Ic[i-nodes+1].Im = 0;
			}
		}
		if (interactive && showNODEVALUES) {
			cout << "Voltages:" << endl;
			for (i = 0; i < nodes-1; i++) {
				cout << V[i] << endl;
			}
			if (nodes < size) {
				cout << "Currents of voltage sources:" << endl;
				for (; i < size; i++) {
					cout << V[i] << endl;
				}
			}
		}
	}		
	matrixdata = save_matrices(matrixdata, 1, size, V, J, Y, invY, DWGRID, Vc, Jc, Yc, invYc, head);
}





component *show_AC_results(component *head, complex *DWGRID, complex *Ic, int Ic_size, int complex_form, char function) {
	
	int ID, position, nnumber;
	unsigned cm_count;
	double current, power, p_max, val;
	complex currentc, *DWGRID2 = NULL;
	component *temp;
	connections x;
	
	reset_timer(1);
	show_elapsed_time();
	cout << "\nCalculating DC Offset." << endl;
	
	DWGRID2 = memAlloc_complex_zero(DWGRID2, NODES+2);

	head = calc_DC_offset(head, DWGRID2);
	
	show_elapsed_time();
	cout << "Done." << endl;
	
	set_special_cursor(PROBEAC_CURSOR);
	
	do {
		position = get_scrolled_window_click();

		if (position > num_menu_boxes) { 
			temp    = head;
			nnumber = NODENUMBER[position];

			if (nnumber >= 0) {
				if (showNODENUMBERS)
					cout << "Net " << nnumber << ": ";
					
				out(DWGRID2[nnumber].Re);
				cout << "V (DC) + \n        ";

				if (complex_form%2) {
					out2(DWGRID[nnumber]);
					if (complex_form >= 3)
						cout << "V  or  \n        ";
					else
						cout << "V ";
				}
				if (complex_form == 2 || complex_form == 3 || complex_form == 6 || complex_form == 7) {
					outp(DWGRID[nnumber]);
					if (complex_form > 3)
						cout << "V  or  \n        ";
					else
						cout << "V ";
				}
				if (complex_form > 3)
					show_equation(DWGRID[nnumber], f, 'V', function);
				cout << endl;
			}
			cm_count = 0;

			while (temp != 0) {
				ID = temp->id;
				if (temp->location == position && (temp->flags & BASE_CIRCUIT)) {
					if (CURR_IN_MATRIX(ID) && cm_count >= (unsigned)Ic_size) {
						internal_error();
						cerr << "Error in Analog Analysis: cm_count = " << cm_count << " >= Ic_size = " << Ic_size << "." << endl;
						return head;
					}
					if (calc_current(temp, DWGRID2, Ic, x, val, current, 1, cm_count)) {
						currentc = calc_param(DWGRID, Ic, x, ID, val, CURR_C, cm_count);

						if (ID == RES_ID) {
							cout << " (DC) + \n        ";
						}
						if (complex_form%2) {
							outp(currentc);
							if (complex_form >= 3)
								cout << "A  or  \n        ";
							else
								cout << "A ";
						}
						if (complex_form == 2 || complex_form == 3 || complex_form == 6 || complex_form == 7) {
							out2(currentc);
							if (complex_form > 3)
								cout << "A  or  \n        ";
							else
								cout << "A ";
						}
						if (complex_form > 3)
							show_equation(currentc, f, 'A', function);

						cout << ",   ";
						p_max = calc_param(DWGRID, Ic, x, ID, val, PMAX_C, cm_count).Re;

						if (ID == RES_ID) {
							cout << " DC Power = ";
							power = calc_param(DWGRID2, Ic, x, ID, val, POW_C, cm_count).Re;
							out(power);
							cout << "W, ";
						}
						cout << " Pmax = ";
						out(p_max);
						cout << "W,";
						if (AC_COMP_ID(ID)) 
							cout << " Pav = 0W" << endl;
						else { 
							cout << " Pav = Prms = ";
							out(p_max/2);
							cout << "W" << endl;
						}
					}
				}
				if (CURR_IN_MATRIX(ID)) {
					++cm_count;
				}
				temp = temp->next;
			}
		}
		if (nnumber >= 0 && graph_sincos && magnitude(DWGRID[nnumber]) != 0) {
			graph_waveform((magnitude(DWGRID[nnumber]) + DWGRID2[nnumber].Re), two_pi*f, phase(DWGRID[nnumber]), function);
			position = 0;		
		}
	} while (position > num_menu_boxes);
	
	delete [] DWGRID2;
	
	return head;
}





void show_DC_results(component *head, complex *DWGRID, complex *Ic, int Ic_size) {
	
	int ID, position, nnumber;
	unsigned cm_count;
	double current, val;
	component *temp;
	connections x;

	set_special_cursor(PROBEDC_CURSOR);
	
	do {
		position = get_scrolled_window_click();

		if (position > num_menu_boxes) { 
			temp    = head;
			nnumber = NODENUMBER[position];

			if (nnumber >= 0) {
				if (showNODENUMBERS)
					cout << "Net " << nnumber << ": ";

				out(DWGRID[nnumber].Re);
				cout << "V" << endl;
			}
			cm_count = 0;

			while (temp != 0) {
				ID = temp->id;
				if (temp->location == position && (temp->flags & BASE_CIRCUIT)) {
					if (CURR_IN_MATRIX(ID) && cm_count >= (unsigned)Ic_size) {
						internal_error();
						cerr << "Error in Analog Analysis: cm_count = " << cm_count << " >= Ic_size = " << Ic_size << "." << endl;
						return;
					}
					if (calc_current(temp, DWGRID, Ic, x, val, current, 0, cm_count)) {
						cout << ",   ";
						out(current*current*val);
						cout << "W" << endl;
					}
					else if (AC_COMP_ID(ID)) {
						cout << "DC Steady State: Power = 0" << endl;
					}
				}
				if (CURR_IN_MATRIX(ID)) {
					++cm_count;
				}
				temp = temp->next;
			}
		}
	} while (position > num_menu_boxes);
}




int calc_current(component *comp, complex *arr, complex *Ic, connections &x, double &val, double &current, int is_complex, unsigned cm_count) {

	int ID(comp->id);

	if (ID == RES_ID || ID == POWER_ID || (is_complex && AC_COMP_ID(ID)) || (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID) || ID == BATTERY_ID/* || ID == OPAMP_ID*/) {
		cout << get_name(ID, 0) << ": ";
		x = get_connections(comp->location, ID, comp->flags);

		if (comp->value > 0) 
			val = comp->value;
		else
			val = comp->state;
		
		if (!is_complex || !AC_COMP_ID(ID)) {
			current = calc_param(arr, Ic, x, ID, val, CURR, cm_count).Re;
			out(current);
			cout << "A";
		}
		return 1;
	}
	return 0;
}




complex calc_param(complex *arr, complex *Ic, connections &x, int ID, double val, int type, unsigned cm_count) {

	complex c, voltage, imped, control;
	double vmag;

	zero(c);

	switch (ID) {

	case RES_ID:
	case CAP_ID:
	case INDUCTOR_ID:
	case XFMR_ID:
		voltage = arr[NODENUMBER[x.in2]] - arr[NODENUMBER[x.out2]];
		imped   = impedance(ID, f, val);
		break;

	case POWER_ID:
		val = supply_voltage;
	case V_SOURCE_ID:
	case BATTERY_ID:
		voltage.Re = val;
		voltage.Im = 0;
		c          = -1*Ic[cm_count];
		imped      = voltage/c;
		break;

	case I_SOURCE_ID:
		voltage = arr[NODENUMBER[x.in2]] - arr[NODENUMBER[x.out2]];
		imped   = voltage/val;
		break;

	case VCVS_ID:
		voltage = arr[NODENUMBER[x.in2]] - arr[NODENUMBER[x.out2]];
		c       = -1*Ic[cm_count];
		imped   = voltage/c;
		break;

	case VCCS_ID:
		voltage = arr[NODENUMBER[x.in2]] - arr[NODENUMBER[x.out2]];
		control = arr[NODENUMBER[x.in1]] - arr[NODENUMBER[x.out1]];
		imped   = voltage/(control*val);
		break;

	case CCVS_ID:
		voltage = arr[NODENUMBER[x.in2]] - arr[NODENUMBER[x.out2]];
		c       = -1*Ic[cm_count];
		imped   = voltage/c;
		break;

	case CCCS_ID:
		zero(voltage);
		zero(imped);
		c = Ic[cm_count]*val;
		break;

	case OPAMP_ID: // NOT USED - don't know c
		voltage = arr[NODENUMBER[x.out2]]; // output assumed referenced to ground
		imped   = voltage/c;
		break;

	default:
		internal_error();
		cerr << "Error: Illegal value parameter type: " << type << "." << endl;
		return c;
	}

	switch (type) {

	case CURR:
	case CURR_C:
		if (CURR_IN_MATRIX(ID)) {
			return c;
		}
		return voltage/imped;

	case POW_C:
		c.Re = voltage.Re*voltage.Re/magnitude(imped);
		return c;

	case PMAX_C:
		vmag = magnitude(voltage);
		c.Re = vmag*vmag/magnitude(imped);
		return c;

	default:
		internal_error();
		cerr << "Error: Illegal value parameter type: " << type << "." << endl;
	}
	return c;
}





component *calc_DC_offset(component *head, complex *DWGRID) {
	
	int useC(useCOMPLEX);
	double old_f(f);
	datasave temp = matrixdata;
		
	f       = useCOMPLEX = bad_error      = 0;	
	DC_only = findr      = force_recreate = 1;
	
	show_clock();
	
	head = analog_analysis(head, 0, DWGRID);
	
	matrixdata = temp;
	DC_only    = force_recreate = 0;
	useCOMPLEX = useC;
	f          = old_f;
	
	return head;
}





void create_matrices(double **&Y, double *&V, double *&J, complex **&Yc, complex *&Vc, complex *&Jc, int size) {
		
	if (useCOMPLEX) {
		Yc = create_matrix(Yc, size, size);		
		Vc = memAlloc_complex_zero(Vc, size+1);
		Jc = memAlloc_complex_zero(Jc, size+1);
	}
	else {
		Y = create_matrix(Y, size, size);		
		V = memAlloc_init(V, size+1, (double)0);
		J = memAlloc_init(J, size+1, (double)0);
	}
}




void reset_matrixdata() {

	int size;

	if (!matrixdata.useSAVE || source_variable)
		return;
		
	size = matrixdata.size;
	
	for (int i = 0; i < size; ++i) {
		if (useCOMPLEX) {
			matrixdata.Vc[i].Re = matrixdata.Vc[i].Im = 0;
			matrixdata.Jc[i].Re = matrixdata.Jc[i].Im = 0;
		}
		else {
			matrixdata.V[i] = matrixdata.J[i] = 0;
		}
	}	
	if (useCOMPLEX) {
		if (matrixdata.Yc != NULL)
			zero(matrixdata.Yc, size, size);
		if (useINV && matrixdata.invYc != NULL)
			zero(matrixdata.invYc, size, 2*size);
	}
	else {
		if (matrixdata.Y != NULL)
			zero(matrixdata.Y, size, size);
		if (useINV && matrixdata.invY != NULL)
			zero(matrixdata.invY, size, 2*size);
	}
}




void cleanup_simulation(double **Y, double *V, double *J, complex **Yc, complex *Vc, complex *Jc, complex *Ic, complex **invYc, double **invY, int size) {

	if (!source_variable /*&& (changed3 || Opened)*/) {
		delete_matrices(Y, V, J, Yc, Vc, Jc, size);
		if (useINV) {
			if (useCOMPLEX)
				delete_matrix(invYc, size);
			else
				delete_matrix(invY, size);
		}
	}
	if (Ic != NULL) {
		delete [] Ic;
	}
	changed4 = 0;
}





void delete_matrices(double **Y, double *V, double *J, complex **Yc, complex *Vc, complex *Jc, int size) {
		
	if (useCOMPLEX) {
		delete_matrix(Yc, size);
		delete [] Vc;
		delete [] Jc;
	}
	else {
		delete_matrix(Y, size);
		delete [] V;
		delete [] J;
	}
}





void get_additional_size(component *head) {
	
	int ID;
	
	additional_size = 0;
		
	while (head != 0) {
		ID = head->id;
		if (ID == POWER_ID || ID == BATTERY_ID || ID == V_SOURCE_ID || ID == VCVS_ID || ID == CCVS_ID || ID == CCCS_ID) { 
			++additional_size;
		}
		if (useCOMPLEX && (ID == V_SOURCE_ID || ID == I_SOURCE_ID) && (head->flags & BASE_CIRCUIT)) { // battery is DC only
			draw_SINEWAVE(head->location);
		}
		head = head->next;
	}
}




double get_minR(int component_type) {

	switch (component_type) {
			
		case 19:
			return Cmin;
		case 20:
			return Rmin;
		case 28:
			return Lmin;
		case -1:
			return -CS_LIMIT;
	}
	return source_min;
}



double get_maxR(int component_type) {

	switch (component_type) {
			
		case 19:
			return Cmax;
		case 20:
			return Rmax;
		case 28:
			return Lmax;
		case -1:
			return CS_LIMIT;
	}
	return source_max;
}






void find_component(component *head, int location, int id, int bode, complex *DWGRID) {
	
	int position, position1, position2, count(0), find_var(0), vertical(0), time1, stop(0), setBOUND, ID, mag_or_phase(1), component_type(0), bad_loc, ip_val;
	double known, known1, known2, comp_value(0), r1, r2, last_r(0), Rmin, Rmax, R_value(0), tolerance(default_tolerance), next_value, slope;
	char selection[10];
	component *temp = head;
	connections x;
	
	setBOUND = no_soultion = 0;
	findr = 1;
	
	draw_status_bar();
	draw_status(" Find Component Value");
	
	cout << "\n";
	
	set_special_cursor(EVAL_CURSOR);
		
	do {
		cout << "Click on the node of known voltage or the component of known current or power dissipation or the value you would like to graph:" << endl;
		
		find_var = 0;
		bad_loc  = 0;
		position = get_scrolled_window_click();			
		temp = head;
	
		while (temp != 0 && !find_var) {
			ID = temp->id;
			if (temp->location == position && (temp->flags & BASE_CIRCUIT) && (ID == RES_ID || (useCOMPLEX && AC_COMP_ID(ID)))) {
				if (position == location) {
					find_var = 1;
					R_value = 1;
				}
				if (temp->value > 0) 
					R_value = temp->value;
				
				if (!id)
					id = temp->id;
				if (temp->flags & ROTATED)
					vertical = 1;
			}
			temp = temp->next;	
		}
		if (position != CANCEL && (position < 0 || (NODENUMBER[position] <= 0 && R_value == 0))) {
			beep();
			if (NODENUMBER[position] == 0) {
				cout << "Ground is fixed at " << ground << "V." << endl;
			}
			cout << "Invalid Selection!" << endl;
			bad_loc = 1;
		}
	} while (bad_loc);
	
	reset_cursor();
	
	if (BASIC_COMP_ID(id)) // RLC passive
		component_type = id;
		
	else if (id >= VCVS_ID && id <= CCCS_ID)
		component_type = -1;
			
	Rmax = get_maxR(component_type);
	Rmin = get_minR(component_type);
		
	if (position == CANCEL) {
		cout << "\nCancel" << endl;
		return;
	}
	if (vertical)
		x = get_connections(position, RES_ID, ROTATED);
	else
		x = get_connections(position, RES_ID, (char)0);
		
	position1 = x.in2;
	position2 = x.out2;

	if (R_value == 0) {
		if (!bode)
			cout << "\nWould you like to plot a graph of voltage vs. the unknown component value?  ";

		if (bode || decision()) {
			if (bode)
				findr = 2;
			plot_circuit_variable(head, R_value, position, find_var, position1, position2, id, bode, DWGRID);
			
			reset_cursor();
			return;
		}
		cout << "\nWhat is the voltage at this location?  ";
	}
	else {
		do {
			if (bode)
				cout << "\nDo you want to plot the current through this component or the power dissipated in it?  ";
			else
				cout << "\nDo you know/want to plot the current through this component or the power dissipated in it?  ";
			strcpy(selection, xin(9));
		
			if (strcmp(selection, "I") == 0 || strcmp(selection, "i") == 0 || strcmp(selection, "C") == 0 || strcmp(selection, "c") == 0 || strcmp(selection, "Current") == 0 || strcmp(selection, "CURRENT") == 0 || strcmp(selection, "current") == 0)
				ip_val = 1;
		
			else if (strcmp(selection, "P") == 0 || strcmp(selection, "p") == 0 || strcmp(selection, "Power") == 0 || strcmp(selection, "POWER") == 0 || strcmp(selection, "power") == 0)
				ip_val = 2;
			
			else {
				beep();
				cout << "Please enter only 'i' or 'p'." << endl;
				ip_val = 0;
			}
		} while (ip_val == 0);
		
		if (!bode) {
			cout << "\nWould you like to plot a graph of ";
			if (ip_val == 1)
				cout << "current";
			else
				cout << "power";
			cout << " vs. the unknown component value? ";
		}
		if (bode || decision()) {
			if (ip_val == 2)
				R_value = -R_value;
		
			plot_circuit_variable(head, R_value, position, find_var, position1, position2, id, bode, DWGRID);
		
			reset_cursor();
			return;
		}
		if (ip_val == 1) 
			cout << "\nWhat is the current through this component(+ is L to R or Down)?  ";
		else {
			R_value = -R_value;
			cout << "\nWhat is the power dissipated in this component?  ";
		}
	}
	known = ind();
	
	if (R_value < 0 && known < 0) {
		known = -known;
		cout << "\nPower has been set to a positive value since it can't be negative." << endl;
	}
	if (R_value >= 0)
		cout << "\nWould you like to pick boundary/initial values to test(faster)?  ";
	
	if (R_value >= 0 && decision()) {
		
		setBOUND = 1;
		
		cout << "\nEnter the lower bound(actual value should lie between upper and lower bounds):  ";
		r1 = ind();
		
		if (component_type > 0 && r1 < 0)
			r1 = -r1;
		
		r1 = max(r1, Rmin);
			
		known1 = analyze_circuit(head, r1, R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
		
		cout << "\nEnter the upper bound(actual value should lie between upper and lower bounds):  ";
		r2 = ind();
		
		if (component_type > 0 && r2 < 0)
			r2 = -r2;
		
		r2 = min(r2, Rmax); 
			
		known2 = analyze_circuit(head, r2, R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
		
		if (r2 < r1) {
			swap(r2, r1);
			swap(known2, known1);
		}	
	}
	else {
		show_clock();
	
		draw_status_bar();
		draw_status(" Finding Initial Values");
		
		known1 = analyze_circuit(head, r1 = Rmin, R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
		known2 = analyze_circuit(head, r2 = Rmax, R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
				
		if (R_value < 0 && ((known1 > known && known2 > known) || (known1 < known && known2 < known))) {
			known1 = analyze_circuit(head, r1 = -char(component_type > 0), R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
			known2 = analyze_circuit(head, r2 = 1, R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID);
		}
	}
	if (r2 != r1 && fabs(known2 - known1) < v_error && R_value >= 0) {
		cout << "\nThe unknown " << get_name(id, 1) << " does not affect the known ";
		if (R_value == 0)
			cout << "voltage";
		else
			cout << "current";
		cout << ". ";
		if (fabs(known2 - known1) < v_error && fabs(known - known1) < v_error)
			cout << "Any value of this " << get_name(id, 1) << " will work." << endl;
		else
			cout << "No value of this " << get_name(id, 1) << " will work." << endl;
		reset_cursor();
		return;
	}
	show_ranges(R_value, known1, known2);
	
	reset_timer(1);
	show_elapsed_time();
	cout << "\nCalculating " << get_name(id, 1) << " - Click on Cancel to stop." << endl;
	
	draw_status_bar();
	draw_status(" Approximating Component Values - Press Cancel to stop");
	
	time1     = get_time();
	bad_error = 0;
	
	while (fabs(solve_for_value(R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID, 1.0, 0.0, 0) - known) > tolerance && last_r != comp_value && !stop && comp_value >= Rmin && comp_value <= Rmax && !bad_error) {
						
		last_r = comp_value;

		comp_value = r1 + ((r2 - r1)/(known2 - known1))*(known - known1); // Variation of Secant Method (Newton-Rhapson?) with something like Bisection
		slope = (r2 - r1)/(known2 - known1);
			
		head = analog_analysis(head, comp_value, DWGRID);
		
		next_value = solve_for_value(R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID, 1.0, 0.0, 0);
			
		if ((next_value >= known && slope >= 0) || (next_value < known && slope < 0)) {
			r2 = comp_value;
			known2 = next_value;
		}
		else if ((next_value <= known && slope >= 0) || (next_value > known && slope < 0)) {
			r1 = comp_value;
			known1 = next_value;
		}
		if (get_time() != time1) {
			if (interrupt())
				stop = 1;
			time1 = get_time();
		} 
		++count;
	}
	if (fabs(solve_for_value(R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID, 1.0, 0.0, 0) - known) > tolerance && last_r == comp_value && !stop && comp_value >= Rmin && comp_value <= Rmax && count < 5) {
		head = analog_analysis(head, 0, DWGRID);
		known1 = solve_for_value(R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID, 1.0, 0.0, 0);
		head = analog_analysis(head, comp_value, DWGRID);
		known2 = solve_for_value(R_value, position, find_var, comp_value, position1, position2, id, mag_or_phase, DWGRID, 1.0, 0.0, 0);
		comp_value = (known - known1)*(comp_value)/(known2 - known1);
		++count;
	}
	cout << "\nSimulation time:";
	show_elapsed_time();
	cout << endl;
	
	draw_status_bar();
	draw_status(" Finished");
	
	cout << "\n" << count << " iteration";
	if (count > 1)
		cout << "s";
	cout << endl;
	
	if (comp_value < Rmin || comp_value > Rmax) {
		no_soultion = 1;
		cout << "\nThe " << get_name(id, 1) << "'s value is out of range or there is no solution." << endl;
		if (setBOUND)
			cout << "You may want to try decreasing the lower bound and/or increasing the upper bound so that the required value lies between them." << endl;
	}
	else if (stop)
		cout << "\nWarning: The calculation process was interrupted, so the calculated " << get_name(id, 1) << " may not be correct." << endl;
	
	display_iterative_results(id, comp_value);
	
	reset_cursor();
}





double analyze_circuit(component *&head, double input, double R_value, int position, int find_var, double &comp_value, int position1, int position2, int ID, int mag_or_phase, complex *DWGRID) {
	
	comp_value = input;		
	bad_error  = 0;
	head       = analog_analysis(head, comp_value, DWGRID);
	return solve_for_value(R_value, position, find_var, comp_value, position1, position2, ID, mag_or_phase, DWGRID, 1.0, 0.0, 0);
}





double solve_for_value(double R_value, int position, int find_var, double comp_value, int position1, int position2, int ID, int mag_or_phase, complex *DWGRID, double ref, double offset, int bode) {
		
	if (AC_COMP_ID(ID) || bode) {
		if (mag_or_phase == 1)
			return magnitude(find_valuec(R_value, position, find_var, comp_value, position1, position2, ID, DWGRID))/fabs(ref);
		else
			return phase(find_valuec(R_value, position, find_var, comp_value, position1, position2, ID, DWGRID));
	}
	return find_value(R_value, position, find_var, comp_value, position1, position2, DWGRID) + offset;
}





double find_value(double R_value, int position, int find_var, double comp_value, int position1, int position2, complex *DWGRID) {
		
	if (R_value == 0) 
		return DWGRID[NODENUMBER[position]].Re; // voltage
		
	if (R_value > 0) {
		if (find_var) 
			R_value = comp_value;
		return (DWGRID[NODENUMBER[position1]].Re - DWGRID[NODENUMBER[position2]].Re)/R_value; // current
	}
	if (find_var)
		R_value = -comp_value;
	
	return -pow((DWGRID[NODENUMBER[position1]].Re - DWGRID[NODENUMBER[position2]].Re), 2)/R_value; // power
}




complex find_valuec(double R_value, int position, int find_var, double comp_value, int position1, int position2, int ID, complex *DWGRID) {
	
	if (R_value == 0)
		return DWGRID[NODENUMBER[position]]; // voltage
		
	if (R_value > 0) {
		if (find_var) 
			R_value = comp_value;
		return (DWGRID[NODENUMBER[position1]] - DWGRID[NODENUMBER[position2]])/impedance(ID, f, R_value); // current
	}
	if (find_var)
		R_value = -comp_value;
				
	return real_to_complex(-pow((magnitude(DWGRID[NODENUMBER[position1]]) - magnitude(DWGRID[NODENUMBER[position2]])), 2)/magnitude(impedance(ID, f, R_value))); // power
}





void show_ranges(double R_value, double known1, double known2) {

	if (R_value == 0) {
		if (known1 == known2) {
			cout << "\nThe voltage is fixed at ";
			out(known1);
			cout << "V." << endl;
		}
		else {
			cout << "\nThe voltage ranges from ";
			if (known1 < known2)
				out(known1);
			else
				out(known2);
			cout << "V to ";
			if (known1 > known2)
				out(known1);
			else
				out(known2);
			cout << "V." << endl;
		}
	}
	else if (R_value > 0) {
		if (known1 == known2) {
			cout << "\nThe current is fixed at ";
			out(known1);
			cout << "A." << endl;
		}
		else {
			cout << "\nThe current ranges from ";
			if (known1 < known2)
				out(known1);
			else
				out(known2);
			cout << "A to ";
			if (known1 > known2)
				out(known1);
			else
				out(known2);
			cout << "A." << endl;
		}
	}
}





void display_iterative_results(int ID, double comp_value) {

	cout << "\nThe value of the " << get_name(ID, 1) << " is ";
	out (comp_value);
	
	calculated_value = comp_value;
		
	switch (ID) {	
		case RES_ID:
		case CCVS_ID:
			cout << OHM_CHAR << "." << endl;
			break;
		case CAP_ID: 
			cout << "F." << endl;
			break;
		case INDUCTOR_ID:
		case XFMR_ID: 
			cout << "H." << endl;
			break;
		case I_SOURCE_ID: 
			cout << "A." << endl;
			break;
		case V_SOURCE_ID:
		case BATTERY_ID:
			cout << "V." << endl;
			break;
		case VCCS_ID:
			cout << "S." << endl;
			break;
		default:
			cout << "." << endl;
	}	
}





void plot_circuit_variable(component *head, double Value, int position, int find_var, int position1, int position2, int id, int bode, complex *DWGRID) {
	
	int stop(0), mag_or_phase(0), npoints(0);
	double input(0), start, end, delta_x, ref(1), offset(0), stopv;
	
	if (!findr)
		findr = 1;
		
	setup_analog_graph();
	
	if (bode) {
		if (Value >= 0) {
			cout << "\nCircuit Solver can plot either magnitude or phase. Do you want to plot the magnitude?  ";
			mag_or_phase = decision();
			
			if (mag_or_phase) {
				if (Value == 0) {
					cout << "\nPlease select a voltage source to use as a reference (value of 1) for the Bode plot or click on Cancel for unity gain." << endl;
					ref = get_ref_value(head, 0);
				}
				else {	
					cout << "\nPlease select a current source to use as a reference (value of 1) for the Bode plot or click on Cancel for unity gain." << endl;
					ref = get_ref_value(head, 1);
				}
			}
			//cout << "\nWould you like to use a log scale for the X axis?  ";
			//useLOG = decision();
			useLOG = 1;
		}
		else
			mag_or_phase = 1;
	}
	else
		useLOG = 0;
		
	if (useLOG && mag_or_phase == 0)
		useLOG = 2;
	
	if (has_menubar == 1) 
		delete_menubar();	
	
	reset_cursor();
	
	if (input_waveform) {
		cout << "\nTime starts at t=0. If the input waveform is periodic, you should end the simulation after a whole number of periods. What time would you like to end the simulation at (in seconds)?  ";
		end = fabs(ind());
		cout << "What power of 2 would you like to use for the number of points? Enter n, where #points = 2^n (max 14):  ";
		npoints = inpg0();
		npoints = min(14, npoints);
	}
	else {	
		set_graphing_prefs(0);
	
		if (!origin_set)
			axis(0);
	}	
	set_buttons(num_buttons, -1, 0);
	
	if (useLOG)
		min_scale = pow(10, -domain);
	
	reset_timer(1);
	show_elapsed_time();
	
	show_clock();
	
	if ((useCOMPLEX || bode) && (!bode || mag_or_phase)) {
		cout << "\nCalculating DC offset.";
		head = calc_DC_offset(head, DWGRID); 
		offset = find_value(Value, position, find_var, input, position1, position2, DWGRID);
		cout << "\nDC offset = ";
		out(offset);
		cout << "V." << endl;
	}
	show_elapsed_time();		
	bad_error = 0;
	
	if (input_waveform)
		plot_circuit_waveform(head, mag_or_phase, end, npoints, Value, position, find_var, input, position1, position2, id, DWGRID, ref, offset);
	else {
		cout << "\nCalculating Points. Click on Cancel to stop." << endl;
		draw_text_on_window(" Calculating Points.");
		init_progress_bar(1);
	
		if (id == RES_ID || bode)
			start = 0;
		else {
			start = -domain*(origin.x/x_limit);
			if (useLOG)
				start = -pow(10, -start);
		}	
		end = domain*(1 - origin.x/x_limit);
	
		if (useLOG) {
			if (id == RES_ID || bode) {
				stopv = pow(10, end);
				end   = pow(10, domain);
			}
			else {
				end   = pow(10, end);
				stopv = end;
			}	
			delta_x = pow(10, domain);
		}
		else {	
			stopv   = end;
			delta_x = (end - start)/num_points;
		}		
		for(input = start; input < stopv && Count < MAX_DATA_SIZE && !stop && !bad_error; ++Count) {
							
			if (bode)
				f = input;
			
			head = analog_analysis(head, input, DWGRID);
			
			graph_data[Count].x = input;
			graph_data[Count].y = solve_for_value(Value, position, find_var, input, position1, position2, id, mag_or_phase, DWGRID, ref, offset, bode);
					
			if (!(Count%int(num_points/AC_mod_num)))
				get_AC_waveform_cursor();
			
			if (!(Count%int(num_points/Graph_mod_num))) {
				update_progress_bar(100*Count/num_points, 1);
				if (got_cancel_click())
					stop = 1;
			}
			input = calc_next_x(start, stopv, delta_x, Count, num_points, input);
		}
		if (input < stopv) {
			if (stop)
				cout << "\nThe calculation of points was interrupted. Not all points were stored.";
			else if (bad_error) {
				cout << "\nThe calculation of points was terminated because of an error.";
			}
			else {
				beep();
				cerr << "\nNot enough graph memory to store all points on graph!";
			}
		}
	}
	update_progress_bar(100, 1);
	
	cout << "\nSimulation time:";
	show_elapsed_time();
	cout << "\n" << Count << " points on the graph were stored." << endl;
	
	reset_cursor();
	
	draw_text_on_window(" Ready.");
	wait(.1);
	whiteout();
	
	if (bad_error) {
		internal_error();
		cerr << "\nError: Circuit could not be simulated because of internal indexing errors." << endl;
	}
	else	
		graph_setup();
		
	set_buttons(num_buttons, -1, 0);
	
	if (has_menubar == 0)
		draw_menubar();
	
	needs_redraw = 1;
	min_scale = 1;
	findr = 0;
	f = 0;
}




void plot_circuit_waveform(component *head, int mag_or_phase, double end, int npoints, double Value, int position, int find_var, double input, int position1, int position2, int id, complex *DWGRID, double ref, double offset) {

	unsigned int size, i, counter(0);
	double *Re = NULL, *Im = NULL, mag, max_mag(0);
	complex *results = NULL, tran_func;
	
	size = 1 << npoints;
	num_points = size;
	origin.x = 0;
	origin.y = y_limit/2;
	domain = end;
	useLOG = 0;
	useCOMPLEX = 1;
	f_phase = 0;
	
	Re = memAlloc(Re, size);
	Im = memAlloc_init(Im, size, (double)0);
	
	stop_timer();
	reset_cursor();
	
	cout << "n = " << npoints << ", size = " << size << endl;
		
	cout << "\nEnter the input waveform, where x is time.";
	get_equation();
	
	start_timer();
	show_clock();
		
	for (i = 0; i < size; ++i) {
		if ((int)i < Count)
			Re[i] = graph_data[i].y;
		else
			Re[i] = graph_data[Count-1].y;
	}		
	fft(Re, Im, npoints, -1);
	merge_complex(results, Re, Im, size, 0);
	
	cout << "FFT done." << endl;
	
	for (i = 0; i < size; ++i) {
		mag = magnitude(results[i]);
		max_mag = max(mag, max_mag);
	}
	cout << "max magnitude = " << max_mag << endl;
	
	// multiply waveform with transfer function
	for (i = 0; i < size; ++i) {
		if (magnitude(results[i]) > min_rel_error*max_mag) {
			if (i < size/2)
				f = i/end;
			else
				f = (size - i)/end;
				
			head = analog_analysis(head, f, DWGRID);
			tran_func = find_valuec(Value, position, find_var, input, position1, position2, id, DWGRID)/fabs(ref);
			tran_func.Re += offset;
			results[i] = results[i]*tran_func;
			++counter;
		}
	}
	cout << "simulations = " << counter << endl;
	
	// IFFT results
	split_complex(results, Re, Im, size, 0);
	fft(Re, Im, npoints, 1);
	merge_complex(results, Re, Im, size, 0);
	
	cout << "IFFT done." << endl;
	
	// write back into points
	for (i = 0; i < size; ++i) {
		if (mag_or_phase == 1)
			graph_data[i].y = magnitude(results[i]);
		else
			graph_data[i].y = phase(results[i]);
	}
	for (i = size; (int)i < Count; ++i)
		graph_data[i].y = 0;
		
	cout << "Done." << endl;
	
    // cleanup
	delete [] Re;
	delete [] Im;
	delete [] results;
}





void setup_analog_graph() {

	Count = using_entered_equation = 0;
	
	if (num_points <= 0)
		num_points = default_points;
	
	if (num_points > MAX_DATA_SIZE)
		num_points = MAX_DATA_SIZE;
}





void graph_waveform(double r, double w, double o, char type) {
	
	int stop(0);
	double t, ox_over_xl;
	
	useLOG = 0;
	
	if (has_menubar == 1) 
		delete_menubar();
	
	reset_cursor();
	setup_analog_graph();
	set_graphing_prefs(0);
	set_buttons(num_buttons, -1, 0);

	ox_over_xl = origin.x/x_limit;
	
	if (useLOG)
		min_scale = pow(10, -domain);
	
	show_clock();
	
	cout << "\nCalculating Points. Click on Cancel to stop." << endl;
	
	draw_text_on_window(" Calculating Points.");
	init_progress_bar(1);
		
	for(t = -domain*ox_over_xl; t < domain*(1 - ox_over_xl) && Count < MAX_DATA_SIZE && !stop; t += domain/num_points) {
		
		graph_data[Count].x = t;
		
		if (comp_char(type, 's')) 
			graph_data[Count].y = r*sin(w*t + o);
		else if (comp_char(type, 'c'))
			graph_data[Count].y = r*cos(w*t + o);
		else
			graph_data[Count].y = 0; 
		
		if (!(Count%((int)(num_points/AC_mod_num))))
			get_AC_waveform_cursor();
		
		if (!(Count%((int)(num_points/Graph_mod_num)))) {
			update_progress_bar(100*Count/num_points, 1);
			if (got_cancel_click())
				stop = 1;
		}
		++Count;	
	}
	update_progress_bar(100, 1);
	
	if (t < domain*(1 - ox_over_xl)) {
		if (stop)
			cout << "\nThe calculation of points was interrupted. Not all points were stored.";
		else {
			beep();
			cerr << "\nNot enough graph memory to store all points on graph!";
		}
	}
	cout << "\n" << Count << " points on the graph were stored." << endl;
	
	reset_cursor();
	
	draw_text_on_window(" Ready.");
	 
	graph_setup();
	
	if (has_menubar == 0)
		draw_menubar();
	
	needs_redraw = 1;
	min_scale    = 1;
	findr        = 0;
	f            = 0;
	useLOG       = 0;
}





double get_ref_value(component *head, int iv_val) {
	
	int position, ID;
	component *temp = head;
	
	set_special_cursor(EVAL_CURSOR);
	
	while (1) {
		position = get_valid_position();
	
		if (position == -1) {
			cout << "\nAssumed gain of 1." << endl;
			return 1;
		}
		while (temp) {
	
			if (temp->location == position && (temp->flags & BASE_CIRCUIT)) {
				ID = temp->id;
				if (iv_val == 0) {
					if (ID == POWER_ID)
						return supply_voltage;
					else if (ID == BATTERY_ID || ID == V_SOURCE_ID)
						return temp->value;
				}
				else if (iv_val == 1 && ID == I_SOURCE_ID)
						return temp->value;
			}	
			temp = temp->next;
		}
		beep();
		cout << "\nInvalid component! Please select a ";
		if (iv_val == 0)
			cout << "voltage";
		else if (iv_val == 1)
			cout << "current";
		cout << " source or click on Cancel for unity gain." << endl;
		
		temp = head;
	}
	return 1;
}




datasave save_matrices(datasave& matrixdata, int useSAVE, int size, double *V, double *J, double **Y, double **invY, complex *DWGRID, complex *Vc, complex *Jc, complex **Yc, complex **invYc, component *head) {
	
	matrixdata.useSAVE = useSAVE;
	matrixdata.size    = size;
	matrixdata.V       = V;
	matrixdata.J       = J;
	matrixdata.Y       = Y;
	matrixdata.invY    = invY;
	matrixdata.DWGRID  = DWGRID;
	matrixdata.Vc      = Vc;
	matrixdata.Jc      = Jc;
	matrixdata.Yc      = Yc;
	matrixdata.invYc   = invYc;
	matrixdata.head    = head;
	
	return matrixdata;
}





void destroy(datasave &matrixdata) {

	delete_matrices(matrixdata.Y, matrixdata.V, matrixdata.J, matrixdata.Yc, matrixdata.Vc, matrixdata.Jc, matrixdata.size);
	if (useINV) {
		if (useCOMPLEX)
			delete_matrix(matrixdata.invYc, matrixdata.size);
		else
			delete_matrix(matrixdata.invY, matrixdata.size);
	}
	delete matrixdata.DWGRID;
	
	matrixdata.useSAVE = 0;
	
	matrixdata.head = reset_circuit(matrixdata.head, NULL, matrixdata.wiresave);
}



