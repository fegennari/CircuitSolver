#include "CircuitSolver.h"
#include "Complex.h"
#include "Equation.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, CircuitSolver.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Main
// By Frank Gennari
//#define GRAPHING_CALCULATOR_ONLY
#define CommandLine

int const QUIT_COUNT = 10;
string const version_ID = "1.0a";


transistor Add;
double R_storage(0), I_storage(0), V_storage(0), P_storage(0), C_storage(0), L_storage(0), F_storage(0);
double R_temp(0), I_temp(0), V_temp(0), P_temp(0), C_temp(0), L_temp(0), F_temp(0);
complex Z_storage, Z_temp;
int function_id(0), ERRORx(0), sound(1), OPEN(0), max_circuits, close_DW(0);
long global_random_seed;
string last_qname = "unknown";
char NAME[MAX_INPUT_CHARACTERS + 1] = {0};


extern point origin;
extern double domain, range, x_scale, y_scale;
extern double supply_voltage, ground;
extern int line_type, num_points, Count, clock_speed;
extern int color, graph_enable, origin_set, scale_set, enable_draw_grid, see_tracer;
extern int save_pref, delete_pref, grid_color, need_node, useDIALOG, DEMO;
extern point *graph_data;



void show_demo_message();
char menu();
char menu_selection();
char analog_menu();
void analog_menu_selection();
char additional_menu();
void additional_menu_selection();
void function_select();
void repeat(int function);
void Equivalent();
double R_equiv();
double C_equiv();
double L_equiv();
void Z_equiv();
double eq_impedance(double Z1, double Z_final);
void ohms_law();
void op_amp();
double gain_calc(double A_gain, double A_used);
double V_out_calc(double power, double ground, double V);
void OP_vstorage(double V_out, double V_ideal, double V_railed);
string Q_menu();
void transistor_calc();
double tr_resistor(string type);
int evaluate_transistor();
void rc_time();
void charge_t(double t, double R, double C, double L, int usingc);
void graph_rc(double t, double V, double R, double C, double L, int usingc, int graphmode);
void impedance();
void impedance_calc(double R, double C, double L, double F);
void TL_Impedance();
void RCL_IV(double R, double Xc, double Xl, double Z);
void RL_calc_dc(double R);
void delta_to_wye();
void filter();
void display();
void new_storage();
void quadratic();
void quad_graph(double a, double b, double c);
void base_conversion();
int base_10(int num1, int b1);
int base_n(int num2, int b2);
void inverse_square();
void ISL_graph(double intensity);
void diode_calc();
void vector_calc();
void calculator();
void clockspeed();
int read_display_bitmap();
int save();
void open();
void undo();
char quit();
void good_bye();

void runVisualizer();

double get_angle(double x, double y);

int Qsearch(transistor& Model, string Q_select, int use_status_bar);
void Q_values();

void open_graph();
void save_graph();
void graph_setup();
void scales(int Axis);

void logic();
void solve_matrix();

void conv();

void quit_app();
int got_click();
void close_drawing_window();

void create_node_array(int num_circuits, int main_window_size);

void init_graphics();

void fsm_design();

void schedule_tasks();




#ifdef WindowsGUI
INT CALLBACK WinMain (HINSTANCE instHandle, HINSTANCE prevHandle, LPSTR commandLine, INT showMode) {
#else 
	#ifdef CommandLine
	int main(int argc, char **argv) {
	#else
	int main(void) {
	#endif
#endif
	
#ifndef TEXT_ONLY
	get_cursors();
#else
	cout << "Running in text mode." << endl;
#endif
	
	global_random_seed = get_time();
	
	max_circuits = max_ic_windows*read_specs(); // not sure about this
	max_circuits = max(1, max_circuits);
	graph_data   = memAlloc(graph_data, MAX_DATA_SIZE);

	create_node_array(0, 1);
	read_data();
#ifndef TEXT_ONLY		
	intro();
	init_graphics();
#endif
	runtest();
	
#ifdef xMAC_OS
	clear_buffer();
#endif
	
	if (DEMO) {
		show_demo_message();
	}
	wait(.1);
	
#if defined(CommandLine) && !defined(WindowsGUI)
	cout << "\nCommand Line:";
	for (int i = 0; i < argc; ++i) {
		cout << " ";
		print_ch((unsigned char *)argv[i]);
	}
	cout << "\nCurrent date and time: ";
	print_date_and_time();
	if (argc > 1) {
		strcpy(NAME, argv[1]);
		OPEN = 3;
		open();
	}
#endif
					
	cout << "\n        CIRCUIT SOLVER v" << version_ID << endl
		 << "        by Frank Gennari" << endl
		 << "        _/\\/\\/\\/\\/\\_" << endl;
	
#ifdef GRAPHING_CALCULATOR_ONLY
	cout << "\nGraphing Calculator Mode only." << endl;
	function_id = 14;
	graph_setup();
	fflush(stdin);
	good_bye();
#else
	if (menu_selection() == '0') {
		good_bye();
	}
#endif
	delete [] graph_data;
	quit_app();
	
	return 0; // not actually necessary	
}
#ifdef NOT_DEFINED // never defined, just makes the function popup in CodeWarrior work
}}
#endif




void show_demo_message() {

	cout << "*** Running in DEMO mode. ***" << endl;

#ifdef xWIN32
	beep();
	demo_message();
#else	
	cout << "The DEMO version of circuit Solver has the following limitations:" << endl
		 << "* Max of " << (MAX_POINTS/5) << " graph points." << endl
		 << "* No new subcircuits or component models." << endl
		 << "* Subcircuits and component models cannot be modified." << endl
		 << "* Circuits cannot be saved." << endl
		 << "* Max of " << default_demo_windows << " subcircuits and " << default_demo_nets << " nets in simulation." << endl;
	wait(6);
#endif
}


	


char menu() {

	char select;
	
	cout << "\n\n        MAIN MENU" << endl
		 << "        ---------" << endl
		 << "	1. (A)nalog Circuits and Components" << endl
		 << "	2. (D)igital and Analog Circuit Design and Testing" << endl
		 << "	3. (G)raphing" << endl
		 << "	4. (W)ork With Stored Values" << endl
		 << "	5. (U)ndo Calculation" << endl
		 << "	6. (R)epeat Function" << endl
		 << "	7. (M)ore Selections" << endl
		 << "	8. (O)pen" << endl
		 << "	9. (S)ave Project" << endl
		 << "	0. (Q)uit" << endl
		 << "	(C)lose Graphics Window, (F)SM Design, (T)ask Scheduling, (B)itmap" << endl
		 << "\nEnter your selection: ";
		 	 
	select = inc();
	
	return select;
}




char menu_selection() {

	char selection;
		
	do {	 		
		selection = menu();		
		
		// USED: a, q, l, d, g, w, u, r, m, o, s, c, v, f, (x)							
		switch (selection) {
			
			case '0':
			case 'q':
			case 'Q':
				selection = quit();
				break;				
			case '1':
			case 'a':
			case 'A':
				analog_menu_selection();
				break;				
			case '2':
			case 'l':
			case 'L':
			case 'd':
			case 'D':
				function_id = 16;
				logic();
				fflush(stdin);
				if (close_DW) {
					close_drawing_window();
 					graph_enable = close_DW = 0;
 				}
				break;			
			case '3':
			case 'g':
			case 'G':
				function_id = 14;
				graph_setup();
				fflush(stdin);
				break;			
			case '4':
			case 'w':
			case 'W':
				display();
				break;				
			case '5':
			case 'u':
			case 'U':
				undo();
				break;				
			case '6':
			case 'r':
			case 'R':
				repeat(function_id);
				break;				
			case '7':
			case 'm':
			case 'M':
				additional_menu_selection();
				break;				
			case '8':
			case 'o':
			case 'O':
				open();
				break;				
			case '9':
			case 's':
			case 'S':
				save();
				break;				
			case 'c':
			case 'C':
				if (graph_enable == 1)
					close_drawing_window();
 				graph_enable = 0;
				break;
			case 'f':
			case 'F':
				function_id = 27;
				fsm_design();
				fflush(stdin);
				if (close_DW) {
					close_drawing_window();
 					graph_enable = close_DW = 0;
 				}
				break;
			case 't':
			case 'T':
				function_id = 28;
				schedule_tasks();
				break;
			case 'b':
			case 'B':
				function_id = 29;
				read_display_bitmap();
				break;				
			/*case 'x': // testcase
			case 'X':
				break;*/				
			default:
				beep();
				cout << "\n\nInvalid selection!" << endl;
				break;				
		} 
	} while (selection != '0');
	
	return selection;
}




char analog_menu() {

	char analog_selection;
	
	cout << "\n\n        ANALOG MENU" << endl
		 << "        -----------" << endl
		 << "	0. (R)eturn to MAIN MENU" << endl
		 << "	1. (E)quivalent Resistance, Capacitance, Inductance, and Impedance" << endl
		 << "	2. (O)hm's Law" << endl
		 << "	3. OP (A)mp Circuit" << endl
		 << "	4. (T)ransistor Circuit" << endl
		 << "	5. (D)iode Problem" << endl
		 << "	6. RC or RL Timing Circuit (X)" << endl
		 << "	7. Reactance and (I)mpedance RLC Circuit" << endl
		 << "	8. Delta to (W)ye Resistor and Impedance Conversions" << endl
		 << "	9. (F)ilter Design" << endl
		 << "\nEnter your selection: ";
		 
	analog_selection = inc();
	
	return analog_selection;
}




void analog_menu_selection() {

	char analog_selection(0);
	
	do {
		analog_selection = analog_menu();
				
		switch(analog_selection) {

			case '0':
			case 'r':
			case 'R':
				analog_selection = '0';
				break;			
			case '1':
			case 'e':
			case 'E':
				Equivalent();
				break;			
			case '2':
			case 'o':
			case 'O':
				ohms_law();
				break;				
			case '3':
			case 'a':
			case 'A':
				op_amp();
				break;				
			case '4':
			case 't':
			case 'T':
				transistor_calc();
				break;				
			case '5':
			case 'd':
			case 'D':
				diode_calc();
				break;				
			case '6':
			case 'x':
			case 'X':
				rc_time();
				break;				
			case '7':
			case 'i':
			case 'I':
				impedance();
				break;				
			case '8':
			case 'w':
			case 'W':
				delta_to_wye();
				break;				
			case '9':
			case 'f':
			case 'F':
				filter();
				break;					
			default:
				beep();
				cout << "\n\nInvalid selection!" << endl;
				break;
			}				
	} while (analog_selection != '0');				
}





