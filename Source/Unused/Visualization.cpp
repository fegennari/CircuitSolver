// Visualization.cc
// Viz - Design Place and Route/Visualization Package Header
// Written by Frank Gennari 11/24/00

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Visualization.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.
#include "Visualization.h"


// Viz global settings
int name_on_top(DEF_name_on_top); 
int min_block_x(DEF_min_block_x);
int min_block_y(DEF_min_block_y);
int init_block_spacing(DEF_init_block_spacing);
int min_block_spacing(DEF_min_block_spacing);
int pick_first_placement(DEF_pick_first_placement); 
int trade_quality_for_speed(DEF_trade_quality_for_speed);
int ask_to_save(DEF_ask_to_save);
int show_legend(DEF_show_legend);
int do_fill(DEF_do_fill);
int draw_bit_slashes(DEF_draw_bit_slashes);
double center_pull(DEF_center_pull);

// defined in Circuit Solver
extern int x_limit, y_limit, graph_enable, drawing_color, whiteout_text_bkg;
extern box num;


void runVisualizer(); 
void vizFreeMemory(vizObject* objects, vizLegendEntry *legend, int num_objects);
void kill_internal_points(vizObject *objects, int num_objects);
void draw_button_panel(char *button_functions);
void draw_legend(vizLegendEntry *legend);
void add_legend_entry(string& name, int color, int fill, int position);
int decode_event(int &position);
int vizOpen(vizObject *&objects, vizLegendEntry *&legend, int& read_placements, int& num_objects);
int vizParse(vizObject *&objects, vizLegendEntry *&legend, int& red_placements, ifstream& infile);
int look_for_keyword(string keyword, ifstream& infile);
int look_for_2keywords(string keyword1, string keyword2, ifstream& infile);
void change_settings(int index, double settings);
void calc_object_size(vizObject& object, double R);
void vizSave(vizObject *objects, vizLegendEntry *legend, int num_objects);
void vizWrite(vizObject *objects, vizLegendEntry *legend, int num_objects, ofstream& outfile);
void vizRedraw(vizObject *objects, vizLegendEntry *legend, int num_objects, char *button_functions, int draw_wires);
void draw_objects(vizObject *objects, int num_objects, int draw_wires);
void draw_one_object(vizObject *objects, int index);
void move_object(vizObject *objects, vizLegendEntry *legend, int num_objects, int *window_grid, int *routing_grid, int position, char *button_functions);
int test_overlap(int position, int *window_grid, vizObject& object, int index);
int check_port_remap(int *window_grid, int *routing_grid, int position, int index);
void reset_grid(int *window_grid, int *routing_grid);
void update_grid(vizObject *objects, int num_objects, int *window_grid);
void sort_objects(vizObject *objects, int num_objects);
int determine_min_area(vizObject *objects, int num_objects, int& spacing);
void calc_wire_params(vizObject *objects, int num_objects, int *window_grid, int *routing_grid);
void place_and_route(vizObject *objects, vizLegendEntry *legend, int num_objects, int *window_grid, int *routing_grid, char *button_functions, int function);
int run_placer(vizObject *objects, int num_objects, int *window_grid, int& spacing);
int place_object(vizObject *objects, int *window_grid, int index, int spacing, int num_objects);
double calc_routing_cost(vizObject *objects, int *window_grid, int index, int position, int num_objects);
int run_router(vizObject *objects, int num_objects, int *window_grid, int *routing_grid, int spacing);
int assign_ports(vizObject *objects, int num_objects, int *window_grid, int *routing_grid);
int map_ports(vizObject *objects, vizObject& object, int *source, int *dest, int *perm, int is_node);
int get_nth_port(vizObject& object, int index);
int test_wire_placement(int *window_grid, int start, int end);
int get_arrow_dir(vizObject& object, int start_loc, int has_arrow);
int get_center_of_obj(vizObject& object);
vizWire default_wire(int source, int dest, int bits, int dest_wire_id);
void add_new_point(vizWire& wire, int location);
int distance(int start, int end);
void vizDrawWire(vizWire& wire, vizObject *objects, int num_objects);
void vizDrawArrow(point c, int orientation);
void vizDrawBitLine(point c, int orientation, int bits);
point get_port_offset(int orientation, int location);


// Defined in Circuit Solver
void set_text_size(int size);

void draw_string(char *display);

void draw_text(string text, int box_num, int t_size);

point get_center(int box_num);

void close_drawing_window();





void runVisualizer() {

	int num_objects(0), position, operation, read_placements, changed(0);
	int window_grid[VIZ_NUM_BOXES], routing_grid[VIZ_NUM_BOXES];
	char button_functions[VIZ_NUM_BUTTONS] = {0};
	vizObject *objects;
	vizLegendEntry *legend;
	
	//y_limit = x_limit/2; // Makes screen bigger, but screws up aspect ratio.
	
	set_buttons(VIZ_NUM_BOXES, VIZ_GRID_COLOR, 0);	
	draw_button_panel(button_functions);
	draw_text(" Welcome to Viz.", num.h_boxes + 1, 16);
	
	reset_grid(window_grid, routing_grid);
		
	cout << "Click on an object to move it or click on a button to the right." << endl;
	
	while (1) {
		operation = decode_event(position);
		
		switch (operation) {
		
			case xNOOP: // do nothing
			case xBLANK: 
				break;
				
			case xOPEN:
				if (num_objects > 0 && changed && ask_to_save) {
					cout << "\nSave current design? ";
					if (decision()) {
						vizSave(objects, legend, num_objects);
						changed = 0;
					}
				}
				reset_grid(window_grid, routing_grid);
				
				if (vizOpen(objects, legend, read_placements, num_objects)) {
					if (num_objects <= 0) 
						button_functions[xSAVE-1] = 0;
					else {
						place_and_route(objects, legend, num_objects, window_grid, routing_grid, button_functions, !read_placements);
						changed = !read_placements;
						button_functions[xSAVE-1] = 1;
					}
				}
				vizRedraw(objects, legend, num_objects, button_functions, 1);
				cout << "Click on an object to move it or click on a button to the right." << endl;
				break;
				
			case xSAVE:
				if (button_functions[xSAVE-1] == 0) 
					break;
				vizSave(objects, legend, num_objects);
				changed = 0;
				cout << "Click on an object to move it or click on a button to the right." << endl;
				break;
				
			case xREDRAW:
				vizRedraw(objects, legend, num_objects, button_functions, 1);
				cout << "Click on an object to move it or click on a button to the right." << endl;
				break;
				
			case xGRID:
				button_functions[xGRID-1] = !button_functions[xGRID-1];
				vizRedraw(objects, legend, num_objects, button_functions, 1);
				cout << "Click on an object to move it or click on a button to the right." << endl;
				break;
				
			case xEXIT:
				if (num_objects > 0 && changed && ask_to_save) {
					cout << "\nSave current design? ";
					if (decision())
						vizSave(objects, legend, num_objects);
				}
				vizFreeMemory(objects, legend, num_objects);
				if (graph_enable == 1) {
					close_drawing_window();
 					graph_enable = 0;
 				}
				return;
				
			case xCLICK:
				move_object(objects, legend, num_objects, window_grid, routing_grid, position, button_functions);
				cout << "Click on an object to move it or click on a button to the right." << endl;
				changed = 1;
				break;
		}
	}
}




void vizFreeMemory(vizObject* objects, vizLegendEntry *legend, int num_objects) { 
	
	kill_internal_points(objects, num_objects);
			
	for (unsigned int i = 0; i < num_objects; ++i) 
		if (objects[i].ports > 0) 		
			delete [] objects[i].wire;
			
	if (num_objects > 0)
		delete [] objects;
		
	if (show_legend) {
		delete [] legend;
		show_legend = 0;
	}		
}




void kill_internal_points(vizObject *objects, int num_objects) {
	
	for (unsigned int i = 0; i < num_objects; ++i)
		for (unsigned int j = 0; j < objects[i].ports; ++j)
			if (objects[i].wire[j].num_points > 2) {
				delete objects[i].wire[j].other_points;
				objects[i].wire[j].num_points = 2;
				objects[i].wire[j].point_cap = 0;
			}
}




