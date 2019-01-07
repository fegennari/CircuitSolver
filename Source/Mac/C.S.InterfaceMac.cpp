#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.InterfaceMac.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S.Interface (MAC);
// By Frank Gennari
CursHandle ClockCursor;
DialogPtr Menubar_Dialog, Progress_Dialog;

int clock_speed, small_screen(1), x_limit(x_window), y_limit(y_window), Frame(0), WinClick(0), progress(0), DEMO(1);
Point pt;

extern int sound, has_menubar, storeDATAA, storeDATAB, grid_color, object_size, click_type;
extern char input[];
extern box num;

void quit_app();
void input_equation(char *equation);
void set_text_size(int size);
void draw_string2(unsigned char *display); 
void clear_buffer();
int decision_dialog(int id);
int got_click();
void close_drawing_window();


void draw_double(double num, char label, int box_num);
void draw_string(char *display);

int get_order(int component_id);

void draw_grid(int grid_color);

int is_in_window(point P);
int is_in_window(Point P);

void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);

void get_AC_waveform_cursor();
void get_DC_waveform_cursor();





void runtest() {
	
	clear_buffer();
}





void quit_app() {
	
	ExitToShell();
}




void input_equation(char *equation) {
	
	strcpy(equation, xin(MAX_ENTERED_CHARACTERS));
}




void set_text_size(int size) {

	TextSize(size);
}




void draw_string2(unsigned char *display) {

	DrawString(display);
}




void reset_cursor() {

	InitCursor();
}




void show_clock() {

	reset_cursor();
	SetCursor(*ClockCursor);
}




void clear_buffer() {
	
	FlushEvents(everyEvent, 0);
}




void wait_for_click() {

	GetClick();
}




int got_click() {
	
	return FoundAClick(&pt);
}




int decision_dialog(int id) {
	
	return 8 - CautionAlert(base_resource_id + id, NULL);
}




void intro() {

	Alert(130, NULL);
}




int prefs() { // Needs some work.

	short item;
    DialogPtr theDlog;
    //Str255 str;
    //long res;
    //Handle hdl;
    //Rect theRect;
    //int type;
    
	theDlog = GetNewDialog(133,(DialogPeek)0L,(WindowPtr)-1L);
	
	/*GetDItem(theDlog, 6, (int *)&type, &hdl, &theRect);
	NumToString(*moves, str);
	SetIText(hdl, str);
	GetDItem(theDlog, 5, (int *)&type, &hdl, &theRect);
	NumToString(*minutes, str);
	SetIText(hdl, str);*/
	
	SetPort(theDlog);
		
	do {
		ModalDialog(NULL, &item);
		cout << "\nValue = " << item << ".";
	} while (item > 10);
	
	/*if (item == 1) {
		GetDItem(theDlog, 6, &type, &hdl, &theRect);
		GetIText(hdl, str);
		StringToNum(str, &res);
		*moves = res;
		GetDItem(theDlog, 5, &type, &hdl, &theRect);
		GetIText(hdl, str);
		StringToNum(str, &res);
		*minutes = res;
	}*/
	
	DisposeDialog(theDlog); 
	
	cout << "\nReturn Value = " << item << "." << endl;
	
	//DisposeDialog(theDlog); // Do I need two of these?
	
	return item;
}




void draw_menubar() {
    
	Menubar_Dialog = GetNewDialog(134,(DialogPeek)0L,(WindowPtr)-1L);
	
	SelectDrawingWindow();
 	SetPort(DrawingWindow);
 	
 	has_menubar = 1;
}



void delete_menubar() {

	DisposeDialog(Menubar_Dialog); 
	
	has_menubar = 0;
}




int memory_error() {
	
	return CautionAlert(131, NULL);
}




void get_cursors() {
	
	ClockCursor = GetCursor(watchCursor);
	HNoPurge((Handle)ClockCursor);
}




void set_special_cursor(int type) {
	
	CursHandle Cursor;
	
	if (type < 0 || type > BDELETE_CURSOR)
		return;
		
	if (type <= PROBEAC_CURSOR)
		Cursor = GetCursor(base_res_id + type);
	else
		Cursor = GetCursor(second_base_cursor_id + type);
	
	HNoPurge((Handle)Cursor);
	SetCursor(*Cursor);
}




