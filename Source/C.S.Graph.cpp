#include "CircuitSolver.h"
#include "Equation.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Graph.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Graph
// By Frank Gennari 
int const whiteout_edge_size  = 4;
int const always_clear_window = 0;
int const useS_rule           = 1;
int const read_using_scanf    = 0;
int const always_draw_axis    = 1;
int const ask_to_open_non_CS  = 0;
int const show_equation       = 0;


typedef int (*comp_type)(const void *, const void *);

#ifdef TEXT_ONLY
int    const menu_driven      = 0;
#else
int    const menu_driven      = 1;
#endif

int    const MAX_EXPONENT     = 300;

double const fit_border_ratio = 1.5;
double const LOG_ZERO		  = 1e-20;

 
point origin, old_origin, offset;
double domain(30), range(10), x_scale, y_scale, old_xsize(0), old_ysize(0), old_originx(0), old_originy(0);
int line_type(2), graph_line_weight(1), num_points(default_points), color(1), Count(0), needs_cleared(0);
int varies_with_x(1), surpress_text(0), do_re_eval(1), graph_enable(0), origin_set(0), scale_set(0);
int enable_draw_grid(1), see_tracer(0), using_entered_equation, useLOG(0), num_div(1);
double saved_number[MAX_EQUATION_CHARACTERS/2+2] = {0};
char saved_equation[MAX_EQUATION_CHARACTERS + 1] = {0};
point *graph_data = NULL;
cs_equation graph_eq(MAX_EQUATION_CHARACTERS);


extern char typed_char;
extern int function_id, x_limit, y_limit, OPEN, DEMO, enablePAINT, is_char, line_weight, click_type, new_eq, window_mode, text_size;
extern int x_blocks, y_blocks, x_screen, y_screen;
extern long global_random_seed;
extern double last_out, IMPULSE_WIDTH;
extern char NAME[];



char menu_items();
char get_menu();
char get_command();
int check_for_equation();
int get_equation();
int calc_equation(cs_equation &eq);
int re_eval_equation(cs_equation &eq);
void graph_setup();
void plot();
point draw_graph(unsigned int i, int &io, point &last);
void draw_grid_for_graph();
void set_graphing_prefs(int needs_redraw);
void draw_pref_window();
void axis(int draw_axes);
void points();
void scales(int Axis);
double round_scales(double x);
void resolution();
int trace(int longest);
int find_point(int longest);
int draw_values(point location, int longest);
void draw_tracer(int i0, double y, point location, point last);
double calc_xy(int i1, double x_val, int xory);
void diff();
void integrate();
double get_f_of_zero();
void findx();
void findy();
int binary_search_graph_array(double search_val, int initial_index);
void random_data();
void make_histogram();
double calc_next_x(double Start, double End, double delta_x, double point_count, double numpoints, double last_x);
bool is_bad_number(double x);
void clear();
void draw_axis();
void number_axis(double mult1, double mult2);
void redraw();
void pan_window();
int do_pan();
void zoom();
void zoom_box();
void fit_to_screen();
void calc_size_params(double zoom_x, double zoom_y);
int  compare_point_xval(void *A, void *B);
int  compare_point_yval(void *A, void *B);
void write_point(FILE *qfilename, double x, double y, char &show_x, char &show_y, int y_val_only);
int  read_point(FILE *qfilename, ifstream &infile, int x_values);
void save_graph();
void open_graph();

int is_in_window(point P);
int is_in_window(Point P);

void draw_grid(int grid_color);
void draw_text(string text, int box_num, int t_size);
void draw_double(double num, char label, int box_num);
int shift_and_remove_zeros(char *display, char *display2);
point get_center(int box_num);

void close_drawing_window();

void draw_sized_string(const char *display, int tsize);
void set_text_size(int size);

void fft_on_graph(int inverse);

void process_message();
void switch_to_graphing_window();
void switch_to_schematic_window();






char menu_items() {

	char item;
	
	cout << "\n        GRAPH MENU" << endl
		 << "        ----------" << endl
		 << "	0. (G)raph" << endl
		 << "	1. (E)nter Equation" << endl
		 << "	2. Set Origin/(A)xis" << endl
		 << "	3. (T)race (mouse must be in graphics window)" << endl
		 << "	4. (L)ocate a Point on the Graph" << endl
		 << "	5. Set Graphing (P)references" << endl
		 << "	6. (Z)oom, Zoom (B)ox" << endl
		 << " 	7. (D)ifferentiate" << endl
		 << " 	8. (I)ntegrate" << endl
		 << "	9. (R)eturn to Menu or Previous Function" << endl
		 << "	X = Calculate x given y, Y = Calculate y given x, k = histogram" << endl
		 << "	(O)pen Graph, (S)ave Graph, (M)ove, W = Smooth/Resolution" << endl
		 << "	(C)lear Graph, (H)old Grid, (F)it to screen, N = fft, V = random" << endl;
		 
	cout << "\n\nWhat is your selection?  ";
		 
	item = inc();

	return item;
}



// free: j, q, u
char get_menu() {
	
	int longest(0);
	char select;
	
	do {
		if (OPEN != 2)
			select = menu_items();		
		else
			select = 'o';

		switch(select) {
				
			case '1':
			case 'e':
			case 'E':
				get_equation();
				break;

			case '6':	
			case 'z':
			case 'Z':
				if (check_for_equation())
					zoom();
				break;
				
			case '7':	
			case 'd':
			case 'D':
				if (check_for_equation())
					diff();
				break;
				
			case '8':	
			case 'i':
			case 'I':
				if (check_for_equation())
					integrate();
				break;
				
			case '9': // return
			case 'r':
			case 'R':
				select = '9';
				break;

			case 'x': // 11
			case 'X':
				if (check_for_equation())
					findx();
				break;
				
			case 'y': // 12
			case 'Y':
				if (check_for_equation())
					findy();
				break;
				
			case 'o': // 13
			case 'O':
				open_graph();
				break;
			
			case 's': // 14
			case 'S':
				if (check_for_equation())
					save_graph();
				break;

			case 'f': // 16
			case 'F':
				fit_to_screen();
				break;

			case 'w': // 18
			case 'W':
				resolution();
				break;
				
			case 'n': // 19
			case 'N':
				fft_on_graph(-1);
				break;
				
			case 'm': // 20
			case 'M':
				pan_window();
				break;
				
			case 'k': // 21
			case 'K':
				make_histogram();
				break;

			case 'v': // 22
			case 'V':
				random_data();
				break;

#ifndef TEXT_ONLY

			case '0': // graph
			case 'g':
			case 'G':
				select = '0';
				break;
			
			case '2':
			case 'a':
			case 'A':
				axis(1);
				break;
				
			case '3':
			case 't':
			case 'T':
				if (check_for_equation()) 
					longest = trace(longest);
				break;
				
			case '4':
			case 'l':
			case 'L':
				if (check_for_equation())
					longest = find_point(longest);
				break;
						
			case '5':
			case 'p':
			case 'P':
				set_graphing_prefs(1);
				break;
			
			case 'b': // 10
			case 'B':
				if (check_for_equation())
					zoom_box();
				break;
						
			case 'c': // 15
			case 'C':
				clear();
				break;
				
			case 'h': // 17
			case 'H':
				if (!enable_draw_grid) {
					enable_draw_grid = 1;
					cout << "\nRedraw Grid is ON." << endl;
				}
				else if (enable_draw_grid) {
					enable_draw_grid = 0;
					cout << "\nRedraw Grid is OFF." << endl;
				}
				break;
				
			case 21: // print 23
				cout << "\nSorry, print is not yet supported. Please use the Print Screen button on your keyboard to take a screenshot." << endl;
				break;

#endif

			default:
				beep();
				cout << "\nInvalid Selection!" << endl;
		}
	} while (select != '0' && select != '9');
	
	return select;
}