void draw_button_panel(char *button_functions) {
	
	char b_name[VIZ_MAX_B_CHAR_LEN + 2] = {0};
	
	SetDrawingColorx(WHITE);
	RectangleDraw(.94*x_limit, 0, x_limit, y_limit);
	
	SetDrawingColorx(TEXT_COLOR);
	RectangleFrame(.94*x_limit, 0, x_limit, y_limit);
	
	set_text_size(12);
	
	for (unsigned int i = 1; i <= VIZ_NUM_BUTTONS; ++i) {
		LineDraw(.94*x_limit, .06*y_limit*i, x_limit, .06*y_limit*i);
		
		if (button_functions[i-1] == 0)
			string_to_char(b_name, VIZ_BUTTON_NAMES1[i-1], VIZ_BUTTON_LENGTHS1[i-1]);
		else
			string_to_char(b_name, VIZ_BUTTON_NAMES2[i-1], VIZ_BUTTON_LENGTHS2[i-1]);
			
		MoveTo(.95*x_limit, y_limit*(.06*i - .02));
		draw_string(b_name);
	}
}




void draw_legend(vizLegendEntry *legend) {
		
	if (!show_legend)
		return;
	
	for (unsigned int i = 0; i < show_legend; ++i) 
		add_legend_entry(legend[i].text, legend[i].color, legend[i].fill, i);
}




void add_legend_entry(string& name, int color, int fill, int position) {

	int y_pos = y_limit*(.06*(VIZ_NUM_BUTTONS + position + 2) - .02);
	
	SetDrawingColorx(TEXT_COLOR);
	set_text_size(9);
	MoveTo(.945*x_limit, y_pos);
	draw_string((char *)shift_chars(name).c_str());
	
	SetDrawingColorx(color);
	if (fill)
		RectangleDraw(.985*x_limit, y_pos-.035*y_limit, .998*x_limit, y_pos+.005*y_limit);
	else
		RectangleFrame(.985*x_limit, y_pos-.035*y_limit, .998*x_limit, y_pos+.005*y_limit);
}




int decode_event(int &position) {
	
	Point pt = GetClick();
		
	if (pt.h > .94*x_limit) {
		if (pt.v > .06*y_limit*VIZ_NUM_BUTTONS) {
			if (show_legend)
				cout << "\nYou cannot modify the legend." << endl;
			return xNOOP;
		}	
		return pt.v/(.06*y_limit) + 1;
	}	
	position = which_box(pt);
		
	return xCLICK;
}




int vizOpen(vizObject *&objects, vizLegendEntry *&legend, int &read_placements, int& num_objects) {
	
	ifstream infile;
	char vizName[MAX_SAVE_CHARACTERS + 1] = {0};
	
	cout << "\nOpen design: ";
	strcpy(vizName, xin(MAX_SAVE_CHARACTERS));
	
	infile.open(vizName, ios::in);
	
	if (infile.fail()) {
		cerr << "\nError opening input file, it cannot be found!" << endl;
		return 0;
	}
	if (num_objects > 0) {
		vizFreeMemory(objects, legend, num_objects);
		num_objects = 0;
	}
	num_objects = vizParse(objects, legend, read_placements, infile);
	infile.close();
		
	if (num_objects < 1) {
		cerr << "The file could not be read because of an error." << endl;
		if (num_objects != 0)
			vizFreeMemory(objects, legend, -num_objects);
			
		num_objects = 0;
		return 0;
	}	
	return 1;
}




