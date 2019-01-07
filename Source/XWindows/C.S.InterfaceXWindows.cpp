#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.InterfaceXWindows.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S.Interface (XWindows);
// By Frank Gennari
int clock_speed, x_pixel, y_pixel, x_limit(x_window), y_limit(y_window), Frame(0), progress(0), nButtons(1), DW_exists(0), showState = SW_SHOWNORMAL, is_char(0), DEMO(1), DWScreen, DWDepth, MBScreen, MBDepth;
char typed_char(0);
//XFontStruct *font;
Font current_font;
Cursor ClockCursor, ArrowCursor, AppStCursor;
Display *DWDisp, *MenubarDisp;
GC DWContext, MBContext;


extern Window DrawingWindow, MenuWindow, ConsoleWindow;

extern Colormap DWColormap, MBColormap;

extern int sound, has_menubar, storeDATAA, storeDATAB, grid_color, object_size, found_click, click_type, text_size, default_colors, must_exit, close_DW;
extern char input[];
extern box num;
extern Point pt, current;

//LONG CALLBACK DoMessage (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam);
//LONG CALLBACK DoMenu    (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam);

void quit_app();
void exit_error();
int decode_menu_click(int id);
void input_equation(char *equation);
void set_text_size(int size);
void draw_string2(unsigned char *display); 
void clear_buffer();
int decision_dialog(int id);
//BOOL APIENTRY AboutDlgProc(HWND hDlg, WORD message,WORD wParam, LONG lParam);
int got_click();
void paint_menu_buttons();
void set_window_for_drawing(Display *TheDisplay, Window TheWindow, GC TheContext);
void end_window_drawing(Window TheWindow, GC TheContext);
void close_drawing_window();
void process_event();
char read_char();


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
	
	//cout << "\nenv = " << getenv("xxx") << endl;
	
	reset_cursor();
}





void quit_app() {
	
	exit(0);
}




void exit_error() {
	
	exit(1);
}



// *
/*LONG CALLBACK DoMessage (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam) { // Get and process one message.

	LONG result = 0; // Function result
	
	switch ( msgCode ) { // Dispatch on message code

		case WM_CLOSE:
			found_click = 1;
			click_type = 114;
			close_DW = 1;
			break;
		
		case WM_LBUTTONDOWN:
			found_click = 1;
			click_type = 0;
			pt.h = LOWORD(lParam);
			pt.v = HIWORD(lParam);
			break;*/
			
		/*case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ClipCursor (NULL);
			break;*/

		/*case WM_RBUTTONDOWN:
			found_click = 1;
			click_type = 11;
			pt.h = LOWORD(lParam);
			pt.v = HIWORD(lParam);
			break;
			
		case WM_KEYDOWN:
			is_char = 1;
			typed_char = 0;
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam);
			break;
			
		case WM_COMMAND:
			found_click = 1;
			click_type = 100 + GET_WM_COMMAND_ID(wParam, lParam);
			break;
		
		//case WM_INITMENU: // menu?
		//case WM_CREATE: // ?
		//case WM_DESTROY: // ?
		//case WM_ACTIVATE: // ?
		//case WM_PAINT: // redraw
		//case WM_MOUSEMOVE: // ignore?
		//case WM_TIMER: // ?
		//case WM_VSCROLL: // must add scrolling
        //case WM_HSCROLL: // must add scrolling

		default:
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam); // Pass message to Windows for default processing
			break;
	} 
	return result;
}*/ 



// *
/*LONG CALLBACK DoMenu (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam) { // Get and process one message.

	LONG result = 0; // Function result
	
	switch ( msgCode ) { // Dispatch on message code

		case WM_LBUTTONDOWN:
			found_click = 1;
			click_type = 1;
			pt.h = LOWORD(lParam);
			pt.v = HIWORD(lParam);
			break;
			
		case WM_RBUTTONDOWN: // Both mouse buttons have the same function in the menu
			found_click = 1;
			click_type = 1;
			pt.h = LOWORD(lParam);
			pt.v = HIWORD(lParam);
			break;
			
		default:
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam); // Pass message to Windows for default processing
			break;
	} 
	return result;
}*/ 