// free: j, k, q, u, v
char get_command() {
	
	int select, longest(0);
	
	do {
		if (OPEN != 2) {
			do {
				select = get_window_click();
				if (is_char)
					do_pan();
				else if (click_type == -2 && origin_set && scale_set && Count) {
					surpress_text = 1;
					needs_cleared = 1;
					plot();
					wait(0.1);
					surpress_text = 0;
				}			
			} while (click_type < 100);
		}	
		else
			select = 13;
		
		switch(select) {
	
			case 0:
				break;
				
			case 1:
				get_equation();
				break; 
			
			case 2:
				axis(1);
				break;
				
			case 3:
				if (check_for_equation()) 
					longest = trace(longest);
				break;
				
			case 4:
				if (check_for_equation())
					longest = find_point(longest);
				break;
						
			case 5:
				set_graphing_prefs(1);
				break;
				
			case 6:	
				if (check_for_equation())
					zoom();
				break;
				
			case 7:	
				if (check_for_equation())
					diff();
				break;
				
			case 8:	
				if (check_for_equation())
					integrate();
				break;
				
			case 9:
				break;
			
			case 10:
				if (check_for_equation())
					zoom_box();
				break;
				
			case 11:
				if (check_for_equation())
					findx();
				break;
				
			case 12:
				if (check_for_equation())
					findy();
				break;
				
			case 13:
				if (menu_driven)
					SelectConsoleWindow();
				open_graph();
				if (menu_driven)
					SelectDrawingWindow();
				break;
			
			case 14:
				if (check_for_equation()) {
					if (menu_driven)
						SelectConsoleWindow();
					save_graph();
					if (menu_driven)
						SelectDrawingWindow();
				}
				break;
						
			case 15:
				clear();
				break;
				
			case 16:
				fit_to_screen();
				break;
				
			case 17:
				if (!enable_draw_grid) {
					enable_draw_grid = 1;
					cout << "\nRedraw Grid is ON." << endl;
				}
				else if (enable_draw_grid) {
					enable_draw_grid = 0;
					cout << "\nRedraw Grid is OFF." << endl;
				}
				break;
				
			case 18:
				resolution();
				break;
				
			case 19:
				fft_on_graph(-1);
				cout << "done." << endl;
				break;
				
			case 20:
				pan_window();
				break;

			case 21:
				make_histogram();
				break;
				
			case 22:
				random_data();
				
				break;
				
			case 23: // print
				cout << "\nSorry, print is not yet supported. Please use the Print Screen button on your keyboard to take a screenshot." << endl;
				break;
				
			case 101: // ???
				intro();
				break;
					
			default:
				beep();
				cout << "\nInvalid Selection!" << endl;
		}
		if (select != 0 && select != 9)
			cout << "Select a command." << endl;

	} while (select != 0 && select != 9);
	
	return (char)select;
}





int check_for_equation() {

	if (!Count) {
		beep();
		cout << "\nYou must first enter an equation and plot a graph." << endl;
		return 0;
	}
	return 1;
}




int get_equation() {

	for (;;) {
		cout << "\nEnter the equation for y as a function of x, enter 'h' for help";
	
		if (saved_equation[0] != 0) {
			cout << ", or enter 'p' to use previous equation";
		}
		cout << ": ";

		graph_eq.input("\nInvalid Equation, Reenter: ");

		if (saved_equation[0] == 0 && comp_char(graph_eq.is_one_char(), 'p'))
			cout << "There is no previous equation to use." << endl;
		else
			break;
	}
	if (saved_equation[0] == 0 || !comp_char(graph_eq.is_one_char(), 'p')) {
		saved_equation[0] = 0;
		for (unsigned int i = 0; i < MAX_EQUATION_CHARACTERS/2+1; ++i)
			saved_number[i] = 0;
				
		if (show_equation)	
			graph_eq.print();
							
		graph_eq.copy_recoded_equation(saved_equation);
		graph_eq.copy_number(saved_number);
	}
	varies_with_x = graph_eq.var_with_x();
	cout << "\nCalculating points..." << endl;

	return calc_equation(graph_eq);
}




// Use equation to calculate point pairs for graphing
int calc_equation(cs_equation &eq) {
	
	int i, j, error(0);
	double start, end, delta_x, stop;	
	point graph_me;
	
	show_clock();
	
	Count = 0;
	using_entered_equation = 1;
	IMPULSE_WIDTH = domain/num_points;
	
	if (useLOG) {
		start   = 0;
		end     = pow(10, domain);
		stop    = sqrt(end);
		delta_x = (end - start);
	}
	else {
		start   = -domain*(origin.x/x_limit);
		end     = domain*(1 - origin.x/x_limit);
		stop    = end;
		delta_x = (end - start)/num_points;
	}
	for(graph_me.x = start; graph_me.x < stop && Count < MAX_DATA_SIZE && !error; ++Count) {
		
		i = j = 0;
		
		if (varies_with_x == 1 || Count == 0) {
			eq.set_prev_eval(graph_data[Count].y);
			graph_me.y = eq.eval(graph_me.x);
		}	
		else {
			graph_me.y = graph_data[0].y;
		}
		if (i < 0) {
			reset_cursor();
			return 0;
		}
		graph_data[Count].x = graph_me.x;
		graph_data[Count].y = graph_me.y;
		
		graph_me.x = calc_next_x(start, end, delta_x, Count, num_points, graph_me.x);
	}
	if (error)
		cout << "\nExpression error encountered while evaluating. Process terminated" << endl;
	else
		new_eq = 0;
		
	if (varies_with_x == 0)
		cout << "\ny = " << graph_data[0].y << endl;
	
	reset_cursor();
	
	return 1; 
}




int re_eval_equation(cs_equation &eq) {

	if (!do_re_eval) {
		return 0;
	}
	if (!using_entered_equation) {
		return 0;
	}
	if (eq.status() != EQ_OK) {
		return 0;
	}
	calc_equation(eq);

	return 1;
}




void graph_setup() {
	
	int menu, initg(scale_set), temp_ep;
	
	#ifdef TEXT_ONLY
		cout << "Since you are running in text mode only some of the menu selections will work." << endl;
		old_origin.x = origin.x = x_limit/2;
		old_origin.y = origin.y = y_limit/2;
	#endif
	
	using_entered_equation = 0;
	temp_ep = enablePAINT;
	enablePAINT = menu_driven;
	
	if (!graph_enable || window_mode == 0) {
		switch_to_graphing_window();
		set_text_size(0); // init font
	}
	if (!origin_set) {
		origin.x = x_limit/2;
		origin.y = y_limit/2;
	}
	do {
		if (menu_driven) {
			cout << "Select a command." << endl;
			menu = get_command();
		}
		else {
			menu = get_menu();
		}
		if ((menu == '0' || menu == 0) && graph_enable) {	
			
			if (needs_cleared) {
				whiteout();
				needs_cleared = 0;
			}
			if (!origin_set)
				axis(0);
			else if (always_clear_window)
				whiteout();
						
			if (!scale_set) 
				set_graphing_prefs(initg);
			
			color = -color;
			
			initg = 1;
			
			if (!Count) 
				get_equation();

			plot();			
		}
	} while (menu != '8' && menu != '9' && menu != 8 && menu != 9);
	
	switch_to_schematic_window();
	enablePAINT = temp_ep;	
}





void plot() {

	point see_me, last;
	int see_graph(0), i0(0), temp_line_weight(line_weight);
	
	last.x = last.y = -1;
	
	if (needs_cleared) {
		whiteout();
		needs_cleared = 0;
	}
	if (enable_draw_grid) 
		draw_grid_for_graph();
	else
		draw_axis();
	
	if (!surpress_text)	
		cout << "\nPlotting Graph..." << endl;
	
	show_clock();
	line_weight = graph_line_weight;
	
	if (color > 0)
		SetDrawingColorx(RED);
	else
		SetDrawingColorx(BLUE);
	
	for (unsigned int i = 0; (int)i < Count; ++i) {
		see_me = draw_graph(i, i0, last);
			
		if (is_in_window(see_me))
			see_graph = 1;
			
		if (see_me.x > x_limit)
			break;
	}	
	SetDrawingColorx(WHITE);
	RectangleDraw(0, y_limit, x_limit, y_limit + whiteout_edge_size);
	RectangleDraw(x_limit, 0, x_limit + whiteout_edge_size, y_limit);
	
	reset_cursor();
	
	if (!surpress_text) {
		cout << "The equation has been graphed. " << Count << " points were plotted. ";
	
		if (useLOG)
			cout << "Note that the x axis is actually the exponent of 10";
		if (useLOG == 2)
			cout << " and the y axis is in dB. ";
		else if (useLOG == 1)
			cout << ". ";
	
		if (!see_graph)
			cout << "Zoom out to see more of the graph by typing in <Z>." << endl;
		else
			cout << endl;
	}
	line_weight = temp_line_weight;
}