char additional_menu() {
	
	char additional_selection;
	
	cout << "\n\n        ADDITIONAL MENU" << endl
		 << "        ---------------" << endl
		 << "	0. (R)eturn to MAIN MENU" << endl
		 << "	1. (S)imultaneous Equations/Matrices" << endl
	 	 << "	2. (Q)uadratic Formula" << endl
		 << "	3. (B)ase Conversion" << endl
		 << "	4. (I)nverse Square Law" << endl
		 << "	5. (V)ector Problems" << endl
		 << "	6. Clock (M)icroprocessor Speed" << endl
		 << "	7. (C)alculator" << endl
		 << "	8. Convolution/Fourier (T)ransforms" << endl
		 << "	9. Impedance Transform (Z) for Transmission Lines" << endl
		 << "	   (F)unction by number" << endl
		 << "\nEnter your selection: ";
		 
	additional_selection = inc();
	
	return additional_selection;
}





void additional_menu_selection() {

	char additional_selection(0);
	
	do {
		additional_selection = additional_menu();
				
		switch(additional_selection) {
		
			case '0':
			case 'r':
			case 'R':
				additional_selection = '0';
				break;				
			case '1':
			case 's':
			case 'S':
				solve_matrix();
				break;				
			case '2':
			case 'q':
			case 'Q':
				quadratic();
				break;				
			case '3':
			case 'b':
			case 'B':
				base_conversion();
				break;				
			case '4':
			case 'i':
			case 'I':
				inverse_square();
				break;				
			case '5':
			case 'v':
			case 'V':
				vector_calc();
				break;				
			case '6':
			case 'm':
			case 'M':
				clockspeed();
				break;				
			case '7':
			case 'c':
			case 'C':
				calculator();
				break;				
			case '8':
			case 't':
			case 'T':
				function_id = 24;
				conv();
				break;				
			case '9':
			case 'z':
			case 'Z':
				function_id = 25;
				TL_Impedance();
				break;				
			case 'f':
			case 'F':
				function_select();
				break;				
			default:
				beep();
				cout << "\n\nInvalid selection!" << endl;
				break;
		}				
	} while (additional_selection != '0');
}





void function_select() {

	int function_number, no_function;

	do {
		cout << "\nSelect a function number(1-" << MAX_FUNCTIONS << ", 0 to exit):  ";
		function_number = in();
	
		if (function_number < 0 || function_number > MAX_FUNCTIONS) {
			beep();
			cout << "\nNo such function!" << endl;
			no_function = 1;
		}
		else
			no_function = 0;
	} while (no_function);
	
	if (function_number)
		repeat(function_number);
}




void repeat(int function) {

	switch (function) {
	
		case 0:
			cout << "\nNo prior function!" << endl << endl;
			break;			
		case 1:
			Equivalent();
			break;			
		case 2:
			ohms_law();
			break;			
		case 3:
			op_amp();
			break;			
		case 4:
			transistor_calc();
			break;			
		case 5:
			Q_values();
			break;			
		case 6:
			rc_time();
			break;			
		case 7:
			impedance();
			break;			
		case 8:
			display();
			break;			
		case 9:
			quadratic();
			break;			
		case 10:
			base_conversion();
			break;			
		case 11:
			inverse_square();
			break;			
		case 12:
			undo();
			break;			
		case 13:
			open();
			break;			
		case 14:
			graph_setup();
			fflush(stdin);
			break;			
		case 15:
			save_graph();
			break;			
		case 16:
			logic();
			fflush(stdin);
			if (close_DW) {
				close_drawing_window();
 				graph_enable = close_DW = 0;
 			}
			break;			
		case 17:
			save();
			break;			
		case 18:
			open_graph();
			break;			
		case 19:
			diode_calc();
			break;			
		case 20:
			vector_calc();
			break;			
		case 21:
			delta_to_wye();
			break;			
		case 22:
			solve_matrix();
			break;			
		case 23:
			filter();
			break;			
		case 24:
			conv();
			break;			
		case 25:
			TL_Impedance();
			break;			
		case 26:
			calculator();
			break;
		case 27:
			fsm_design();
			fflush(stdin);
			if (close_DW) {
				close_drawing_window();
 				graph_enable = close_DW = 0;
 			}
			break;
		case 28:
			schedule_tasks();
			break;
		case 29:
			read_display_bitmap();
			break;		
		default:
			beep();
			cout << "\nERROR: Function could not be found!" << endl;
			break;
	}
}




void Equivalent() {

	char function;
	double R_return, C_return, L_return;
	
	function_id = 1;
	
	do {
		cout << "\n\nWould you like to solve for equivalent (r)esistance, (c)apacitance, (i)nductance, or (z)=impedance?  ";
		function = inc();
		
		if (!comp_char(function, 'r') && !comp_char(function, 'c') && comp_char(function, 'i') && comp_char(function, 'z'))
			beep();
			
	} while (!comp_char(function, 'r') && !comp_char(function, 'c') && comp_char(function, 'i') && comp_char(function, 'z'));

	if (comp_char(function, 'r')) {
		R_return = R_equiv();
				
		cout << "\nThe equivalent resistance is ";
		out(R_return);
		cout << OHM_CHAR << "." << endl;
				
		R_temp = R_storage;
		R_storage = R_return;
	}
	else if (comp_char(function, 'c')) {
		C_return = C_equiv();
		
		cout << "\nThe equivalent capacitance is ";
		out(C_return);
		cout << "F." << endl;
	
		C_temp = C_storage;
		C_storage = C_return;
	}
	else if (comp_char(function, 'i')) {
		L_return = L_equiv();
		
		cout << "\nThe equivalent inductance is ";
		out(L_return);
		cout << "H." << endl;
	
		L_temp = L_storage;
		L_storage = L_return;
	}
	else {
		Z_equiv();
	}
}




double R_equiv() {

	double r, R_final(0);
		
	do {
		cout << "\nEnter a resistance value(+# for series; -# for parallel; 0 to end): "; 
		r = ind();					
		R_final = eq_impedance(-r, R_final);
					
	} while (r != 0);
				
	return R_final;
}




double C_equiv() {

	double c, C_final(0);
	
	do {
		cout << "\nEnter a capacitance value in F(+# for series; -# for parallel; 0 to end): "; 
		c = ind();
		C_final = eq_impedance(c, C_final);		
						
	} while (c != 0);
				
	return C_final;
}




double L_equiv() {

	double l, L_final(0);
	
	do {
		cout << "\nEnter an inductance value in H(+# for series; -# for parallel; 0 to end): "; 
		l = ind();
		L_final = eq_impedance(-l, L_final);
		
	} while (l != 0);
				
	return L_final;
}




double eq_impedance(double Z1, double Z_final) {

	if (Z1 == 0)
		return Z_final;

	if (Z1 < 0)
		return -Z1 + Z_final;
	
	if (Z_final == 0)
		return Z1;

	return 1/((1/Z1) + (1/Z_final));
}




void ohms_law() {

	double r_ohm, v_ohm, i_ohm, p_ohm;
	int check1(0);
	char iv;
	
	function_id = 2;
	
	cout << "\n\nEnter the resistance(0 to use stored value; -# for multiple resistors): ";
	r_ohm = ind();
				
	if (r_ohm < 0)
		r_ohm = R_equiv();
				
	if (!r_ohm) 
		r_ohm = R_storage;
					
	do {
		cout << "\nDo you know i or v? ";
		iv = inc();
				
		if (comp_char(iv, 'i')) {
			cout << "\nEnter the current(0 to use stored value): ";
			i_ohm = ind();
					
			if (i_ohm == 0) 
				i_ohm = I_storage;
						
			v_ohm = i_ohm*r_ohm;
			p_ohm = i_ohm*v_ohm;
			check1 = 1;
		}
		else if (comp_char(iv, 'v')) {
			cout << "\nEnter the voltage(0 to use stored value): ";
			v_ohm = ind();
					
			if (v_ohm == 0)
				v_ohm = V_storage;
						
			i_ohm = v_ohm/r_ohm;
			p_ohm = i_ohm*v_ohm;
			check1 = 1;
		}
		else {
			cout << "\nOhms Law cannot be calculated!" << endl;
			beep();
			check1 = 0;
		}
	} while (!check1);
				
	if (r_ohm == 0) 
		p_ohm = 0;
								
	cout << "\nThe resistance is ";
	out(r_ohm);
	cout << OHM_CHAR << "." << endl << "\nThe voltage is ";
	out(v_ohm);
	cout << "V." << endl << "\nThe current is ";
	out(i_ohm);
	cout << "A." << endl << "\nThe power is ";
	out(p_ohm);
	cout << "W." << endl
		 << "\nDo you want to store these values of I,V, and P in memory?  ";
					 
	if (decision()) {
		V_temp = V_storage;
		I_temp = I_storage;
		P_temp = P_storage;
		 
		V_storage = v_ohm;
		I_storage = i_ohm;
		P_storage = p_ohm;
	}
}




