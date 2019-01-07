#include "CircuitSolver.h"
#include "FSMDesign.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.FSMDesign.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver graphical FSM (Finite State Machine) design
// By Frank Gennari
// * NOT FINISHED *
int report_errors;


extern int object_size, x_limit, y_limit, x_blocks, y_blocks, x_screen, y_screen, click_type, is_char;
extern char typed_char;


struct component;


void fsm_design();
void init_fsm_graphics();
void new_fsm(string &fsm_name, char &fsm_type, unsigned &input_bits, unsigned &output_bits);
int  open_fsm(FSM *fsm, unsigned &num_inputs, unsigned &num_outputs, string &fsm_name, char &fsm_type);
int  save_fsm(FSM *fsm);
int  add_state(FSM *fsm, int location, unsigned output_bits, char fsm_type);
int  add_transition(FSM *fsm, int start, int end, unsigned input_bits, unsigned output_bits, char fsm_type);
unsigned input_value(istream &in, unsigned nbits);
int get_button_press(int &is_special);
void draw_button_panel();
void redraw_fsm(FSM *fsm, int show_grid);
void draw_state_circle(int location);
void draw_trans_arrow(int start, int end, char type);
string num_to_bool_text(unsigned val, unsigned num_bits);
int assign_port(int pt1, int pt2);
void report_FSM_error(string error_str);
void check_legal_location(int location);


void replace_spaces(string &str, char replace_char);
void set_text_size(int size);
int get_scrolled_window_click();
point get_center(int box_num);
string shift_chars(string text);
void draw_text(string text, int box_num, int t_size);
void draw_string(const char *display);
void print_to_window(char *display, int box_num);
void scroll_DW(component *head, string name, int pins, string description, int scroll_type, int scroll_param);
void draw_ARROW(double x1, double y1, double x2, double y2, double ah_len, double ah_angle);



/************ Common Code ************/

// *** write: run, modify, delete ***
void fsm_design() {

	int selection, needs_redraw(0), needs_save(0), show_grid(1), fsm_ready(0), is_special(0), mode(MODE_NULL), tstart(0), temp_osize(object_size);
	unsigned input_bits, output_bits, step_out;
	char fsm_type;
	string fsm_name = "";
	FSM fsm("", 0, 0, 0);

	#ifdef TEXT_ONLY
		cerr << "FSM Design is not available when running in text mode." << endl;
		return;
	#endif

	report_errors = 1;
	init_fsm_graphics();
	SelectConsoleWindow();

	do {
		cout << "Create a new FSM?  ";
		if (decision()) {
			new_fsm(fsm_name, fsm_type, input_bits, output_bits);
			fsm.set_params(fsm_name, fsm_type, input_bits, output_bits);
			fsm_ready = 1;
		}
		else {
			fsm_ready = open_fsm(&fsm, input_bits, output_bits, fsm_name, fsm_type);
		}
	} while (!fsm_ready);

	SelectDrawingWindow();
	redraw_fsm(&fsm, show_grid);
	fsm_ready = 0;
	cout << "Click to make a selection." << endl;

	do {
		selection = get_button_press(is_special);

		if (is_special) {
			switch (selection) {
			case FSM_NULL:
				break;

			case FSM_NEW:
				fsm.delete_all();
				fsm.init();
				new_fsm(fsm_name, fsm_type, input_bits, output_bits);
				fsm.set_params(fsm_name, fsm_type, input_bits, output_bits);
				needs_redraw = 1;
				needs_save   = 1;
				fsm_ready    = 0;
				break;

			case FSM_OPEN:
				open_fsm(&fsm, input_bits, output_bits, fsm_name, fsm_type);
				needs_redraw = 1;
				needs_save   = 0;
				fsm_ready    = 0;
				break;
				
			case FSM_SAVE:
				save_fsm(&fsm);
				needs_save = 0;
				break;

			case FSM_EXIT:
				mode = MODE_EXIT;
				cout << "Cleaning up..." << endl;
				break;

			case FSM_REDRAW:
				needs_redraw = 1;
				break;

			case FSM_GRID:
				show_grid = !show_grid;
				needs_redraw = 1;
				break;

			case FSM_ADDSTATE:
				mode = MODE_ADDSTATE;
				cout << "Click to place states." << endl;
				break;

			case FSM_ADDTRANS:
				mode = MODE_ADDTRANS1;
				cout << "Click at the source state of the transition." << endl;
				break;

			case FSM_MOVE:
				mode = MODE_MOVE;
				cout << "Click on the state you would like to move." << endl;
				break;

			case FSM_CHANGE:
				mode = MODE_CHANGE;
				cout << "Click on the state or transition you would like to change." << endl;
				break;

			case FSM_DELETE:
				mode = MODE_DELETE;
				cout << "Click on the state you would like to delete." << endl;
				break;

			case FSM_SETINIT:
				mode = MODE_MAKEINIT;
				cout << "Click on the initial state." << endl;
				break;

			case FSM_INFO:
				fsm.print_info(cout);
				cout << "Click on a state to get info on it." << endl;
				mode = MODE_INFO;
				break;

			case FSM_COMPILE:
				fsm.complete();
				fsm_ready    = 1;
				needs_save   = 1;
				needs_redraw = 1;
				break;

			case FSM_STEP:
				SelectConsoleWindow();
				cout << "Enter the input value as a string of " << input_bits << " ones and zeros:  ";
				step_out = fsm.step(input_value(cin, input_bits), 1);
				cout << "output value = " << num_to_bool_text(step_out, output_bits) << endl;
				break;

			case FSM_RUN:
				//int run(unsigned *inputs, unsigned *outputs, num_inputs);
				break;
			}
		}
		else {
			switch (mode) {
			case MODE_NULL:
				cerr << "Please select an option from the command bar." << endl;
				break;

			case MODE_ADDSTATE:
				if (add_state(&fsm, selection, output_bits, fsm_type)) {
					needs_save = 1;
					fsm_ready  = 0;
				}
				else
					cout << "Could not add state at this location." << endl;
				break;

			case MODE_ADDTRANS1:
				tstart = selection;
				cout << "Click on the sink state of the transition." << endl;
				mode = MODE_ADDTRANS2;
				break;

			case MODE_ADDTRANS2:
				if (add_transition(&fsm, tstart, selection, input_bits, output_bits, fsm_type)) {
					needs_save = 1;
					fsm_ready  = 0;
				}
				else
					cout << "Could not add transition at this location." << endl;
				cout << "Click on the source state of the transition." << endl;
				mode = MODE_ADDTRANS1;
				break;

			case MODE_MOVE:
				// ???
				break;

			case MODE_CHANGE:
				// ???
				break;

			case MODE_DELETE:
				// ???
				break;

			case MODE_MAKEINIT:
				if (fsm.make_state_init(selection)) {
					needs_save   = 1;
					fsm_ready    = 0;
					needs_redraw = 1;
					cout << "Initial state set." << endl;
				}
				else
					cout << "Could not make state initial." << endl;
				break;

			case MODE_INFO:
				fsm.print_info_at_loc(cout, selection);
				break;

			case MODE_EXIT:
				break;

			default:
				internal_error();
				cerr << "Error: Illegal mode in switch." << endl;
			}
		}
		if (needs_redraw) {
			redraw_fsm(&fsm, show_grid);
			needs_redraw = 0;
		}
	} while (mode != MODE_EXIT);

	fsm.write(cout); // temporary, for testing

	set_DW_title(init_window_title);
 	whiteout();
	object_size = temp_osize;
}