int decode_menu_click(int id) {
	
		if (id == 100) { // berzerk delete
			click_type = 200;
			return 2;
		}
		else if (id == 101) { // menu
			intro();
			return get_window_click();
		}
		return id;
}




void input_equation(char *equation) {
	
	strcpy(equation, xin(MAX_ENTERED_CHARACTERS));
}



// ?
void set_text_size(int size) {
	
	if (size <= 5)
		current_font = XLoadFont(DWDisp, "5x8");
	else if (size == 6)
		current_font = XLoadFont(DWDisp, "6x10");
	else if (size == 7)
		current_font = XLoadFont(DWDisp, "7x13");
	else if (size == 8)
		current_font = XLoadFont(DWDisp, "8x13");
	else if (size == 9)
		current_font = XLoadFont(DWDisp, "9x15");
	else if (size == 10)
		current_font = XLoadFont(DWDisp, "10x20");
	else
		current_font = XLoadFont(DWDisp, "12x24");
	
	XSetFont(DWDisp, DWContext, font->fid);
}



// ?
void draw_string2(unsigned char *display) {

	XTextItem text_item;
	
	text_item.chars = display;
	text_item.nchars = display.length();
	text_item.delta = 0;
	text_item.font = current_font;
	
	XDrawText(DWDisp, DrawingWindow, DWContext, current.h, current.v, text_item, 1);
	
	//XDrawString(DWDisp, DrawingWindow, DWContext, current.h, current.v, display, display.length());
	
	//font = XLoadQueryFont(DWDisp, "FontName")	
	//XSetFont(DWDisp, DWContext, font->fid);
	//XDrawImageString(DWDisp, DrawingWindow, DWContext, current.h, current.v, display, display.length());
}




void wait_for_click() {

	GetClick();
}




int got_click() {
	
	return FoundAClick(&pt);
}



// *
int decision_dialog(int id) { // y = 1, n = 0, c = 2
	
	int msgResult(0);
	
	/*UINT msgStyle = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNOCANCEL | MB_SETFOREGROUND; // dialog box params 
	
	switch (id) {
		case 1:
			msgResult = MessageBox (ConsoleWindow, "Do you want to save the current circuit?", "Save Circuit?", msgStyle);// Display message box
			break;
			
		case 2:
			msgResult = MessageBox (ConsoleWindow, "Do you want to save the current project?", "Save Project?", msgStyle);// Display message box
			break;
			
		case 4: // This should be Cancel/Continue
			msgResult = MessageBox (ConsoleWindow, "Warning: This circuit may have too many nodes for Circuit Solver to evaluate without running out of memory.", "Memory Warning", msgStyle);// Display message box
			break;
	}
	if (msgResult == 6)
		return 1;
	else if (msgResult == 7)
		return 0;*/

	return msgResult;
}



// *
/*BOOL APIENTRY AboutDlgProc(HWND hDlg, WORD message, WORD wParam, LONG lParam){
		
  if (message == WM_COMMAND) {
      EndDialog(hDlg, TRUE);
      return 1;
  }
  if (message == WM_INITDIALOG)
      return 1;
  else
      return 0;
      
  UNREFERENCED_PARAMETER(wParam);   
  UNREFERENCED_PARAMETER(lParam);   
}*/



// *
void intro() {

	//FARPROC lpprocAbout = MakeProcInstance((FARPROC)AboutDlgProc, thisInstance);
	//DialogBox(thisInstance, MAKEINTRESOURCE(3), DrawingWindow, (DLGPROC)lpprocAbout);
}



// *
int prefs() {

	return 0;
}