int vizParse(vizObject *&objects, vizLegendEntry *&legend, int& read_placements, ifstream& infile) {

	int num_objects(0), num_edges(0), color_id(0), val, index, bit_width(1), ports;
	int next_wire(0), num_nodes(0), MAX_EDGES, s_pos, e_pos;
	unsigned int i, j;
	char direction;
	double aspect_ratio(1), settings;
	string str, color_str, fill_str;
	StringTable objNames(0), colorNames(0), sourceNames(0), setNames(0);
	
	// check for placed or unplaced	
	val = look_for_2keywords(".placed", ".unplaced", infile);
	if (val == 1) {
		read_placements = 1;
		cout << "\nThis design has already been placed." << endl;
	}
	else if (val == 2)
		read_placements = 0;
	else 
		return 0;
		
	for (i = 0; i <= num_colors; ++i)
		colorNames.insert(color_names[i], i);
		
	for (i = 0; i < VIZ_NUM_SETTINGS; ++i)
		setNames.insert(viz_settings[i], i);
	
	// read settings	
	if (infile.good() && infile >> str && str == ".settings") {
		while (infile.good() && infile >> str) {
			index = setNames.search(str);
			if (index == default_st_val) {
				if (str == ".nodes")
					val = 1;
				else if (str == ".legend")
					val = 2;
				else {
					cerr << "\nError: Unrecognized symbol in settings: " << str << "." << endl;
					return 0;
				}
				break;
			}
			else {
				if (infile.good() && infile >> settings)
					change_settings(index, settings);
				else {
					cerr << "\nError: Unable to read settings for variable " << str << "." << endl;
				}
			}
		}
	}
	else {
		infile.clear();
		infile.seekg(0);
		infile >> str;
		val = 0;
	}		
	if (val == 0)
		val = look_for_2keywords(".nodes", ".legend", infile);
			
	if (val == 0)
		return 0;
	
	// read legend	
	else if (val == 2) { 
		if (!(infile.good() && infile >> val && val > 0 && val < VIZ_MAX_LEGEND)) {
			cerr << "\nError reading legend info. Make sure there are at most " << VIZ_MAX_LEGEND << " legend entries." << endl;
			return 0;
		}
		show_legend = val;
		if (!(legend = new vizLegendEntry[val])) {
			cout << "\nError: Not enough memory to create legend." << endl;
			exit(1);
		}
		for (i = 0; i < val; ++i) {
			if (!(infile.good() && infile >> str >> color_str >> fill_str)) {
				cerr << "\nError reading legend info." << endl;
				delete [] legend;
				show_legend = 0;
				return 0;
			}
			legend[i].text = str;
			color_id = colorNames.search(color_str);
			if (color_id < 1) {
				if (color_id == default_st_val)
					cout << "Warning: Unrecognized color: " << color_str << "." << endl;
				legend[i].color = DEFAULT_COLOR;
			}
			else
				legend[i].color = color_id;
				
			if (fill_str == "solid") 
				legend[i].fill = 1;
			else if (fill_str == "outline")
				legend[i].fill = 0;
			else {
				cout << "Warning: Legend fill string does not match 'solid' or 'outline': " << fill_str << "." << endl;
				legend[i].fill = do_fill;
			}
		}
		if (!look_for_keyword(".nodes", infile)) {
			delete [] legend;
			show_legend = 0;
			return 0;
		}
	}
	else
		show_legend = 0;
	
	// read node info			
	if (!(infile.good() && infile >> num_objects && num_objects > 0 && num_objects < VIZ_MAX_OBJECTS)) {
		cerr << "\nError reading number of objects." << endl;
		if (show_legend) {
			delete [] legend;
			show_legend = 0;
		}
		return 0;
	}
	if (!(objects = new vizObject[num_objects])) {
		cerr << "\nError: Not enough memory to create objects." << endl;
		exit(1);
	}
	for (i = 0; i < num_objects; ++i)
		objects[i].ports = 0;
	
	// max that will fit in the screen
	MAX_EDGES = 2*(num.h_boxes + num.v_boxes) - 4*min_block_spacing;
	
	// read node definitions	
	for (i = 0; i < num_objects; ++i) {	
		val = look_for_2keywords(".node", ".conn", infile);
		if (val == 0)
			return -num_objects;
		
		if (!(infile.good() && infile >> objects[i].name >> str >> color_str >> fill_str >> aspect_ratio)) {
			cerr << "\nError reading node definition." << endl;
			return -num_objects;
		}
		objects[i].type = str;
		
		color_id = colorNames.search(color_str);
		if (color_id < 1) {
			if (color_id == default_st_val)
				cout << "Warning: Unrecognized color: " << color_str << "." << endl;
			objects[i].color = DEFAULT_COLOR;
		}
		else
			objects[i].color = color_id;
			
		if (fill_str == "solid") 
			objects[i].is_filled = 1;
		else if (fill_str == "outline")
			objects[i].is_filled = 0;
		else {
			cout << "Warning: Object fill string does not match 'solid' or 'outline': " << fill_str << "." << endl;
			objects[i].is_filled = do_fill;
		}
		if (!(infile.good() && infile >> num_edges && num_edges >= 0 && num_objects < MAX_EDGES)) {
			cerr << "\nError reading number of edges." << endl;
			return -num_objects;
		}
		objects[i].ports = num_edges;
		if (val == 1)
			objects[i].next_wire = NODE_WIRE;
			
		if (num_edges > 0) {
			if (!(objects[i].wire = new vizWire[num_edges])) {
				cerr << "\nError: Not enough memory to create wires." << endl;
				exit(1);
			}
		}
		else
			objects[i].wire = NULL;
		
		if (val == 1) { // node
			++num_nodes;
			objects[i].bit_width = 1;
		}
		else if (val == 2) { // connection
			if (infile.good() && infile >> bit_width && bit_width > 0)
				objects[i].bit_width = bit_width;
			else {
				cerr << "\nError: Invalid bit width." << endl;
				return -num_objects;
			}
			objects[i].next_wire = 0;
		}				
		calc_object_size(objects[i], aspect_ratio);
		
		if (read_placements) {
			if (infile.good() && infile >> val && val >= 0 && val < VIZ_NUM_BOXES)
				objects[i].location = val;
			else {
				cerr << "\nError: Invalid placement of object in placed design." << endl;
				return -num_objects;
			}
		}
		else
			objects[i].location = NOWHERE;
	}
	// sort by number of ports
	sort_objects(objects, num_objects);
	
	for (i = 0; i < num_objects; ++i)
		objNames.insert(objects[i].name, i);
		
	if (!look_for_keyword(".edges", infile))
		return -num_objects;
		
	// read edges	
	for (i = 0; i < num_nodes; ++i) {
		if (!look_for_keyword(".edge", infile))
			return -num_objects;
		
		if (!(infile.good() && infile >> str)) {
			cerr << "\nError: Cannot read source edge name." << endl;
			return -num_objects;
		}
		index = objNames.search(str);
		if (index == default_st_val || index < 0 || index > num_objects) {
			cerr << "\nError: Invalid source edge name." << endl;
			return -num_objects;
		}
		if (sourceNames.search(str) == 0) {
			cerr << "\nError: Duplicate edges entry for node " << str << "." << endl;
			return -num_objects;
		}
		sourceNames.insert(str, 0);
		
		ports = objects[index].ports;
		for (j = 0; j < ports; ++j) {
			if (!(infile.good() && infile >> str >> direction)) {
				cerr << "\nError: expecting more edge destinations." << endl;
				return -num_objects;
			}
			val = objNames.search(str);
			if (val == default_st_val || val < 0 || val > num_objects) {
				cerr << "\nError: Unrecognized destination object name: " << str << "." << endl;
				return -num_objects; 
			}	
			next_wire = objects[val].next_wire;
			
			if (next_wire >= objects[val].ports) {
				cerr << "\nError: Too many ports referenced on block " << objects[val].name << "." << endl;
				return -num_objects;
			}
			objects[index].wire[j] = default_wire(index, val, objects[val].bit_width, next_wire); // node -> comm
			objects[val].wire[next_wire] = default_wire(val, index, objects[val].bit_width, j); // comm -> node
			
			if (read_placements) {
				if (infile.good() && infile >> s_pos >> e_pos && s_pos >= 0 
					&& s_pos < VIZ_NUM_BOXES && e_pos >= 0 && e_pos < VIZ_NUM_BOXES) {
					objects[index].wire[j].start = s_pos;
					objects[index].wire[j].end = e_pos;
					objects[index].wire[j].location_set = 1;
					objects[val].wire[next_wire].start = e_pos;
					objects[val].wire[next_wire].end = s_pos;
					objects[val].wire[next_wire].location_set = 1;	
				}
				else {
					cerr << "\nError: Invalid placement of edge/wire in placed design." << endl;
					return -num_objects;
				}
			}
			++objects[val].next_wire;
			
			switch (direction) {
				case 'i':
				case 'I': // input
					objects[index].wire[j].s_arrow = 1;
					objects[val].wire[next_wire].e_arrow = 1;
					break;
				case 'o':
				case 'O': // output
					objects[index].wire[j].e_arrow = 1;
					objects[val].wire[next_wire].s_arrow = 1;
					break;
				case 'b':
				case 'B': // bidirectional
					objects[index].wire[j].s_arrow = objects[index].wire[j].e_arrow = 1;
					objects[val].wire[next_wire].s_arrow = objects[val].wire[next_wire].e_arrow = 1;
					break;
				case 'n':
				case 'N': // no direction
					break;
				default:
					cerr << "\nError: Invalid edge direction: " << direction << " - does not match i, o, b, n. Default of no direction (n) will be asigned to this edge.";				
			}
		}		
	}
	// check edges
	for (i = 0; i < num_objects; ++i) {
		if (objects[i].next_wire != NODE_WIRE && objects[i].next_wire != objects[i].ports)
			cerr << "\nError: object " << objects[i].name << " does not have the correct number of ports assigned to it. Will continue anyway." << endl;
	}
	if (!look_for_keyword(".end", infile))
		cout << "\nThe file was otherwise read successfully." << endl;
			
	return num_objects;
}




int look_for_keyword(string keyword, ifstream& infile) {

	string str;

	if (!(infile.good() && infile >> str && str == keyword)) {
		cerr << "\nError reading file: Expecting keyword " << keyword << "." << endl;
		return 0;
	}
	return 1;
}




int look_for_2keywords(string keyword1, string keyword2, ifstream& infile) {

	string str;

	if (infile.good() && infile >> str) {
		if (str == keyword1)
			return 1;
		else if (str == keyword2)
			return 2;
		cerr << "\nError reading file: Expecting keywords " << keyword1 << " or " << keyword2 << "." << endl;
		return 0;
	}
	cerr << "\nError reading file: Expecting keywords " << keyword1 << " or " << keyword2 << "." << endl;
	return 0;
}




void change_settings(int index, double settings) {

	switch (index) {
		case 0:
			name_on_top = settings;
			break;
		case 1:
			min_block_x = settings;
			break;
		case 2:
			min_block_y = settings;
			break;
		case 3:
			init_block_spacing = settings;
			break;
		case 4:
			min_block_spacing = settings;
			break;
		case 5:
			pick_first_placement = settings;
			break;
		case 6:
			trade_quality_for_speed = settings;
			break;
		case 7:
			ask_to_save = settings;
			break;
		case 8:
			do_fill = settings;
			break;
		case 9:
			draw_bit_slashes = settings;
			break;
		case 10:
			center_pull = settings;
			break;
	}
}




void calc_object_size(vizObject& object, double R) {

	int P = object.ports;

	object.xsize = ceil(0.5*R*P/(1+R));
	object.ysize = floor(0.5*P/(1+R));
	
	if (object.xsize < min_block_x) {
		object.xsize = min_block_x;
		object.ysize = min_block_x/R;
	}
	if (object.ysize < min_block_y) {
		object.ysize = min_block_y;
		object.xsize = min_block_y*R;
	}	
}



// adds position information
void vizSave(vizObject *objects, vizLegendEntry *legend, int num_objects) { 

	ofstream outfile;
	char vizName[MAX_SAVE_CHARACTERS + 1] = {0};

	if (num_objects == 0) {
		cout << "\nThere is no design to save!" << endl;
		return;
	}
	cout << "\nSave design as: ";
	strcpy(vizName, xin(MAX_SAVE_CHARACTERS));
	
	outfile.open(vizName, ios::out | ios::trunc);
	
	if (outfile.fail()) {
		cerr << "\nError creating output file, it cannot be written!" << endl;
		return;
	}
	vizWrite(objects, legend, num_objects, outfile);
	outfile.close();
}