void init_fsm_graphics() {

	init_menu(default_num_buttons, GRID_COLOR, 0);
	set_text_size(FSM_TEXT_SIZE);
	set_DW_title("FSM Design");

	object_size = x_limit/x_blocks + 1;
}




void new_fsm(string &fsm_name, char &fsm_type, unsigned &input_bits, unsigned &output_bits) {

	SelectConsoleWindow();

	cout << "\nWhat is the name of this FSM?  ";
	cin >> fsm_name;
	fsm_name = shift_chars(fsm_name);
	cout << "Is this a Moore machine (Mealy otherwise)?  ";
	fsm_type = (char)decision();
	cout << "How many inputs does this FSM have (MAX " << MAX_INPUT_BITS << ")?  ";
	input_bits  = inpg0();
	cout << "How many outputs does this FSM have (MAX " << MAX_OUTPUT_BITS << ")?  ";
	output_bits = inpg0();

	if (input_bits > MAX_INPUT_BITS) {
		cerr << "Error: Max inputs is " << MAX_INPUT_BITS << "." << endl;
		input_bits = MAX_INPUT_BITS;
	}
	if (output_bits > MAX_OUTPUT_BITS) {
		cerr << "Error: Max outputs is " << MAX_OUTPUT_BITS << "." << endl;
		output_bits = MAX_OUTPUT_BITS;
	}
}




int open_fsm(FSM *fsm, unsigned &num_inputs, unsigned &num_outputs, string &fsm_name, char &fsm_type) {

	int ret_val;
	ifstream infile;
	char filename[MAX_SAVE_CHARACTERS + 1] = {0};

	SelectConsoleWindow();

	cout << "\nOpen FSM named:  ";
	cin  >> ws;
	strcpy(filename, xin(MAX_SAVE_CHARACTERS));

	if (!infile_file(infile, filename)) {
		beep();	
		cerr << "\nThe Circuit Solver FSM " << filename << " could not be opened!" << endl;
		return 0;
	}
	if (!filestream_check(infile)) {
		return 0;
	}
	ret_val = fsm->read(infile, num_inputs, num_outputs, fsm_name, fsm_type);
	infile.close();

	return ret_val;
}