// ?
void  init(void)  {

	//XSetErrorHandler(WriteXError);

	/*DrawingWindow = XCreateWindow(DWDisp, RootWindow(DWDisp, DefaultScreen(DWDisp)), 
		0, 
		y_pixel - y_limit - window_nonclient_width - 45 - menuwidth, 
		x_limit + window_nonclient_width, 
		y_limit + window_nonclient_height + menuwidth,
		Border_Width, CopyFromParent, CopyFromParent, CopyFromParent,
		(CWBorderPixel | CWBackPixel), &attributes);*/
		
	//XFlush(DWDisp);
	//XSync(DWDisp, 0);	
	
	int black_c, white_c;
	XEvent xevent;
	XTextProperty text_prop;
	char wname[16] = "Circuit Solver";
	
	DWDisp = XOpenDisplay(NULL);
	if (DWDisp == NULL) {
		beep();
		cout << "\nError: Cannot create graphics display. Quitting..." << endl;
		wait(3);
		exit_error();
	}
	DWScreen = DefaultScreen(DWDisp);
	DWColormap = DefaultColormap(DWDisp, DWScreen);
	DWDepth = DefaultDepth(DWDisp, DWScreen);
	black_c = BlackPixel(DWDisp, DefaultScreen(DWDisp));
	white_c = WhitePixel(DWDisp, DefaultScreen(DWDisp));
	
	// create window
	DrawingWindow = XCreateSimpleWindow(DWDisp, DefaultRootWindow(DWDisp), 
					0, y_pixel - y_limit - window_nonclient_width - 45 - menuwidth,
					x_limit + window_nonclient_width, y_limit + window_nonclient_height + menuwidth,
					0, black_c, black_c);
	if (DrawingWindow == NULL) {
		beep();
		cout << "\nError: Cannot create graphics window. Quitting..." << endl;
		wait(3);
		exit_error();
	}					
	XSelectInput(DWDisp, DrawingWindow, StructureNotifyMask);
	XMapWindow(DWDisp, DrawingWindow);
	
	// label window
	XStringListToTextProperty(&wname, 1, &text_prop);
	XSetWMName(DWDisp, DrawingWindow, &text_prop);
	XSetStandardProperties(DWDisp, DrawingWindow, wname, wname, None, NULL, 0, NULL);
	
	// wait for MapNotify event
	while (1) {
		XNextEvent(DWDisp, &xevent);
		if (xevent.type == MapNotify)
			break;
	}
	
	// set event types
	XSelectInput(DWDisp, DrawingWindow, ExposureMask | ButtonPressMask | KeyPressMask);
	
	// erase window, select window
	whiteout();
	SelectDrawingWindow();
	set_text_size(12);
}



// ?
void draw_menubar() {

	/*MenuWindow = XCreateWindow(MenubarDisp, RootWindow(MenubarDisp, DefaultScreen(MenubarDisp)), 
		x_pixel - MENU_WIDTH - MENU_X_OFFSET, 
		0, 
		MENU_WIDTH + window_nonclient_width, 
		MENU_HEIGHT + window_nonclient_height + 6,
		Border_Width, CopyFromParent, CopyFromParent, CopyFromParent,
		(CWBorderPixel | CWBackPixel), &attributes);*/
    
    //XSelectInput(MenubarDisp, MenuWindow, ButtonPressMask|EnterWindowMask|LeaveWindowMask); // ?
    //XFlush(DWDisp);
    
    int black_c, white_c;
	XEvent xevent;
	XTextProperty text_prop;
	char wname[16] = "Menu";
	
	DWDisp = XOpenDisplay(NULL);
	if (DWDisp == NULL) {
		beep();
		cout << "\nError: Cannot create menu display. Quitting..." << endl;
		wait(3);
		exit_error();
	}
	MBScreen = DefaultScreen(MenubarDisp);
	MBColormap = DefaultColormap(MenubarDisp, MBScreen);
	MBDepth = DefaultDepth(MenubarDisp, MBScreen);
	black_c = BlackPixel(MenubarDisp, DefaultScreen(MenubarDisp));
	white_c = WhitePixel(MenubarDisp, DefaultScreen(MenubarDisp));
	
	// create window
	DrawingWindow = XCreateSimpleWindow(MenubarDisp, DefaultRootWindow(MenubarDisp), 
					x_pixel - MENU_WIDTH - MENU_X_OFFSET, 0,
					MENU_WIDTH + window_nonclient_width, MENU_HEIGHT + window_nonclient_height + 6,
					0, black_c, black_c);
	if (MenuWindow == NULL) {
		beep();
		cout << "\nError: Cannot create menu window. Quitting..." << endl;
		wait(3);
		exit_error();
	}					
	XSelectInput(MenubarDisp, MenuWindow, StructureNotifyMask);
	XMapWindow(MenubarDisp, MenuWindow);
	
	// label window
	XStringListToTextProperty(&wname, 1, &text_prop);
	XSetWMName(MenubarDisp, MenuWindow, &text_prop);
	XSetStandardProperties(MenubarDisp, MenuWindow, wname, wname, None, NULL, 0, NULL);
	
	// wait for MapNotify event
	while (1) {
		XNextEvent(MenubarDisp, &xevent);
		if (xevent.type == MapNotify)
			break;
	}
	
	// set event types
	XSelectInput(MenubarDisp, MenuWindow, ExposureMask | ButtonPressMask | KeyPressMask);
	
	// erase window, select window
	whiteout();
	XMapRaised(DWDisp, DrawingWindow);	
	set_text_size(12); 	
 	has_menubar = 1;	
 	paint_menu_buttons();
}