point draw_graph(unsigned int i, int &i0, point &last) {
	
	point see_me;
	
	if (useLOG) {
		see_me.x = log10(graph_data[i].x)*x_scale + origin.x;
		if (useLOG == 2)
			see_me.y = -graph_data[i].y*y_scale + origin.y;
		else
			see_me.y = -20*log10(graph_data[i].y)*y_scale + origin.y;	
	}
	else { 
		see_me.x = graph_data[i].x*x_scale + origin.x;	
		see_me.y = -graph_data[i].y*y_scale + origin.y;
	}
	if ((int)see_me.x == (int)last.x && (int)see_me.y == (int)last.y)
		return see_me;
	
	if (see_me.x > 0) {
		if (graph_data[i].y != -bound && is_in_window(see_me)) {
			if (line_type == 2) { // line		
				if (i0 == 0) 
					MoveTo(see_me.x, see_me.y);
								
				if (!is_in_window(last) && (i0 != 0 || last.x > 0 || last.y > 0)) {
					if (last.y > y_limit) 
						MoveTo((int)(last.x + (see_me.x-last.x)*((y_limit-last.y)/(see_me.y-last.y))), y_limit);
					else if (last.y < 0) 
						MoveTo((int)(last.x + (see_me.x-last.x)*(last.y/(last.y-see_me.y))), 0);
					else if (i0 == 0) 
						MoveTo(see_me.x, see_me.y);
				}
				LineTo(see_me.x, see_me.y);		
				i0 = 1;
			}
			else if (line_type == 1) { // dot
				#ifdef WIN32
				LineDraw(see_me.x, see_me.y, see_me.x+1, see_me.y+1);
				#else
				LineDraw(see_me.x, see_me.y, see_me.x, see_me.y);
				#endif
			}
		}
		else if(line_type == 2 && !is_in_window(see_me) && i > 0) { // line
			if (is_in_window(last)) {
				if (see_me.y > y_limit)
					LineTo((int)(last.x + (see_me.x-last.x)*((y_limit-last.y)/(see_me.y-last.y))), y_limit);
				else if (see_me.y < 0)
					LineTo((int)(last.x + (see_me.x-last.x)*(last.y/(last.y-see_me.y))), 0);
			}
			else {
				if (see_me.y < 0 && last.y > y_limit)
					LineDraw((int)(see_me.x - (see_me.x-last.x)*(-see_me.y/(last.y-see_me.y))), 0, (int)(last.x + (see_me.x-last.x)*((last.y-y_limit)/(last.y-see_me.y))), y_limit);
				else if (see_me.y > y_limit && last.y < 0)
					LineDraw((int)(see_me.x - (see_me.x-last.x)*((see_me.y-y_limit)/(see_me.y-last.y))), y_limit, (int)(last.x + (see_me.x-last.x)*(-last.y/(see_me.y-last.y))), 0);
			}
		}
	}
	last.x = see_me.x;
	last.y = see_me.y;
	
	return see_me;
}




void draw_grid_for_graph() { // maybe change for Log scale
	
	int counter(0);
	double mult1(1), mult2(1), numh, numv;
		
	numv = range;
	numh = domain;
	
	if (useLOG && domain < 1)
		return;
	
	while (numv > 20 && counter < MAX_EXPONENT) {
		numv /= 10;
		mult1 *= 10;
		++counter;
	}
	while (numv < 4 && numv != 0 && counter > -MAX_EXPONENT) {
		numv *= 10;
		mult1 /= 10;
		--counter;
	}
	if (numv > 20) {
		numv /= 2;
		mult1 *= 2;
	}
	if (numv < 8) {
		numv *= 2;
		mult1 /= 2;
	}
	while (numh > 20 && counter < MAX_EXPONENT) {
		numh /= 10;
		mult2 *= 10;
		++counter;
	}	
	while (numh < 4 && numh != 0 && counter > -MAX_EXPONENT) {
		numh *= 10;
		mult2 /= 10;
		--counter;
	}
	if (numh > 20) {
		numh /= 2;
		mult2 *= 2;
	}
	if (numh < 10) {
		numh *= 2;
		mult2 /= 2;
	}
	if (needs_cleared) {
		whiteout();		
		needs_cleared = 0;
	}
	x_blocks = x_screen = (int)numh;
	y_blocks = y_screen = (int)numv;

	offset.x = origin.x/(x_limit/(double)x_screen);
	offset.y = origin.y/(y_limit/(double)y_screen);
	
	offset.x = (offset.x - (int)offset.x)*(x_limit/(double)x_screen) - (x_limit/(double)x_screen)/2;
	offset.y = (offset.y - (int)offset.y)*(y_limit/(double)y_screen) - (y_limit/(double)y_screen)/2;

	number_axis(mult1, mult2);
	draw_axis();
}




void set_graphing_prefs(int needs_redraw) {
	
	int selection(0);
	
	cout << "\nClick on the boxes to change the graphing preferences." << endl;
	
	do {
		if (selection != -1)
			draw_pref_window();
			
		selection = box_clicked();
		
		if (is_char)
			selection = -1;
		
		switch (selection) {
		
			case 1:
				scales(1);
				needs_cleared = 1;
				break;
				
			case 2:
				scales(2);
				needs_cleared = 1;
				break;
				
			case 3:
				points();
				break;
				
			case 4:
				if (line_type == 1)
					line_type = 2;
				else
					line_type = 1;
				break;
				
			case 5:
				if (enable_draw_grid == 0) {
					enable_draw_grid = 1;
					cout << "\nEnter the grid divisions (1 for solid grid lines, >1 for dotted grid lines):  ";
					num_div = inpg0();
				}
				else if (enable_draw_grid == 1)
					enable_draw_grid = 2;
				else if (enable_draw_grid == 2) 
					enable_draw_grid = 0;
				break;
				
			case 6:
				if (see_tracer)
					see_tracer = 0;
				else
					see_tracer = 1;
				break;
				
			case 7:
				if (useLOG == 0)
					useLOG = 1;
				else if (useLOG == 1)
					useLOG = 2;
				else
					useLOG = 0;
				break;
					
			case 8:
				cout << "Enter the line weight you would like to use (1 - 10):  ";
				graph_line_weight = in();
				graph_line_weight = min(10, max(1, abs(graph_line_weight)));
				break;
		}
	} while (selection != 0);
	
	x_scale = x_limit/domain;
	y_scale = y_limit/range;
	
	if (needs_redraw) {
		if (!Count || !scale_set)
			whiteout();
		else
			redraw();
	}	
	scale_set = 1;
}





void draw_pref_window() {
		
	init_menu(9, 10, 0);
	
	if (useLOG == 1)
		draw_text(" LOG", 7, 12);
	else if (useLOG == 2)
		draw_text(" SEMILOG", 7, 12);
	else
		draw_text(" LINEAR", 7, 12);
	
	draw_text(" OK", reserved_pos_value, 12);  
	draw_text(" Domain", 1, 12);
	draw_text(" Range", 2, 12);
	draw_text(" Number of Points", 3, 12);
	draw_text(" Linetype", 4, 12);
	draw_text(" Grid", 5, 12);
	draw_text(" See Tracer (Slow)", 6, 12);
	draw_text(" Line Weight", 8, 12);
	
	draw_double(domain, 'X', -1);
	draw_double(range, 'X', -2);	
	draw_double(num_points, 'X', -3);
	draw_double((double)graph_line_weight, 'X', -8);
	
	if (line_type == 1)
		draw_text(" DOT", -4, 12);
	else
		draw_text(" LINE", -4, 12);
		
	if (enable_draw_grid == 0)
		draw_text(" OFF", -5, 12);
	else if (enable_draw_grid == 1)
		draw_text(" ON", -5, 12);
	else
		draw_text(" Numbers Only", -5, 12);
			
	if (see_tracer)
		draw_text(" ON", -6, 12);
	else
		draw_text(" OFF", -6, 12);
}




void axis(int draw_axes) {

	Point pt;
	
	origin_set = needs_cleared = 1;
	
	whiteout();
	
	cout << "\nSet origin at center of graphics window?  ";
		
	if (decision()) {
		old_origin.x = origin.x = x_limit/2;
		old_origin.y = origin.y = y_limit/2;
		
		if (draw_axes)
			draw_axis();
	}
	else do {
				
		cout << "\nClick on the graphics window to place the origin." << endl;
				
		whiteout();
		
		pt = GetDWClick();
		
		old_origin.x = origin.x = pt.h;
		old_origin.y = origin.y = pt.v;
		
		draw_axis();
			
		cout << "\nOrigin coordinates are: (" << origin.x << ", " << origin.y << ")." << endl;
		
		cout << "\nIs this origin location correct?  ";
		
	} while (!decision());
}