int save_fsm(FSM *fsm) {

	int ret_val;
	ofstream outfile;
	char filename[MAX_SAVE_CHARACTERS + 1] = {0};

	SelectConsoleWindow();

	cout << "\nSave FSM as(max 31 characters):  ";
	cin  >> ws;
	strcpy(filename, xin(MAX_SAVE_CHARACTERS));
	
	if (!overwrite_file(filename)) {
		cout << "FSM was not saved." << endl;
		return 0;
	}
	outfile.open(filename, ios::out | ios::trunc);
	
	if (outfile.fail()) {
		beep();	
		cerr << "\nThe Circuit Solver FSM " << filename << " could not be saved!" << endl;
		return 0;
	}
	if (!filestream_check(outfile)) {
		return 0;
	}
	ret_val = fsm->write(outfile);
	outfile.close();

	return ret_val;
}




int add_state(FSM *fsm, int location, unsigned output_bits, char fsm_type) {

	unsigned out_val(0);
	string label;
	FSM_state *state;

	if (location < 0 || location >= x_blocks*y_blocks) {
		cerr << "Error: Invalid location for state." << endl;
		return 0;
	}
	if (fsm->find_state_by_loc(location) != NULL) {
		cerr << "Error: State is too close to an existing state." << endl;
		return 0;
	}
	SelectConsoleWindow();

	cout << "Enter a label for this state:  ";
	cin >> label;
	label = shift_chars(label);

	if (fsm_type == 1) { // Moore
		cout << "Enter the output value of this state as a string of " << output_bits << " ones and zeros:  ";
		out_val = input_value(cin, output_bits);
	}
	state = new FSM_state(location, out_val, label, output_bits);
	fsm->add_state(state, 0);
	state->draw(fsm_type);

	return 1;
}



// ***
int add_transition(FSM *fsm, int start, int end, unsigned input_bits, unsigned output_bits, char fsm_type) {

	unsigned in_val(0), out_val(0), loc1, loc2;
	string label;
	FSM_trans *trans;
	FSM_state *source = NULL, *sink = NULL;
	
	if (start < 0 || start >= x_blocks*y_blocks || end < 0 || end >= x_blocks*y_blocks) {
		cerr << "Error: Invalid location for transition." << endl;
		return 0;
	}
	source = fsm->find_state_by_loc(start); // *** check for repeat transitions ***

	if (source == NULL && !fsm->was_init()) {
		cerr << "Error: Transition must start at a valid state." << endl;
		return 0;
	}
	sink   = fsm->find_state_by_loc(end);

	if (sink == NULL) {
		cerr << "Error: Transition must end at a valid state." << endl;
		return 0;
	}
	loc1  = source->get_loc();
	loc2  = sink->get_loc();
	start = assign_port(loc1, loc2);
	end   = assign_port(loc2, loc1);

	SelectConsoleWindow();

	cout << "Enter a label for this transition:  ";
	cin >> label; // *** support for multiple input values ***
	label = shift_chars(label);
	cout << "Enter the input value of this transition as a string of " << input_bits << " ones and zeros:  ";
	in_val = input_value(cin, input_bits);

	if (fsm_type == 0) { // Mealy
		cout << "Enter the output value of this transition as a string of " << output_bits << " ones and zeros:  ";
		out_val = input_value(cin, output_bits);
	}
	trans = new FSM_trans(start, end, in_val, out_val, label, source, sink, input_bits, output_bits);
	fsm->add_transition(trans);
	trans->draw(fsm_type);

	return 1;
}




unsigned input_value(istream &in, unsigned nbits) {

	unsigned i, num;
	int bad_input(0);
	string str;

	if (nbits == 0) {
		cerr << "Error: Requested number of bits is 0." << endl;
		return 0;
	}
	while (1) {
		num       = 0;
		bad_input = 0;
		in >> str;

		if (str.size() != nbits) {
			cerr << "Error: Bit string must be of length " << nbits << ". Reenter:  ";
			continue;
		}
		for (i = 0; i < nbits && !bad_input; ++i) {
			switch (str[i]) {
			case '0':
				break;
			case '1':
				num += 1 << (nbits - i - 1);
				break;
			case 'x':
			case 'X':
			case '-':
				cerr << "Error: Don't cares are not yet supported. Reenter:  ";
				bad_input = 1;
				break;
			default:
				cerr << "Error: Unrecognized symbol in Boolean string: " << str[i] << ". Reenter:  ";
				bad_input = 1;
			}
		}
		if (!bad_input)
			return num;
	}
}



// only four locations (N, E, S, W) supported to far
int assign_port(int pt1, int pt2) {

	unsigned i;
	int x1, y1, x2, y2, dist, best_dist(-1), val, best_val(0), locs[4] = {0, 0, 0, 0};

	x1 = pt1%x_blocks;
	y1 = pt1/x_blocks;
	x2 = pt2%x_blocks;
	y2 = pt2/x_blocks;

	if (y1 >= (int)DEF_STATE_RADIUS)
		locs[0] = -DEF_STATE_RADIUS*x_blocks; // N

	if (x1 < (int)(x_blocks-DEF_STATE_RADIUS))
		locs[1] = DEF_STATE_RADIUS; // E

	if (y1 < (int)(y_blocks-DEF_STATE_RADIUS))
		locs[2] = DEF_STATE_RADIUS*x_blocks; // S

	if (x1 >= (int)DEF_STATE_RADIUS)
		locs[3] = -DEF_STATE_RADIUS; // W

	for (i = 0; i < 4; ++i) {
		if (locs[i] != 0) {
			val = pt1 + locs[i];
			dist = (val%x_blocks - x2)*(val%x_blocks - x2) + (val/x_blocks - y2)*(val/x_blocks - y2);
			if (best_dist < 0 || dist < best_dist) {
				best_dist = dist;
				best_val = val;
			}
		}
	}
	return best_val;
}