// ?
void delete_menubar() {
	
	XFreeGC(MenubarDisp, MBContext);
	XUnmapWindow(MenubarDisp, MenuWindow);
	XDestroyWindow(MenubarDisp, MenuWindow);
	XCloseDisplay(MenubarDisp);
	has_menubar = 0;
}



// ?
void paint_menu_buttons() {

	int button_width = MENU_HEIGHT/num_menu_boxes;
	char b_name[16] = {0};
	Font menu_font = XLoadFont(DWDisp, "12x24");
	XTextItem text_item;
	XColor color;
	
	text_item.delta = 0;
	text_item.font = menu_font;
	
	set_window_for_drawing(MenubarDisp, MenuWindow, MBContext);	
	
	color.red = 181;
	color.green = 182;
	color.blue = 184;
	XAllocColor(MenubarDisp, MBColormap, &color); // light gray
	XSetForeground(MenubarDisp, MBContext, color.pixel);
	
	XFillRectangle(MenubarDisp, MenuWindow, MBContext, 0, 0, MENU_WIDTH + MENU_X_OFFSET, MENU_HEIGHT + 6);
	
	color.red = 255;
	color.green = 255;
	color.blue = 255;
	XAllocColor(MenubarDisp, MBColormap, &color); // black
	XSetForeground(MenubarDisp, MBContext, color.pixel);
	
	string_to_char(b_name, button_names[0], bn_lengths[0]);
	text_item.chars = b_name;
	text_item.nchars = bn_lengths[0];
	XDrawText(MenubarDisp, MenuWindow, MBContext, 12, 0.17*button_width, text_item, 1);
	
	for (int i = 1; i <= num_menu_boxes; ++i) {
		string_to_char(b_name, button_names[i], bn_lengths[i]);
		text_item.chars = b_name;
		text_item.nchars = bn_lengths[i];
		XDrawLine(MenubarDisp, MenuWindow, MBContext, 0, i*button_width, MENU_WIDTH + MENU_X_OFFSET, i*button_width);
		XDrawText(MenubarDisp, MenuWindow, MBContext, 12, (i+0.17)*button_width, text_item, 1);
	}
	end_window_drawing(MenuWindow, MBContext);
}



// *
int memory_error() {
	
	int msgResult(0);
	
	/*UINT msgStyle = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND; // dialog box params
	
	msgResult = MessageBox (ConsoleWindow, "Warning: Circuit Solver may not have enough memory to simulate a circuit with this many nodes. Continue?", "Memory Warning?", msgStyle);// Display message box
			
	if (msgResult == 6)
		return 0;
	else if (msgResult == 7)
		return 1;*/

	return msgResult;
}