void scales(int Axis) {
	
	double temp;

	scale_set = 1;
	
	if (Axis == 3) {
		cout << "\nCurrent domain = " << domain << " and current range = " << range << "." << endl
		 	 << "Change this?  "; 
		 	 
		if (decision())
			Axis = 4;
	}
	if (Axis == 1 || Axis == 4) {
		cout << "\nEnter the new Domain";
		if (useLOG)
			cout << " in decades(powers of 10, > 1)"; 
		cout << ":  ";
		
		temp   = domain;
		domain = ind();
		
		if (domain == 0)
			domain = 1;

		domain = min(fabs(domain), max_domain);

		if (domain != temp) {
			re_eval_equation(graph_eq);
		}
		if (Axis == 4)
			Axis = 2;
	}	
	if (Axis == 2) {	
		cout << "\nEnter the new Range"; 
		if (useLOG == 1)
			cout << "(in dB)";
		cout << ":  ";
		
		range = ind();
		
		if (range == 0)
			range = 1;
		
		range = min(fabs(range), max_domain/window_ratio);
	}
	calc_size_params(1, 1);
}




double round_scales(double x) { 
	
	float exponent(0);
	
	if (x == 0)
		return 0;
	
	while (x > 20 && exponent < MAX_EXPONENT) {
		x /= 10;
		++exponent;
	}
	while (x < 2 && exponent > -MAX_EXPONENT) {
		x *= 10;
		--exponent;
	}
	return roundx(x)*pow(10.0f, exponent);
}




void resolution() { // won't work for unsorted x values

	int res, i, index;
	double x_step;
	point *temp = NULL;

	if (Count < 2)
		return;

	cout << "\nEnter the resolution factor (integer, {-1,0,1} = same resolution, > 1 = increase resolution (smooth), < 1 decrease resolution): ";
	res = in();
	
	if (res < -1 && res > -Count/2 && Count > 2) { // decrease resolution
		res = -res;
		num_points = Count/res;
			
		for (i = 1; i < num_points-1; ++i) 
			graph_data[i] = graph_data[i*res];
			
		graph_data[i] = graph_data[Count-1];
		Count = num_points;
	}
	else if (res > 1 && Count < MAX_POINTS/2) { // increase resolution
		res = min(res, MAX_POINTS/Count);
		num_points = Count*res;
		temp = memAlloc(temp, num_points+1);

		if (!temp) { // change for entered equations
			num_points = Count;
			return;
		}
		x_step = (graph_data[1].x - graph_data[0].x)/((double)res);
			
		for (i = 0; i < num_points; ++i) { // won't work for log graphs
			if (i%res == 0)
				temp[i] = graph_data[i/res];
			else {
				temp[i].x = graph_data[i/res].x + (i%res)*x_step;
				
				if (using_entered_equation) {
					temp[i].y = graph_eq.eval(temp[i].x);
				}
				else {
					index = (int)((i/(double)res) + 1.5);
					temp[i].y = calc_xy(index, temp[i].x, 1); // still needs some work				
				}
			}
		}
		for (i = 0; i < num_points; ++i)
			graph_data[i] = temp[i];
			
		delete [] temp;			
		Count = num_points;
	}
}




void points() {
	
	int pointz, max_p = MAX_POINTS;
	
	if (DEMO)
		max_p /= 5;
	
	cout << "\nEnter the new number(" << max_p << " max):  ";		
	pointz = in();
	num_points = min(max(abs(pointz), 1), max_p);
	cout << "\nGraph must be recalculated for changes to take place." << endl;
}




int trace(int longest) {
	
	Point location1, location2; 
	point trace;
	
	location2.h = location2.v = 0;
	
	cout << "\nPoint on the graph to see the x and y coordinates and f(x). Move the pointer out "
		 << "of the graphics window to end Trace." << endl;
		 
	if (useLOG)
		cout << "Note that the x axis is actually the exponent of 10";
	if (useLOG == 1)
		cout << " and the y axis is in dB." << endl;
	else if (useLOG == 2)
		cout << "." << endl;	
	
	set_special_cursor(TRACE_CURSOR);
	location1 = GetMouseLocation();
	
	do {
		if (location1.h != location2.h || location1.v != location2.v) {		
			trace.x = domain*((location1.h - origin.x)/x_limit);
			trace.y = range*((origin.y - location1.v)/y_limit);			
			longest = draw_values(trace, longest);
		}
		location2 = location1;
		location1 = GetMouseLocation();

	} while(is_in_window(location1));
	
	reset_cursor();
	redraw();
	
	return 0;	
}




int find_point(int longest) {
	
	Point location;
	point find;
	
	cout << "\nPoint and click." << endl;
	
	if (useLOG) {
		cout << "Note that the x axis is actually the exponent of 10";
		if (useLOG == 2)
			cout << " and the y axis is in dB." << endl;
		else
			cout << "." << endl;
	}
	else
		cout << endl << endl;
	
	set_special_cursor(TRACE_CURSOR);
			
	location = GetDWClick();
		
	find.x = domain*((location.h - origin.x)/x_limit);
	find.y = range*((origin.y - location.v)/y_limit);
	
	reset_cursor();
	
	needs_cleared = 1;
	
	return draw_values(find, longest);
}




int draw_values(point location, int longest) {
	
	point last;
	int i, i0(0), length;
	char display[MAX_CHARACTERS + 1] = {0};
	double y;
	
	last.x = last.y = -1;
	
	i = binary_search_graph_array(location.x, -useLOG);

	y = calc_xy(i, location.x, 1);
	
	if (see_tracer) {
		draw_tracer(i0, y, location, last);
	}
	if (y == -0)
		y = 0;
		
	if (!useLOG && ((location.x < graph_data[0].x) || (location.x > graph_data[Count - 1].x)))
		y = 0;
			
	else if (useLOG) {
		if (!(log10(graph_data[0].x) >= 0) && !(log10(graph_data[0].x) < 0))
			graph_data[0].x = UnknownV;
		if ((location.x < log10(graph_data[0].x)) || (location.x > log10(graph_data[Count - 1].x)))
			y = 0;
	}	
	strcpy(display, " x = ");
	strcat(display, double_to_char(location.x));
	strcat(display, "  y = ");
	strcat(display, double_to_char(location.y));
	strcat(display, "  f(x) = ");
	strcat(display, double_to_char(y));
	
	if (useLOG) {
		if (useLOG == 1) {
			y = pow(10, y/20);
			location.y = pow(10, location.y/20);
		}
		location.x = pow(10, location.x);
		
		strcat(display, " (LOG) | x = ");
		strcat(display, double_to_char(location.x));
		strcat(display, "  y = ");
		strcat(display, double_to_char(location.y));
		strcat(display, "  f(x) = ");
		strcat(display, double_to_char(y));
		strcat(display, " (LINEAR)");
	}
	length = strlen(display)*7;
	longest = length = max(longest, length);
	
	SetDrawingColorx(BLACK);
	RectangleFrame(1, 1, length, 20);
	
	SetDrawingColorx(WHITE);
	RectangleDraw(2, 2, length, 20);
		
	MoveTo(4, 15);
	SetDrawingColorx(BLACK);
	draw_string(display);

#ifdef xWIN32
	process_message(); // required to update text on screen for WinNT/Win2000
#endif

	return longest;
}