// *** almost
int get_button_press(int &is_special) {

	int position;
	
	SelectDrawingWindow();
	is_special = 1;
	position   = get_scrolled_window_click();

	if (click_type == -2) { // move/resize window
		wait(0.1);
		return FSM_REDRAW;
	}
	if (click_type == -3) { // should not happen
		return FSM_NULL;
	}
	if (click_type > 10 && click_type < 100) { // right mouse button
		// *** DO SOMETHING ***
		cout << "The right mouse button is currently ignored." << endl;
		return FSM_NULL;
	}
	else if (is_char) { // scroll
		is_char = 0;
		scroll_DW((component *)NULL, "", 0, "", 1, (typed_char - 36)); // ???
		return FSM_REDRAW;
	}
	else {
		if (position%x_blocks >= BUT_BLOCK_WIDTH || position/x_blocks > NUM_FSM_BUTTONS) { // normal
			is_special = 0;
			return position;
		}
	}
	return (position/x_blocks + 1);
}




void draw_button_panel() {

	unsigned  i, button_width((x_limit*BUT_BLOCK_WIDTH)/x_blocks);
	double    button_height(((double)y_limit)/((double)y_blocks));
	char     *bname;

	SetDrawingColorx(WHITE);
	RectangleDraw(0, 0, button_width, y_limit);
	SetDrawingColorx(BUTTON_COLOR);
	RectangleFrame(0, 0, button_width, y_limit);
	set_text_size(FSM_BUTTON_T_SIZE);

	for (i = 0; i < NUM_FSM_BUTTONS; ++i) {
		LineDraw(0, (int)((i+1)*button_height), button_width, (int)((i+1)*button_height));
		MoveTo(button_width/5, ((int)((i + 0.5)*button_height) + 4));
		bname = (char *)FSM_button_names[i].c_str();
		draw_string(bname);
	}
}




void redraw_fsm(FSM *fsm, int show_grid) {

	int color;

	if (show_grid)
		color = GRID_COLOR;
	else
		color = WHITE;

	whiteout();
	init_menu(default_num_buttons, color, 0);
	draw_button_panel();
	fsm->draw();
}




void draw_state_circle(int location) {

	point pt;

	if (location < 0)
		return; // off screen

	pt = get_center(location);

	CircleFrame(pt.x, pt.y, DEF_STATE_RADIUS*object_size); // variable radius?
}



// ***
void draw_trans_arrow(int start, int end, char type) {

	point pt1, pt2;

	if (start < 0 && end < 0 || type == 4)
		return; // off screen

	//if (start < 0 || end < 0)
		// ???

	if (type == 1) {
		// self loop
	}
	pt1 = get_center(start);
	pt2 = get_center(end);

	draw_ARROW(pt1.x, pt1.y, pt2.x, pt2.y, ARROWHEAD_LENGTH, ARROWHEAD_ANGLE); // line
}




string num_to_bool_text(unsigned val, unsigned bits) {

	unsigned i;
	char text[MAX_OUTPUT_BITS + 1] = {0};

	bits = min(bits, 8*sizeof(unsigned));

	for (i = 0; i < bits; ++i) {
		if (val%2)
			text[bits-i-1] = '1';
		else
			text[bits-i-1] = '0';
		val >>= 1;
	}
	return shift_chars((string)text);
}




void report_FSM_error(string error_str) {

	if (report_errors)
		cerr << "FSM Error: " << error_str << "." << endl;
}



void check_legal_location(int location) {

	if (location < 0)
		report_FSM_error("Invalid object location");

	if (location%x_blocks < BUT_BLOCK_WIDTH)
		report_FSM_error("Object under menu column");
}




/************ FSM_state Member Functions ************/


FSM_state::FSM_state(int position, unsigned output_value, const string &s_label, unsigned output_bits) {

	location    = position;
	output_val  = output_value;
	label       = s_label;
	id          = 0;
	in_bits     = tsize   = 0;
	n_in        = n_out   = 0;
	in_cap      = out_cap = 0;
	out_bits    = output_bits;
	table_built = 0;
	inputs      = outputs = NULL;
	tran_table  = NULL;

	output_bits = max(0, min(MAX_OUTPUT_BITS, output_bits));

	if (output_value > MAX_OUTPUT_VALUE)
		report_FSM_error("Output value is too large");
}



FSM_state::~FSM_state() {

	if (n_in  > 0 && inputs  != NULL)
		delete [] inputs;
	if (n_out > 0 && outputs != NULL)
		delete [] outputs;
	if (table_built && tran_table != NULL)
		delete [] tran_table;

	// might want to delete connections to transitions
}