// *
void memory_message() {
	
	//MessageBox(NULL, "Circuit Solver has run out of memory and will now quit.", "Memory Error", 0);
}



// ?
void reset_cursor() {

	SetCursor(ArrowCursor);
}



// ?
void show_clock() {

	SetCursor(ClockCursor);
}



// ?
void get_cursors() {

	*ClockCursor = XCreateFontCursor(DWDisp, 0);
	*ArrowCursor = XCreateFontCursor(DWDisp, 1);
	*AppStCursor = XCreateFontCursor(DWDisp, 2);
	
	XSync(DWDisp, 0); // ?
	
	//ClockCursor = LoadCursor (NULL, IDC_WAIT);
	//ArrowCursor = LoadCursor (NULL, IDC_ARROW);
	//AppStCursor = LoadCursor (NULL, IDC_APPSTARTING);
}



// *
void set_special_cursor(int type) {
	
	Cursor cursor = ArrowCursor;
	
	//curBlank = LoadCursor(hInstance, MAKEINTRESOURCE(1));
	
	if (type < 0 || type > BDELETE_CURSOR)
		return;
		
	if (type <= PROBEAC_CURSOR)
		;//Cursor = GetCursor(128 + type);
	else
		;//Cursor = GetCursor(1000 + type);
		
	SetCursor(cursor);
}



// *
void set_cursor(int ID, int is_rotated) {
	
	int ID2, order, offset(/*base_s_cursor_id*/0);	
	Cursor ComponentCursor = ArrowCursor;
	
	if (is_rotated)
		ID2 = ID /*+ cursor_offset*/;	
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
			//ComponentCursor = GetCursor(offset + ID2 - ID);
			break;	
		
		default:
			;//ID2 += init_cursor_id;
			/*if ((ID2 >= base_s_cursor_id && ID2 <= max_cursor_id) || (ID2 >= base_s_cursor_id + cursor_offset && ID2 <= max_cursor_id + cursor_offset))
				ComponentCursor = GetCursor(ID2);
			else
				ComponentCursor = GetCursor(init_cursor_id);*/
	}
	SetCursor(ComponentCursor);		
}



// *
void get_AC_waveform_cursor() {
	
	Cursor ACCursor = ArrowCursor;
	
	//ACCursor = GetCursor(AC_cursor_frame0_id + Frame);
	SetCursor(ACCursor);
	
	if (Frame == 3)
		Frame = 0;
	else
		++Frame;
}



// *
void get_DC_waveform_cursor() {

	Cursor DCCursor = ArrowCursor;
	
	//DCCursor = GetCursor(DC_cursor_frame0_id + Frame);
	SetCursor(DCCursor);
	
	if (Frame == 3)
		Frame = 0;
	else
		++Frame;
}




int interrupt() {
	
	if (got_click()) {
		found_click = 0;
		if (click_type == 1 && int(pt.v*(num_menu_boxes + 1)/MENU_HEIGHT) == CANCEL)	
			return 1; // click on menubar
		else if (which_box(pt) == CANCEL)  // click on graphics window
			return 1;
		else if (click_type == 100 + CANCEL)
			return 1;
	}		
	return 0;
}




void get_continue() {
		
	#ifdef CAN_SCROLL		
		cout << "\n You may now scroll the console window. Type in any character and press return to continue designing/testing your circuit." << endl;
		xin(MAX_CHARACTERS);
	#endif
}




int get_window_click() {

	Point pt;

	pt = GetClick();
		
	if (click_type == 1) { // click on menubar
		if (pt.h > MENU_WIDTH + MENU_X_OFFSET)
			return -1;
		return int(pt.v*(num_menu_boxes + 1)/MENU_HEIGHT);
	}
	if (click_type > 100) // menu selection (handled like a click) 
		return decode_menu_click(click_type - 100);
	
	return which_box(pt);
}



