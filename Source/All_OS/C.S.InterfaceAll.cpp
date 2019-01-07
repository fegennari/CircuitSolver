#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.InterfaceAll.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S.Interface (All OS);
// By Frank Gennari
int const ERROR_QUIT_SECONDS = 6;

int clock_speed, x_pixel(x_window), y_pixel((int)(2.5*y_window)), x_limit(x_window), y_limit(y_window), DEMO(1), time_ms(0);
int is_char(0), window_mode(0), enablePAINT(0);
char typed_char(0);

int use_system_tick_for_delay = 1;

const char s_class_name[32]  = "";
const char g_class_name[32]  = "";
const char s_window_name[32] = "";
const char g_window_name[32] = "";


extern int sound, storeDATAA, storeDATAB, grid_color, object_size, found_click, click_type, text_size, default_colors, must_exit, ar_changed, graph_enable, useDIALOG;
extern char input[];
extern double DW_ratio;
extern Point pt;


void quit_app();
void exit_error();
int decode_menu_click(int id);
void input_equation(char *equation);
void set_text_size(int size);
void draw_string2(unsigned char *display);
void switch_to_graphing_window();
void switch_to_schematic_window(); 
void clear_buffer();
int decision_dialog(int id);
void init_graphics();
void init_graphics_window(int is_graphing);
int got_click();
void paint_menu_buttons();
void close_drawing_window();
void process_message();
char read_char();


void draw_double(double num, char label, int box_num);
void draw_string(char *display);

int get_order(int component_id);

void draw_grid(int grid_color);

void draw_cancel_button();

int is_in_window(point P);
int is_in_window(Point P);

void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);

void get_AC_waveform_cursor();
void get_DC_waveform_cursor();




void runtest() {

	

}





void quit_app() {
	
	exit(0);
}




void exit_error() {
	
	beep();
    cerr << "Quitting in " << ERROR_QUIT_SECONDS << " seconds." << endl;
    wait(ERROR_QUIT_SECONDS);
	exit(1);
}




int decode_menu_click(int id) {
	
	if (id == 100) { // berzerk delete
		click_type = 200;
		return 2;
	}
	else if (id == 101) { // help-about
		intro();
		return get_window_click();
	}
	return id;
}




void input_equation(char *equation) {
	
	strcpy(equation, xin(MAX_ENTERED_CHARACTERS));
}




void set_text_size(int size) {

}




void draw_string2(unsigned char *display) {

}




void wait_for_click() {

	GetClick();
}




int got_click() {
	
	return FoundAClick(&pt);
}




int got_cancel_click() {

	return (got_click() && pt.h < cancel_button_length && pt.v < cancel_button_width);	
}




int decision_dialog(int id) { // y = 1, n = 0, c = 2
	
	useDIALOG = 0;

	return xdecision(0);
}




void intro() {

}




int prefs() {

	return 0;
}




void init_graphics() {

}




void init_graphics_window(int is_graphing)  { // create graphics window
    
}




void draw_menubar() {  
 	
}



void delete_menubar() {

}




void paint_menu_buttons() {

}




void switch_to_graphing_window() {

}





void switch_to_schematic_window() {

}




int memory_error() {
	
	cout << "This may take awhile." << endl;
	return 0;
}



void memory_message() {
	
}




void reset_cursor() {

}




void show_clock() {

}




void get_cursors() {
	
}




void set_special_cursor(int type) {
	
}




void set_cursor(int ID) {
		
}




void get_AC_waveform_cursor() {
	
}




void get_DC_waveform_cursor() {

}




int interrupt() {
	
	if (got_click())
		return 1;
			
	return 0;
}




void get_continue() {
	
	#ifdef CAN_SCROLL		
		cout << "\n You may now scroll the console window. Type in any character and press return to continue designing/testing your circuit." << endl;
		xin(MAX_CHARACTERS);
	#endif
}




int get_window_click() {

	pt = GetClick();

	return which_box(pt);
}




void whiteout() {
	
}





Point GetDWClick() {
	
	pt = GetClick();
	
	return pt;
}