void set_cursor(int ID, int is_rotated) {
	
	int ID2, order, offset(base_s_cursor_id);
	
	CursHandle ComponentCursor;
	
	if (is_rotated)
		ID2 = ID + cursor_offset;
	else
		ID2 = ID;
	
	order = get_order(ID);
	
	switch (order) {
	
		case wirex:
			++offset;
			
		case icx:
			++offset;
			
		case transistorx:
			++offset;
			
		case flipflopx:
			ComponentCursor = GetCursor(offset + ID2 - ID);
			break;
			
		default:
			ID2 += init_cursor_id;
			if ((ID2 >= base_s_cursor_id && ID2 <= max_cursor_id) || (ID2 >= base_s_cursor_id + cursor_offset && ID2 <= max_cursor_id + cursor_offset))
				ComponentCursor = GetCursor(ID2);
			else
				ComponentCursor = GetCursor(init_cursor_id);
	}
	HNoPurge((Handle)ComponentCursor);
	SetCursor(*ComponentCursor);		
}




void get_AC_waveform_cursor() {
	
	CursHandle ACCursor;
	
	ACCursor = GetCursor(AC_cursor_frame0_id + Frame);
	HNoPurge((Handle)ACCursor);
	SetCursor(*ACCursor);
	
	if (Frame == 3)
		Frame = 0;
	else
		++Frame;
}




void get_DC_waveform_cursor() {

	CursHandle DCCursor;
	
	DCCursor = GetCursor(DC_cursor_frame0_id + Frame);
	HNoPurge((Handle)DCCursor);
	SetCursor(*DCCursor);
	
	if (Frame == 3)
		Frame = 0;
	else
		++Frame;
}




int interrupt() {
	
	if (got_click()) {	
		if (WinClick == 2 && int(pt.v*(num_menu_boxes + 1)/MENU_HEIGHT) + MENU_OFFSET == CANCEL)	
			return 1; // click on menubar
		else
			if (which_box(pt) == CANCEL)  // click on graphics window
				return 1;
	}		
	return 0;
}




void get_continue() {
		
	cout << "\n You may now scroll the console window. Type in any character and press return to continue designing/testing your circuit." << endl;
	xin(MAX_CHARACTERS);
}



int get_window_click() {

	Point pt;

	pt = GetClick();
	
	if (get_event())
		click_type = 11; // ??? *****************
	else
		click_type = 0;
			
	if (WinClick == 2) { // click on menubar
		if (pt.h > MENU_WIDTH)
			return -1;
		return int(pt.v*(num_menu_boxes + 1)/MENU_HEIGHT) + MENU_OFFSET;
	}
	else if (WinClick == 1)  // click on console window
		return -1;
	else
		return which_box(pt); // click on graphics window
		
	return -1;
}




void whiteout() {

	SetColor(WHITE);
	RectangleDraw(0, 0, (int)(1.5*x_limit + 2), (int)(1.5*y_limit + 2));
	
	SetColor(12);
	RectangleFrame(0, 0, x_limit, y_limit);
}





Point GetDWClick() {

	Point pt;
	
	do {
		pt = GetClick();
	} while (WinClick == 1 || !is_in_window(pt));
	
	return pt;
}




void draw_text_on_window(string text) {
	
	char display[MAX_CHARACTERS + 1];
	
	string_to_char(display, text, MAX_CHARACTERS);
		
	whiteout();
	
	MoveTo((int)(x_limit/2 - strlen(display)*3.5), (int)(y_limit/2 + 3));
	SetColor(BLACK);
	draw_string(display);
}



// Must write if possible.
void set_DW_title(const char *title) { 

	
}




void init_progress_bar(int mode) {
	
	int x_pos, y_pos;
	
	if (mode == 1) {
		x_pos = x_limit/2;
		y_pos = y_limit/2;
	}
	else {
		x_pos = 124;
		y_pos = -12;
	}
	SetColor(BLACK);
	RectangleFrame((x_pos - 120), (y_pos + 16), (x_pos + 120), (y_pos + 30));
	SetColor(8);
	RectangleDraw((x_pos - 119), (y_pos + 17), (x_pos + 119), (y_pos + 29));
	
	/*if (!progress) {
		Progress_Dialog = GetNewDialog(135,(DialogPeek)0L,(WindowPtr)-1L);
		progress = 1;
	}
	SetPort(Progress_Dialog);
	
	SetColor(BLACK);
	RectangleFrame(600, 50, 800, 70);
	SetColor(8);
	RectangleDraw(601, 51, 799, 69);*/
}