// ?
void whiteout() {
	
	/*SetDrawingColorx(WHITE);
	RectangleDraw(0, 0, (int)(1.5*x_limit + 2), (int)(1.5*y_limit + 2));
	
	SetDrawingColorx(12);
	RectangleFrame(0, 0, x_limit, y_limit);*/
	
	XClearWindow(DWDisp, DrawingWindow);
}





Point GetDWClick() {

	Point pt;
	
	do {
		pt = GetClick();
	} while ((click_type != 0 && click_type != 11) || !is_in_window(pt)); // not in graphics window
	
	return pt;
}




void draw_text_on_window(string text) {
	
	char display[MAX_CHARACTERS + 1] = {0};
	
	string_to_char(display, text, MAX_CHARACTERS);
		
	whiteout();
	
	MoveTo((int)(x_limit/2 - strlen(display)*3.5), (int)(y_limit/2 + 3));
	SetDrawingColorx(BLACK);
	draw_string(display);
}



// ?
void set_DW_title(const char *title) {
	
	XStoreName(DWDisp, DrawingWindow, title);
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
	SetDrawingColorx(BLACK);
	RectangleFrame((x_pos - 120), (y_pos + 16), (x_pos + 120), (y_pos + 30));
	SetDrawingColorx(8);
	RectangleDraw((x_pos - 119), (y_pos + 17), (x_pos + 119), (y_pos + 29));
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
	SetDrawingColorx(14);
	RectangleDraw((x_pos - 119), (y_pos + 17), (int)(x_pos - 119 + 2.38*complete), (y_pos + 29));
	
	if (mode == 0 && complete == 100) {
		SetDrawingColorx(WHITE);
		RectangleDraw(4, 4, 244, 18);
	}
}




int update_timer(int init_time, int last_time) {

	int current_time, hr(0), minute(0), sec(0);
	char time[32];
	
	current_time = get_time();
	
	if (last_time == current_time)
		return current_time;
		
	if (init_time == 0 || last_time == 0) {
		draw_status_bar();
		return current_time;
	}
	
	/*struct tm *ptm;
	ptm = localtime(&current_time);
	hr = ptm->tm_hour;
	minute = ptm->tm_min;
	sec = ptm->tm_sec;*/
	
	hr =  (current_time - init_time)/3600;
	minute = ((current_time - init_time)/60)%60;
	sec = (current_time - init_time)%60;
	
	if (minute < 10) {
		if (sec < 10)
			sprintf(time, "x%i:0%i:0%i", hr, minute, sec);
		else
			sprintf(time, "x%i:0%i:%i", hr, minute, sec);
	}
	else {
		if (sec < 10)
			sprintf(time, "x%i:%i:%i", hr, minute, sec);
		else
			sprintf(time, "x%i:%i:%i", hr, minute, sec);
	}	
	draw_status_bar();
	draw_status(time);
	
	return current_time;
}




void show_timer() {
	
	for (int time = 0, time0 = update_timer(0, 0); !interrupt(); time = update_timer(time0, time)) {
		process_event();
	}
}



// *
void Window_setup() {
	
	int y_size = y_pixel - y_limit - 2*window_nonclient_height;
	
	//ConsoleWindow = ; // ???
	
	SetDrawingColorx(BLACK);
	init();
	DW_exists = 1;
	
	SetDrawingColorx(12);
	RectangleFrame(0, 0, x_limit, y_limit);
	
	XMoveResizeWindow(DWDisp, DrawingWindow, 0, 0, y_size*Console_X/Console_Y, y_size);
	
	//XConfigureWindow(DWDisp, DrawingWindow, (CWX | CWY | CWWidth | CWHeight | CWStackMode), &values);
	//MoveWindow(ConsoleWindow, 0, 0, y_size*Console_X/Console_Y, y_size, 1); // Move to corner of screen
	set_window_for_drawing(DWDisp, DrawingWindow, DWContext); 
	SelectDrawingWindow();
}