void FSM_state::add_input(FSM_trans *transition) {

	inputs = check_double_array(inputs, n_in, in_cap);
	inputs[n_in++] = transition;
}



void FSM_state::add_output(FSM_trans *transition) {

	outputs = check_double_array(outputs, n_out, out_cap);
	outputs[n_out++] = transition;
}



// ***
int FSM_state::build_table(unsigned input_bits) {

	unsigned i, tin, num_inputs_added(0);

	if (input_bits > MAX_INPUT_BITS) {
		report_FSM_error("Too many input bits");
		return 0;
	}
	in_bits = input_bits;
	tsize   = 1 << in_bits;

	if (tsize == 0 || n_out == 0 || outputs == NULL) // hmmmm...
		return 1;

	tran_table = memAlloc_init(tran_table, tsize, n_out); // initialize with illegal value

	// *** need support for multiple input values on transitions ***

	// fill table: tran_table[input] = transition_id, currently should be one to one
	for (i = 0; i < n_out; ++i) {
		tin = outputs[i]->get_input();
		if (tin >= tsize) {
			report_FSM_error("Input value of transition is out of range");
			delete [] tran_table;
			return 0;
		}
		if (tran_table[tin] != n_out) {
			report_FSM_error("Multiple transitions with same input value created nondeterministic FSM");
			delete [] tran_table;
			return 0;
		}
		tran_table[tin] = i;
		++num_inputs_added;
	}
	if (num_inputs_added > tsize) {
		report_FSM_error("Error creating state transition table");
		delete [] tran_table;
		return 0;
	}
	if (num_inputs_added < tsize) {
		report_FSM_error("State does not transition on all input values");
		if (report_errors) {
			cerr << "Missing output transitions cover input values:";
			for (i = 0; i < tsize; ++i) {
				if (tran_table[i] == n_out)
					cerr << i << " ";
			}
			cerr << endl ;
			//cerr << "These will be treated as self transitions." << endl;
		}
		delete [] tran_table;
		return 0;
	}
	table_built = 1;

	return 1;
}



FSM_trans *FSM_state::get_transition(unsigned input) {

	if (input >= tsize) {
		report_FSM_error("Input value out of range");
		return NULL;
	}
	if (outputs == NULL) {
		report_FSM_error("State transitions not built");
		return NULL;
	}
	if (tran_table == NULL) {
		table_built = 0; // should already be 0, but just in case...
	}
	if (table_built) {
		return outputs[tran_table[input]];
	}
	for (unsigned i = 0; i < n_out; ++i) {
		if (outputs[i] != NULL && outputs[i]->get_input() == input)
			return outputs[i];
	}
	return NULL;
}



int FSM_state::find_input_from(FSM_state *state) {

	if (inputs == NULL)
		return 0;

	for (unsigned i = 0; i < n_in; ++i) {
		if (inputs[i]->get_source() == state)
			return 1;
	}
	return 0;
}




void FSM_state::draw(char fsm_type) {

	if (location < 0)
		return; // off screen

	draw_state_circle(location);

	if (label.size() != 0)
		draw_text(label, -(location - x_blocks), FSM_TEXT_SIZE);

	if (fsm_type == 1) // Moore machine
		draw_text(num_to_bool_text(output_val, out_bits), -location, FSM_TEXT_SIZE);
}



int FSM_state::print(ostream &out, int length) {

	if (!out.good())
		return 0;

	if (length == 0) {
		if (!(out << id << " " << label << " " << output_val << " " << location << endl))
			return 0;
	}
	else {
		if (!(out << "state: id = " << id << ", label = " << label << ", output = " << num_to_bool_text(output_val, out_bits) << "(" << output_val << "), location = " << location << ", fanin = " << n_in << ", fanout = " << n_out << endl))
			return 0;
	}
	return 1;
}



/************ FSM_trans Member Functions ************/

// ***
FSM_trans::FSM_trans(int t_start, int t_end, unsigned input_val, unsigned output_val, const string &t_label, FSM_state *source_state, FSM_state *sink_state, unsigned input_bits, unsigned output_bits) {

	in_val   = input_val;
	out_val  = output_val;
	label    = t_label;
	source   = source_state;
	sink     = sink_state;
	id       = 0;
	in_bits  = input_bits;
	out_bits = output_bits;

	in_bits  = max(0, min(MAX_INPUT_BITS,  in_bits));
	out_bits = max(0, min(MAX_OUTPUT_BITS, out_bits));

	if (source != NULL)
		source->add_output(this);
	if (sink   != NULL)
		sink->add_input(this);

	if (source == NULL && sink == NULL) {
		report_FSM_error("Source and Sink are NULL");
		type = 4;
	}
	else if (source == sink) {
		type = 1; // self loop
		// *** curved transition ***
	}
	else if (source == NULL) {
		type = 2; // init/input
		//t_start -= FSM_IO_STUB_LEN;
	}
	else if (sink == NULL) {
		type = 3; // output
		//t_end += FSM_IO_STUB_LEN;
	}
	else {
		type = 0; // normal
	}
	start    = t_start;
	end      = t_end;
	center   = (start%x_blocks + end%x_blocks)/2 + x_blocks*((start/x_blocks + end/x_blocks)/2); // ???
}