void op_amp() {
	
	double V_out, V_ideal;
	double vcc, vss, vs, vin_p, vin_n, offset(0), bias(0), R_feedback, R_input, R_out(0), biasr;
	double gain, gain2, R_ratio, A_used(A), Ra, Rb;
	int invalid_v(0), nonideal;
	
	function_id = 3;
	
	cout << "\nCalculate NON-Ideal OP Amp?  ";
	nonideal = decision();
	
	cout << "\nWhat is Vcc(+V)? ";
	vcc = ind();
	cout << "What is Vss(-V)? ";
	vss = ind();
				
	if ((vcc - vss < 3 && nonideal) || vcc <= vss) {
		beep();
		cout << "\nSupply voltage is insufficient for OP Amp to function properly!" << endl
			 << "Will try it anyway." << endl << endl;
		invalid_v = 1;
	}	 
	else
		invalid_v = 0;	
	
	if (nonideal) {		
		cout << "Enter the offset voltage in volts: ";
		offset = ind();
	
		cout << "Enter the bias current in amps: ";
		bias = ind();
		
		cout << "Enter A (0 to use default of 1x10-6): ";
		A_used = fabs(ind());
		if (A_used == 0)
			A_used = A;
			
		cout << "Enter the internal output resistance Ro: ";
		R_out = fabs(ind());
	}
	cout << "Enter R-feedback from -Vin to Vo(-# for multiple resistors): ";
	R_feedback = ind();
	
	if (R_feedback < 0) {
		R_feedback = R_equiv();
		
		cout << "\nThe equivalent feedback resistor is ";
		out(R_feedback);
		cout << OHM_CHAR << "." << endl;
	}
	
	cout << "Enter R-input from -Vin to -Input of OP Amp(-# for multiple resistors): ";
	R_input = ind();
		
	if (R_input < 0) {
		R_input = R_equiv();
		
		cout << "\nThe equivalent input resistor is ";
		out(R_input);
		cout << OHM_CHAR << "." << endl;
	}
	if (R_input == 0) 
			R_input = almost_zero;
	
	R_ratio = R_feedback/R_input;
	
	cout << "Enter R between +Vin and +Input of OP Amp(-# for multiple resistors): ";
	Ra = ind();
	
	if (Ra < 0) {
		Ra = R_equiv();
		
		cout << "\nThe equivalent resistor is ";
		out(Ra);
		cout << OHM_CHAR << "." << endl;
	}
	if (Ra == 0) 
			Ra = 1e20;
	
	cout << "Enter R between +Vin and ground(-# for multiple resistors, 0 for none): ";
	Rb = ind();
	
	if (Rb < 0) {
		Rb = R_equiv();
		
		cout << "\nThe equivalent feedback resistor is ";
		out(Rb);
		cout << OHM_CHAR << "." << endl;
	}	
	gain = gain_calc(R_ratio, A_used);
	
	do {
		cout << "Enter +Vin: ";
		vin_p = ind();
		cout << "Enter -Vin: ";
		vin_n = ind();
		
		if (Rb != 0)
			vin_p *= Rb/(Rb + Ra);
				
		if (vin_n == 0) {
			vs = vin_p;
			gain2 = R_ratio;
		}
					
		else {
			vs = -vin_n;
			gain2 = -R_ratio;
		}
		V_out = (vin_p - vin_n)*fabs(gain) + offset*(R_input + R_feedback)/R_input + bias*R_feedback - bias*Rb*(R_input + R_feedback)/R_input;
						
		V_ideal = vin_p + (vin_p - vin_n)*R_ratio + offset*(R_input + R_feedback)/R_input + bias*R_feedback - bias*Rb*(R_input + R_feedback)/R_input;
		
		biasr = eq_impedance(R_feedback, R_input);
				
		if (vin_n > vin_p)
			gain = -gain;		
		
		if (R_feedback < 0)
			vs = -vs;
				
		else if (R_feedback == 0) {
			V_out = (vin_p - vin_n);
			V_ideal = vs;
		}
		else if (R_feedback >= 1e20) {
			V_out = A_used*(vin_p - vin_n);
			V_ideal = A_used*vs;
		}
		if (R_feedback == 0 && vin_n != 0) {
			V_ideal = V_out = vin_n;
			gain = 1;
		}		
		if (invalid_v)
			cout << "\nThe OP Amp is low on supply voltage, so the values "
				 << "may be incorrect!" << endl;
		
		if (nonideal) {				
			cout << "\nThe gain of the NON-Ideal OP Amp is " << gain << "." << endl
			 	 << "The output voltage of the NON-Ideal OP Amp is ";
			out(V_out_calc(vcc, vss, V_out));
			cout << "V." << endl;
		}
		cout << "\nThe gain of the Ideal OP Amp is " << gain;
		if (vin_p != 0 && vin_n == 0 && R_feedback != 0) {
			if (gain > 0)
				cout << "(+1)";
			else
				cout << "(-1)";
		}
		cout << "." << endl
			 << "The output voltage of the Ideal OP Amp is ";
		out(V_ideal);
		if (V_ideal > vcc || V_ideal < vss) {
			cout << "V railed at ";
			if (V_ideal > vcc)
				out(vcc);
			else
				out(vss);
		}
		cout << "V." << endl;
		if (nonideal) {
			if (biasr != 0) {
				cout << "\nThe suggested bias resistor value(connected between +Vin and ground) is ";
				out(biasr);
				cout << OHM_CHAR << "." << endl;
			}
			if (R_out != 0) {
				cout << "\nThe equivalent output resistance as seen from an external load is ";
				out((1 + R_ratio)*R_out/A_used);
				cout << OHM_CHAR << "." << endl;
			}
		}	
		cout << "\nWould you like to store the";
		if (nonideal)
			cout << " (n)on-ideal,";
		cout << " (i)deal";
		if (nonideal)
			cout << ",";
		cout << " or (r)ailed ideal" 
			 << " output voltage (anything else stores none)? ";
			 
		OP_vstorage(V_out, V_ideal, V_out_calc(vcc, vss, V_ideal));
					 
		cout << "\nWould you like to solve the OP Amp circuit again with new input voltages?  ";
						 
	} while (decision());
}




double gain_calc(double A_gain, double A_used) {
						
	if (!A_gain)
		A_gain = 1;
				
	if (A_gain > A_used)
		A_gain = A_used;
					
	if (A_gain < -A_used)
		A_gain = -A_used;
	
	return fabs(A_gain);
}




double V_out_calc(double power, double ground, double V) {

	if (power > -.6) {
		if (V > (power - .6))
			V = power - .6;
	}	
	else {
		if (V > (power + .6))
		V = power + .6;
	}		
	if (ground < 1.2) {	
		if (V < (ground + 1.2))
			V = ground + 1.2;
	}		
	else {
		if (V < (ground - 1.2))
			V = ground - 1.2;
	}
	return V;
}




void OP_vstorage(double V_out, double V_ideal, double V_railed) {

	char value = inc();
	
	switch (value) {
	
		case 'n':
		case 'N':
			V_temp = V_storage;
			V_storage = V_out;
			break;
					
		case 'i':
		case 'I':
			V_temp = V_storage;
			V_storage = V_ideal;
			break;
			
		case 'r':
		case 'R':
			V_temp = V_storage;
			V_storage = V_railed;
			break;
											
		default:
			cout << "No voltage was put into storage." << endl;
			break;
	}
}

	
	

string Q_menu() {

	string choice;
		
	cout << "\nPlease enter the part number, all lowercase(NTE cross referenced) of the transistor to"
		 << " be used or: " << endl
		 << "	(N)ew to add to datafile" << endl
		 << "	(R)ecall to use new transistor" << endl
		 << "	(A)gain to use the same transistor as before" << endl
		 << "	(S)pecs to enter ideal values" << endl
		 << "	(M)enu to return to menu" << endl
		 << "\nWhat is your selection? ";
		 
	choice = xin(MAX_INPUT_CHARACTERS);
	
	return choice;
}