void vizWrite(vizObject *objects, vizLegendEntry *legend, int num_objects, ofstream& outfile) { // *** Change ***

	unsigned int i, j;
	vizObject object;
	
	// write settings
	outfile << ".placed" << endl << ".settings" << endl << "name_on_top " << name_on_top
			<< endl << "min_block_x " << min_block_y << endl << "min_block_y " << min_block_x
			<< endl << "init_block_spacing " << init_block_spacing << endl << "min_block_spacing " 
			<< min_block_spacing << endl << "pick_first_placement " << pick_first_placement << endl 
			<< "trade_quality_for_speed " << trade_quality_for_speed << endl << "ask_to_save " 
			<< ask_to_save << endl << "do_fill " << do_fill << endl << "draw_bit_slashes "
			<< draw_bit_slashes << endl << "center_pull " << center_pull << endl; 
	
	// write legend
	if (show_legend) {		
		outfile << ".legend " << show_legend << endl;
		for (i = 0; i < show_legend; ++i) {
			outfile << legend[i].text << " " << color_names[legend[i].color];
			if (legend[i].fill == 1)
				outfile << " solid" << endl;
			else
				outfile << " outline" << endl;
		}	
	}
	outfile << ".nodes " << num_objects << endl;
	
	// write nodes
	for (i = 0; i < num_objects; ++i) {		
		if (objects[i].next_wire == NODE_WIRE)
			outfile << ".node ";
		else
			outfile << ".conn ";	
	
		outfile << objects[i].name << " " << objects[i].type << " "
				<< color_names[objects[i].color];
				
		if (objects[i].is_filled == 1)
			outfile << " solid ";
		else
			outfile << " outline ";
			
		outfile << ((double)objects[i].xsize/(double)objects[i].ysize) << " " << objects[i].ports;
		
		if (objects[i].next_wire != NODE_WIRE)
			outfile << " " << objects[i].bit_width;
		
		outfile << " " << objects[i].location << endl; 
	}
	outfile << ".edges" << endl;
	
	// write edges
	for (i = 0; i < num_objects; ++i) {
		object = objects[i];
		if (object.next_wire == NODE_WIRE) {
			outfile << ".edge " << object.name << " ";
		
			for (j = 0; j < object.ports; ++j) {
				outfile << objects[object.wire[j].dest].name;
				 
				if (object.wire[j].s_arrow > 0) {
					if (object.wire[j].e_arrow > 0)
						outfile << " b ";
				 	else
				 		outfile << " i ";
				 }
				 else if (object.wire[j].e_arrow > 0)
				 	outfile << " o ";
				 else
				 	outfile << " n ";
				 	
				 outfile << object.wire[j].start << " " << object.wire[j].end << " ";
			}
			outfile << endl;
		}
	}
	outfile << ".end" << endl;
}




void vizRedraw(vizObject *objects, vizLegendEntry *legend, int num_objects, char *button_functions, int draw_wires) {
	
	whiteout();
	
	if (button_functions[xGRID-1] == 1) 
		set_buttons(VIZ_NUM_BOXES, WHITE, 0);	
	else 
		set_buttons(VIZ_NUM_BOXES, VIZ_GRID_COLOR, 0);
	
	draw_objects(objects, num_objects, draw_wires);
	draw_button_panel(button_functions);
	draw_legend(legend);
}




void draw_objects(vizObject *objects, int num_objects, int draw_wires) {

	unsigned int i, j;
	
	// draw blocks
	for (i = 0; i < num_objects; ++i)
		draw_one_object(objects, i);
	
	// draw wires (edges, ports)
	if (draw_wires)
		for (i = 0; i < num_objects; ++i)
			if (objects[i].next_wire == NODE_WIRE)
				for (j = 0; j < objects[i].ports; ++j)
					vizDrawWire(objects[i].wire[j], objects, num_objects);
}




void draw_one_object(vizObject *objects, int index) {

	double gpix = x_limit/num.h_boxes;
	int name_pos;
	point c;
	vizObject object = objects[index];
	
	if (object.location < 0 || object.location >= VIZ_NUM_BOXES)
		return;
	
	c = get_center(object.location);
	
	c.x -= gpix/2;
	c.y -= gpix/2;
	
	SetDrawingColorx(object.color);
	
	// draw block rectangle
	if (object.is_filled == 1) {
		whiteout_text_bkg = 0;
		RectangleDraw(c.x, c.y-1, c.x+gpix*object.xsize-1, c.y+.98*gpix*object.ysize);
	}
	else		
		RectangleFrame(c.x, c.y-1, c.x+gpix*object.xsize-1, c.y+.98*gpix*object.ysize);
	
	// draw name
	name_pos = object.location - num.h_boxes*name_on_top + 1 + object.name.length()*(2.5*num.h_boxes/x_limit);
	
	if (object.name.length() == 1 || object.name.length()%2 == 0)
		--name_pos;
		
	draw_text(shift_chars(object.name), -name_pos, 9);
	
	if (object.is_filled == 1)
		whiteout_text_bkg = 1;
}




void move_object(vizObject *objects, vizLegendEntry *legend, int num_objects, int *window_grid, int *routing_grid, int position, char *button_functions) {
	
	int position2(0), loc2, index, out_of_window(0), overlap(0), operation, o_test, is_wire, dest, wire_id;
	unsigned int j, k;
	vizObject object;
	vizWire wire;
	
	if (position >= VIZ_NUM_BOXES) {
		cerr << "\nError: Invalid position: " << position << "." << endl;
		return;
	}
	index = window_grid[position];
			
	if (index == NO_OBJECT)
		return;
	if (index >= num_objects || (index + 2) < -num_objects) {
		cerr << "\nError: Invalid grid array entry: " << index << "." << endl;
		return;
	}
	if (index >= 0) {
		object = objects[index];
		objects[index].location = NOWHERE;
		is_wire = 0;
	}
	else {
		index = -index - 2;
		is_wire = 1;
		wire_id = routing_grid[position];
		if (wire_id < 0) {
			cerr << "\nError: Invalid routing ID: " << wire_id << "." << endl;
			return;
		}
		wire = objects[index].wire[wire_id];
		dest = wire.dest;
		objects[index].wire[wire_id].start = NOWHERE;
		objects[index].wire[wire_id].end = NOWHERE;
		objects[dest].wire[wire.dest_match_wire_id].start = NOWHERE;
		objects[dest].wire[wire.dest_match_wire_id].end = NOWHERE; 
	}
	button_functions[xEXIT-1] = 1;
	vizRedraw(objects, legend, num_objects, button_functions, is_wire);
	
	if (is_wire)
		cout << "\nClick to remap this wire." << endl;
	else
		cout << "\nBlock: "<< object.name << ", type: " << object.type << ", ports: " << object.ports << endl;
		
	do {
		operation = decode_event(position2);
		if (operation == xEXIT || position == position2) { // cancel
			button_functions[xEXIT-1] = 0;
			
			// put it back where it was
			if (is_wire) {
				objects[index].wire[wire_id].start = wire.start;
				objects[index].wire[wire_id].end = wire.end;
				objects[dest].wire[wire.dest_match_wire_id].start = wire.end;
				objects[dest].wire[wire.dest_match_wire_id].end = wire.start;
			}	
			else
				objects[index].location = object.location; 
				
			vizRedraw(objects, legend, num_objects, button_functions, 1);
			return;
		}
		else if (operation == xCLICK && position >= 0 && position2 < VIZ_NUM_BOXES) {
			if (is_wire) {
				o_test = check_port_remap(window_grid, routing_grid, position2, index);
				if (o_test == 0) {
					cerr << "\nError: Wire must be connected to an unoccupied port on the same block!";
					cout << "\nClick to place the object." << endl;
					operation = xNOOP;
				}
			}
			else {
				o_test = test_overlap((position2 - position + object.location), window_grid, object, index);	
				if (o_test == 1) {
					cerr << "\nError: Cannot place this object on top of another object or off the screen!";
					cout << "\nClick to place the object." << endl;
					operation = xNOOP;
				}
				else if (o_test == 2) {
					cerr << "\nError: Object Cannot be placed here!";
					cout << "\nClick to place the object." << endl;
					operation = xNOOP;
				}
			}
		}	
	} while (operation != xCLICK || position2 > VIZ_NUM_BOXES);
	
	if (is_wire) {
		objects[index].wire[wire_id].start = position2;
		objects[index].wire[wire_id].end = wire.end;
		objects[dest].wire[wire.dest_match_wire_id].start = wire.end;
		objects[dest].wire[wire.dest_match_wire_id].end = position2;
		
		if (wire.s_arrow != 0) {
			objects[index].wire[wire_id].s_arrow = abs(wire.s_arrow)/wire.s_arrow*o_test;
			objects[dest].wire[wire.dest_match_wire_id].e_arrow = abs(wire.s_arrow)/wire.s_arrow*o_test;
		}
		window_grid[position2] = window_grid[position];
		routing_grid[position2] = routing_grid[position];
		window_grid[position] = NO_OBJECT;
		routing_grid[position] = NO_OBJECT;
		button_functions[xEXIT-1] = 0;
	}
	else {
		position2 -= position - object.location;
	
		for (j = 0; j < object.xsize; ++j) {
			for (k = 0; k < object.ysize; ++k) {
				loc2 = object.location + j + num.h_boxes*k;
				if (loc2 >= 0 && loc2 < VIZ_NUM_BOXES && window_grid[loc2] == index)
					window_grid[loc2] = NO_OBJECT;	
			}
		}
		for (j = 0; j < object.xsize; ++j) {
			for (k = 0; k < object.ysize; ++k) {
				loc2 = position2 + j + num.h_boxes*k;
				if (loc2 >= 0 && loc2 < VIZ_NUM_BOXES && ((loc2%(int)num.h_boxes) < .94*num.h_boxes)) {
					if (window_grid[loc2] != index) {
						if (window_grid[loc2] >= 0) 
							overlap = 1;
						else
							window_grid[loc2] = index;
					}
				}
				else
					out_of_window = 1;
			}
		}
		if (out_of_window)
			cout << "\nWarning: Part of the object extends out of the valid drawing area." << endl;
		if (overlap)
			cout << "\nWarning: Part of the newly placed object overlaps with another object." << endl;
		
		objects[index].location = position2;
		button_functions[xEXIT-1] = 0;		
		vizRedraw(objects, legend, num_objects, button_functions, 1);
		run_router(objects, num_objects, window_grid, routing_grid, min_block_spacing); // ???
	}
	vizRedraw(objects, legend, num_objects, button_functions, 1);
}