FSM_trans::~FSM_trans() {

	// MUST DELETE TRANSITIONS BEFORE STATES
	// might want to delete connections to states
}




void FSM_trans::draw(char fsm_type) {

	char text_c[MAX_INPUT_BITS + MAX_OUTPUT_BITS + 2];
	string text;

	if (type == 2)
		SetDrawingColorx(SPEC_DRAW_COLOR);

	draw_trans_arrow(start, end, type);

	if (center < 0)
		return;

	if (label.size() != 0)
		draw_text(label, -(center - x_blocks), FSM_TEXT_SIZE);

	text = num_to_bool_text(in_val, in_bits);

	if (fsm_type == 0) { // Mealy machine
		sprintf(text_c, "%s/%s", text.c_str(), (num_to_bool_text(out_val, out_bits).c_str()+1));
		text = (string)text_c;
	}
	draw_text(text, -(center + x_blocks), FSM_TEXT_SIZE);

	if (type == 2)
		SetDrawingColorx(NORM_DRAW_COLOR);
}



int FSM_trans::print(ostream &out, int length) {

	unsigned source_id, sink_id;

	if (!out.good())
		return 0;

	if (length == 0) {
		if (!(out << id << " " << label << " " << in_val << " " << out_val << " " << start << " " << end << " "))
			return 0;
	}
	else {
		if (!(out << "transition: id = " << id << ", label = " << label << ", input = " << num_to_bool_text(in_val, in_bits) << "(" << in_val << "), output = " << num_to_bool_text(out_val, out_bits) << "(" << out_val << "), start = " << start << ", end = " << end))
			return 0;
	}
	if (source == NULL)
		source_id = (unsigned int)INVALID_ID;
	else {
		source_id = source->get_id();
		if (length == 1) {
			if (!(out << " source = " << source->get_label() << "(id = " << source_id << ")"))
				return 0;
		}
	}
	if (sink == NULL)
		sink_id = (unsigned int)INVALID_ID;
	else {
		sink_id = sink->get_id();
		if (length == 1) {
			if (!(out << " sink = " << sink->get_label() << "(id = " << sink_id << ")"))
				return 0;
		}
	}
	if (length == 0) {
		if (!(out << (int)source_id << " " << (int)sink_id << endl))
			return 0;
	}
	else {
		if (!(out << endl))
			return 0;
	}
	return 1;
}



/************ FSM Member Functions ************/


FSM::FSM(const string &fsm_name, char fsm_type, unsigned input_bits, unsigned output_bits) {

	set_params(fsm_name, fsm_type, input_bits, output_bits);
	init();	
}



FSM::~FSM() {

	delete_all();
}




void FSM::init() {

	n_states      = n_trans    = 0;
	state_cap     = trans_cap  = 0;
	init_state    = curr_state = 0;
	is_ready      = 0;
	st_id         = -1;
	states        = NULL;
	transitions   = NULL;
	last_t_hilite = NULL;

	// ND FSM
	fsm_dtype   = 0; // deterministic
	num_init    = num_curr = 0;
	init_states = curr_states = NULL;
}



void FSM::delete_all() {

	unsigned i;

	if (states != NULL) {
		for (i = 0; i < n_states; ++i) {
			delete states[i];
		}
		delete [] states;
	}
	if (transitions != NULL) {
		for (i = 0; i < n_trans; ++i) {
			delete transitions[i];
		}
		delete [] transitions;
	}
	if (init_states != NULL)
		delete [] init_states;

	if (curr_states != NULL)
		delete [] curr_states;
}




void FSM::set_params(const string &fsm_name, char fsm_type, unsigned input_bits, unsigned output_bits) {

	name     = fsm_name;
	type     = (fsm_type != 0); // Mealy (0), Moore (1)
	in_bits  = input_bits;
	out_bits = output_bits;
	ib_size  = 1 << in_bits;
	ob_size  = 1 << out_bits;

	replace_spaces(name, REPLACE_SPACE);
}




void FSM::add_state(FSM_state *state, char is_initial) {

	state->assign_id(n_states);
	if (is_initial)
		init_state = n_states;
	states = check_double_array(states, n_states, state_cap);
	states[n_states++] = state;
}



void FSM::add_transition(FSM_trans *transition) {

	transition->assign_id(n_trans);
	transitions = check_double_array(transitions, n_trans, trans_cap);
	transitions[n_trans++] = transition;
}



int FSM::make_state_init(int location) {

	find_state_by_loc(location);

	if (st_id < 0) {
		report_FSM_error("No state found at location");
		return 0;
	}
	init_state = st_id;

	return 1;
}