void transistor_calc() {

	double q_vcc, q_vbe, q_vcesat, q_beta, q_rb, q_re, q_rc, q_vcemax(almost_infinite), q_vbemax(almost_infinite), G(0);
	double q_icmax(almost_infinite), q_cbmax(almost_infinite), q_pdmax(almost_infinite), q_tf(0), amp, efficiency1, efficiency2;
	double Q_Ib, Q_Ic, Q_Ie, Q_Vce, Q_Vcr, Q_Ver, Q_Pq, Q_Pb, Q_Pc, Q_Pe, Q_Ps, x_max, x_min;
	double Q_vin, Q_Ve, Q_Vc, input_sat(0);  
	int check2(1), warning(0), d_var, graph_q, no_store; 
	char q('i');
	string Q_name, Q_select, name;
	transistor Model;
	
	function_id = 4;

	do {
		q_vcemax = q_vbemax = q_icmax = q_cbmax = q_pdmax = almost_infinite;
		q_tf = graph_q = no_store = 0;
		q = 'i';
		
		Q_select = Q_menu();
		
		if (Q_select == "a" || Q_select == "A")
			Q_select = last_qname;
					
		if (Q_select == "m" || Q_select == "M") {
			check2 = 0;						
			break;
		}					
		else if (Q_select == "r" || Q_select == "R") {
			
			Q_select = Add.name;								
			q = Add.z;
			q_icmax = Add.q_icmax;
			q_cbmax = Add.q_cbmax;
			q_vcemax = Add.q_vcemax;
			q_vbemax = Add.q_vbemax;
			q_beta = Add.q_beta;
			q_pdmax = Add.q_pdmax;
			q_tf = Add.q_tf;
			q_vbe = Add.q_vbe;
			q_vcesat = Add.q_vcesat;
			
			if (comp_char(q, 'n'))
				cout << "\nThe transistor's specs have been put into memory." << endl;				
			else
				cout << "\nThe transistor was not found!" << endl;
			
			check2 = 1;
		}					
		else if (Q_select == "n" || Q_select == "N" || Q_select == "New" || Q_select == "NEW") {			
			Q_values();			
			function_id = 4;					 	
			q = 'j';
			check2 = 1;	
		}
		else if (Q_select == "s" || Q_select == "S") {
			cout << "\nIs the transistor (n)pn or(p)np? ";
			q = inc();
		}
		else {
			cout << "\nOpening/Searching component data";
					
			if (Qsearch(Model, Q_select, 2) > 0) {
						
				q = Model.z;
				q_icmax = Model.q_icmax;
				q_cbmax = Model.q_cbmax;
				q_vcemax = Model.q_vcemax;
				q_vbemax = Model.q_vbemax;
				q_beta = Model.q_beta;
				q_pdmax = Model.q_pdmax;
				q_tf = Model.q_tf;
				q_vbe = Model.q_vbe;
				q_vcesat = Model.q_vcesat;
			}		
		}
		if (comp_char(q, 'n')) {							
			cout << "\nWhat is Vcc(0 to use stored value)? ";
			q_vcc = ind();
			
			cout << "What is ground(Vss)?  ";
			G = ind();
					 	
			if (Q_select == "s" || Q_select == "S") {
				cout << "Enter V B-E: ";
				q_vbe = ind();
				cout << "Enter V C-E sat: ";
				q_vcesat = ind();
				cout << "Enter beta: ";
				q_beta = ind();
			}
			q_rb = tr_resistor("base");			
			q_re = tr_resistor("emitter");			
			q_rc = tr_resistor("collector");
					 	
			check2 = 0;
					 	
			if (q_vcc <= G) {
				beep();			
				cout << "\nVcc must be greater than Ground for the circuit to function properly."
					 << endl << "The transistor is reverse biased, and the calculated values"
					 << " may not be correct!" << endl;
					 
				warning = 1;
			}
			else
				warning = 0;
			
			x_max = q_vcc;
			x_min = G;
					 	
			if (comp_char(q, 'p')) {
				swap(q_vcc, G);
			}		 		 			 					 			 					 			 					 	
			if (q_vbe < 0)
				q_vbe = default_vbe;
					 		
			if (q_vcesat < 0)
				q_vcesat = default_vcesat;
					 	
			if (!q_vcc)
				q_vcc = V_storage;
				
			if (comp_char(q, 'n')) 
				cout << "\nThe transistor is on when Vin > " << q_vbe + G << " V." << endl;			
			else 
				cout << "\nThe transistor is on when Vin < " << G - q_vbe << " V." << endl;					 		
											 		
			do {
				if (!graph_q) {
					cout << "\nWould you like to evaluate this transistor circuit for all possible input voltages"
					 	 << "(This must be done before the circuit can be graphed)?  ";
					 
					graph_q = decision();
					
					if (!graph_q)
						graph_q = 2;
				}				
				if (graph_q == 1) {				
					Count = 0;					
					scales(3);					
					d_var = evaluate_transistor();
				}
				else {			
					cout << "\nEnter Vin(0 to use stored value): ";				
					Q_vin = ind();
					 		
					if (!Q_vin)
						Q_vin = V_storage;
				}				
				if (graph_q == 1)
					Q_vin = x_min;
				
				do {										
					if (((Q_vin - q_vbe - G) <= 0 && comp_char(q, 'n')) || ((G - Q_vin - q_vbe) <= 0 && comp_char(q, 'p')) || warning) {					
						if (graph_q != 1)
							cout << "\nThe transistor is cutoff:" << endl; // cutoff
							
						Q_Ib = Q_Ic = Q_Ie = Q_Vcr = Q_Ver = 0;		
						Q_Vc = q_vcc;
						Q_Vce = fabs(q_vcc - G);
						Q_Ve = G;
					}
					else {
						if (comp_char(q, 'n')) 
							Q_Ib = (Q_vin - q_vbe - G)/((q_beta + 1)*q_re + q_rb);					
						else
							Q_Ib = (G - Q_vin - q_vbe)/((q_beta + 1)*q_re + q_rb);
						
						Q_Ic = q_beta*Q_Ib;
						Q_Ie = (q_beta + 1)*Q_Ib;					
						Q_Vcr = Q_Ic*q_rc;					
					
						if (comp_char(q, 'n')) {
							Q_Vc = q_vcc - Q_Vcr;
							Q_Vce = Q_Vc - Q_Ie*q_re - G;
							Q_Ve = Q_Vc - Q_Vce;
						}						
						else {
							Q_Vc = q_vcc + Q_Vcr;
							Q_Vce = G - Q_Ie*q_re - Q_Vc;
							Q_Ve = Q_Vc + Q_Vce;
						}					
						Q_Ver = fabs(Q_Ve - G);
										
						if (Q_Vce > q_vcesat) {					
							if (graph_q != 1) 
								cout << "\nThe transistor is forward active." << endl << endl; // forward active
						
						if (graph_q != 2 && comp_char(q, 'n'))
							input_sat = Q_vin;																			
						}							
						else {				
							if (graph_q != 1)
								cout << "\nThe transistor is saturated." << endl << endl; // saturated
					
							Q_Vce = q_vcesat;
							q_vbe += VcesatExtra;
															
							if (comp_char(q, 'n')) {
								Q_Ie = (q_rc*(Q_vin - q_vbe - G) + q_rb*(q_vcc - Q_Vce - G))/(q_rb*q_re + q_rc*q_re + q_rc*q_rb);
								Q_Ic = (q_vcc - Q_Vce - Q_Ie*q_re - G)/q_rc;
							}						
							else {
								Q_Ie = (q_rc*(G - Q_vin - q_vbe) + q_rb*(G - q_vcc - Q_Vce))/(q_rb*q_re + q_rc*q_re + q_rc*q_rb);
								Q_Ic = (G - q_vcc - Q_Vce - Q_Ie*q_re)/q_rc;
							}						
							Q_Ib = Q_Ie - Q_Ic;
							Q_Vcr = Q_Ic*q_rc;						
						
							if (comp_char(q, 'n')) {
								Q_Vc = q_vcc - Q_Vcr;
								Q_Ve = Q_Vc - Q_Vce;
							}														
							else {
								Q_Vc = q_vcc + Q_Vcr;											
								Q_Ve = Q_Vc + Q_Vce;
							}						
							Q_Ver = fabs(Q_Ve - G);
							
							if (graph_q != 2 && comp_char(q, 'p'))
								input_sat = Q_vin;

							q_vbe -= VcesatExtra;						
						}
					}										
					graph_data[Count].x = Q_vin;
					
					if (d_var)
						graph_data[Count].y = Q_Ve;
					else
						graph_data[Count].y = Q_Vc;
						
					Q_vin += (x_max - x_min)/num_points;
					
					if (graph_q == 1)
						++Count;
				
				} while (graph_q == 1 && (Q_vin < (x_max - (x_max - x_min)/num_points) && Q_vin > x_min) && Count < MAX_DATA_SIZE);
				
				if (graph_q != 2 && ((q_vcc - input_sat > (q_vcc - G)/1000) && (input_sat - G > (q_vcc - G)/1000))) {	
					cout << "\nThe transistor switches from forward active to saturated when Vin = ";
					out(input_sat);
					cout << "V." << endl;
				}
				
				if (graph_q == 1) {
					cout << "\nThe transistor circuit has been analyzed. " << Count << " points were calculated."
						 << endl << "\nWould you like to graph the Vin vs. Vout curve of this circuit?  ";
					
					if (decision()) {
						graph_setup();						
						cout << "\nReturn to analog menu?  ";
						
						if (decision()) {
							check2 = 0;
							no_store = 2;
							break;
						}
					}
				}				
				if (graph_q == 2) {													
					Q_Pq = q_vbe*Q_Ib + Q_Vce*Q_Ic;
					Q_Pb = Q_Ib*Q_Ib*q_rb;
					Q_Pc = Q_Ic*Q_Ic*q_rc;
					Q_Pe = Q_Ie*Q_Ie*q_re;
					Q_Ps = fabs(Q_Ic*(q_vcc - G) + (Q_vin - G)*Q_Ib);
				
					amp = Q_Ic/Q_Ib;
				
					if (!Q_Ib)
						amp = 0;
						
					efficiency1 = (Q_Pc/Q_Ps)*100;
					efficiency2 = (Q_Pe/Q_Ps)*100;
				
					if (Q_Ps == 0) {
						efficiency1 = 0;
						efficiency2 = 0;
					}						
					cout << "\nAmplification is " << amp << "." << endl << "Ib = ";
					out(Q_Ib); 
					cout << "A" << endl << "Ic = ";
					out(Q_Ic); 
					cout << "A" << endl << "Ie = ";
					out(Q_Ie); 
					cout << "A" << endl << "Vce = "; 
					out(Q_Vce); 
					cout << "V" << endl << "The voltage at the emitter is "; 
					out(Q_Ve); 
					cout << "V" << endl << "The voltage at the collector is "; 
					out(Q_Vc);
					cout << "V" << endl << "The voltage across the collector resistor is ";
					out(Q_Vcr);
					cout << "V." << endl << "The voltage across the emitter resistor is ";
					out(Q_Ver);
					cout << "V." << endl << "The power dissipated in the transistor is ";
					out(Q_Pq);
					cout << "W." << endl << "The power dissipated in the base resistor(s) is ";
					out(Q_Pb);
					cout << "W." << endl << "The power dissipated in the collector resistor(s) is ";
					out(Q_Pc);
					cout << "W." << endl << "The power dissipated in the emitter resistor(s) is ";
					out(Q_Pe);
					cout << "W." << endl << "The power supplied by the source vcc is ";
					out(Q_Ps);
					cout << "W." << endl << "The supplied power dissipated in the collector resistor(s) is " 
				 		 << efficiency1 << "%." << endl
				 		 << "The supplied power dissipated in the emitter resistor(s) is "
				 		 << efficiency2 << "%." << endl << endl;
				 	 
					if ((comp_char(q, 'n') && Q_vin > q_vcc) || (comp_char(q, 'p') && Q_vin > G))
						cout << "Warning: The input voltage is greater than the supply voltage vcc!"
							 << endl;
						 
					if ((comp_char(q, 'n') && Q_vin < G) || (comp_char(q, 'p') && Q_vin < q_vcc))
						cout << "Warning: The input voltage is less than ground (vss)!"
							 << endl;					
				
					if (Q_Ic > q_icmax)
						cout << "Warning: The collector current is above the rated value of this transistor!" 
							 << endl;
				
					if ((Q_Vce - q_vbemax) > q_cbmax)
						cout << "Warning: The collector-base voltage is above the rated value of this transistor!"
							 << endl;
				
					if (q_vbe > q_vbemax)
						cout << "Warning: The base-emitter voltage is above the rated value of this transistor!"
							 << endl;
				
					if (Q_Vce > q_vcemax)
						cout << "Warning: The col.-emitter voltage is above the rated value of this transistor!"
							 << endl;
				
					if (Q_Pq > q_pdmax)
						cout << "Warning: The power dissipated in the transistor is above its rated value!"
							 << endl;
							  	 	
					cout << "\nWould you like to repeat the calculations using a different Vin?  ";					
					no_store = 0;
				}
				else {
					graph_q = 2;
					no_store = 1;					
					cout << "\nWould you like to solve the transistor circuit with a specific Vin?  "; 	
				}		
			} while (decision());
			
			if (graph_q != 1 && no_store != 1) {
				cout << "\nWould you like to store Vc in memory?  ";			
				if (decision()) {
					V_temp = V_storage;
					V_storage = Q_Vc;
				}
				else {
					cout << "\nWould you like to store Ve in memory?  ";				
					if (decision()) {
						V_temp = V_storage;
						V_storage = Q_Ve;
					}
				}
			}
			if (no_store != 2)
				check2 = 1;							
		}
		else if (!comp_char(q, 'j')) {
			beep();
			cout << "\nCircuit Solver can only calculate values for npn and pnp BJTs!" << endl;				 
			check2 = 1;
		}
	} while (check2);
}