int test_overlap(int position, int *window_grid, vizObject& object, int index) {

	int location, yline = position/num.h_boxes;
	
	if (position < 0)
		return 2;
		
	for (unsigned int j = 0; j < object.xsize; ++j) {
		if ((int)((position + j)/num.h_boxes) != yline) 
			return 2; // off left side or top of screen
		
		for (unsigned int k = 0; k < object.ysize; ++k) {
			location = position + j + num.h_boxes*k;	
			if (location < 0) 
				return 2; // position error
				
			if (location < VIZ_NUM_BOXES && window_grid[location] >= 0 
				&& window_grid[location] != index)
				return 1; // overlaps another object (block or wire)	
		}
	}
	return 0;
}




int check_port_remap(int *window_grid, int *routing_grid, int position, int index) {
		
	if (position >= VIZ_NUM_BOXES || routing_grid[position] != NO_OBJECT
		|| window_grid[position] != NO_OBJECT) // port occupied
		return 0;
	
	int h_boxes = num.h_boxes;
		
	if ((position+h_boxes) < VIZ_NUM_BOXES && window_grid[position+h_boxes] == index)
		return 1; // \/
		
	if ((position-1) >= 0 && window_grid[position-1] == index)
		return 2; // <-
		
	if ((position-h_boxes) >= 0 && window_grid[position-h_boxes] == index)	
		return 3; // /
		
	if ((position+1) < VIZ_NUM_BOXES && window_grid[position+1] == index)
		return 4; // ->	
				
	return 0; // invalid port
}




void reset_grid(int *window_grid, int *routing_grid) {

	for (unsigned int i = 0; i < VIZ_NUM_BOXES; ++i)
		window_grid[i] = routing_grid[i] = NO_OBJECT;
}




void update_grid(vizObject *objects, int num_objects, int *window_grid) {

	int overlap(0), this_overlap, out_of_window(0), this_oow, loc2;
	unsigned int i;
	vizObject object;
		
	for (i = 0; i < num_objects; ++i) {
		this_overlap = this_oow = 0;
		object = objects[i];
		for (unsigned int j = 0; j < object.xsize; ++j) {
			for (unsigned int k = 0; k < object.ysize; ++k) {
				loc2 = object.location + j + num.h_boxes*k;
				if (loc2 >= 0 && loc2 < VIZ_NUM_BOXES) {
					if (window_grid[loc2] != NO_OBJECT)
						this_overlap = 1;
					window_grid[loc2] = i;
				}
				else
					this_oow = 1;
			}
		}
		if (this_overlap)
			++overlap;
		if (this_oow)
			++out_of_window;
	}
	if (overlap) {
		cout << "\nWarning: " << overlap << " overlapping object";
		if (overlap > 1)
			cout << "s were found." << endl;
		else
			cout << " was found." << endl;
	}
	if (out_of_window) {
		cout << "\nWarning: " << out_of_window << " object";
		if (out_of_window > 1)
			cout << "s are out of the valid drawing area." << endl;
		else
			cout << " is out of the valid drawing area." << endl;
	
	}
}




// not the fastest sort, but it's not like we're going to have over 1000
// objects on the screen at once
void sort_objects(vizObject *objects, int num_objects) {

	unsigned int i, j, k;
	vizObject *sorted_objects = new vizObject[num_objects];
	
	for (i = 0; i < num_objects; ++i)
		sorted_objects[i].ports = sorted_objects[i].xsize = sorted_objects[i].ysize = 0;
	
	for (i = 0; i < num_objects; ++i) {
		for (j = 0; j < num_objects && objects[i].ports < sorted_objects[j].ports
			 || (objects[i].ports == sorted_objects[j].ports && objects[i].xsize*objects[i].ysize
			 < sorted_objects[j].xsize*sorted_objects[j].ysize); ++j) {};
		
		for (k = num_objects - 1; k > j; --k)
			sorted_objects[k] = sorted_objects[k-1];
			
		sorted_objects[j] = objects[i];
	}
	for (i = 0; i < num_objects; ++i)
		objects[i] = sorted_objects[i];
	
	delete [] sorted_objects;
}




int determine_min_area(vizObject *objects, int num_objects, int& spacing) {
	
	int area;
	unsigned int i;
	
	++spacing;
	
	do {
		--spacing;
		area = spacing*(num.h_boxes + num.v_boxes - spacing);
		
		for (i = 0; i < num_objects; ++i)
			area += (objects[i].xsize + spacing)*(objects[i].ysize + spacing);
		
		if (area > VIZ_NUM_BOXES)
			cout << "Placement with a spacing of " << spacing << " is impossible." << endl;
				
	} while (spacing > min_block_spacing && area > VIZ_NUM_BOXES);
		
	return area;
}