void update_progress_bar(int complete, int mode) {
	
	int x_pos, y_pos;
	
	if (mode == 1) {
		x_pos = x_limit/2;
		y_pos = y_limit/2;
	}
	else {
		x_pos = 124;
		y_pos = -12;
	}
	SetColor(14);
	RectangleDraw((x_pos - 119), (y_pos + 17), (int)(x_pos - 119 + 2.38*complete), (y_pos + 29));
	
	/*SetColor(14);
	RectangleDraw(601, 51, (601 + 2*complete), 69);
	
	if (complete >= 100 && progress) {
		DisposDialog(Progress_Dialog);
		progress = 0;
	}*/
	if (mode == 0 && complete == 100) {
		SetColor(WHITE);
		RectangleDraw(4, 4, 244, 18);
	}
}




int update_timer(int init_time, int last_time) {

	int current_time, hr(0), min(0), sec(0);
	char time[32];
	
	current_time = get_time();
	
	if (last_time == current_time)
		return current_time;
		
	if (init_time == 0 || last_time == 0) {
		draw_status_bar();
		return current_time;
	}
	hr =  (current_time - init_time)/3600;
	min = ((current_time - init_time)/60)%60;
	sec = (current_time - init_time)%60;
	if (min < 10) {
		if (sec < 10)
			sprintf(time, "x%i:0%i:0%i", hr, min, sec);
		else
			sprintf(time, "x%i:0%i:%i", hr, min, sec);
	}
	else {
		if (sec < 10)
			sprintf(time, "x%i:%i:%i", hr, min, sec);
		else
			sprintf(time, "x%i:%i:%i", hr, min, sec);
	}	
	draw_status_bar();
	draw_status(time);
	
	return current_time;
}




void show_timer() {
	
	EventRecord	 theEvent;
	
	for (int time = 0, time0 = update_timer(0, 0); !interrupt(); time = update_timer(time0, time)) {
		GetNextEvent(everyEvent, &theEvent);
	}
}




void Window_setup() {
	
	SetColor(BLACK);
	
	if (small_screen == 1)
		MoveWindow(FrontWindow(), 160, 40, 1);
	
	else if (small_screen == 2)
		MoveWindow(FrontWindow(), 80, 40, 1);
	
	else
		MoveWindow(FrontWindow(), 164, 40, 1);
		
	init();
	
	SetColor(12);
	RectangleFrame(0, 0, x_limit, y_limit);
}




void close_drawing_window() {
	
	CloseWindow(DrawingWindow);
}





int get_event() {

	EventRecord	 theEvent;
		
	clear_buffer();
		
	GetNextEvent(everyEvent, &theEvent);
	
	//CHAR = theEvent.message & charCodeMask;
		
	if ((theEvent.modifiers & cmdKey)) 
		return 1;
		
	return 0;
}




void wait(double time) {
	
	EventRecord theEvent;
		
	for (unsigned int x = 0; x < DELAY2*time*clock_speed; ++x)
		GetNextEvent(everyEvent, &theEvent);
}





int get_time() {
	
	int time;
	
	GetDateTime((unsigned long *)&time);
	
	return time;
}




int read_specs() {

	ifstream infile;
	string title, type;
	int x_pixel, windows(0), key;
		
	if (!infile_file(infile, "C.S.MachineSpecs")) {
		cout << "\nSince the file <C.S. Machine Specs> was not found, clock speed must be calculated.";
		
		clock_speed = clock_microprocessor();
		small_screen = 1;
	}
	else {
		infile >> title >> type >> title >> clock_speed >> title >> x_pixel >> title >> sound >> title >> windows >> title >> storeDATAA >> title >> storeDATAB >> title >> key;
		
		x_limit = x_pixel;
		y_limit = (int)(x_pixel/window_ratio);
		
		if (x_pixel < 640)
			small_screen = 2;
		else if (x_pixel < 1024)
			small_screen = 1;
		else
			small_screen = 0;
	
		if (type == "68K")
			cout << "\nWarning: Circuit Solver runs best on a PowerMac or G3, and may not run at all on a 68K machine!"
				 << endl;
				 
		else if (type != "PPC" && type != "G3")
			cout << "\nError: Unknown Machine Type!" << endl;
	}
	infile.close();
	
	if (key == key_val)
		DEMO = 0;
	else
		windows = default_demo_windows;
		
	if (clock_speed < 4)
		clock_speed = clock_microprocessor();
	
	object_size = (int)(sqrt(float(x_limit*y_limit))/pi);
	
	return windows;
}




int clock_microprocessor() {
	
	int time1, speed;
	unsigned long i;
		
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

	int i;
	
	SelectConsoleWindow();
	
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
			
		} while (input[size] != '\n');
	
	input[size] = 0;
			
	return input;
}





void beep() {
	
	if (sound) {
		SysBeep(10);
	}
}