double tr_resistor(string type) {
	
	double resistor;
	
	cout << "Enter the " << type << " resistor(-# for multiple resistors): ";
	resistor = ind();
			
	if (resistor < 0) {
		resistor = R_equiv();
				
		cout << "\nThe equivalent " << type << " resistor is ";
		out(resistor);
		cout << OHM_CHAR << "." << endl;
	}
	return resistor;
}





int evaluate_transistor() {

	char evaluate;	
	int output;
	
	do {											
		cout << "\nWould you like to evaluate the circuit in terms of the voltage at the emitter(e) or collector(c)?  ";
						 					
		evaluate = inc();
						
		if (comp_char(evaluate, 'e'))
			output = 1;							
		else if (comp_char(evaluate, 'c'))
			output = 0;							
		else {
			beep();
			cout << "Invalid selection! Please enter only c or e." << endl;
		}						
	} while (!comp_char(evaluate, 'e') && !comp_char(evaluate, 'c'));
	
	return output;	
}





void rc_time() {
	
	char function[4];
	int usingc, lc_val;
	double R, C, L;
	double t, T, F;
	
	function_id = 6;
	
	cout << "\n\nEnter R in ohms(-# for multiple, 0 for stored value):  ";
	R = ind();
	
	if (!R)
		R = R_storage;
		
	if (R < 0) {
		R = R_equiv();
		
		cout << "\nThe equivalent resistance is ";
		out(R);
		cout << OHM_CHAR << "." << endl;
	}	
	do {
		cout << "\n\nWould you like to solve a RC (c) or RL (l) circuit?  ";
		strcpy(function, xin(3));
		
		if (strcmp(function, "l") == 0 || strcmp(function, "L") == 0 || strcmp(function, "rl") == 0 || strcmp(function, "RL") == 0 || strcmp(function, "Rl") == 0 || strcmp(function, "rL") == 0)
			lc_val = 1;		
		else if (strcmp(function, "c") == 0 || strcmp(function, "C") == 0 || strcmp(function, "rc") == 0 || strcmp(function, "RC") == 0 || strcmp(function, "Rc") == 0 || strcmp(function, "rC") == 0)
			lc_val = 2;			
		else {
			beep();
			cout << "Please enter only 'l' or 'c'." << endl;
			lc_val = 0;
		}
	} while (lc_val == 0);
	
	if (lc_val == 2) {
		usingc = 1;	
		cout << "\nEnter C in Farads(-# for multiple, 0 for stored value):  ";
		C = ind();
	
		if (!C)
			C = C_storage;
		
		if (C < 0) {
			C = C_equiv();
		
			cout << "\nThe equivalent capacitance is ";
			out(C);
			cout << "F." << endl;
		}	
		t = R*C;
	}
	else {
		usingc = 0;
		
		cout << "\nEnter L in Henrys(-# for multiple, 0 for stored value):  ";
		L = ind();
	
		if (!L)
			L = L_storage;
		
		if (L < 0) {
			L = L_equiv();
		
			cout << "\nThe equivalent inductance is ";
			out(L);
			cout << "H." << endl;
		}	
		t = L/R;
	}
	T = -log(.5)*t;
	F = 1/T;
	
	if (T == 0)
		F = 0;
	
	cout << "\nTime Constant t(";
	if (usingc)
		cout << "RC";
	else
		cout << "LC";
	cout << ") = " << t << endl << "Time = ";
	out(T);
	cout << "s" << endl << "Frequency = ";
	out(F);
 	cout << "Hz" << endl;
	
	if (usingc)
		cout << "\nWould you like to find the voltage across the capacitor and current through the capacitor in terms of time?  ";
	else
		cout << "\nWould you like to find the current through the inductor in terms of time?  ";
	if (decision())
		charge_t(t, R, C, L, usingc); 	
}




void charge_t(double t, double R, double C, double L, int usingc) {
	
	int graph(0), selection;
	double Vo, Vc, Is, Ip, time;
	
	if (t == 0)
		t = almost_zero;
	
	if (usingc)
		cout << "\nEnter Vo (max voltage across capacitor):  ";
	else
		cout << "\nEnter Io (max current through inductor):  ";
	Vo = ind();
	
	do {
		if (!graph) {
			cout << "\nWould you like to plot a graph?  ";			
			if (decision()) {				
				graph = 1;
				
				cout << "\n1. ";			
				if (usingc)
					cout << "Vc";
				else
					cout << "IL";
				cout << " vs. Time" << endl << "2. Power vs. Time" << endl << "3. Energy vs. Time" << endl;
				
				do {
					cout << "\nEnter your selection:  ";
					selection = in();
					
					if (selection == 1 || selection == 2 || selection == 3)
						graph_rc(t, Vo, R, C, L, usingc, selection);
					else {
						beep();
						cout << "Invalid Selection! ";
					}
				} while (selection != 1 && selection != 2 && selection != 3);
					
				cout << "\nReturn to analog menu?  ";
						
				if (decision())
					break;	
			}
		}
		cout << "\nEnter the time(+ for charging, - for discharging):  ";
		time = ind();
	
		if (time >= 0) 
			Vc = Vo*(1 - exp(-time/t));
		
		else 
			Vc = Vo*exp(time/t);
		
		if (usingc) {
			cout << "\nThe voltage across the capacitor is ";
			out(Vc);
			cout << "V." << endl;
		}
		else {
			cout << "\nThe current through the inductor is ";
			out(Vc);
			cout << "A." << endl;
		}
		if (usingc) {
			Is = (Vo - Vc)/R;
			Ip = Vc/R;
		
			cout << "\nThe current through the series form of this RC circuit is ";
			out(Is);
			cout << "A.";
			cout << "\nThe current through the parallel form of this RC circuit is ";
			out(Ip);
			cout << "A.";
		
			cout << "\n\nWould you like to store the voltage?  ";
		
			if (decision()) {
				V_temp = V_storage;
				V_storage = Vc;
			}
			cout << "\nWould you like to store the series current?  ";
		
			if (decision()) {
				I_temp = I_storage;
				I_storage = Is;
			}
			else {
				cout << "\nWould you like to store the parallel current?  ";
		
				if (decision()) {
					I_temp = I_storage;
					I_storage = Ip;
				}		
			}
		}
		else {
			Is = Vc*R;
		
			cout << "\nThe voltage across the resistor of the series form of this LC circuit is ";
			out(Is);
			cout << "V.";
		
			cout << "\n\nWould you like to store the current?  ";
		
			if (decision()) {
				I_temp = I_storage;
				I_storage = Vc;
			}
			
			cout << "\n\nWould you like to store the voltage?  ";
		
			if (decision()) {
				V_temp = V_storage;
				V_storage = Is;
			}
		}
		cout << "\nWould you like to calculate ";
		if (usingc)
			cout << "Vc";
		else
			cout << "IL";

		cout << " at a different time?  ";

	} while (decision());
}




void graph_rc(double t, double V, double R, double C, double L, int usingc, int graphmode) {

	double ct1, ct2, dt1, dt2, t_min, t_max, T, last_c(V), last_d(V), init_v(0), last;

	cout << "\nEnter the interval for charging(t1, t2):  ";
	ct1 = ind();
	ct2 = ind();
	cout << "Enter the interval for discharging(t1, t2):  ";
	dt1 = ind();
	dt2 = ind();
	
	if (ct1 > ct2) {
		swap(ct1, ct2);
	}
	if (dt1 > dt2) {
		swap(dt1, dt2);
	}
	if (ct1 < dt1 && dt1 < ct2)
		dt1 = ct2;
		
	if (dt1 < ct1 && ct1 < dt2)
		ct1 = dt2;
		
	if (ct1 < dt1 || (ct1 == dt1 && dt2 > ct2)) {
		t_min = ct1;
		t_max = dt2;
	}
	else {
		t_min = dt1;
		t_max = ct2;
	}
	scales(3);		
	Count = 0;
	
	for (T = t_min; T < t_max && Count < MAX_DATA_SIZE; T += (t_max - t_min)/num_points) {
	
		graph_data[Count].x = T;
		
		if (ct1 <= T && T <= ct2 && ct1 != ct2) { // charging
		
			last_c = init_v + V*(1 - exp(-(T - ct1)/t));
			last = last_c;
		}	
		else if (dt1 <= T && T <= dt2 && dt1 != dt2) { // discharging
		
			init_v = last_c*exp(-((T - dt1)/t));
			last_d = init_v;
			last = last_d;
		}	
		else {		
			if (ct1 < dt1 || (ct1 == dt1 && dt2 > ct2))
				last = last_c;				
			else 
				last = last_d;	
		}
		switch (graphmode) {
				
			case 1:
				graph_data[Count].y = last;
				break;					
			case 2:
				if (usingc) 
					graph_data[Count].y = -last*last/R;
				else
					graph_data[Count].y = -last*last*R;
				break;					
			case 3:
				if (usingc) 
					graph_data[Count].y = .5*last*last*C;
				else
					graph_data[Count].y = .5*last*last*L;
				break;
		}			
		++Count;
	}
	cout << "\nThe curve has been calculated. " << Count << " points were stored." << endl;	
	
	graph_setup();
}




void impedance() {
	
	string acdc;
	int ACDC(0);
	double R, C, L, F, Fr, AV;
	
	function_id = 7;
	
	while (ACDC == 0) {	
		cout << "\nIs the supply voltage AC or DC?  ";
		acdc = xin(MAX_INPUT_CHARACTERS);
	
		if (acdc == "ac" || acdc == "AC" || acdc == "a" || acdc == "A")
			ACDC = 1;
		
		else if (acdc == "dc" || acdc == "DC" || acdc == "d" || acdc == "D")
			ACDC = 2;
	}
	cout << "\nEnter the resistance(-# to use stored value; 0 for multiple or none):  ";
	R = ind();	
		
	if (!R) {
		R = R_equiv();
		
	if (R < 0)
		R = R_storage;
		
		if (R) {
			cout << "\nThe equivalent resistance in the RLC circuit is ";
			out(R);
			cout << OHM_CHAR << "." << endl;
		}
		else 
			cout << "No resistor" << endl;
	}
	if (ACDC == 2) {
		RL_calc_dc(R);
		return;
	}	
	cout << "Enter the capacitance-F(-# to use stored value; 0 for multiple or none):  ";
	C = ind();	
		
	if (!C) {
		C = C_equiv();
		
	if (C < 0)
		C = C_storage;
		
		if (C) {
			cout << "\nThe equivalent capacitance in the RLC circuit is ";
			out(C);
			cout << "F." << endl;
		}
		else
			cout << "No capacitor" << endl;
	}
		
	cout << "Enter the inductance in Henrys(-# to use the same value as before):  ";
	L = ind();
	
	if (L < 0)
		L = L_storage;
		
	if (!L)
		cout << "No inductor" << endl;
	
	L_temp = L_storage;	
	L_storage = L;
	
	Fr = 1/(two_pi*sqrt(L*C));
	AV = sqrt(1/(L*C) - R*R/(4*L*L));
	
	if (!(L*C))
		Fr = 0;
	
	cout << "\nThe Resonant Frequency of this RLC circuit is ";
	out(Fr);
	cout << "Hz." << endl << "\nThe Angular Velocity w` is ";
	out(AV);
	cout << "." << endl;
	cout << "Enter the Input Frequency of the circuit in Hz(-# for resonance):  ";
	F = ind();
	
	if (F < 0)
		F = Fr;
	
	impedance_calc(R, C, L, F); 
}




