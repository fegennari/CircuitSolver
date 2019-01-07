#ifndef _FSM_DESIGN_H_
#define _FSM_DESIGN_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, FSMDesign.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Finite State Machine (FSM) Design Header
// By Frank Gennari
#define MAX_INPUT_BITS    (unsigned)16
#define MAX_OUTPUT_BITS   (unsigned)16
#define INVALID_ID        -1
#define DEF_STATE_RADIUS  2
#define DEF_TRANS_OUT     0
#define FSM_IO_STUB_LEN   2
#define FSM_TEXT_SIZE     10
#define FSM_BUTTON_T_SIZE 12
#define NUM_FSM_BUTTONS   16
#define BUT_BLOCK_WIDTH   4
#define ARROWHEAD_LENGTH  0.7 // blocks
#define ARROWHEAD_ANGLE   30 // actually half angle in degrees

#define GRID_COLOR        AQUA
#define NORM_DRAW_COLOR   BLACK
#define BUTTON_COLOR      BLACK
#define INIT_DRAW_COLOR   BLUE
#define SPEC_DRAW_COLOR   BLUE
#define HILITE_DRAW_COLOR RED

unsigned const MAX_INPUT_VALUE  = (1 << MAX_INPUT_BITS)  - 1;
unsigned const MAX_OUTPUT_VALUE = (1 << MAX_OUTPUT_BITS) - 1;
unsigned const ERROR_STEP_VALUE = 1 << MAX_OUTPUT_BITS;
unsigned const TEXT_PIXEL_WIDTH = (3*FSM_TEXT_SIZE)/5;
char     const REPLACE_SPACE    = '_';

string const FSM_button_names[NUM_FSM_BUTTONS] = 
{" New", " Open", " Save", " Exit", " Redraw", " Grid", " Add State", " Add Trans", " Move",
 " Change", " Delete", " Set Init", " Info", " Compile", " Step", " Run"};

enum {FSM_NULL = 0, FSM_NEW, FSM_OPEN, FSM_SAVE, FSM_EXIT, FSM_REDRAW, FSM_GRID, FSM_ADDSTATE, 
      FSM_ADDTRANS, FSM_MOVE, FSM_CHANGE, FSM_DELETE, FSM_SETINIT, FSM_INFO, FSM_COMPILE,
	  FSM_STEP, FSM_RUN};

enum {MODE_NULL = 0, MODE_ADDSTATE, MODE_ADDTRANS1, MODE_ADDTRANS2, MODE_MOVE, MODE_CHANGE,
      MODE_DELETE, MODE_MAKEINIT, MODE_INFO,MODE_EXIT};




class FSM_trans;

template<class T> T *check_double_array(T *arr, unsigned size, unsigned &cap);



class FSM_state {

public:
	FSM_state(int position, unsigned output_value, const string &s_label, unsigned output_bits);
	~FSM_state();

	inline void assign_id(unsigned state_id) {id = state_id;}
	void add_input(FSM_trans *transition);
	void add_output(FSM_trans *transition);

	int build_table(unsigned input_bits);

	inline unsigned   get_id()    {return id;}
	inline int        get_loc()   {return location;}
	inline unsigned   get_val()   {return output_val;}
	inline string     get_label() {return label;}
	FSM_trans *get_transition(unsigned input);
	int        find_input_from(FSM_state *state);

	void draw(char fsm_type);
	int print(ostream &out, int length);

private:
	unsigned    id, output_val, n_in, n_out, in_cap, out_cap, in_bits, tsize, out_bits;
	unsigned   *tran_table;
	int         location;
	char        table_built;
	string      label;
	FSM_trans **inputs, **outputs;
};



class FSM_trans {

public:
	FSM_trans(int t_start, int t_end, unsigned input_val, unsigned output_val, const string &t_label, FSM_state *source_state, FSM_state *sink_state, unsigned input_bits, unsigned output_bits);
	~FSM_trans();

	inline void assign_id(unsigned trans_id) {id = trans_id;}

	inline FSM_state *get_source() {return source;}
	inline FSM_state *get_sink()   {return sink;}
	inline unsigned get_input()    {return in_val;}
	inline unsigned get_val()      {return out_val;}

	void draw(char fsm_type);
	int  print(ostream &out, int length);

private:
	char       type;
	unsigned   id, in_val, out_val, in_bits, out_bits;
	int        start, end, center;
	string     label;
	FSM_state *source, *sink;
};

// type = normal (0), self loop (1), input/initial (2), output (3), error (4)



class FSM {

public:
	FSM(const string &fsm_name, char fsm_type, unsigned input_bits, unsigned output_bits);
	~FSM();

	void init();
	void delete_all();

	void set_params(const string &fsm_name, char fsm_type, unsigned input_bits, unsigned output_bits);
	void add_state(FSM_state *state, char is_initial);
	void add_transition(FSM_trans *transition);

	inline void reset_state() {curr_state = init_state;}
	int  make_state_init(int location);
	inline int  was_init() {return (st_id == (int)init_state);}
	int  run(unsigned *inputs, unsigned *outputs, unsigned num_inputs);
	unsigned step(unsigned input, int do_hilite);
	int  complete();
	FSM_state *find_state_by_loc(int location);

	void draw();
	void print_name(ostream &out) {out << name << endl;}
	void print_info(ostream &out);
	void print_info_at_loc(ostream &out, int location);

	int  read(istream &in);
	int  read(istream &in, unsigned &num_inputs, unsigned &num_outputs, string &fsm_name, char &fsm_type);
	int  write(ostream &out);

private:
	void add_init_transition();

	char        type, is_ready; // Mealy (0) or Moore (1)
	unsigned    n_states, n_trans, state_cap, trans_cap, init_state, curr_state, in_bits, out_bits, ib_size, ob_size;
	int         st_id;
	string      name;
	FSM_state **states;
	FSM_trans **transitions, *last_t_hilite;

	// for non-deterministic FSM
	char      fsm_dtype;
	unsigned *init_states, *curr_states, num_init, num_curr;
};



// *** WRITE ***
class petri_net {

public:
	petri_net() {};
	~petri_net() {};

private:
	FSM  *fsm;
	int  *tokens;
	char *state_types;
};





template<class T> T *check_double_array(T *arr, unsigned size, unsigned &cap) {

	unsigned i;
	T *arr_new = NULL;

	if (size < cap)
		return arr;

	if (cap == 0)
		cap = 1;
	else
		cap *= 2;

	arr_new = memAlloc(arr_new, cap);

	for (i = 0; i < size; ++i)
		arr_new[i] = arr[i];

	delete [] arr;

	return arr_new;
}




#endif