void draw_text_on_window(string text) {
	
}




void set_DW_title(const char *title) {

}




void init_progress_bar(int mode) {
	
}




void update_progress_bar(int complete, int mode) {
	
}




int update_timer(int init_time, int last_time) {

	int current_time, etime;
	char time[32];
	
	time_ms = 0;
	current_time = get_time();
	
	if (last_time == current_time)
		return current_time;
		
	if (init_time == 0 || last_time == 0) {
		draw_status_bar();
		return current_time;
	}
	etime = current_time - init_time;
	format_time(etime, time);
		
	draw_status_bar();
	draw_status(time);
	
	return current_time;
}




void show_timer() {
	
}




void Window_setup(int is_graphing) {
	
}




void close_drawing_window() {
	
}




void wait(double time) { // *** ??? ***

	unsigned int x;
	
	if (use_system_tick_for_delay) {
		//sleep((unsigned)time);
		//for (x = GetTickCount(); (GetTickCount() - x) < time*1000; process_message()) {};
	}
	else	
		for (x = 0; x < DELAY2*time*clock_speed; ++x)
			process_message();
}




void process_message() {

}





int get_time() {
	
	//if (time_ms)
		//return GetTickCount();
		
	time_t currTime;	
	time(&currTime);
		
	return currTime;
}




int read_specs() {

	ifstream infile;
	string title, type;
	int windows(0), key;
		
	if (!infile_file(infile, "C.S.MachineSpecs")) {
		cout << "\nSince the file <C.S. Machine Specs> was not found, clock speed will be calculated.";
		clock_speed = clock_microprocessor();
	}
	else {
		if (infile.good())
			infile >> title >> type >> title >> clock_speed >> title >> x_pixel >> title >> sound >> title >> windows >> title >> storeDATAA >> title >> storeDATAB >> title >> key;
	
		if (type != "PC")
			cout << "Warning: Computer type is not PC. Please check C.S. Machine Specs." << endl;
	}
	infile.close();
	
	x_limit = x_pixel - window_nonclient_width;
	y_limit = (int)(x_limit/window_ratio);
	
	cout << "\nDisplay resolution is " << x_pixel << "x" << y_pixel;
	
	if (key == key_val)
		DEMO = 0;
	else
		windows = default_demo_windows;
		
	if (clock_speed < 4) {
		cout << "\nClock speed read from C.S. Machine Specs is only " << clock_speed << "MHz. Recalculating to verify..." << endl;
		clock_speed = clock_microprocessor();
	}
	else
		cout << endl;

	object_size = (int)(sqrt(float(x_limit*y_limit))/pi);

	return windows;
}




int clock_microprocessor() {
	
	int time1, speed;
	double i; 
		
	cout << "\nCalculating clock speed. Please wait." << endl;
	show_clock();
	
	time1 = get_time();
	
	while (get_time() == time1){};
		
	time1 = get_time();
	
	for (i = 0; get_time() < (time1 + 1); ++i){};
	
	speed = (int)(i/time_constant);
	
	reset_cursor();
	cout << "\nEquivalent Clock Speed = " << speed << " MHz." << endl;
	
	return speed;
}




char *xin(int size) {

	int i, counter(0);
	
	if (size > MAX_ENTERED_CHARACTERS)
		size = MAX_ENTERED_CHARACTERS;
		
	for (i = 0; i < (size - 1); ++i) {
		cin.get(input[i]);
		if(input[i] == '\n') {
			if (i == 0)
				--i;
			else {
				input[i] = 0;
				break;
			}
		}
	}
	if (i == size - 1)
		do {
			cin.get(input[size]);			
			++counter;
			if (counter > 10000) { // we have a problem
				cerr << "\nError entering input." << endl;
				cin.clear();
				break;
			}
		} while (input[size] != '\n');
	
	input[size] = 0;
	cout << endl;
			
	return input;
}




char read_char() { // used in xin function

	return 0;
}





void beep() {
	
	if (sound) {
		printf("%c", (char)7);
	}
}