void calc_wire_params(vizObject *objects, int num_objects, int *window_grid, int *routing_grid) {
	
	int start_loc;
	vizObject object;
	
	// for previously routed wires
	for (unsigned int i = 0; i < num_objects; ++i) {
		object = objects[i];
		for (unsigned int j = 0; j < object.ports; ++j) {
			start_loc = object.wire[j].start;
			if (start_loc < 0 || start_loc >= VIZ_NUM_BOXES) {
				cerr << "\nError: Starting location of wire is out of the drawing window: " << start_loc << "." << endl;
			}
			else {
				if (window_grid[start_loc] != NO_OBJECT || routing_grid[start_loc] != NO_OBJECT)
					cerr << "\nError: Illegal wire starting location read from placed and routed file." << endl;
								
				window_grid[start_loc] = -(i + 2);
				routing_grid[start_loc] = j;
				objects[i].wire[j].s_arrow = get_arrow_dir(object, start_loc, object.wire[j].s_arrow);
				objects[i].wire[j].e_arrow = get_arrow_dir(objects[object.wire[j].dest], object.wire[j].end, object.wire[j].e_arrow);
			}
		}
	}	
}




void place_and_route(vizObject *objects, vizLegendEntry *legend, int num_objects, int *window_grid, int *routing_grid, char *button_functions, int function) {

	int place_status(1), route_status, spacing = init_block_spacing;

	if (num_objects <= 0)
		return;
	
	if (init_block_spacing < min_block_spacing)
		cout << "\nWarning: Initial block spacing of " << init_block_spacing 
			 << " is less than minimum block spacing of " << min_block_spacing << "." << endl;
	
	if (function != 0 && determine_min_area(objects, num_objects, spacing) > VIZ_NUM_BOXES) {
		cout << "Placement has failed because the minimally spaced objects cannot possibly fit in the drawing area." << endl;
		place_status = 0;
		update_grid(objects, num_objects, window_grid);
	}
	if (function == 0) {
		update_grid(objects, num_objects, window_grid);
		vizRedraw(objects, legend, num_objects, button_functions, 1);
		calc_wire_params(objects, num_objects, window_grid, routing_grid);
	}	
	else {
		if (place_status == 1) {	
			cout << "\nAttempting placement with initial block spacing of " << spacing << "." << endl;
			place_status = run_placer(objects, num_objects, window_grid, spacing);
		}
		if (place_status == 0) 
			cout << "Placement failed." << endl;
		else
			cout << "Placement successful." << endl;
		
		if (spacing < 2)
			cout << "\nWarning: Proper routing may be impossible with a spacing of only " << spacing << "." << endl;
		
		vizRedraw(objects, legend, num_objects, button_functions, 1);
		
		if (place_status == 0) {
			cout << "Attempt to route the design anyway? ";
			if (!decision())
				return;
		}
		cout << "\nAttempting to route the design." << endl; 
		route_status = run_router(objects, num_objects, window_grid, routing_grid, spacing);
	
		if (route_status == 0) 
			cout << "Routing failed." << endl;
		else
			cout << "Routing successful." << endl;
	}	
	cout << endl;
}




int run_placer(vizObject *objects, int num_objects, int *window_grid, int& spacing) {

	int fail;
	unsigned int i;
	
	do {
		fail = 0;
		for (i = 0; i < VIZ_NUM_BOXES; ++i)
			window_grid[i] = NO_OBJECT; // erase previously placed objects
							
		for (i = 0; i < num_objects && fail != 1; ++i)
			if (!place_object(objects, window_grid, i, spacing, num_objects)) {
				if (spacing <= min_block_spacing) {
					cout << "Failed to place object " << objects[i].name << ".";
					if (fail == 0)
						cout << " Placement for this object has failed. Continuing to place other objects..." << endl;
					else
						cout << endl;
						
					fail = 2;
				}
				else {
					fail = 1;
					--spacing;
					cout << "Failed to place object " << objects[i].name << ".";
				 	cout << " Decreasing block spacing to " << spacing << "." << endl;
				}
			}
	} while (fail == 1);
	
	if (fail == 2)
		return 0;
		
	return 1;
}



// this is slow
int place_object(vizObject *objects, int *window_grid, int index, int spacing, int num_objects) { 
	
	int curr_loc, good_location(0), min_cost_location(0), placed(0), offset;
	int Xsize, Ysize, max_x(0.94*num.h_boxes + 1);
	double cost, min_cost(-1);
	unsigned int i, j, k;
	vizObject object = objects[index];
		
	offset = spacing*(num.h_boxes + 1);
	Xsize = object.xsize + 2*spacing;
	Ysize = object.ysize + 2*spacing;
	
	if (Xsize > num.h_boxes || Ysize > num.v_boxes)
		return 0;
	
	for (i = 0; (i + offset) < VIZ_NUM_BOXES; ++i) {
		good_location = 1;
		for (j = 0; j < Xsize && good_location; ++j) {
			for (k = 0; k < Ysize && good_location; ++k) {
				curr_loc = i + j + num.h_boxes*k;
				if (curr_loc >= VIZ_NUM_BOXES || window_grid[curr_loc] != NO_OBJECT 
					|| (curr_loc%(int)num.h_boxes) >= max_x)
						good_location = 0; // occupied or out of window
			}
		}
		if (good_location) {
			placed = 1;
			if (pick_first_placement) {
				min_cost_location = i + offset;
				break;
			}
			cost = calc_routing_cost(objects, window_grid, index, i + offset, num_objects);
			if (cost == LARGE_COST) 
				return 0;
			if (cost < min_cost || min_cost == -1) {
				min_cost = cost;
				min_cost_location = i + offset; // best routing driven placement
			}
			if (trade_quality_for_speed)
				i += j - 1;
		}
		else
			i += j - 1;
	}
	if (!placed) { // place failed
		if (spacing > min_block_spacing)
			return 0; // try again with closer spacing
		else
			objects[index].location = 0; // last attempt, just put it down
	}
	else
		objects[index].location = min_cost_location;
		
	for (j = 0; j < object.xsize; ++j) { // flag as occupied
		for (k = 0; k < object.ysize; ++k) {
			curr_loc = min_cost_location + j + num.h_boxes*k;
			window_grid[curr_loc] = index;
		}
	}
	return placed;
}




double calc_routing_cost(vizObject *objects, int *window_grid, int index, int position, int num_objects) {

	int cost_sum(0), dest_obj, screen_cent;
	point screen_center;
	vizObject object = objects[index];
	
	object.location = position;
	
	screen_center.x = x_limit/2;
	screen_center.y = y_limit/2;
	screen_cent = which_box(screen_center);
	
	if (window_grid[position] != NO_OBJECT) {
		cerr << "\nPlacement Error!" << endl;
		return LARGE_COST;
	}	
	for (unsigned int i = 0; i < object.ports; ++i) {		
		if (object.wire[i].dest != index) 
			dest_obj = object.wire[i].dest;
		else {
			cerr << "\nError: Illegal wire source or destination: Block is connected to itself!" << endl;
			return LARGE_COST;
		}
		if (dest_obj < 0 || dest_obj > num_objects) {
			cerr << "\nError: Illegal wire source or destination: Wire connects to nonexistent block!" << endl;
			return LARGE_COST;
		}
		if (dest_obj < index) // destination already placed
			cost_sum += distance(get_center_of_obj(object), get_center_of_obj(objects[dest_obj]));		
	} 
	return cost_sum + center_pull*distance(screen_cent, get_center_of_obj(object)); // pull towards the center
}




int run_router(vizObject *objects, int num_objects, int *window_grid, int *routing_grid, int spacing) { 
	
	int /*x_diff, y_diff,*/ new_point;
	unsigned int i, j;
	vizObject object;
	
	if (spacing < 1) {
		cout << "\nDesign could not be routed because of insufficient spacing between the blocks." << endl;
		return 0;
	}
	kill_internal_points(objects, num_objects);
	
	for (i = 0; i < VIZ_NUM_BOXES; ++i) {
		routing_grid[i] = NO_OBJECT; // erase wires
		if (window_grid[i] < 0)
			window_grid[i] = NO_OBJECT; // erase wire->object refs
	}
	if (!assign_ports(objects, num_objects, window_grid, routing_grid)) {
		cerr << "\nError: Design could not be routed because of invalid port assignments." << endl;
		return 0;
	}
	for (i = 0; i < num_objects; ++i) {
		object = objects[i];
		for (j = 0; j < object.ports; ++j) {	
			if (object.next_wire == NODE_WIRE && test_wire_placement(window_grid, object.wire[j].start, object.wire[j].end) == 0) {				
				//x_diff = (object.wire[j].start - object.wire[j].end)%(int)num.h_boxes;
				//y_diff = object.wire[j].start/num.h_boxes - object.wire[j].end/num.h_boxes;
				
				new_point = object.wire[j].start%(int)num.h_boxes 
							+ num.h_boxes*((int)(object.wire[j].end/num.h_boxes));
				
				add_new_point(object.wire[j], new_point); // must add points to both source and dest
				//return 0;
			}
		}
	}	
	return 1;
}