void draw_tracer(int i0, double y, point location, point last) {
	
	double logx, logy;
	
	whiteout();
	/*SetDrawingColorx(WHITE);
	
	if (useLOG) 
		RectangleDraw(log10(graph_data[i].x*x_scale + origin.x - trace_bounds), 0, log10(graph_data[i].x*x_scale + origin.x + trace_bounds), y_window);			
	else
		RectangleDraw(graph_data[i].x*x_scale + origin.x - trace_bounds, 0, graph_data[i].x*x_scale + origin.x + trace_bounds, y_window);
	*/

#ifdef xWIN32
	draw_axis(); // redundant call for graphics ordering bug in Windows NT/2000
#endif

	if (enable_draw_grid && domain < max_grid_domain && range < max_grid_domain/window_ratio) {
		draw_grid_for_graph();
	}
	draw_axis();
	
	if (color > 0)
		SetDrawingColorx(RED);
	else
		SetDrawingColorx(BLUE);
	
	for (unsigned int j = 0; (int)j < Count; ++j) {
		draw_graph(j, i0, last);
	}	
	if (useLOG) {
		logx = location.x;//log10(graph_data[i].x);
		
		/*if (useLOG == 1)
			logy = -20*log10(graph_data[i].y);
		else
			logy = -graph_data[i].y;
		*/	
		logy = -y;						
		
		SetDrawingColorx(PURPLE);
		if ((logx*x_scale + origin.x) <= x_limit)
			LineDraw((int)(logx*x_scale + origin.x), 0, (int)(logx*x_scale + origin.x), y_limit);
		if ((logy*y_scale + origin.y) <= y_limit)
			LineDraw(0, (int)(logy*y_scale + origin.y), x_limit, (int)(logy*y_scale + origin.y));
		
		if ((logx*x_scale + origin.x - 2) <= x_limit && (logy*y_scale + origin.y - 2) <= y_limit) {
			SetDrawingColorx(GREEN);
			RectangleDraw((logx*x_scale + origin.x - 2), (logy*y_scale + origin.y - 2), (logx*x_scale + origin.x + 2), (logy*y_scale + origin.y + 2));
		}
	}
	else {
		/*SetDrawingColorx(PURPLE);
		LineDraw((graph_data[i].x*x_scale + origin.x), 0, (graph_data[i].x*x_scale + origin.x), y_limit);
		LineDraw(0, (-graph_data[i].y*y_scale + origin.y), x_limit, (-graph_data[i].y*y_scale + origin.y));
		
		SetDrawingColorx(GREEN);
		RectangleDraw((graph_data[i].x*x_scale + origin.x - 2), (-graph_data[i].y*y_scale + origin.y - 2), (graph_data[i].x*x_scale + origin.x + 2), (-graph_data[i].y*y_scale + origin.y + 2));*/
		
		SetDrawingColorx(PURPLE);
		if ((location.x*x_scale + origin.x) <= x_limit)
			LineDraw((int)(location.x*x_scale + origin.x), 0, (int)(location.x*x_scale + origin.x), y_limit);
		if ((-y*y_scale + origin.y) <= y_limit)	
			LineDraw(0, (int)(-y*y_scale + origin.y), x_limit, (int)(-y*y_scale + origin.y));
		
		if ((location.x*x_scale + origin.x - 2) <= x_limit && (-y*y_scale + origin.y - 2) <= y_limit) {
			SetDrawingColorx(GREEN);
			RectangleDraw((location.x*x_scale + origin.x - 2), (-y*y_scale + origin.y - 2), (location.x*x_scale + origin.x + 2), (-y*y_scale + origin.y + 2));
		}
	}	
}




double calc_xy(int i1, double x_val, int xory) {

	double x0, x1, x2, x3, y0, y1, y2, y3, zi, zi1;	
	int i = min(max(i1, 2), Count - 2);
	
	if (useLOG) {
		for (; i < Count && graph_data[i-1].x <= 0; ++i) {};
			
		if (useLOG == 1)
			for (; i < Count && graph_data[i-2].y <= 0; ++i) {};
			
		if (xory == 2)
			x_val = log10(x_val);
			
		x0 = log10(graph_data[i-1].x);
		
		if (graph_data[i].x > 0)
			x1 = log10(graph_data[i].x);
		else 
			return 0;
			
		x2 = log10(graph_data[i-2].x);
		x3 = log10(graph_data[i+1].x);
		
		if (useLOG == 2) {
			y0 = graph_data[i-1].y;
			y1 = graph_data[i].y;
			y2 = graph_data[i-2].y;
			y3 = graph_data[i+1].y;
		}
		else {
			if (xory == 0)
				x_val = 20*log10(x_val);
				
			y0 = 20*log10(graph_data[i-1].y);
			
			if (graph_data[i-1].y > 0)
				y1 = 20*log10(graph_data[i].y);
			else
				return 0;
				
			y2 = 20*log10(graph_data[i-2].y);
			y3 = 20*log10(graph_data[i+1].y);
		}	
	}
	else {
		x0 = graph_data[i-1].x;
		x1 = graph_data[i].x;
		x2 = graph_data[i-2].x;
		x3 = graph_data[i+1].x;
		y0 = graph_data[i-1].y;
		y1 = graph_data[i].y;
		y2 = graph_data[i-2].y;
		y3 = graph_data[i+1].y;
	}
	if (xory >= 1)	{			
		zi  = (y1 - y2)/(x1 - x2);
		zi1 = (y3 - y0)/(x3 - x0);
									// Y - Quadratic Spline Interpolation
		return (zi1 - zi)*(x_val - x0)*(x_val - x0)/(2*(x1 - x0)) + zi*(x_val - x0) + y0; 
	}		
	zi  = (x1 - x2)/(y1 - y2);
	zi1 = (x3 - x0)/(y3 - y0);
									// X - Quadratic Spline Interpolation
	return (zi1 - zi)*(x_val - y0)*(x_val - y0)/(2*(y1 - y0)) + zi*(x_val - y0) + x0;
}




void diff() {
	
	double temp1(graph_data[0].y), temp2(0);
	
	using_entered_equation = 0;
	
	graph_data[0].y = (graph_data[1].y - graph_data[0].y)/(graph_data[1].x - graph_data[0].x);
	temp2 = temp1;
	
	for (int i = 1; i < Count - 1; ++i) {
		temp1 = graph_data[i].y;
		graph_data[i].y = (graph_data[i+1].y - temp2)/(graph_data[i+1].x - graph_data[i-1].x);
		temp2 = temp1;
	}
	graph_data[Count-1].y = (graph_data[Count-2].y - temp2)/(graph_data[Count-2].x - graph_data[Count-1].x);
	graph_data[Count].y = 0;
	
	redraw();
}




void integrate() {
	
	int i(0), j;
	double last_y(0), y_val(0), C(0), f_of_zero(get_f_of_zero());
	graph_data[0].y = 0;
	
	using_entered_equation = 0;
	
	j = binary_search_graph_array(0, 0);
	
	++j;
					
	for (i = j; i < Count; ++i) {
		if (useS_rule)
			y_val = y_val + (4*graph_data[i].y + last_y + graph_data[i+1].y)*(graph_data[i].x - graph_data[i-1].x)/6; // Simpson's Rule
		else
			y_val = y_val + .5*(graph_data[i].y + last_y)*(graph_data[i].x - graph_data[i-1].x);	// Midpoint Rule
		
		last_y = graph_data[i].y;
		graph_data[i].y = y_val;
	}
	if (j-1 < Count) 
		last_y = graph_data[j-1].y;
	else
		last_y = 0;
	
	for (i = j-1; i >= 0; --i) {
		if (useS_rule)
			y_val = graph_data[i+1].y + (4*graph_data[i].y + last_y + graph_data[i-1].y)*(graph_data[i].x - graph_data[i+1].x)/6; // Simpson's Rule
		else	
			y_val = graph_data[i+1].y + .5*(graph_data[i].y + last_y)*(graph_data[i].x - graph_data[i+1].x); // Midpoint Rule
		
		last_y = graph_data[i].y;
		graph_data[i].y = y_val;
	}
	cout << "C = f(0) is " << f_of_zero << "." << endl;
	cout << "Would you like to change this value?  ";
	if (decision()) {
		cout << "\nEnter C (f at 0):  ";
		C = ind();
	}
	else {
		C = f_of_zero;
	}
	for (i = 0; i < Count; ++i) // For integration, must add C.
		graph_data[i].y += C;
		
	redraw();
}




double get_f_of_zero() {
	
	int i;
	double x(0), y;
	
	if (useLOG)
		x = 1;
	
	if (using_entered_equation)
		y = graph_eq.eval(x);	
	else {
		i = binary_search_graph_array(x, 0);
	
		if (i == 0 || i == Count)
			return 0;
	
		return calc_xy(i, x, 2);
	}
	return 0;
}




void findx() {
	
	int i(0);
	double y, start_x;
	
	cout << "\nEnter y:  ";
	y = ind();
	
	if (useLOG == 1)
		y = pow(10, y/20.0);
	
	cout << "\nWould you like to enter a starting x value?  ";
	if (decision()) {
	
		cout << "\nEnter the x value at the place you would like to start from:  ";
		start_x = ind();
		
		i = binary_search_graph_array(start_x, 0);
	}
	// run past all NAN y values
	for (   ; i < Count && is_bad_number(graph_data[i].y); ++i) {};	
	for (++i; i < Count && (fabs(graph_data[i].x) == LOG_ZERO || fabs(graph_data[i].y) == LOG_ZERO); ++i) {};
			
	if (graph_data[i].y < y) {
		for (; i < Count && graph_data[i].y < y; ++i) {}; // graph_data[i].y is not sorted and cannot be binary searched
	}	
	else {
		for (; i < Count && graph_data[i].y > y; ++i) {};
	}
	if (i == 0 || i == Count) {
		cout << "This value of y is out of the range of calculated points. " << endl;
		return;
	}	
	last_out = calc_xy(i, y, 0);

	cout << "x = " << last_out << endl;
	if (useLOG)
		cout << "x (linear) = " << pow(10, last_out) << endl;
}