void impedance_calc(double R, double C, double L, double F) {

	double Xl, Xc, Z, A;
	
	Xl = two_pi*F*L;
	Xc = 1/(two_pi*F*C);
	
	if (!(F*C))
		Xc = 0;
	
	Z = sqrt(R*R + (Xl - Xc)*(Xl - Xc));
	A = (pi_under_180)*atan(Xl/R);
	
	if (!R)
		A = 0;	
	
	cout << "\nThe Inductive Reactance of this RLC circuit is ";
	out(Xl);
	cout << OHM_CHAR << "." << endl << "The Capacitive Reactance of this RLC circuit is ";
	out(Xc);
	cout << OHM_CHAR << "." << endl << "The Impedance of this RLC circuit is ";
	out(Z);
	cout << OHM_CHAR << "." << endl << "The Phase Angle of the RLC circuit is " << A << " degrees." << endl		 
	     << "\nWould you like to store the impedance in memory as a resistance?  ";
	
	if (decision()) {
		R_temp = R_storage;
		R_storage = Z;
	}
	cout << "\nWould you like to calculate I and V across the components?  ";
	
	if (decision())
		RCL_IV(R, Xc, Xl, Z);		
}




void TL_Impedance() {
	
	double zo, L, F, wavelength, Er;
	complex zi, zl, Zi, Zl, Gamma_i, Gamma_l;
	
	function_id = 25;
	
	cout << "\nEnter the frequency in Hz (use negative number for rad/sec):  ";
	F = ind();
	if (F < 0)
		F = -F/two_pi;
		
	cout << "\nEnter the length of the transmission line in meters:  ";
	L = fabs(ind());
	
	cout << "\nEnter the impedance of the transmission line (Zo) in " << OHM_CHAR << ":  ";
	zo = fabs(ind());
	
	cout << "\nEnter the relative permittivity of the transmission line (Er):  ";
	Er = fabs(ind());
	
	cout << "\nEnter the complex impedance of the load (Zl) in " << OHM_CHAR << " (Re, Im):  ";
	zl.Re = ind();
	zl.Im = ind();
	
	wavelength = speed_of_light/sqrt(Er)/F;	
	Zl = zl/zo;
	Gamma_l = (Zl - 1)/(Zl + 1);
	Gamma_i = Gamma_l*expc(-4*pi*L/wavelength);
	Zi = (Gamma_i + 1)/(Gamma_i - 1)*-1;
	zi = Zi*zo;
	
	cout << "\nThe input impedance Zi = " << zi << " " << OHM_CHAR << ".";
}




void RCL_IV(double R, double Xc, double Xl, double Z) {

	double E, I, Er, Ec, El, AP, PF, TP;
	
	cout << "\nWhat is the total supplied voltage(-# to use stored value)?  ";
	E = ind();
	
	if (E < 0)
		E = V_storage;
	
	PF = R/Z;
	I = E/Z;
	AP = I*E;
	TP = AP*PF;
	Er = I*R;
	Ec = I*Xc;
	El = I*Xl;
	
	cout << "\nThe current through the RLC circuit is ";
	out(I);
	cout << "A." << endl << "The Apparent Power is ";
	out(AP);
	cout << "W." << endl << "The True Power is ";
	out(TP);
	cout << "W." << endl << "The Power Factor is ";
	out(PF);
	cout << "." << endl << "The voltage across the resistor is ";
	out(Er);
	cout << "V." << endl << "The voltage across the capacitor is ";
	out(Ec);
	cout << "V." << endl << "The voltage across the inductor is ";
	out(El);
	cout << "V." << endl
		 << "\nWould you like to store the current in memory?  ";
		 
	if (decision()) {
		I_temp = I_storage;	
		I_storage = I;
	}
	cout << "\nWould you like to store the True Power in memory?  ";
	
	if (decision()) {
		P_temp = P_storage;
		P_storage = TP;		
	}
}





void RL_calc_dc(double R) {

	double L, E, I, Io, t;
	
	cout << "\nEnter the inductance(-# to use the same as before) :  ";
	L = ind();
	
	if (L < 0)
		L = L_storage;
	
	if (R == 0 || L == 0) {
	
		cout << "\nThis is not a valid RL circuit!" << endl;
		impedance();
		return;
	}	
	cout << "\nEnter the supplied DC voltage E:  ";
	E = ind();
	
	cout << "\nEnter the initial current I(0):  ";
	Io = ind();
	
	cout << "\nEnter the time in seconds:  ";
	t = ind();
	
	if (t < 0)
		t = -t;
	
	I = E/R + (Io - E/R)*exp(-(R/L)*t);
	
	cout << "\n\nThe current after ";
	out(t);
	cout << " second";
	
	if (t != 1)
		cout << "s";
	
	cout << " = ";	 
	out(I);
	cout << "A" << endl;	
	cout << "\n\nWould you like to store this current value in memory?  ";
	
	if (decision()) {
		I_temp = I_storage;
		I_storage = I;
	}
}





void delta_to_wye() {
	
	int dw;
	double r1, r2, r3, ra, rb, rc;
	
	function_id = 21;
	
	do {
		cout << "\nWhich conversion? \n  1. Delta to Wye \n  2. Wye to Delta" << endl;
		dw = in();
		
		if (dw != 1 && dw != 2)
			cout << "Invalid Selection! Please enter only one or two." << endl;
			
	} while (dw != 1 && dw != 2);
		
	if (dw == 1) {
		cout << "\nEnter Ra(-# for multiple):  ";
		ra = ind();
		
		if (ra < 0) {
			ra = R_equiv();
		
			cout << "\nThe equivalent Ra is ";
			out(ra);
			cout << OHM_CHAR << "." << endl;
		}
		cout << "\nEnter Rb(-# for multiple):  ";
		rb = ind();
		
		if (rb < 0) {
			rb = R_equiv();
		
			cout << "\nThe equivalent Rb is ";
			out(rb);
			cout << OHM_CHAR << "." << endl;
		}
		cout << "\nEnter Rc(-# for multiple):  ";
		rc = ind();
		
		if (rc < 0) {
			rc = R_equiv();
		
			cout << "\nThe equivalent Rc is ";
			out(rc);
			cout << OHM_CHAR << "." << endl;
		}
		
		r1 = (rb*rc)/(ra + rb + rc);
		r2 = (rc*ra)/(ra + rb + rc);
		r3 = (ra*rb)/(ra + rb + rc);
		
		cout << "\nR1 = ";
		out(r1);
		cout << OHM_CHAR << " \nR2 = ";
		out(r2);
		cout << OHM_CHAR << " \nR3 = ";
		out(r3);
		cout << OHM_CHAR << endl;
	}
	else {
		cout << "\nEnter R1(-# for multiple):  ";
		r1 = ind();
		
		if (r1 < 0) {
			r1 = R_equiv();
		
			cout << "\nThe equivalent R1 is ";
			out(r1);
			cout << OHM_CHAR << "." << endl;
		}
		cout << "\nEnter R2(-# for multiple):  ";
		r2 = ind();
		
		if (r2 < 0) {
			r2 = R_equiv();
		
			cout << "\nThe equivalent R2 is ";
			out(r2);
			cout << OHM_CHAR << "." << endl;
		}
		cout << "\nEnter R3(-# for multiple):  ";
		r3 = ind();
		
		if (r3 < 0) {
			r3 = R_equiv();
		
			cout << "\nThe equivalent R3 is ";
			out(r3);
			cout << OHM_CHAR << "." << endl;
		}		
		ra = (r1*r2 + r2*r3 + r3*r1)/r1;
		rb = (r1*r2 + r2*r3 + r3*r1)/r2;
		rc = (r1*r2 + r2*r3 + r3*r1)/r3;
		
		cout << "\nRa = ";
		out(ra);
		cout << OHM_CHAR << " \nRb = ";
		out(rb);
		cout << OHM_CHAR << " \nRc = ";
		out(rc);
		cout << OHM_CHAR << endl;
	}
}





void filter() {

	double wp, ws, Ap, As, wc;
	int N;
	
	function_id = 23;
	
	cout << "\nButterworth Low Pass Filter" << endl;
	
	cout << "\nEnter the pass band cutoff frequency in rad/sec (negative for Hz):  ";
	wp = ind();
	if (wp < 0)
		wp = -wp*two_pi;

	cout << "\nEnter the stop band cutoff frequency in rad/sec (negative for Hz):  ";
	ws = ind();
	if (ws < 0)
		ws = -ws*two_pi;

	cout << "\nEnter the maximum pass band attenuation in dB:  ";
	Ap = ind();
	cout << "\nEnter the minimum stop band attenuation in dB:  ";
	As = ind();
	
	N = (int)(ceil(log10((pow(10.0,As/10.0)-1.0)/(pow(10.0,Ap/10.0)-1.0))/(2.0*log10(ws/wp))));
	
	cout << "\nN = " << N << endl;	
	cout << "\nDo you want to meet the pass band attenuation specification and exceed the stop band attenuation specification?  ";	
	
	if (decision()) 
		wc = wp/pow(pow(10.0,Ap/10.0)-1.0,1/(2.0*N));
	else
		wc = ws/pow(pow(10.0,As/10.0)-1.0,1/(2.0*N));
	
	cout << "\nWc = " << wc << " rad/sec or " << wc/two_pi << "Hz" << endl;	
	cout << "\nThe poles are at:" << endl;
	
	for (int n = 0; n < N; ++n)
		 cout << (n*pi/N + pi/(2*N) + pi/2) << " radians or " << (n*180.0/N + 90.0/N + 90.0) << " degrees" << endl;
}