int FSM::run(unsigned *inputs, unsigned *outputs, unsigned num_inputs) {

	unsigned i, val;

	if (inputs == NULL)
		return 0;
	if (num_inputs == 0)
		return 1;

	if (outputs == NULL)
		outputs = memAlloc(outputs, num_inputs);

	for (i = 0; i < num_inputs; ++i) {
		val = step(inputs[i], 0);
		if (val == ERROR_STEP_VALUE)
			return 0; // delete outputs if created?
		outputs[i] = val;
	}
	return 1;
}



unsigned FSM::step(unsigned input, int do_hilite) {

	FSM_state *state;
	FSM_trans *trans;

	if (!is_ready) {
		report_FSM_error("FSM is incomplete");
		return ERROR_STEP_VALUE;
	}
	if (states[curr_state] == NULL) {
		report_FSM_error("NULL source state");
		return ERROR_STEP_VALUE;
	}
	trans = states[curr_state]->get_transition(input);
	if (trans == NULL) {
		report_FSM_error("NULL step transition, assuming self-transition");
		//return ERROR_STEP_VALUE;
	}
	if (do_hilite) {
		SetDrawingColorx(NORM_DRAW_COLOR);
		if (last_t_hilite != NULL)
			last_t_hilite->draw(type);
		if (curr_state < n_states && states[curr_state] != NULL)
			states[curr_state]->draw(type);
	}
	last_t_hilite = trans;
	if (trans != NULL) {
		state = trans->get_sink();
		if (state == NULL) {
			report_FSM_error("NULL sink state");
			return ERROR_STEP_VALUE;
		}
		curr_state = state->get_id();
	}
	if (do_hilite) {
		SetDrawingColorx(HILITE_DRAW_COLOR);
		if (trans != NULL)
			trans->draw(type);
		if (curr_state < n_states && states[curr_state] != NULL)
			states[curr_state]->draw(type);
		SetDrawingColorx(NORM_DRAW_COLOR);
	}
	if (type == 0) { // Mealy
		if (trans == NULL)
			return DEF_TRANS_OUT;
		return trans->get_val();
	}
	if (states[curr_state] == NULL) {
		report_FSM_error("NULL new state");
		return ERROR_STEP_VALUE;
	}
	return states[curr_state]->get_val();
}




void FSM::add_init_transition() {

	int init_loc;
	FSM_state *init_st;
	FSM_trans *init_trans;

	if (states == NULL) {
		init_st = NULL;
		init_loc = -1;
	}
	else {
		init_st = states[init_state];
		if (init_st == NULL)
			init_loc = -1;
		else {
			if (init_st->find_input_from((FSM_state *)NULL)) // init already exists
				return;

			init_loc = init_st->get_loc() - DEF_STATE_RADIUS;
		}
	}
	init_trans = new FSM_trans((init_loc - FSM_IO_STUB_LEN), init_loc, 0, 0, "_init", NULL, init_st, in_bits, out_bits);
	add_transition(init_trans);
}



// ***
int FSM::complete() {

	unsigned i;

	// verify FSM: transition input values out of each state, repeat names/labels, duplicates(?)

	add_init_transition();

	for (i = 0; i < n_states; ++i)
		states[i]->build_table(in_bits);

	reset_state();

	is_ready = 1;

	return 1;
}



FSM_state *FSM::find_state_by_loc(int location) {

	int stloc, distsq, max_distsq, xval, yval;

	st_id = -1;

	if (location < 0)
		return NULL;

	max_distsq = DEF_STATE_RADIUS*DEF_STATE_RADIUS;
	xval       = location%x_blocks;
	yval       = location/x_blocks;

	for (unsigned i = 0; i < n_states; ++i) {
		stloc  = states[i]->get_loc();
		distsq = (stloc%x_blocks - xval)*(stloc%x_blocks - xval) + (stloc/x_blocks - yval)*(stloc/x_blocks - yval);
		if (distsq <= max_distsq) {
			st_id = i;
			return states[i];
		}
	}
	return NULL;
}



void FSM::draw() {

	unsigned i;
	char *window_title = new char[name.size() + 16];

	sprintf(window_title, "FSM Design - %s", name.c_str());
	set_DW_title(window_title);
	SetDrawingColorx(NORM_DRAW_COLOR);

	if (states != NULL) {
		for (i = 0; i < n_states; ++i) {
			if (i == init_state)
				SetDrawingColorx(INIT_DRAW_COLOR);
			states[i]->draw(type);
			if (i == init_state)
				SetDrawingColorx(NORM_DRAW_COLOR);
		}
	}
	if (transitions != NULL) {
		for (i = 0; i < n_trans; ++i) {
			transitions[i]->draw(type);
		}
	}
	delete [] window_title;
}



void FSM::print_info(ostream &out) {

	out << "FSM Name:    " << name << endl;
	out << "Type:        ";

	if (type == 0)
		out << "Mealy, ";
	else
		out << "Moore, ";

	if (fsm_dtype == 0)
		out << "Deterministic"     << endl;
	else
		out << "Non-Deterministic" << endl;

	out << "States:      " << n_states << endl;
	out << "Transitions: " << n_trans  << endl;
	out << "Input Bits:  " << in_bits  << endl;
	out << "Output Bits: " << out_bits << endl;
}