void findy() {

	int i;
	double x, y;
	
	cout << "\nEnter x:  ";
	x = ind();
	
	if (useLOG)
		x = pow(10, x);
	
	if (using_entered_equation) {
		y = graph_eq.eval(x);
		cout << "y = " << y << endl;
	}
	else {
		i = binary_search_graph_array(x, 0);
		
		if (i == 0 || i == Count) {
			cout << "This value of x is out of the range of calculated points. " << endl;
			return;
		}
		last_out = calc_xy(i, x, 2);
	
		cout << "y = " << last_out << endl;
		if (useLOG == 1)
			cout << "y (linear) = " << pow(10, last_out/20.0) << endl;
	}
}




int binary_search_graph_array(double search_val, int initial_index) {

	char log(0);
	int start, end, mid;
	double x_val(0);
	
	if (initial_index < 0) {
		initial_index = 0;
		log = 1;
	}
	if (initial_index >= Count)
		return Count - 1;
		
	else if (initial_index == 0) {
		if (log || useLOG) {
			start = (int)(Count*((search_val - log10(graph_data[Count-1].x))/domain + 1));
			
			if (start > 0 && start < Count && log10(graph_data[start].x) > search_val && log10(graph_data[start-1].x) < search_val)
				return start;
			if (start >= 0 && (start + 1) < Count && log10(graph_data[start+1].x) > search_val && log10(graph_data[start].x) < search_val)
				return start + 1;
		}
		if (!log) {
			start = (int)(Count*((search_val - graph_data[Count-1].x)/domain + 1));
			
			if (start > 0 && start < Count && graph_data[start].x > search_val && graph_data[start-1].x < search_val) 
				return start;
		}
	}	
	start = initial_index;
	end = Count - 1;
	mid = (start + end)/2;
		
	while (mid > start && mid < end) {
		
		if (log)
			x_val = log10(graph_data[mid].x);
		else
			x_val = graph_data[mid].x;
			
		if (x_val > search_val) {
			end = mid;
			mid = (start + end + 1)/2;
		}
		else if (x_val < search_val) {
			start = mid;
			mid = (start + end)/2;
		}
		else 
			return mid;
	}
	if (x_val < search_val)
		++mid;
	
	return mid;
}



void random_data() {

	unsigned int i, j, n_val;
	long rand_seed1(global_random_seed), rand_seed2 = get_time();
	double sigma, range, random_num, this_num;
	
	cout << "Do you want to use a uniform distribution?  ";
	if (decision()) {
		cout << "Enter the range:  ";
		range = ind();
		Count = num_points;
		for (i = 0; (int)i < Count; ++i) {
			randome(rand_seed1, rand_seed2, random_num);
			graph_data[i].x = i;
			graph_data[i].y = random_num*range;
		}
		return;
	}
	cout << "Do you want to use a Gaussian distribution?  ";
	if (decision()) {
		cout << "Enter sigma:  ";
		sigma = ind();
		cout << "Enter n:  ";
		n_val = abs(in());
		n_val = max((int)1, n_val);
		Count = num_points;
		
		for (i = 0; (int)i < Count; ++i) {
			this_num = 0;
			for (j = 0; j < n_val; ++j) {
				randome(rand_seed1, rand_seed2, random_num);
				this_num += (random_num - 0.5);
			}
			graph_data[i].x = i;
			graph_data[i].y = this_num*sigma*sqrt(12.0/n_val);
		}
		return;
	}
	cout << "Do you want to use the current graph as the random distribution?  ";
	if (decision()) {
		cout << "Not yet supported." << endl;
		return;
	}
	cout << "Not yet supported." << endl;
}




void make_histogram() {

	unsigned int i;
	int bin_i;
	double nbins, min_yval, max_yval, bin_width;

	cout << "Enter the number of bins in the histogram:   ";
	nbins = abs(in());
	nbins = min((double)MAX_POINTS, max(1.0, nbins));
	
	min_yval = max_yval = graph_data[0].y;
	
	for (i = 1; (int)i < Count; ++i) {
		if (graph_data[i].y < min_yval)
			min_yval = graph_data[i].y;
		else if (graph_data[i].y > max_yval)
			max_yval = graph_data[i].y;
	}
	cout << "The values range between " << min_yval << " and " << max_yval << "." << endl;	
	bin_width = (max_yval - min_yval)/nbins;
	
	for (i = 0; i < nbins; ++i)
		graph_data[i].x = 0;
		
	for (i = 0; (int)i < Count; ++i) {
		bin_i = (int)((graph_data[i].y - min_yval)/bin_width);
		++graph_data[bin_i].x;
	}
	for (i = 0; i < nbins; ++i) {
		graph_data[i].y = graph_data[i].x;
		graph_data[i].x = min_yval + i*bin_width;
	}
	num_points = Count = (int)nbins;
}




double calc_next_x(double Start, double End, double delta_x, double point_count, double numpoints, double last_x) {
		
	double Startlog;
	
	if (useLOG) {
		if (point_count == 0)
			return LOG_ZERO; // Log(0) is bad.
		else {
			if (Start == 0) 
				Startlog = 1;
			else
				Startlog = log10(fabs(Start));
				
			return pow(10, 2*(log10(End)-Startlog+1)*(point_count/numpoints - 0.5/*origin.x/x_limit*/)); // Logarithmic Distribution
		}
	}
	else
		return last_x + delta_x; // Standard Distribution
}




bool is_bad_number(double x) {

	return (fabs(x) > almost_infinite /*|| fabs(x) < almost_zero*/);
}




void clear() {

	whiteout();
	
	if (origin_set) {
		if (always_draw_axis)
			draw_axis();
		else {
			cout << "\nWould you like to redraw the x and y axis?  ";
			
			if (decision()) 	
				draw_axis();	
			else
				origin_set = 0;
		}
	}
}




void draw_axis() {
	
	SetDrawingColorx(BLACK);
	
	if (origin.x >= 0 && origin.x <= x_limit)
		LineDraw((int)origin.x, 0, (int)origin.x, y_limit);
		
	if (origin.y >= 0 && origin.y <= y_limit)
		LineDraw(0, (int)origin.y, x_limit, (int)origin.y);
}





void number_axis(double mult1, double mult2) {

	char display[MAX_CHARACTERS + 1] = {0}, display2[MAX_CHARACTERS + 1] = {0};
	int counter, num_div2, num_offset;
	unsigned int div, pos;
	double number;
	point c1, P;
	
	num_div2 = max(1, (int)(num_div*window_ratio));
	num_offset = (int)((x_screen*origin.x)/x_limit);
	
	SetDrawingColorx(LTBLUE);
	text_size = 10;
	set_text_size(text_size);
	
	// label x axis, draw x grid
	for (counter = 0; counter <= x_screen; ++counter) {	
		number = mult2*(counter - num_offset);		
		memset(display2, 0, MAX_CHARACTERS-1);
						
		strcpy(display, double_to_char(number));
		shift_and_remove_zeros(display, display2);
		
		if (counter == x_screen) {
			c1 = get_center(counter-1);
			c1.x += x_limit/(double)x_screen;
		}
		else
			c1 = get_center(counter);
			
		if (enable_draw_grid == 1 && (c1.x + offset.x) <= x_limit) {
			SetDrawingColorx(LTGREY);
			if (num_div == 1) 
				LineDraw((int)(c1.x + offset.x), 1, (int)(c1.x + offset.x), (y_limit - 2));
			else 
				for (div = 0, pos = 1; (int)pos <= (y_limit - 2); pos += y_limit/num_div, ++div) 
					if (div%2)
						LineDraw((c1.x + offset.x), pos, (c1.x + offset.x),  (pos + y_limit/num_div - 1));
		}
		SetDrawingColorx(LTBLUE);
		P.x = (c1.x + offset.x - (strlen(display2) - 1)*3);
		P.x += (strlen(display2) - 1)*6;
		
		if (origin.y < 6)
			P.y = 15;
		else if (origin.y > y_limit - 13)
			P.y = y_limit - 4;
		else
			P.y = (origin.y + 15);
		
		if (is_in_window(P)) {
			P.x -= (strlen(display2) - 1)*6;
			MoveTo(P.x, P.y);
			draw_sized_string(display2, 9);
		}
		if (c1.x + offset.x + x_limit/(double)x_screen >= x_limit)
			counter = x_screen + 1;
	}
	num_offset = (int)((y_screen*origin.y)/y_limit);
	
	// label y axis, draw y grid
	for (counter = 0; counter <= y_screen; ++counter) {			
		number = mult1*(num_offset - counter);	
		memset(display2, 0, MAX_CHARACTERS-1);
			
		strcpy(display, double_to_char(number));
		shift_and_remove_zeros(display, display2);
		
		if (counter == y_screen) {
			c1 = get_center(x_screen*(counter-1));
			c1.y += y_limit/(double)y_screen;
		}
		else	
			c1 = get_center(x_screen*counter);
			
		if (enable_draw_grid == 1 && (c1.y + offset.y) <= y_limit) {
			SetDrawingColorx(LTGREY);
			if (num_div == 1)
				LineDraw(1, (int)(c1.y + offset.y), (x_limit - 2), (int)(c1.y + offset.y));
			else
				for (div = 0, pos = 1; (int)pos <= (x_limit - 2); pos += x_limit/num_div2, ++div) 
					if (div%2)
						LineDraw(pos, (c1.y + offset.y), (pos + x_limit/num_div2 - 1), (c1.y + offset.y));
		}
		SetDrawingColorx(LTBLUE);
		P.y = (c1.y + offset.y + 4);
		
		if (origin.x < 15)
			P.x = 15;
		else if (origin.x > x_limit)
			P.x = x_limit - (strlen(display2) - 1)*6;
		else
			P.x = (origin.x - (strlen(display2) - 1)*6);
				
		P.x += (strlen(display2) - 1)*6;
		
		if (is_in_window(P)) {
			P.x -= (strlen(display2) - 1)*6;
			MoveTo(P.x, P.y);
			draw_sized_string(display2, 9);
		}
		if (c1.y + offset.y + y_limit/(double)y_screen >= y_limit)
			counter = y_screen + 1;
	}
}