void display() {

	function_id = 8;

	cout << "\n\nThe stored resistance is ";
	out(R_storage);
	cout << OHM_CHAR << "." << endl << "The stored current is ";
	out(I_storage);
	cout << "A." << endl << "The stored voltage is ";
	out(V_storage);
	cout << "V." << endl << "The stored power is ";
	out(P_storage);
	cout << "W." << endl << "The stored capicitance is ";
	out(C_storage);
	cout << "F." << endl << "The stored inductance is ";
	out(L_storage);
	cout << "H." << endl << "The stored impedance is ";
	out(Z_storage);
	cout << endl << "The stored frequency is ";
	out(F_storage);
	cout << "Hz.";
	
	cout << "\nWould you like to change these values?  ";
	
	if (decision())
		new_storage();
	
	return;
}




void new_storage() {

	do {
		R_temp = R_storage;
		I_temp = I_storage;
		V_temp = V_storage;
		P_temp = P_storage;
		C_temp = C_storage;
		L_temp = L_storage;
		Z_temp = Z_storage;
		F_temp = F_storage;
	
		cout << "\nEnter the new resistance(0 to initialize):  ";
		R_storage = ind();
		cout << "Enter the new current(0 to initialize):  ";
		I_storage = ind();
		cout << "Enter the new voltage(0 to initialize):  ";
		V_storage = ind();
		cout << "Enter the new power(0 to initialize):  ";
		P_storage = ind();
		cout << "Enter the new capacitance(0 to initialize):  ";
		C_storage = ind();
		cout << "Enter the new inductance(0 to initialize):  ";
		L_storage = ind();
		cout << "\nEnter real part of the new impedance(0 to initialize):  ";
		Z_storage.Re = ind();
		cout << "Enter imaginary part of the new impedance(0 to initialize):  ";
		Z_storage.Im = ind();
		cout << "Enter the new frequency(0 to initialize):  ";
		F_storage = ind();
	
		cout << "\nR = ";
		out(R_storage);
		cout << OHM_CHAR << endl << "I = ";
		out(I_storage);
		cout << "A" << endl << "V = ";
		out(V_storage);
		cout << "V" << endl << "P = ";
		out(P_storage);
		cout << "W" << endl << "C = ";
		out(C_storage);
		cout << "F" << endl << "L = ";
		out(L_storage);
		cout << "H" << endl << "Z = ";
		out(Z_storage);
		cout << endl << "F = ";
		out(F_storage);
		cout << "Hz" << "\nIs this correct?  ";

	} while (!decision());
	
	return;
}




void quadratic() {

	double rad, num1, num2, a, b, c; 
	
	function_id = 9;
	
	cout << "\n<y = ax^2 + bx + c>	Enter a, b, and c:  ";
	a = ind();
	b = ind();
	c = ind();
	
	rad = b*b - 4*a*c;
	
	if (rad < 0) {
		cout << "There are no solutions to the equation " << a << "x^2 + " << b << "x + " << c
			 << " = 0." << endl;
	}
	else {
		num1 = (-b + sqrt(rad))/(2*a);
		num2 = (-b - sqrt(rad))/(2*a);
		
		cout << "The solutions to the equation " << a << "x^2 + " << b << "x + " << c
			 << " = 0 are:" << endl << "x = " << num1 << " and x = " << num2 << "." << endl;		
	}
	cout << "\nWould you like to graph this quadratic equation?  ";
		
	if (decision())
		quad_graph(a, b, c);
			 
	return;
}




void quad_graph(double a, double b, double c) {

	point quad;

	scales(3);
	Count = 0;
										
	for(quad.x = -domain; quad.x < domain && Count < MAX_DATA_SIZE; quad.x += 2*domain/num_points) {					
		quad.y = a*quad.x*quad.x + b*quad.x + c;
							
		graph_data[Count].x = quad.x;
		graph_data[Count].y = quad.y;
		
		++Count;	
	}
	cout << "\n" << Count << " points on the graph of y = " << a << "x^2 + " << b << "x + " << c 
		 << " were stored." << endl;
		 
	graph_setup();
}




void base_conversion() {

	int num, negative, base1, base2, base_error, num_10, final_num(0);
	
	function_id = 10;

	cout << "\n\nEnter the original number:  ";
	num = in();
	
	if (num < 0) {
		negative = 1;
		num = -num;
	}
	else 
		negative = 0;
	
	do {
		base_error = 0;	
		cout << "Enter the old base:  ";
		base1 = in();
	
		if (base1 < 2 || base1 > 10) {
			beep();
			cout << "\nBases can only be 2 thru 10!" << endl;		
			base_error = 1;
		}
	} while (base_error);
	
	do {
		base_error = 0;	
		cout << "Enter the new base:  ";
		base2 = in();
		
		if (base2 < 2 || base2 > 10) {
			beep();
			cout << "\nBases can olly be 2 thru 10!" << endl;			
			base_error = 1;
		}
	} while (base_error);
	
	num_10 = base_10(num, base1);
	
	if (ERRORx) {
		beep();
		cout << "\nERROR: " << num << " is not a valid number in base " << base1 << "." << endl;
		ERRORx = 0;
	}
	else {
		final_num = base_n(num_10, base2);
	
		if (negative)
			final_num = -final_num;
		
		cout << "The number " << num << " in the new base " << base2 << " is " << final_num << "." << endl;
	}
}




int base_10(int num1, int b1) {

	int b10_num(0), i(0);
	
	while (num1) {		
		if (num1 % 10 >= b1) {
			ERRORx = 1;
			b10_num = 0;
			break;
		}
		else {
			b10_num += (int)((num1 % 10) * pow((float)b1, (float)i++));
			num1 /= 10;	
		}
	}
	return b10_num;
}




int base_n(int num2, int b2) {

	int num_out(0), i(0);
	
	while (num2) {	
		num_out += (int)((pow(10.0f, (float)i++) * (num2 % b2)));
		num2 /= b2;
	}
	return num_out;
}




void inverse_square() {

	double distance, new_distance, intensity, new_intensity, intensity_1;
	
	function_id = 11;
	
	cout << "\n\nEnter a known distance:  ";
	distance = ind();
	cout << "Enter the intensity at that distance:  ";
	intensity = ind();

	intensity_1 = intensity*distance*distance;
	
	cout << "\nWould you like to graph this?  ";
	
	if (decision())
		ISL_graph(intensity_1);
	
	else {
		cout << "\nEnter the new distance(0 to end):  ";
		new_distance = ind();
	
		while (new_distance) {		
			new_intensity = intensity_1*(1/(new_distance*new_distance));
		
			cout << "\nThe new intensity is " << new_intensity << endl;		
			cout << "\nEnter the new distance(0 to end):  ";
			new_distance = ind();	
		}
	} 
}




void ISL_graph(double intensity) {

	point inv_sq;
	
	scales(3);
	Count = 0;
										
	for(inv_sq.x = 0; inv_sq.x < domain && Count < MAX_DATA_SIZE; inv_sq.x += domain/num_points) {
		
		inv_sq.y = intensity/(inv_sq.x*inv_sq.x);
							
		graph_data[Count].x = inv_sq.x;
		graph_data[Count].y = inv_sq.y;
		
		++Count;	
	}
	cout << "\n" << Count << " points on the graph were stored." << endl;
		 
	graph_setup();
}





void diode_calc() {

	double Vd, Id, Vd_ON;
	
	function_id = 19;
	
	cout << "\n\nEnter Vd ON:  ";
	Vd_ON = ind();
	
	do {
		cout << "\nEnter Vd(0 to end):  ";
		Vd = ind();
		
		if (Vd) {						
			Id = diode_sat*(exp(Vd/(.035*Vd_ON)) - 1);
			cout << "\nModel1: Id = ";
			out(Id);
			cout << "A" << endl;
			
			/*get_default_diode(diode);
			Id = cacl_i(diode, Vd);			
			cout << "\nModel2: Id = ";
			out(Id);
			cout << "A" << endl;*/
		}
		
	} while (Vd != 0);
	
	cout << "\nWould you like to store the last calculated current?  ";
	
	if (decision()) {		
		I_temp = I_storage;
		I_storage = Id;
	}
}





void vector_calc() {

	double V, V2, R, Rx, Ry, Angle, R_Angle;
	int quadrant;
	
	function_id = 20;	
	
	do {
		Rx = Ry = 0;
		
		cout << "\nEnter the magnitude of the first vector:  ";
		V = ind();
	
		while (V) {				
			quadrant = 1;
			
			cout << "\nEnter the angle of the vector(rotated ccw with east = 0 degrees):  ";
			Angle = ind();
		
			while (Angle > 5e9 || Angle < -5e9) {
				cout << "\nAngle is out of range and cannot be calculated!" << endl
					 << "Enter new angle:  ";
				Angle = ind();
			}	
			while (Angle < 0)
				Angle += 360;
			while (Angle >= 360)
				Angle -= 360;
			
			while (Angle >= 90) {
			
				++quadrant;
				Angle -= 90;
			}
			switch (quadrant) {
		
				case 1:
					Rx += V*cos(pi_over_180*Angle);
					Ry += V*sin(pi_over_180*Angle);
					break;				
				case 2:
					Rx -= V*sin(pi_over_180*Angle);
					Ry += V*cos(pi_over_180*Angle);
					break;				
				case 3:
					Rx -= V*cos(pi_over_180*Angle);
					Ry -= V*sin(pi_over_180*Angle);
					break;				
				case 4:
					Rx += V*sin(pi_over_180*Angle);
					Ry -= V*cos(pi_over_180*Angle);
					break;
			}
			cout << "\nEnter the magnitude of the next vector(0 to end):  ";
			V2 = V;
			V = ind();
		}
		if (fabs(Rx) < fabs(V2/1000))
			Rx = 0;
		
		if (fabs(Ry) < fabs(V2/1000)) 
			Ry = 0; 
	
		cout << "\nRx = ";
		out(Rx);
		cout << endl << "Ry = ";
		out(Ry);
		cout << endl;
	
		R_Angle = get_angle(Rx, Ry);		
		R = sqrt(Rx*Rx + Ry*Ry);
		
		cout << "R = "; 
		out(R);
		cout << " @ " << R_Angle << " degrees" << endl;
		cout << "R = "; 
		out(R);

		if (R == 0)
			R = 1e-20;

		cout << " " << Rx/R << "x ";

		if (Ry > 0)
			cout << "+ ";
		else
			cout << "- ";

		cout << fabs(Ry/R) << "y" << endl << "\nWould you like to solve another vector problem?  ";
		
	} while (decision());
}




