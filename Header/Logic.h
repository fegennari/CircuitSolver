#ifndef _LOGIC_H_
#define _LOGIC_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Logic.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Header
// By Frank Gennari

// inlined logic functions
inline char known(char value) {

	return !(value & ~1);
}


inline char unknown(char value) {

	return (value == 2 || value == -2);
}


inline char opposite(char value1, char value2) {

	return ((value1 == 0 && value2 == 1) || (value1 == 1 && value2 == 0));
}

#include "IOVector.h"


typedef void (*draw_func)(int);


#define IS_LOGIC_GATE(ID) ((ID >= BUFFER_ID && ID <= TXNOR_ID) || (ID >= QAND_ID && ID <= QXNOR_ID) || ID == T_ST_BUF_ID || ID == T_ST_INV_ID)
#define IS_ROT90(ID)      (ID == RES_ID || ID == CAP_ID || ID == DIODE_ID || ID == XFMR_ID || ID == BATTERY_ID || ID == DELAY_ID || (ID >= V_SOURCE_ID && ID <= MAX_DEP_SRC_ID))


enum {IN1x = 0, IN2x, IN3x, OUT1x, OUT2x, OUT3x};


int const HILITE_COLOR = BLUE;

// component flags
#define BASE_CIRCUIT 0x01 // used to be circuit_id
#define ROT90        0x02
#define ROT180       0x04
#define ROTATED      0x06 // ROT90 | ROT180



struct component {	// 22 bytes
	int id, location;
	char flags, state;
	double value; 
	component *next;
};


struct comp_sim { // 49 bytes
	int id, location, conn[6], conn_loc;
	double value;
	char flags, state;
};


struct wire {
	int start, end;
};


struct special {
	component *head;
	int selection;
};


struct node_data {
	component *head, *temp;
	connections x;
};


struct clist {

	int conflict, type;
	clist *next;
};


#define mark_all_changed() changed = changed2 = changed3 = changed4 = 1;



void logic();
void simulate_only();
void save_only();
int circuit_has_changed();
component *select_component(component *head, string name, int pins, string description);
void fill_boxes_with_logic_icons();
component *pick_ic(component *head, int opened, string name, int pins, string description);
void find_ic(IC &ic, int id);
void specs(string name, int pins, string description);
component *pick_transistor(component *head, string name, int pins, string description);
component *pick_ff(component *head, string name, int pins, string description);
component *pick_source(component *head, string name, int pins, string description);
int get_edge();
string get_name_of_ff(int id);
component *draw_component_at_click(int component_id, component *head, string name, int pins, string description);
component *process_button_click(component *head, string name, int pins, string description, complex *DWGRID, int position, int &component_id);
component *draw_individual_component(component *head, int component_id, int position, double value, char flags);
int draw_CLINE(int position);
component *split_wires(component *head, int start, int end);
component *split_one_wire(component *comp, component *head, int new_location);
component *remove_component(component *head, string name, int pins, string description);
component *remove_berserk(component *head, string name, int pins, string description, int position, int position2);
void get_info(component *head);
int modify_component(component *head, string name);
void block_move(component *head, int position, int position2);
component *move_component(component *head, int &move, int position, string name, int pins, string description);
component *organize_components(component *head);
connections get_connections(int location, int component_id, char flags);
void set_all_connections(connections &x, int val);
string get_name(int component_id, int show_name);
int get_order(int component_id);
unsigned char *get_units(int id, double value, int for_drawing);
unsigned char *get_units_multiplier(double value, unsigned char *units, int for_drawing);
int get_ic_type(string icname);
void init_and_draw_circuit(component *& head, string name, int pins, string description, int organize);
void draw_circuit(component *& head, string name, int pins, string description, int organize);
void show_values(component *head);
component *update_circuit(int id, int location, component *head, double state, double value);
int check_placement(component *head, int id, int location, double state);
void Error_check(component *temp, int id, int location, double state, int &errorA, int &errorB);
double get_value(int id);
double range_value(double value1);
int has_value(int id);
void init_buttons(int is_schematic_window);
string shift_chars(string text);
void save_circuit(component *& head);
void set_circuit_prefs();
void draw_circuit_pref_window();
void set_AC_circuit_prefs();
void draw_AC_circuit_pref_window();
component *open_circuit(component *head);

void copy_components(component *head);
component *cut_components(component *head);
component *paste_components(component *head);
component *hilite_component(component *comp, int hilite_on);



template<class T> char LOGIC(T NUM);
template<class T> char COMPARE(T N, T P);


template<class T> char LOGIC(T NUM) {
	
	if (NUM > 0)
		return 1;
	
	return 0;
}

template<class T> char COMPARE(T N, T P) {

	if (P > N)
		return 1;
	else if (P < N)
		return -1;
	else
		return 0;
}



void inb(BINX &BINARY, int nbits);
int valid_binary(char bin, int dontcare_ok);

char known(char value);
char unknown(char value);
char opposite(char value1, char value2);
char BooleanNot(char in);

char AtoD(double NUM);

double DtoA(char NUM);

char DIODE(char A);
char DIODE2(char A);

double DIODEf(double A, double Vd_ON);

char NTRANSISTOR(char B, char C, char E);
char PTRANSISTOR(char B, char E, char C);
char NFET(char G, char D, char S);
char PFET(char G, char S, char D);

double OPAMPd(double N, double P);
char OPAMP(char N, char P);

char SAME(char A, char B, char C, char D);

char TRIGGER_CK(char CK);

char DELAY_ELEM(char A);

char ITE(char I, char T, char E);

char BUFFER(char A);
char NOT(char A);
char AND(char A, char B);
char OR(char A, char B);
char NAND(char A, char B);
char NOR(char A, char B);
char XOR(char A, char B);
char XNOR(char A, char B);
char TRIPLE_AND(char A, char B, char C);
char TRIPLE_OR(char A, char B, char C);
char TRIPLE_NAND(char A, char B, char C);
char TRIPLE_NOR(char A, char B, char C);
char TRIPLE_XOR(char A, char B, char C);
char TRIPLE_XNOR(char A, char B, char C);
char QUAD_AND(char A, char B, char C, char D);
char QUAD_OR(char A, char B, char C, char D);
char QUAD_NAND(char A, char B, char C, char D);
char QUAD_NOR(char A, char B, char C, char D);
char QUAD_XOR(char A, char B, char C, char D);
char QUAD_XNOR(char A, char B, char C, char D);

char TRI_STATE_BUFFER(char A, char C);
char TRI_STATE_INVERTER(char A, char C);

char RS_LATCH(char Q, char R, char S, char P, char C);
char RS_FF(char Q, char R, char S, char CK, char P, char C);
char TRANSPARENT_D_FF(char Q, char D, char E, char S, char R);
char D_FF(char Q, char D, char CK, char S, char R);
char JK_FF(char Q, char J, char K, char CK, char S, char R);
char T_FF(char Q, char T, char CK, char S, char R);
char MS_FF(char Q, char D, char CK, char S, char R);

char HALF_ADDER(char A, char B);
char FULL_ADDER(char A, char B, char CARRY_IN);

char MUX2to1(char A0, char A1, char S);



#endif