void redraw() {
	
	whiteout();
		
	if (origin_set) {
		draw_axis();			
		if (scale_set && Count)
			plot();
	}
	else
		draw_text_on_window(" Ready");
}




void pan_window() {
	
	cout << "\nUse the arrow keys to scroll the screen. Press any other key to end." << endl;
	SelectDrawingWindow();
	
	do {
		typed_char = 0;
		process_message();
	} while (do_pan());		
}




int do_pan() {
	
	int operation;
	
	surpress_text = 1;

	if (is_char) {
		if      (typed_char == 37) operation = 3;
		else if (typed_char == 38) operation = 1;
		else if (typed_char == 39) operation = 4;
		else if (typed_char == 40) operation = 2;
		else if (typed_char <= 0) // something is screwed up here
			return 1;
		else
			return 0;
	}
	else
		return 1;
		
	old_originx = origin.x;
	old_originy = origin.y;
	
	if (operation == 1) // up
		origin.y += y_limit/3;
	
	if (operation == 2) // down
		origin.y -= y_limit/3;
	
	if (operation == 3) // left
		origin.x += x_limit/3;
	
	if (operation == 4) // right
		origin.x -= x_limit/3;
	
	re_eval_equation(graph_eq);
	redraw();
	surpress_text = 0;
	
	return 1;	
}




void zoom() {
	
	double zoom, xval, yval;
	
	needs_cleared = 1;
	
	cout << "\nEnter zoom factor(0 = zoom previous, < 1 = zoom in, > 1 = zoom out, from .1 to 10):  ";	
	zoom = ind();
	
	if (zoom == 1)
		return;
	
	if (zoom < 0)
		zoom = -zoom;
		
	if (zoom < .1 && zoom != 0)
		zoom = .1;
		
	if (zoom > 10)
		zoom = 10;
	
	calc_size_params(zoom, zoom);
	
	old_originx = origin.x;
	old_originy = origin.y;
	
	if (zoom != 0) {
		xval = x_limit/2 - origin.x;
		yval = y_limit/2 - origin.y;
		origin.x = x_limit/2 - xval/zoom;
		origin.y = y_limit/2 - yval/zoom;
	}
	re_eval_equation(graph_eq);
	redraw();
}




void zoom_box() {

	Point p1, p2;
	double x_box, y_box; // lesser values
	
	cout << "\nClick where you would like to put the lower left corner of the zoom box." << endl;
	p1 = GetDWClick();
	cout << "Click where you would like to put the upper right corner of the zoom box." << endl;
	p2 = GetDWClick();
	
	SetDrawingColorx(PURPLE);
	RectangleFrame(p1.h, p1.v, p2.h, p2.v);
	
	wait(0.25);
	
	if (p2.h > p1.h)
		x_box = p1.h;
	else
		x_box = p2.h;
		
	if (p2.v > p1.v)
		y_box = p1.v;
	else
		y_box = p2.v;
	
	old_originx = origin.x;
	old_originy = origin.y;
		
	calc_size_params((double)(fabs((double)(p2.h - p1.h))/x_limit), (double)(fabs((double)(p2.v - p1.v))/y_limit));

	origin.x = (origin.x - x_box)/(domain/old_xsize);
	origin.y = (origin.y - y_box)/(range/old_ysize);
	
	re_eval_equation(graph_eq);
	redraw();
}




void fit_to_screen() {
	
	double x_min(0), x_max(0), y_min(0), y_max(0), this_x, this_y;
	unsigned int i(0);
	
	if (useLOG) { // not sure about this log stuff...
		for (; (int)i < Count && graph_data[i].x <= 0; ++i) {};
			
		if (useLOG == 1)
			for (; (int)i < Count && graph_data[i].y <= 0; ++i) {};
	}
	for (; (int)i < Count; ++i) {
		if (useLOG)
			this_x = log10(graph_data[i].x);
		else
			this_x = graph_data[i].x;
			
		if (useLOG == 1)
			this_y = log10(graph_data[i].y);
		else
			this_y = graph_data[i].y;
		
		if (i == 0 || this_x < x_min)
			x_min = this_x;
		if (i == 0 || this_x > x_max)
			x_max = this_x;
		if (i == 0 || this_y < y_min)
			y_min = this_y;
		if (i == 0 || this_y > y_max)
			y_max = this_y;
	}	
	if (y_min >= 0)
		y_min /= fit_border_ratio;
	else
		y_min *= fit_border_ratio;
	if (y_max >= 0)
		y_max *= fit_border_ratio;
	else
		y_max /= fit_border_ratio;
	
	if (y_max == 0)
		y_max = -y_min*((fit_border_ratio - 1.0)/2.0);
	if (y_min == 0)
		y_min = -y_max*((fit_border_ratio - 1.0)/2.0);

	if (x_max == x_min) {
		x_max += 1;
		x_min -= 1;
	}
	if (y_max == y_min) {
		y_max += 1;
		y_min -= 1;
	}
	old_originx = origin.x;
	old_originy = origin.y;
	
	origin.x = x_limit*(-x_min/(x_max - x_min));
	origin.y = y_limit*(y_max/(y_max - y_min));
		
	calc_size_params((x_max - x_min)/domain, (y_max - y_min)/range);

	redraw();
}




void calc_size_params(double zoom_x, double zoom_y) {

	if (zoom_x == 0 || zoom_y == 0)	{
		if (old_xsize == 0 || old_ysize == 0) {
			beep();
			cout << "There were no previous zoom scales to return to!" << endl;
			return;
		}
		else {
			domain = old_xsize;
			range  = old_ysize;
		}
		if (old_originx != 0 && old_origin.y != 0) {
			origin.x = old_originx;
			origin.y = old_originy;
		}
	}
	else {	
		old_xsize = domain;
		old_ysize = range;
	
		domain = fabs(round_scales(domain*zoom_x));
		range  = fabs(round_scales(range*zoom_y));
	}
	if (domain <= 0)
		domain = 1;
			
	if (range <= 0)
		range = 1;
		
	x_scale = x_limit/domain;
	y_scale = y_limit/range;
}




int compare_point_xval(void *A, void *B) {

	point *a = (point *)A, *b = (point *)B;

	if (a->x < b->x)
		return -1;
	else if (a->x > b->x)
		return 1;
	else
		return 0;
}


// not used yet but may be useful
int compare_point_yval(void *A, void *B) {

	point *a = (point *)A, *b = (point *)B;

	if (a->y < b->y)
		return -1;
	else if (a->y > b->y)
		return 1;
	else
		return 0;
}