void calculator() {

	double result, x(0);
	static double last_val(0);
	static cs_equation calc_eq(MAX_INPUT_CHARACTERS);
	
	function_id = 26;

	while (1) {
		cout << "Enter an expression ('q' to quit):  ";
		calc_eq.input("Expression Error! Reenter:  ");

		if (comp_char(calc_eq.is_one_char(), 'q'))
			return;

		if (comp_char(calc_eq.is_one_char(), 'p'))
			result = last_val;
		else {
			if (calc_eq.var_with_x()) {
				cout << "\nEnter x:  ";
				x = ind();
			}
			result = last_val = calc_eq.eval(x);
		}
		cout << "Result = " << result << endl;
	}
}





void clockspeed() {
	
#ifdef xMAC_OS
	int clock = clock_microprocessor();

	if (clock != clock_speed) {
		cout << "\nMachine specifications state clock speed as " << clock_speed << ". Use newly calculated value?  ";
		
		if (decision())
			clock_speed = clock;
	}
	else
		cout << "\nCalculated clock speed agrees with the value in the machine specifications." << endl;
#else
	clock_microprocessor();
#endif
}




int read_display_bitmap() {

	unsigned i, j, offset, xsize, ysize;
	double *values = NULL, val, val1, val2, val3;
	char filename[MAX_SAVE_CHARACTERS], windowname[MAX_SAVE_CHARACTERS+10];
	ifstream infile;

	cout << "Enter the name of the input bitmap file:  ";
	cin  >> ws;
	strcpy(filename, xin(MAX_SAVE_CHARACTERS));

	if (!infile_file(infile, filename)) {
		beep();
		cerr << "The input file " << filename << " could not be opened!" << endl;
		return 0;
	}
	if (!filestream_check(infile)) {
		cerr << "Error in file stream." << endl;
		return 0;
	}
	if (!(infile >> xsize >> ysize >> val1 >> val2)) {
		cerr << "Error reading bitmap header." << endl;
		infile.close();
		return 0;
	}
	if (xsize == 0 || ysize == 0) {
		cerr << "Error: Bitmap size of zero (" << xsize << " x " << ysize << ")." << endl;
		infile.close();
		return 0;
	}
	val3   = val1*val2;
	values = memAlloc(values, xsize*ysize);

	init_menu(1, WHITE, 0);
	sprintf(windowname, "Bitmap - %s", filename);
	set_DW_title(windowname);

	cout << "Reading bitmap file " << filename << "." << endl;
	init_progress_bar(0);

	for (i = 0; i < ysize; ++i) {
		offset = i*xsize;
		for (j = 0; j < xsize; ++j) {
			if (!filestream_check(infile) || !(infile >> val)) {
				cerr << "Error reading value in input file." << endl;
				infile.close();
				close_drawing_window();
				delete [] values;
				return 0;
			}
			values[offset + j] = val*val3;
		}
		if (i%(ysize/100+1)) {
			update_progress_bar(100*i/ysize, 0);
		}
	}
	infile.close();
	update_progress_bar(100, 0);

	cout << "Creating bitmap." << endl;
	whiteout();

	for (i = 0; i < ysize; ++i) {
		offset = i*xsize;
		for (j = 0; j < xsize; ++j) {
			val = values[offset + j];
			if (val < 0) {
				SetDrawingColorx(RED);
			}
			else if (val > 0) {
				SetDrawingColorx(BLUE);
			}
			else {
				SetDrawingColorx(GREEN);
			}
			LineDraw((int)j, (int)i, (int)j+1, (int)i+1);
		}
	}
	delete [] values;
	reset_cursor();

	return 1;
}





int save() {
	
	ofstream outfile;
	char s_name[MAX_SAVE_CHARACTERS + 1] = {0};
	
	function_id = 17;

	cout << "\n\nSave project as(max 31 characters):  ";
	cin >> ws;					
	strcpy(s_name, xin(MAX_SAVE_CHARACTERS));
	
	if (!overwrite_file(s_name))
		return 0;
		
	outfile.open(s_name, ios::out | ios::trunc);
	
	if (outfile.fail() || !filestream_check(outfile)) {
		beep();
		cerr << "\nError: Project could not be saved as " << s_name << "!" << endl;
		return 0;
	}
	outfile << "Circuit_Solver_Saved_Project" << endl << last_qname << endl << function_id << endl
			<< R_storage << endl << I_storage << endl << V_storage << endl << P_storage << endl
			<< C_storage << endl << L_storage << endl << Z_storage.Re << endl << Z_storage.Im << endl 
			<< F_storage << endl << R_temp << endl << V_temp << endl << I_temp << endl << V_temp 
			<< endl << P_temp << endl << C_temp << endl << L_temp << endl << Z_temp.Re << endl 
			<< Z_temp.Im << endl << F_temp << endl << origin.x << endl << origin.y << endl << domain 
			<< endl << range << endl << x_scale << endl << y_scale << endl << line_type << endl 
			<< num_points << endl << origin_set << endl << scale_set << endl<< enable_draw_grid << endl 
			<< see_tracer << endl << save_pref << endl << delete_pref << endl << need_node 
			<< endl << grid_color << endl << supply_voltage << endl << ground << endl;
			
	if (!outfile.good()) {
		beep();
		cerr << "\nError: Not enough space to save entire project!" << endl;
	}
	outfile.close();
	cout << "\nThe current Circuit Solver project has been saved as: " << s_name << "." << endl;
	
	return 1;
}




void open() {
	
	int input_loc;
	ifstream infile;
	char o_name[MAX_SAVE_CHARACTERS + 1] = {0};
	string name_id;
	
	function_id = 13;
	
	if (OPEN != 3) {
		cout << "\n\nOpen Project, Graph, Circuit, or IC named:  ";
		cin  >> ws;
		strcpy(o_name, xin(MAX_SAVE_CHARACTERS));
	}
	else
		strcpy(o_name, NAME);
		
	input_loc = infile_file(infile, o_name);
	
	if (!input_loc) {
		beep();	
		cerr << "\nThe Circuit Solver save " << o_name << " could not be opened!" << endl;
		return;
	}
	if (OPEN != 3) {
		strcpy(NAME, "");	
		if (input_loc == 2) 
			strcat(NAME, data_folder);
		else if (input_loc == 3)
			strcat(NAME, circuit_folder);
		
		strcat(NAME, o_name);
	}
	if (!filestream_check(infile))
		return;
	
	infile >> name_id;
	
	if (name_id == "Circuit_Solver_Saved_IC" || name_id == "Circuit_Solver_Saved_IC_GV" || name_id == "Circuit_Solver_Saved_Circuit" || name_id == "Circuit_Solver_Saved_Circuit_GV" || name_id == "Circuit_Solver_Saved_Subcircuit" || name_id == "Circuit_Solver_Saved_Subcircuit_GV" || name_id == "Circuit" 
		|| (name_id != "" && (name_id[0] == '#' || name_id[0] == '*'))) {
		infile.close();
		OPEN = 1;
		logic();
		if (close_DW) {
			close_drawing_window();
 			graph_enable = close_DW = 0;
 		}
		return;
	}
	else if (name_id == "Circuit_Solver_Saved_Graph" || name_id == "Circuit_Solver_Saved_Log_Graph" || name_id == "Circuit_Solver_Saved_Logx_Graph") {
		infile.close();
		OPEN = 2;
		graph_setup();
		return;
	
	}
	else if (name_id == "Circuit_Solver_Saved_Project") {
		OPEN = 0;
		
		if (!filestream_check(infile))
			return;
		
		infile >> last_qname >> function_id >> R_storage >>  I_storage  >> V_storage  >> P_storage 
		   	   >> C_storage >> L_storage >> Z_storage.Re >> Z_storage.Im >> F_storage >> R_temp >> V_temp 
		   	   >> I_temp >> V_temp >> P_temp >> C_temp >> L_temp >> Z_temp.Re >> Z_temp.Im >> F_temp  
		  	   >> origin.x >> origin.y >> domain >> range >> x_scale >> y_scale >> line_type >> num_points
		  	   >> origin_set >> scale_set >> enable_draw_grid >> see_tracer >> save_pref >> delete_pref 
		  	   >> need_node >> grid_color >> supply_voltage >> ground;
				
		cout << "\nThe project has been opened successfully!" << endl;
	}
	else {
		beep();
		cerr << "\nThe file " << o_name << " is not a Circuit Solver save!" << endl;
	}	
	infile.close();
}




void undo() {

	char undo;
	
	function_id = 12;

	cout << "\nWhich value calculation do you want to undo(R,I,V,P,C,L,Z,or F)?  ";
	undo = inc();
	
	switch (undo) {
	
		case 'r':
		case 'R':
			swap(R_storage, R_temp);
			break;			
		case 'i':
		case 'I':
			swap(I_storage, I_temp);
			break;			
		case 'v':
		case 'V':
			swap(V_storage, V_temp);
			break;			
		case 'p':
		case 'P':
			swap(P_storage, P_temp);
			break;			
		case 'c':
		case 'C':
			swap(C_storage, C_temp);
			break;			
		case 'l':
		case 'L':
			swap(L_storage, L_temp);
			break;			
		case 'z':
		case 'Z':
			swap(Z_storage, Z_temp);
			break;			
		case 'f':
		case 'F':
			swap(F_storage, F_temp);
			break;			
		default:
			cout << "\nNothing was undone." << endl;
			break;	
	}
}





char quit() {
	
	if (!save_pref || !function_id)
		return '0';
	
	if (!useDIALOG)
		cout << "\nSave project before you quit";
		
	switch (xdecision(2)) {
	
		case 0:
			return '0';	
			
		case 1:
			while (!save()) {
				cout << "Save project?  ";
				if (!decision())
					return '0';
			}
			return '0';
			
		case 2:
			return 'Q';
	}
	return 'X';					
  }




void good_bye() {	
	
	int count = 0;
	
	close_drawing_window();
	
	cout << "\nThanks for trying Circuit Solver, by Frank Gennari. Come back again some time!" << endl;							
	cout << "\n\n        _/\\/\\/\\/\\/\\_" << endl;
	
	while (!got_click() && count < QUIT_COUNT) {	
		delay(.1);
		++count;
	}
}