// ?
void set_window_for_drawing(Display *TheDisplay, Window TheWindow, GC TheContext) {

	// get context
	DWContext = XCreateGC(TheDisplay, TheWindow, 0, NULL);
	XSetBackground(TheDisplay, TheContext, WhitePixel(TheDisplay, DefaultScreen(TheDisplay)));
	XSetForeground(TheDisplay, TheContext, BlackPixel(TheDisplay, DefaultScreen(TheDisplay)));
	
	XSetLineAttributes(TheDisplay, TheContext, 1, LineSolid, CapButt, JoinMiter);
}



// ?
void end_window_drawing(Window TheWindow, GC TheContext) {
	
	XFreeGC(TheWindow, TheContext);
}



// ?
void close_drawing_window() {
	
	if (!DW_exists)
		return;
	
	end_window_drawing(DrawingWindow, DWContext);
	
	XUnmapWindow(MenubarDisp, DrawingWindow);
	XDestroyWindow(DWDisp, DrawingWindow);
	XCloseDisplay(DWDisp);	
	DW_exists = 0;
}




void wait(double time) {
		
	for (int x = 0; x < DELAY2*time*clock_speed; ++x)
		process_event();
}



// *
void process_event() { 

	XEvent event;
		
	XNextEvent(DWDisp, &event); // ????????????
	
	//event = GetEvent(PointerMotionMask | ButtonReleaseMask);
	
	//x = XEventsQueued(DWDisp, QueuedAlready);
}




// ?
int get_time() {
	
	time_t currTime;	
	time(&currTime);	
	return currTime;
}



// *
int read_specs() {

	ifstream infile;
	string title, type;
	int windows(0), key;
	
	#ifndef TEXT_ONLY
	SetCursor(AppStCursor);
	#endif
		
	if (!infile_file(infile, "C.S.MachineSpecs")) {
		cout << "\nSince the file <C.S. Machine Specs> was not found, clock speed must be calculated.";
		
		clock_speed = clock_microprocessor();
	}
	else {
		infile >> title >> type >> title >> clock_speed >> title >> x_pixel >> title >> sound >> title >> windows >> title >> storeDATAA >> title >> storeDATAB >> title >> key;
		
		//x_pixel = GetDeviceCaps(GetDC((HWND)NULL), HORZRES);
		//y_pixel = GetDeviceCaps(GetDC((HWND)NULL), VERTRES);
		
		/*if (!XGetGeometry(DWDisp, RootWindow(DWDisp, DefaultScreen(DWDisp)), &root, &x, &y, &Root_Width, &Root_Height, &border_width, &Root_Depth)) {
			beep();
			cout << "/nError: Cannot retrieve display information." << endl;
		}*/
		 
		x_limit = x_pixel - window_nonclient_width;
		y_limit = (int)(x_limit/window_ratio);
	
		if (type != "UNIX")
			cout << "Warning: Computer type is not UNIX. Please check C.S. Machine Specs." << endl;
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
	double i,x;
		
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



// ?
char *xin(int size) {

	int i;
	
	SelectConsoleWindow();
	
	if (size > MAX_ENTERED_CHARACTERS)
		size = MAX_ENTERED_CHARACTERS;
		
	for (i = 0; i < (size - 1); ++i) {
		//cin.get(input[i]);
		input[i] = read_char();
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
			//cin.get(input[size]);
			input[size] = read_char();
			
		} while (input[size] != '\n');
	
	input[size] = 0;
	
	cout << endl;
			
	return input;
}



// ?
char read_char() { // used in xin function
	
	char character;
	XEvent event;
	
	if(XLookupString(&event.xkey, *character, 1, NULL, NULL)) // ????????
		return character;

	is_char = 0;
	
	//must_exit = 1;
	return 0;
}




// ?
void beep() {
	
	/*if (sound) {
		XBell(DWDisp, 100); // Why does it need the display?
		//XFlush(DWDisp);
	}*/
	if (sound) {
		printf("%c", (char)7);
	}
}