void write_point(FILE *qfilename, double x, double y, char &show_x, char &show_y, int y_val_only) {
	
	int exponent;
	
	if (!y_val_only && is_bad_number(x)) {
		if (show_x) {
			cout << "\nWarning: Cannot write x values of infinity or NAN to file.";
			show_x = 0;
		}
		return;
	}
	if (is_bad_number(y)) {
		if (show_y) {
			cout << "\nWarning: Cannot write y values of infinity or NAN to file.";
			show_y = 0;
		}
		return;
	}
	if (!y_val_only) {
		exponent = get_num_exp(x);
		
		if (exponent == 0) 
			fprintf(qfilename, "%f ", x);
		else 
			fprintf(qfilename, "%fe%i ", x, exponent);
	}
	exponent = get_num_exp(y);
		
	if (exponent == 0)
		fprintf(qfilename, "%f\n", y);
	else 
		fprintf(qfilename, "%fe%i\n", y, exponent);
}




int read_point(FILE *qfilename, ifstream &infile, int x_values) {

	if (read_using_scanf) {
		if (x_values) // currently doesn't work right
			return fscanf(qfilename, "%lf %lf", &graph_data[Count].x, &graph_data[Count].y);
		else {
			graph_data[Count].x = Count;
			return fscanf(qfilename, "%lf", &graph_data[Count].y);
		}
	}
	else {
		if (x_values)
			return (infile.good() && infile >> graph_data[Count].x >> graph_data[Count].y);
		else {
			graph_data[Count].x = Count;
			return (infile.good() && infile >> graph_data[Count].y);
		}
	}
}




void save_graph() {

	function_id = 18;
	
	char show_x(1), show_y(1);
	int quicksave = 0, y_val_only(0);
	FILE *qfilename = NULL;
	ofstream outfile;
	char sg_name[MAX_SAVE_CHARACTERS + 1] = {0};
	
	if (!Count) {
		beep();
		cout << "\nThere is no graph to save!" << endl;
	}
	
	else {
		cout << "\n\nWarning:This may take a few seconds and will use about " << ((Count + 1)*2*sizeof(double) + 436) << " bytes of disk space." << endl;
		cout << "Save graph as(max 31 characters):  ";
		cin >> ws;					
		strcpy(sg_name, xin(MAX_SAVE_CHARACTERS));
		
		if (!overwrite_file(sg_name))
			return;
		
		cout << "\nQuicksave (larger file size)?  ";
		quicksave = decision();

		if (useLOG == 0 && Count > 1 && (graph_data[1].x - graph_data[0].x) == 1.0) {
			cout << "Print x values along with y values?  ";
			if (!decision())
				y_val_only = 1;
		}		
		init_progress_bar(0);
		show_clock();
		
		if (quicksave)
			qfilename = fopen(sg_name, "w");
		else
			outfile.open(sg_name, ios::out | ios::trunc);

		if (outfile.fail() || !filestream_check(outfile)) {
			beep();
			reset_cursor();
			cerr << "\nError: Graph could not be saved as " << sg_name << "!" << endl;
			return;
		}
		if (quicksave) {
			if (useLOG == 1)
				fprintf(qfilename, "Circuit_Solver_Saved_Log_Graph\n");
			else if (useLOG == 2)
				fprintf(qfilename, "Circuit_Solver_Saved_Logx_Graph\n");
			else		
				fprintf(qfilename, "Circuit_Solver_Saved_Graph\n");
		}
		else {
			if (useLOG == 1)
				outfile << "Circuit_Solver_Saved_Log_Graph" << endl;
			else if (useLOG == 2)
				outfile << "Circuit_Solver_Saved_Logx_Graph" << endl;
			else		
				outfile << "Circuit_Solver_Saved_Graph" << endl;
		}
		for (unsigned int i = 0; (int)i < Count; ++i) {
			if (!(i%Graph_mod_num)) 
				update_progress_bar(100*i/Count, 0);
			
			if (quicksave) {
				write_point(qfilename, graph_data[i].x, graph_data[i].y, show_x, show_y, y_val_only);
			}
			else {
				if (!y_val_only) {
					outfile << graph_data[i].x << " ";
				}
				outfile << graph_data[i].y << endl;
			}			
			if (!quicksave && !outfile.good()) {
				cerr << "\nError: Not enough space available to save entire graph!\nOnly " 
					 << i << " points were saved" << endl;
					 
				outfile.close();
				redraw();
				reset_cursor();
				return;
			}
		} 		
		if (quicksave) {
			fflush(qfilename);
			fclose(qfilename);
		}
		else
			outfile.close();
		
		update_progress_bar(100, 0);
						
		reset_cursor();
	
		cout << "\nThe current Circuit Solver graph has been saved as: " << sg_name << "." << endl;		
	}
}





void open_graph() {

	function_id = 15;

	FILE *qfilename = NULL;
	ifstream infile;
	char og_name[MAX_SAVE_CHARACTERS + 1] = {0}, new_graph(0);
	int monotonic_error(0), x_values(1);
	string gname_id;
	
	if (OPEN != 2) {
		cout << "\n\nOpen graph named:  ";
		cin  >> ws;
		strcpy(og_name, xin(MAX_SAVE_CHARACTERS));
	}
	else
		strcpy(og_name, NAME);
		
	OPEN = 0;		
	show_clock();
	
	if (!infile_file(infile, og_name)) {
		beep();	
		reset_cursor();	
		cerr << "\nThe Circuit Solver graph " << og_name << " could not be opened!" << endl;
		return;
	}
	if (!filestream_check(infile))
		return;
	
	infile >> gname_id;
	
	if (gname_id == "Circuit_Solver_Saved_Graph")
		useLOG = 0;
	else if (gname_id == "Circuit_Solver_Saved_Log_Graph")
		useLOG = 1;	
	else if (gname_id == "Circuit_Solver_Saved_Logx_Graph")
		useLOG = 2;
	else {
	 	beep();
	 	if (!filestream_check(infile))
			return;
			
		if ((gname_id[0] >= '0' && gname_id[0] <= '9') || gname_id[0] == '.' || gname_id[0] == '-')	 	
	 		infile >> gname_id;
	 		
		cout << "\nWarning: The file " << og_name << " is not a Circuit Solver graph." << endl;
		
		if (ask_to_open_non_CS) {
			cout << "Open anyway?  ";	
			if (!decision()) {
				infile.close();
				reset_cursor();
				return;
			}
		}
		infile.seekg(0);
		
		cout << "\nDoes this graph include X values as well as Y values?  ";
		if (!decision())
			x_values = 0;	
	}
	if (!Count)
		new_graph = 1;
	
	if (read_using_scanf) {
		infile.close();
		qfilename = fopen(og_name, "r");
	}
	
	Count = 0;	
	init_progress_bar(0);
			
	while (Count < MAX_POINTS && read_point(qfilename, infile, x_values)) {
		
		if (!monotonic_error && Count > 0 && graph_data[Count].x < graph_data[Count-1].x) {
			beep();
			cout << "\nWarning: X values are not motonically increasing. The graph may not be displayed properly and some operations may not work correctly." << endl;
			cout << "Would you like Circuit Solver to sort the point file by x value? ";
			if (decision())
				monotonic_error = 2;
			else
				monotonic_error = 1;
		}
		if (!(Count%Graph_mod_num)) 
			update_progress_bar(Count/200, 0);
					
		++Count;
	}
	if (Count == 0)
		cout << "Saved graph point file is empty or contains only invalid points!" << endl;
	else
		using_entered_equation = 0;
	
	reset_cursor();	
	update_progress_bar(100, 0);
	
	if (monotonic_error == 2) {
		cout << "\nSorting points..." << endl;
		qsort((void *)graph_data, Count, sizeof(point), comp_type(compare_point_xval));
		
		// this will sort by y values:
		//qsort((void *)graph_data, Count, sizeof(point), comp_type(compare_point_yval));
	}
	if (new_graph) {
		domain = fabs(graph_data[Count - 1].x - graph_data[0].x);
		
		if (useLOG)
			domain = 2*log10(domain);
		if (domain == 0)
			domain = 1;

		domain = 1.0001*fabs(domain);
		domain = min(domain, max_domain);
		
		if (x_values)
			old_origin.x = origin.x = 10; // origin on left side because X starts at 0
		
		calc_size_params(1, 1);
	}		
	cout << "\nThe graph has been opened successfully!" << endl;

	if (read_using_scanf)
		fclose(qfilename);
	else
		infile.close();
			
	num_points = Count;
}