// calculate optimal logical port to physical grid point mapping
// by assigning start and end locations for the wires
int assign_ports(vizObject *objects, int num_objects, int *window_grid, int *routing_grid) {

	int max_ports(0), success(1);
	unsigned int i, j;
	vizObject object;
	vizWire wire;
	int *dest, *port, *perm;

	for (i = 0; i < num_objects; ++i)
		max_ports = max(max_ports, 2*(objects[i].xsize + objects[i].ysize));
		
	if (!(dest = new int[max_ports]) 
		|| !(port = new int[max_ports]) 
		|| !(perm = new int[max_ports])) {
			cerr << "\nInsufficient memory to create temporary portmap." << endl;
			exit(1);
	}	
	// first run on nodes
	for (i = 0; i < num_objects; ++i)  
		if (objects[i].next_wire == NODE_WIRE)
			if (map_ports(objects, objects[i], port, dest, perm, 1) == 0)
				success = 0;	

	// then run on communication channels
	for (i = 0; i < num_objects; ++i) 
		if (objects[i].next_wire == objects[i].ports) 
			if (map_ports(objects, objects[i], port, dest, perm, 0) == 0)
				success = 0;
				
	// run on nodes again now that comm ports have been assigned (is this necessary?)
	for (i = 0; i < num_objects; ++i)  
		if (objects[i].next_wire == NODE_WIRE)
			if (map_ports(objects, objects[i], port, dest, perm, 1) == 0)
				success = 0;
			
	for (i = 0; i < num_objects; ++i) {
		object = objects[i];
		for (j = 0; j < object.ports; ++j) { // copy wire end parameters from destination object		
			wire = objects[object.wire[j].dest].wire[object.wire[j].dest_match_wire_id];
			objects[i].wire[j].end = wire.start;
			objects[i].wire[j].e_arrow = wire.s_arrow;
			if (object.next_wire == NODE_WIRE) {
				routing_grid[object.wire[j].start] = j;
				routing_grid[object.wire[j].end] = object.wire[j].dest_match_wire_id;
				window_grid[object.wire[j].start] = -(i + 2);
				window_grid[object.wire[j].end] = -(object.wire[j].dest + 2);
			}
		}	
	}
	delete [] port;
	delete [] dest;
	delete [] perm;
	
	return success; 
}




int map_ports(vizObject *objects, vizObject& object, int *source, int *dest, int *perm, int is_node) {

	int obj_port = object.ports, avail_ports, sign;
	unsigned int j;
	
	avail_ports = 2*(object.xsize + object.ysize);
	
	if (avail_ports < obj_port)
		return 0; // insufficient ports
	
	for (j = 0; j < avail_ports; ++j) {
		if (j < obj_port) {
			if (is_node)
				dest[j] = get_center_of_obj(objects[object.wire[j].dest]);
			else // and you thought this was easy to understand?
				dest[j] = objects[object.wire[j].dest].wire[object.wire[j].dest_match_wire_id].start;
		}
		else 
			dest[j] = 0; 
		
		source[j] = get_nth_port(object, j);
		perm[j] = j;
	}
	for (unsigned int i = 0; i < avail_ports; ++i) {
		for (unsigned int j = 0; j < avail_ports; ++j) {
			if (i < obj_port && j < obj_port) {
				if (i != j && (distance(source[perm[i]], dest[i]) + distance(source[perm[j]], dest[j])) 
							> (distance(source[perm[i]], dest[j]) + distance(source[perm[j]], dest[i])))
		
					swap(perm[i], perm[j]);
			}
			else if (i < obj_port) {
				if (distance(source[perm[i]], dest[i]) > distance(source[perm[j]], dest[i]))
					swap(perm[i], perm[j]);
			}
			else if (j < obj_port) {
				if (distance(source[perm[j]], dest[j]) > distance(source[perm[i]], dest[j]))
					swap(perm[i], perm[j]);
			}
		}
	}		
	for (j = 0; j < obj_port; ++j) { 
		object.wire[j].location_set = 1;
		object.wire[j].start = source[perm[j]];
		
		if (object.wire[j].s_arrow <= 0)
			sign = -1;
		else
			sign = 1; 
		
		if (perm[j] < object.xsize) 
			object.wire[j].s_arrow = sign;		
		else if (perm[j] < object.xsize + object.ysize) 
			object.wire[j].s_arrow = 2*sign;
		else if (perm[j] < 2*object.xsize + object.ysize) 
			object.wire[j].s_arrow = 3*sign;
		else 
			object.wire[j].s_arrow = 4*sign; 
	}
	return 1;
}




int get_nth_port(vizObject& object, int index) {
	
	if (index < 0 || index >= 2*(object.xsize + object.ysize)) {
		cerr << "\nError: Illegal port index: " << index << "." << endl;
		return 0;
	}
	if (index < object.xsize) 
		return object.location + index - num.h_boxes;
		
	else if (index < object.xsize + object.ysize) 
		return object.location + num.h_boxes*(index - object.xsize) + object.xsize;
		
	else if (index < 2*object.xsize + object.ysize) 
		return object.location + (2*object.xsize + object.ysize - index - 1) + num.h_boxes*object.ysize;
	
	else 
		return object.location + num.h_boxes*(2*(object.ysize + object.xsize) - index - 1) - 1;
}




int test_wire_placement(int *window_grid, int start, int end) {

	unsigned int i;
	int x1(start%(int)num.h_boxes), x2(end%(int)num.h_boxes);
	int y1(start/num.h_boxes), y2(end/num.h_boxes), val;	
		
	if (start < 0 || end < 0 || start >= VIZ_NUM_BOXES || end >= VIZ_NUM_BOXES)
		return 0; // illegal points
	
	if ((x1 == x2 && y1 == y2) || (x1 != x2 && y1 != y2)) 
		return 0; // invalid orthogonal segment
		
	if (x1 == x2) { // vertical
		if (y1 > y2)
			swap(y1, y2);
			
		val = num.h_boxes;					
		for (i = y1+1; i < y2; ++i) {
			if (window_grid[x1 + i*val] != NO_OBJECT)
				return 0; // line intersects an object
		}
	}
	else { // horizontal		
		if (x1 > x2)
			swap(x1, x2);
		
		val = y1*num.h_boxes;				
		for (i = x1+1; i < x2; ++i) {
			if (window_grid[val + i] != NO_OBJECT)
				return 0; // line intersects an object
		}
	}
	return 1; 
}




int get_arrow_dir(vizObject& object, int start_loc, int has_arrow) {

	int x_val = start_loc%(int)num.h_boxes;
	int y_val = start_loc/num.h_boxes;
	int x1 = object.location%(int)num.h_boxes - 1;
	int x2 = x1 + object.xsize + 1;
	int y1 = object.location/num.h_boxes - 1;
	int y2 = y1 + object.ysize + 1;
	int sign = -1 + 2*has_arrow;
	
	if (y_val == y1 && x_val > x1 && x_val < x2)
		return sign;
		
	if (x_val == x2 && y_val > y1 && y_val < y2)
		return 2*sign;
		
	if (y_val == y2 && x_val > x1 && x_val < x2)
		return 3*sign;
		
	if (x_val == x1 && y_val > y1 && y_val < y2)
		return 4*sign;
		
	cerr <<"\nError: Endpoint of wire is not assigned to a valid port on the correct block." << endl;
	return 0;
}