void FSM::print_info_at_loc(ostream &out, int location) {

	FSM_state *state = find_state_by_loc(location);

	if (state == NULL) {
		report_FSM_error("There is nothing at that location");
		return;
	}
	state->print(out, 1);
}




int FSM::read(istream &in) {

	unsigned i, id, in_val, out_val, start, end, num_states, num_trans;
	int location, source_id, sink_id;
	string str, label;
	FSM_state *state, *source, *sink;
	FSM_trans *trans;

	if (!in.good())
		return 0;

	delete_all();
	init();

	if (!(in >> str >> name >> type >> init_state >> in_bits >> out_bits)) {
		report_FSM_error("Failed to read FSM header");
		return 0;
	}
	if (str != "FSM") {
		report_FSM_error("Input file should begin with keyword 'FSM'");
		return 0;
	}
	if (in_bits > MAX_INPUT_BITS) {
		report_FSM_error("Too many input bits");
		in_bits = MAX_INPUT_BITS;
	}
	if (out_bits > MAX_OUTPUT_BITS) {
		report_FSM_error("Too many output bits");
		out_bits = MAX_OUTPUT_BITS;
	}
	if (in_bits == 0) {
		report_FSM_error("There must be at least one input bit.");
		in_bits = 1;
	}
	if (out_bits == 0) {
		report_FSM_error("There must be at least one output bit.");
		out_bits = 1;
	}
	type -= '0';
	ib_size = 1 << in_bits;
	ob_size = 1 << out_bits;
	replace_spaces(name, REPLACE_SPACE);

	if (!(in >> str >> num_states)) {
		report_FSM_error("Failed to read state header");
		return 0;
	}
	if (str != "states") {
		report_FSM_error("Expecting 'states' keyword");
		return 0;
	}
	if (init_state >= num_states) {
		report_FSM_error("Init state out of range");
		return 0;
	}
	for (n_states = 0, i = 0; i < num_states; ++i) {
		if (!(in >> id >> label >> out_val >> location)) {
			report_FSM_error("Failed to read state data");
			return 0;
		}
		if (out_val >= ob_size) {
			report_FSM_error("State output value is out of range");
			return 0;
		}
		if (report_errors && id != i) {
			cout << "Warning: Expected state ID of " << i << " but got ID of " << id << "." << endl;
		}
		check_legal_location(location);
		state = new FSM_state(location, out_val, label, out_bits);
		add_state(state, (char)(i == init_state));
		state->assign_id(id); // ???
	}
	if (!(in >> str >> num_trans)) {
		report_FSM_error("Failed to read transition information");
		return 0;
	}
	if (str != "transitions") {
		report_FSM_error("Expecting 'transitions' keyword");
		return 0;
	}
	for (n_trans = 0, i = 0; i < num_trans; ++i) {
		if (!(in >> id >> label >> in_val >> out_val >> start >> end >> source_id >> sink_id)) {
			report_FSM_error("Failed to read state data");
			return 0;
		}
		if (source_id >= (int)n_states || sink_id >= (int)n_states) {
			report_FSM_error("State referenced from transition is nonexistent");
			return 0;
		}
		if (in_val >= ib_size) {
			report_FSM_error("Transition input value is out of range");
			return 0;
		}
		if (out_val >= ob_size) {
			report_FSM_error("Transition output value is out of range");
			return 0;
		}
		if (report_errors && id != i) {
			cout << "Warning: Expected transition ID of " << i << " but got ID of " << id << "." << endl;
		}
		check_legal_location(start);
		check_legal_location(end);
		if (source_id >= 0)
			source = states[source_id];
		else
			source = NULL;

		if (sink_id >= 0)
			sink = states[sink_id];
		else
			sink = NULL;

		trans = new FSM_trans(start, end, in_val, out_val, label, source, sink, in_bits, out_bits);
		add_transition(trans);
		trans->assign_id(id); // ???
	}
	return complete();
}



int FSM::read(istream &in, unsigned &num_inputs, unsigned &num_outputs, string &fsm_name, char &fsm_type) {

	if (!read(in))
		return 0;

	num_inputs  = in_bits;
	num_outputs = out_bits;
	fsm_name    = name;
	fsm_type    = type;

	return 1;
}



int FSM::write(ostream &out) {

	unsigned i;

	if (!out.good())
		return 0;

	if (!(out << "FSM " << name << " " << (unsigned)type << " " << init_state << " " << in_bits << " " << out_bits << endl))
		return 0;

	out << "states " << n_states << endl;
	if (states != NULL) {
		for (i = 0; i < n_states; ++i) {
			if (!states[i]->print(out, 0))
				return 0;
		}
	}
	out << "transitions " << n_trans << endl;
	if (transitions != NULL) {
		for (i = 0; i < n_trans; ++i) {
			if(!transitions[i]->print(out, 0))
				return 0;
		}
	}
	return 1;
}