int get_center_of_obj(vizObject& object) {

	point ul, lr, center;
	
	ul = get_center(object.location);
	lr = get_center(object.location + (object.xsize-1) + num.h_boxes*(object.ysize-1));
	
	center.x = 0.5*(ul.x + lr.x);
	center.y = 0.5*(ul.y + lr.y);
	
	return which_box(center);
}




vizWire default_wire(int source, int dest, int bits, int dest_wire_id) {

	vizWire wire;
	
	wire.source = source;
	wire.dest = dest;
	wire.bits = bits;
	wire.dest_match_wire_id = dest_wire_id;
	
	// assigned by the router
	wire.start = wire.end = 0;
	wire.s_arrow = wire.e_arrow = 0; 
	wire.location_set = 0;
	wire.num_points = 2;
	wire.point_cap = 0;
	wire.other_points = NULL;
		
	return wire;
}




void add_new_point(vizWire& wire, int location) {
	
	if (wire.point_cap == 0) {
		wire.other_points = new int[1];
		wire.point_cap = 1;
	}
	else if ((wire.num_points - 2) >= wire.point_cap) { // double size
		
		wire.point_cap <<= 1;
		
		int *new_points = new int[wire.point_cap];
			
		for (unsigned int i = 0; i < wire.num_points-2; ++i)
			new_points[i] = wire.other_points[i];
		
		delete [] wire.other_points;
		wire.other_points = new_points;
	}
	wire.other_points[wire.num_points-2] = location;
	++wire.num_points;
}



// x + y distance between two grid points, by location ID
int distance(int start, int end) {
	
	return abs(start%(int)num.h_boxes - end%(int)num.h_boxes) 
		 + abs((int)(start/num.h_boxes) - (int)(end/num.h_boxes));
}




void vizDrawWire(vizWire& wire, vizObject *objects, int num_objects) { 

	int second, second_to_last;
	point start, end, pt;
	
	if (wire.start == NOWHERE || wire.end == NOWHERE)
		return;
		
	if (!wire.location_set) {
		if (wire.source < 0 || wire.source >= num_objects || wire.dest < 0 || wire.dest >= num_objects) {
			cerr << "\nError: Illegal wire or source destinaltion: Not a valid object." << endl;
			return;
		}
		wire.start = get_center_of_obj(objects[wire.source]);			 
		wire.end = get_center_of_obj(objects[wire.dest]);
	}
	start = get_port_offset(wire.s_arrow, wire.start);
	end   = get_port_offset(wire.e_arrow, wire.end);
	
	SetDrawingColorx(WIRE_COLOR);
	MoveTo(start.x, start.y);
	
	if (wire.other_points != NULL && wire.num_points > 2) {
		second = wire.other_points[0];
		second_to_last = wire.other_points[wire.num_points-3];
		for (int i = 0; i < wire.num_points-2; ++i) {
			pt = get_center(wire.other_points[i]);
			LineTo(pt.x, pt.y);
		}
	}
	else {
		second = wire.end;
		second_to_last = wire.start;
	}
	LineTo(end.x, end.y);
	
	start = get_center(wire.start);
	end = get_center(wire.end);
	
	vizDrawArrow(start, wire.s_arrow);
	vizDrawArrow(end, wire.e_arrow);
	
	if (draw_bit_slashes && wire.bits > 1 && wire.bits < VIZ_MAX_BITS) {
		vizDrawBitLine(start, wire.s_arrow, wire.bits);
		vizDrawBitLine(end, wire.e_arrow, wire.bits);
	}
}




void vizDrawArrow(point c, int orientation) { 
	
	if (orientation == 0)
		return;
	
	int x, y;	
	
	SetDrawingColorx(WIRE_COLOR);
	
	switch (orientation) {
		
		case 1: // \/
			x = arrow_width*x_limit/num.h_boxes;
			y = arrow_length*y_limit/num.v_boxes;
			c.y += arrow_offset*y_limit/num.v_boxes;
			MoveTo(c.x, c.y + y/2);
			LineTo(c.x + x/2, c.y - y/2);
			LineTo(c.x - x/2, c.y - y/2);
			LineTo(c.x, c.y + y/2);
			return;
			
		case 2: // <-
			x = arrow_length*x_limit/num.h_boxes;
			y = arrow_width*y_limit/num.v_boxes;
			c.x -= arrow_offset*x_limit/num.h_boxes;
			MoveTo(c.x - x/2, c.y);
			LineTo(c.x + x/2, c.y - y/2);
			LineTo(c.x + x/2, c.y + y/2);
			LineTo(c.x - x/2, c.y);
			return;
		
		case 3: // /
			x = arrow_width*x_limit/num.h_boxes;
			y = arrow_length*y_limit/num.v_boxes;
			c.y -= arrow_offset*y_limit/num.v_boxes;
			MoveTo(c.x, c.y - y/2);
			LineTo(c.x + x/2, c.y + y/2);
			LineTo(c.x - x/2, c.y + y/2);
			LineTo(c.x, c.y - y/2);
			return;
		
		case 4: // ->
			x = arrow_length*x_limit/num.h_boxes;
			y = arrow_width*y_limit/num.v_boxes;
			c.x += arrow_offset*x_limit/num.h_boxes;
			MoveTo(c.x + x/2, c.y);
			LineTo(c.x - x/2, c.y - y/2);
			LineTo(c.x - x/2, c.y + y/2);
			LineTo(c.x + x/2, c.y);
			return;
		
		// no arrows	
		case -1: 
			LineDraw(c.x, c.y, c.x, c.y + 0.5*y_limit/num.v_boxes);
			return;			
		case -2: 
			LineDraw(c.x, c.y, c.x - (0.5*x_limit/num.h_boxes + 1), c.y);
			return;			
		case -3: 
			LineDraw(c.x, c.y, c.x, c.y - (0.5*y_limit/num.v_boxes + 1));
			return;
		case -4: 
			LineDraw(c.x, c.y, c.x + 0.5*x_limit/num.h_boxes, c.y);
			return;
			
		default: // not assigned
			return;
	}
}




void vizDrawBitLine(point c, int orientation, int bits) {
	
	switch (abs(orientation)) { 
		case 0: 
			break;			
		case 1: // \/
			c.y -= y_limit/num.v_boxes;
			LineDraw(c.x - 0.4*x_limit/num.h_boxes, c.y, c.x + 0.4*x_limit/num.h_boxes, c.y);
			c.y += 1.5*y_limit/num.v_boxes;
			break;			
		case 2: // <-
			c.x += x_limit/num.h_boxes;
			LineDraw(c.x, c.y - 0.4*y_limit/num.v_boxes, c.x, c.y + 0.4*y_limit/num.v_boxes);
			c.x -= 1.5*x_limit/num.h_boxes;
			break;			
		case 3: // /
			c.y += y_limit/num.v_boxes;
			LineDraw(c.x - 0.4*x_limit/num.h_boxes, c.y, c.x + 0.4*x_limit/num.h_boxes, c.y);
			c.y += 0.4*y_limit/num.v_boxes;
			break;
		case 4: // ->
			c.x -= x_limit/num.h_boxes;
			LineDraw(c.x, c.y - 0.4*y_limit/num.v_boxes, c.x, c.y + 0.4*y_limit/num.v_boxes);
			c.x -= 0.4*x_limit/num.h_boxes;
			break;			
	}
	set_text_size(8);
	SetDrawingColorx(TEXT_COLOR);
	MoveTo(c.x, c.y);
	DrawInt(bits);
}




point get_port_offset(int orientation, int location) {

	point c = get_center(location);
	
	switch (orientation) { // input port
		case 0: 
			break;			
		case 1: // \/
			c.y += (-0.5*arrow_length + arrow_offset)*y_limit/num.v_boxes;
			break;			
		case 2: // <-
			c.x -= (-0.5*arrow_length + arrow_offset)*x_limit/num.h_boxes;
			break;			
		case 3: // /
			c.y -= (-0.5*arrow_length + arrow_offset)*y_limit/num.v_boxes + 1;
			break;
		case 4: // ->
			c.x += (-0.5*arrow_length + arrow_offset)*x_limit/num.h_boxes;
			break;			
	}
	return c;
}













