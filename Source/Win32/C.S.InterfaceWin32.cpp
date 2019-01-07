#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.InterfaceWin32.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S.Interface (Win32);
// By Frank Gennari
#define USE_MENU_COORD_OFFSET // remove when not using menu resources

int const ERROR_QUIT_SECONDS = 6;
int const MIN_TEXT_SIZE = 5;

int has_sound_card(1), clock_speed, x_pixel(x_window), y_pixel((int)(2.5*y_window)), x_limit(x_window), y_limit(y_window), Frame(0), progress(0), nButtons(1), DW_exists(0), showState = SW_SHOWNORMAL, is_char(0), DEMO(1), enablePAINT(0), time_ms(0), window_mode(0);
char typed_char(0);
HINSTANCE thisInstance = NULL;
HDC DWContext;
HICON ClockCursor, ArrowCursor, AppStCursor;
HFONT DWFont = NULL;
HACCEL AccelTable, ATGraph, ATSchematic;
HWND TempWindow;
WNDCLASS SchematicWindow, GraphingWindow;
RECT DW_coords;

int use_system_tick_for_delay = 1;

const char s_class_name[32]  = "Win Circuit Solver Schematic";
const char g_class_name[32]  = "Win Circuit Solver Graphing";
const char s_window_name[32] = "Circuit Solver Schematic";
const char g_window_name[32] = "Circuit Solver Graphing";


extern int sound, has_menubar, storeDATAA, storeDATAB, grid_color, object_size, found_click, click_type, text_size, default_colors, must_exit, close_DW, ar_changed, graph_enable;
extern char input[];
extern double DW_ratio;
extern Point pt;
extern HWND DrawingWindow, MenuWindow, ConsoleWindow;
extern HBRUSH currBrush, coloredBrushes[];
extern HPEN currPen, coloredPens[];

LONG CALLBACK DoMessage (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam);
LONG CALLBACK DoMenu    (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam);

void quit_app();
void exit_error();
int decode_menu_click(int id);
void input_equation(char *equation);
void set_text_size(int size);
void switch_to_graphing_window();
void switch_to_schematic_window(); 
void clear_buffer();
int decision_dialog(int id);
BOOL APIENTRY AboutDlgProc(HWND hDlg, WORD message,WORD wParam, LONG lParam);
void message_window(int id);
void init_graphics();
void init_graphics_window(int is_graphing);
int got_click();
void paint_menu_buttons();
void find_console_window();
void set_window_for_drawing(HWND Window, HDC& TheContext);
void end_window_drawing(HWND Window, HDC TheContext);
void close_drawing_window();
void process_message();
char read_char();


void draw_double(double num, char label, int box_num);

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



// Add to
LONG CALLBACK DoMessage (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam) { // Get and process one message.

	LONG result = 0; // Function result
	is_char = 0;
	RECT wsize;
	
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
			break;
			
		/*case WM_LBUTTONUP:
		case WM_RBUTTONUP:
			ClipCursor (NULL);
			break;*/

		case WM_RBUTTONDOWN:
			found_click = 1;
			click_type = 11;
			pt.h = LOWORD(lParam);
			pt.v = HIWORD(lParam);
			break;
			
		case WM_KEYDOWN:
			click_type = 0;
			is_char = 1;
			typed_char = (char)wParam;
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
		//case WM_MOUSEMOVE: // ignore?
		//case WM_TIMER: // ?
		//case WM_VSCROLL: // must add scrolling
        //case WM_HSCROLL: // must add scrolling
        
        case WM_SIZE:
        	if (GetWindowRect(DrawingWindow, &wsize)) {
        		x_limit = wsize.right - wsize.left - window_nonclient_width;
        		y_limit = wsize.bottom - wsize.top - window_nonclient_height;
#ifdef USE_MENU_COORD_OFFSET
				y_limit -= menuwidth;
#endif
        		DW_ratio = ((double)x_limit/(double)y_limit)/window_ratio;
        		ar_changed = 1;
        	}
		
		case WM_PAINT:
			if (enablePAINT) {
				found_click = 1;
				click_type = -2;
			}
		default:
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam); // Pass message to Windows for default processing
			break;
	} 
	return result;
} 




LONG CALLBACK DoMenu (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam) { // Get and process one message.

	LONG result = 0; // Function result
	
	switch (msgCode) { // Dispatch on message code

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
			
		case WM_PAINT:
			found_click = 1;
			click_type = -3;
						
		default:
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam); // Pass message to Windows for default processing
			break;
	} 
	return result;
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

	LOGFONT FontStruct;
	HFONT last_font;
	int is_cs_created(0);
	static int last_size(0);
	
	if (DWFont && size == last_size && size != 0)
		return;

	if (text_size == 0)
		text_size = 12; // default - initialization

	size = max(size, MIN_TEXT_SIZE); // 5 point text is about the smallest you can see

	if (DWFont)
		is_cs_created = 1;
		
	FontStruct.lfUnderline      = FALSE;
    FontStruct.lfStrikeOut      = FALSE;
    FontStruct.lfItalic         = FALSE;
    FontStruct.lfEscapement     = FALSE;
    FontStruct.lfOrientation    = FALSE;
    FontStruct.lfOutPrecision   = OUT_DEFAULT_PRECIS;
    FontStruct.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    FontStruct.lfCharSet        = ANSI_CHARSET;
    FontStruct.lfQuality        = DRAFT_QUALITY;
    FontStruct.lfWeight         = FW_NORMAL;
    FontStruct.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;
    
    FontStruct.lfHeight = (4*size)/3;
    FontStruct.lfWidth  = (3*size)/5;
    
    DWFont = CreateFontIndirect(&FontStruct);
    if (DWFont == NULL) {
    	cerr << "Error creating font." << endl;
    	exit_error();
    }
    last_font = (HFONT)SelectObject(DWContext, DWFont);
	if (is_cs_created)
		DeleteObject(last_font);
	last_size = size;
}




void draw_string2(const unsigned char *display) {

	unsigned int oldX, oldY;
	tagPOINT oldPoint;
	
	MoveToEx(DWContext, 0, 0, &oldPoint); // Move pen and save old position
	oldX = oldPoint.x; // Extract previous coordinates
	oldY = oldPoint.y; // from returned point
	MoveToEx(DWContext, oldX, oldY, NULL); // Restore previous position
	
	oldY -= text_size;
	
	if (oldX < 0 || oldY < 0 || oldX > (unsigned int)x_limit || oldY > (unsigned int)y_limit) {
		return; // out of window
	}
	//SetBkColor(DWContext, RGB(255, 255, 255)); // White Background 
	
	SetBkMode(DWContext, TRANSPARENT);
	TextOut(DWContext, oldX, oldY, (const char *)(display + 1), display[0]);
	SetBkMode(DWContext, OPAQUE);
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
	
	int msgResult(0);	
	UINT msgStyle; // dialog box params 
	
	switch (id) {
		case 1:
			msgStyle  = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNOCANCEL | MB_SETFOREGROUND;
			msgResult = MessageBox (ConsoleWindow, "Do you want to save the current circuit?", "Save Circuit?", msgStyle);// Display message box
			break;
			
		case 2:
			msgStyle  = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNOCANCEL | MB_SETFOREGROUND;
			msgResult = MessageBox (ConsoleWindow, "Do you want to save the current project?", "Save Project?", msgStyle);// Display message box
			break;

		case 5:
			msgStyle  = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND;
			msgResult = MessageBox (ConsoleWindow, "Do you want to redraw the screen?", "Redraw?", msgStyle);// Display message box
			break;
			
		case 4: // This should be Cancel/Continue
			msgStyle  = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND;
			#ifdef USE_VM
				return 1;
			#else
				msgResult = MessageBox (ConsoleWindow, "Warning: This circuit may have too many nets for Circuit Solver to evaluate without running out of memory. Continue?", "Memory Warning", msgStyle);// Display message box
			#endif
			break;
	}
	if (msgResult == 6)
		return 1;
	else if (msgResult == 7)
		return 0;

	return msgResult;
}




BOOL APIENTRY AboutDlgProc(HWND hDlg, WORD message, WORD wParam, LONG lParam){
		
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
}




void intro() {

	message_window(3);
}




void demo_message() {

	message_window(4);
}




void message_window(int id) {

	FARPROC lpprocAbout = MakeProcInstance((FARPROC)AboutDlgProc, thisInstance);
	DialogBox(thisInstance, MAKEINTRESOURCE(id), DrawingWindow, (DLGPROC)lpprocAbout);
}



// *
int prefs() {

	return 0;
}




void init_graphics() {

	HICON progIcon = LoadIcon(NULL, IDI_APPLICATION);  // Program's screen icon
	HCURSOR arrowCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
	HBRUSH bkBrush = HBRUSH(COLOR_WINDOW + 1);         // Brush for painting window background
	
	if (thisInstance == NULL)
		thisInstance = GetModuleHandle(0);
	
	ATSchematic = LoadAccelerators(thisInstance, MAKEINTRESOURCE(4));
	ATGraph     = LoadAccelerators(thisInstance, MAKEINTRESOURCE(5)); // Load accelerator table
	
	SchematicWindow.lpszMenuName = MAKEINTRESOURCE(1);
	SchematicWindow.lpszClassName = "Win Circuit Solver Schematic";
	SchematicWindow.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	SchematicWindow.lpfnWndProc = DoMessage;
	SchematicWindow.cbClsExtra = 0;
	SchematicWindow.cbWndExtra = 0;
	SchematicWindow.hInstance = thisInstance;
	SchematicWindow.hIcon = progIcon;
	SchematicWindow.hCursor = arrowCursor;
	SchematicWindow.hbrBackground = bkBrush;
	
	GraphingWindow.lpszMenuName = MAKEINTRESOURCE(3);
	GraphingWindow.lpszClassName = "Win Circuit Solver Graphing";	
	GraphingWindow.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	GraphingWindow.lpfnWndProc = DoMessage;
	GraphingWindow.cbClsExtra = 0;
	GraphingWindow.cbWndExtra = 0;
	GraphingWindow.hInstance = thisInstance;
	GraphingWindow.hIcon = progIcon;
	GraphingWindow.hCursor = arrowCursor;
	GraphingWindow.hbrBackground = bkBrush;
  
    RegisterClass(&GraphingWindow);
    RegisterClass(&SchematicWindow);
       	 
    DW_coords.left   = 0;
    DW_coords.right  = x_limit + window_nonclient_width;
    DW_coords.top    = y_pixel - y_limit - window_nonclient_width - 45;
    DW_coords.bottom = y_pixel - window_nonclient_width - 45 + window_nonclient_height;

#ifdef USE_MENU_COORD_OFFSET
	DW_coords.top -= menuwidth;
#endif
}




void init_graphics_window(int is_graphing)  { // create graphics window
    
    const char *class_name, *window_name;
    
    if(is_graphing) {
		class_name  = g_class_name;
		window_name = g_window_name;
		AccelTable  = ATGraph;
    }
    else {
    	class_name  = s_class_name;
		window_name = s_window_name;
		AccelTable  = ATSchematic;
    }
	DrawingWindow = CreateWindow (class_name,
		window_name,     // Set window title
		WS_OVERLAPPEDWINDOW, // Use standard window style
		DW_coords.left, // Initial x
		DW_coords.top, // and y position
		DW_coords.right - DW_coords.left, // Initial width
		DW_coords.bottom - DW_coords.top, // and height
		NULL,                // No parent window
		NULL,                // Use menu from window class
		thisInstance,        // Use current program instance
		NULL);
		
    if (DrawingWindow == NULL) {
    	cerr << "Error creating drawing window." << endl;
    	exit_error();
    }        
	ShowWindow(DrawingWindow, showState);
	UpdateWindow(DrawingWindow);
	
	set_text_size(12);
}




void draw_menubar() {  
 	
    WNDCLASS menuWindow;	
	HICON progIcon = LoadIcon(NULL, IDI_APPLICATION);  // Program's screen icon
	HCURSOR arrowCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
	HBRUSH bkBrush = HBRUSH(COLOR_WINDOW + 1);         // Brush for painting window background

	if (thisInstance == NULL) {
		thisInstance = GetModuleHandle(0);
	}
	find_console_window();

	menuWindow.style = CS_HREDRAW | CS_VREDRAW | CS_NOCLOSE;
	menuWindow.lpfnWndProc = DoMenu;
	menuWindow.cbClsExtra = 0;
	menuWindow.cbWndExtra = 0;
	menuWindow.hInstance = thisInstance;
	menuWindow.hIcon = progIcon;
	menuWindow.hCursor = arrowCursor;
	menuWindow.hbrBackground = bkBrush;
	menuWindow.lpszMenuName = NULL;
	menuWindow.lpszClassName = "Win Circuit Solver Menu";
  
    RegisterClass (&menuWindow);
    
	MenuWindow = CreateWindow ("Win Circuit Solver Menu",
		"Menu",     // Set window title
		WS_OVERLAPPEDWINDOW, // Use standard window style
		x_pixel - MENU_WIDTH - MENU_X_OFFSET, // Initial x
		0,                   // and y position
		MENU_WIDTH + window_nonclient_width, // Initial width
		MENU_HEIGHT + window_nonclient_height + 6, // and height
		NULL,                // No parent window
		NULL,                // Use menu from window class
		thisInstance,        // Use current program instance
		NULL);               // No special creation parameters
    
    if (MenuWindow == NULL) {
    	cerr << "Error creating menu window." << endl;
    	exit_error();
    }
	ShowWindow(MenuWindow, showState);
	UpdateWindow(MenuWindow);
	BringWindowToTop(MenuWindow);
	SetFocus(MenuWindow);
 	
 	has_menubar = 1;
 	
	paint_menu_buttons();
 	
 	//if ( canUndo )
		//EnableMenuItem (theMenu, Undo_Item, MF_ENABLED); // Enable Undo command
	//else
		//EnableMenuItem (theMenu, Undo_Item, MF_GRAYED); // Gray out Undo command
}



void delete_menubar() {

	DestroyWindow(MenuWindow);	
	has_menubar = 0;
}




void paint_menu_buttons() {

	int button_width = MENU_HEIGHT/num_menu_boxes;
	char b_name[16] = {0};
	tagRECT r;
	HDC MenuContext;

	set_window_for_drawing(MenuWindow, MenuContext);	
	
	SetDrawingColorx(LTGREY);
	SetRect(&r, 0, 0, MENU_WIDTH + MENU_X_OFFSET, MENU_HEIGHT + 6);
	FillRect(MenuContext, &r, currBrush);

	SelectObject(MenuContext, GetStockObject(BLACK_PEN));
	SetBkMode(MenuContext, TRANSPARENT);
	
	string_to_char(b_name, button_names[0], bn_lengths[0]);
	TextOut(MenuContext, 12, (int)(0.17*button_width), b_name, bn_lengths[0]);
	
	for (int i = 1; i <= num_menu_boxes; ++i) {
		MoveToEx(MenuContext, 0, i*button_width, NULL);
		LineTo(MenuContext, MENU_WIDTH + MENU_X_OFFSET, i*button_width);
		string_to_char(b_name, button_names[i], bn_lengths[i]);
		TextOut(MenuContext, 12, (int)((i+0.17)*button_width), b_name, bn_lengths[i]);
	}
	end_window_drawing(MenuWindow, MenuContext);
}




void switch_to_graphing_window() {

	if (window_mode == 1)
		return;
		
	window_mode = 1;

	if (!graph_enable) {
		Window_setup(1);
		graph_enable = 1;
		return;
	}
	GetWindowRect(DrawingWindow, &DW_coords);
	close_drawing_window();
	delete_menubar();
	Window_setup(1);

	if(DrawingWindow == NULL) {
		cerr << "Error creating Drawing Window." << endl;
		exit_error();
	}
	TempWindow   = DrawingWindow;
	graph_enable = 1;
}





void switch_to_schematic_window() {

	if (window_mode == 0)
		return;
		
	window_mode = 0;

	if (!graph_enable)
		return;

	close_drawing_window();
	DrawingWindow = TempWindow;
}




int memory_error() {
	
	int msgResult;
	
	#ifdef USE_VM
		cout << "This may take awhile." << endl;
		return 0;
	#else
		UINT msgStyle = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND; // dialog box params
		msgResult = MessageBox (ConsoleWindow, "Warning: Circuit Solver may not have enough memory to simulate a circuit with this many nets. Continue?", "Memory Warning?", msgStyle); // Display message box
	#endif
			
	if (msgResult == 6)
		return 0;
	else if (msgResult == 7)
		return 1;

	return msgResult;
}



void memory_message() {
	
	MessageBox(HWND_DESKTOP, "Circuit Solver has run out of memory and will now quit.", "Memory Error", MB_ICONEXCLAMATION | MB_OK);
}




void reset_cursor() {

	SetCursor(ArrowCursor);
}




void show_clock() {

	SetCursor(ClockCursor);
}



// Add to?
void get_cursors() {
	
	ClockCursor = LoadCursor (NULL, IDC_WAIT);
	ArrowCursor = LoadCursor (NULL, IDC_ARROW);
	AppStCursor = LoadCursor (NULL, IDC_APPSTARTING);
}



// *
void set_special_cursor(int type) {

	return;
	
	HICON Cursor = ArrowCursor;
	
	//curBlank = LoadCursor(hInstance, MAKEINTRESOURCE(1));
	
	if (type < 0 || type > BDELETE_CURSOR)
		return;
		
	if (type <= PROBEAC_CURSOR)
		;//Cursor = GetCursor(128 + type);
	else
		;//Cursor = GetCursor(1000 + type);
	
	SetCursor(Cursor);
}



// *
void set_cursor(int ID, int is_rotated) {

	return;
	
	int ID2, order, offset(/*base_s_cursor_id*/0);	
	HICON ComponentCursor = ArrowCursor;
	
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

	return;
	
	HICON ACCursor = ArrowCursor;
	
	//ACCursor = GetCursor(AC_cursor_frame0_id + Frame);
	SetCursor(ACCursor);
	
	if (Frame == 3)
		Frame = 0;
	else
		++Frame;
}



// *
void get_DC_waveform_cursor() {

	return;

	HICON DCCursor = ArrowCursor;
	
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

	pt = GetClick();
		
	if (click_type == 1) { // click on menubar
		if (pt.h > MENU_WIDTH + MENU_X_OFFSET)
			return -1;
		return (int)(min(14, (pt.v*(num_menu_boxes + 1)/MENU_HEIGHT)));
	}
	if (click_type >= 100) // menu selection (handled like a click) 
		return decode_menu_click(click_type - 100);
		
	if (is_char)
		return 0;

	return which_box(pt);
}




void whiteout() {
	
	SetDrawingColorx(WHITE);
	RectangleDraw(0, 0, (int)(1.5*x_limit + 2), (int)(1.5*y_limit + 2));
	
	SetDrawingColorx(LTBLUE);
	RectangleFrame(0, 0, x_limit, y_limit);
	
	//InvalidateRect(DrawingWindow, NULL, TRUE); // Force repaint of Graphics Window
}





Point GetDWClick() {
	
	do {
		pt = GetClick();
	} while ((click_type != 0 && click_type != 11 && click_type != -2 && click_type != -3) || !is_in_window(pt)); // not in graphics window
	
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




void set_DW_title(const char *title) {

	SetWindowText(DrawingWindow, title);
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
	SetDrawingColorx(AQUA);
	RectangleDraw((x_pos - 119), (y_pos + 17), (x_pos + 120), (y_pos + 30));
	
	if (mode == 1)
		draw_cancel_button();
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
	SetDrawingColorx(LTGREY);
	RectangleDraw((x_pos - 119), (y_pos + 17), (int)(x_pos - 118 + 2.38*complete), (y_pos + 30));
	
	if (mode == 0 && complete == 100) {
		SetDrawingColorx(WHITE);
		RectangleDraw(4, 4, 245, 19);
	}
	if (mode == 1)
		draw_cancel_button();
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
	
	/*struct tm *ptm;
	ptm = localtime(&current_time);
	hr = ptm->tm_hour;
	minute = ptm->tm_min;
	sec = ptm->tm_sec;*/
	
	etime = current_time - init_time;
	format_time(etime, time);
		
	draw_status_bar();
	draw_status(time);
	
	return current_time;
}




void show_timer() {
	
	for (int time = 0, time0 = update_timer(0, 0); !interrupt(); time = update_timer(time0, time)) {
		process_message();
	}
}




void Window_setup(int is_graphing) {
	
	int y_size = y_pixel - y_limit - 2*window_nonclient_height - menuwidth;

	find_console_window();
	MoveWindow(ConsoleWindow, 0, 0, y_size*Console_X/Console_Y, y_size, 1); // Move to corner of screen
	
	init_graphics_window(is_graphing);
	DW_exists = 1;

	set_window_for_drawing(DrawingWindow, DWContext); 
	SelectDrawingWindow();
	whiteout();

	SetDrawingColorx(LTBLUE);
	RectangleFrame(0, 0, x_limit, y_limit);
	SetDrawingColorx(BLACK);
}




void find_console_window() {

	char orig_title[MAX_CHARACTERS], test_title[MAX_CHARACTERS];

	if (ConsoleWindow == NULL) {
		// hack suggested by Microsoft
		GetConsoleTitle(orig_title, MAX_CHARACTERS);
		SetConsoleTitle("Microsoft Sucks");
		Sleep(40);
		ConsoleWindow = FindWindow(NULL, "Microsoft Sucks");
		SetConsoleTitle(orig_title);

		if(ConsoleWindow == NULL) {
			cerr << "Error: NULL Console Window." << endl;
			exit_error();
		}
		Sleep(40);
		GetWindowText(ConsoleWindow, test_title, MAX_CHARACTERS);
		
		if (strcmp(orig_title, test_title) != 0) {
			cerr << "Error: Failed to establish correct console window handle." << endl;
			cout << "target = " << orig_title << ", result = " << test_title << endl;
			exit_error();
		}
	}
}




void set_window_for_drawing(HWND Window, HDC& TheContext) {

	//PAINTSTRUCT paintInfo; // Info structure for painting
	//TheContext = BeginPaint (Window, &paintInfo); // Open device context
	
	TheContext = GetDC(Window); // Get device context for client area
	
	if (TheContext == NULL) {
		cerr << "Error creating graphics context." << endl;
		exit_error();
	}
}




void end_window_drawing(HWND Window, HDC TheContext) {

	//EndPaint (Window, &paintInfo); // Close device context
	
	ReleaseDC(Window, TheContext); // Release device context
}




void close_drawing_window() {
	
	if (!DW_exists)
		return;
		
	end_window_drawing(DrawingWindow, DWContext);
	DestroyWindow (DrawingWindow); // Destroy the window
	
	for (int i = 0; i < NUM_COLORS; ++i) {
		if (coloredBrushes[i] != NULL) {
			DeleteObject(coloredBrushes[i]);
			coloredBrushes[i] = NULL;
		}
		if (coloredPens[i] != NULL) {
			DeleteObject(coloredPens[i]);
			coloredPens[i] = NULL;
		}
	}
	if (DWFont)
		DeleteObject(DWFont);
		
	//DestroyWindow (DrawingWindow); // Destroy the window
	DW_exists = 0;
	graph_enable = 0;
}




void wait(double time) {

#ifdef USE_SLEEP
	Sleep(DWORD(time*1000));
	return;
#endif

	unsigned int x;
	
	if (use_system_tick_for_delay) {
		for (x = GetTickCount(); (GetTickCount() - x) < time*1000; process_message()) {};
	}
	else	
		for (x = 0; x < DELAY2*time*clock_speed; ++x)
			process_message();
}




void process_message() { // for wait, but does not execute in constant time

	MSG theMessage;

	PeekMessage(&theMessage, NULL, 0, 0, 1); // Get next message
	TranslateMessage (&theMessage); // Convert virtual keys to characters
	DispatchMessage (&theMessage); // Send message to window procedure
}





int get_time() {
	
	if (time_ms)
		return GetTickCount();
		
	time_t currTime;	
	time(&currTime);
		
	return (int)currTime;
}




int read_specs() {

	ifstream infile;
	string title, type;
	int windows(0), key(0);
	
	#ifndef TEXT_ONLY
	SetCursor(AppStCursor);
	#endif
		
	if (!infile_file(infile, cs_machine_specs)) {
		beep();
		cout << "\nSince the file <" << cs_machine_specs << "> was not found, clock speed will be calculated.";
		clock_speed = clock_microprocessor();
	}
	else {
		if (infile.good())
			infile >> title >> type >> title >> clock_speed >> title >> x_pixel >> title >> sound >> title >> has_sound_card >> title >> windows >> title >> storeDATAA >> title >> storeDATAB >> title >> key;
	
		if (type != "PC")
			cout << "Warning: Computer type is not PC. Please check " << cs_machine_specs << "." << endl;
	}
	infile.close();
	
	#ifndef TEXT_ONLY
	x_pixel = GetDeviceCaps(GetDC((HWND)NULL), HORZRES);
	y_pixel = GetDeviceCaps(GetDC((HWND)NULL), VERTRES);
	#endif
	
	x_limit = x_pixel - window_nonclient_width;
	y_limit = (int)(x_limit/window_ratio);
	
	cout << "\nDisplay resolution is " << x_pixel << "x" << y_pixel;
	
	if (key == key_val)
		DEMO = 0;
	else
		windows = default_demo_windows;
		
	if (clock_speed < 4) {
		cout << "\nClock speed read from " << cs_machine_specs << " is only " << clock_speed << "MHz. Recalculating to verify..." << endl;
		clock_speed = clock_microprocessor();
	}
	object_size = (int)(sqrt(float(x_limit*y_limit))/pi);
	
	#ifndef TEXT_ONLY
	nButtons = GetSystemMetrics(SM_CMOUSEBUTTONS);
	
	if (nButtons == 0)
		cout << "\nNo mouse detected." << endl;
	else
		cout << "\n" << nButtons << " button mouse detected." << endl;
	#endif
		
	cout << "Available Memory: At least " << GetFreeSpace(0) << " bytes" << endl;	
	
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
	
	for (i = 0; get_time() < (time1 + 1); ++i){}; // too unreliable in Windows
	
	speed = (int)(i/time_constant);
	
	reset_cursor();
	cout << "\nEquivalent Clock Speed = " << speed << " MHz." << endl;
	cout << "Warning: The Win32 microprocessor clocker is not accurate." << endl;
	
	return speed;
}



// *
char *xin(int size) {

	int i, counter(0);
	
	SelectConsoleWindow();
	
	if (size > MAX_ENTERED_CHARACTERS)
		size = MAX_ENTERED_CHARACTERS;
		
	for (i = 0; i < (size - 1); ++i) {
		cin.get(input[i]);
		//input[i] = read_char();
		if(input[i] == '\n') {
			if (i == 0)
				--i;
			else {
				input[i] = 0;
				break;
			}
		}
	}
	if (i == size - 1) {
		do {
			cin.get(input[size]);
			//input[size] = read_char();
			
			++counter;
			if (counter > 10000) { // we have a problem
				cerr << "\nError entering input." << endl;
				cin.clear();
				break;
			}
		} while (input[size] != '\n');
	}
	input[size] = 0;
	cout << endl;
			
	return input;
}



// *
char read_char() { // used in xin function

	BOOL ContinueFlag = TRUE;
	MSG theMessage; // Next message to process
	
	is_char = 0;
		
	while(ContinueFlag) {
		ContinueFlag = GetMessage(&theMessage, NULL, 0, 0); // Get next message
		if (theMessage.message == WM_CHAR) {
			typed_char = (char)theMessage.wParam;
			is_char = 1;
		}
		TranslateMessage (&theMessage); // Convert virtual keys to characters	
		DispatchMessage (&theMessage); // Send message to window procedure
		if (is_char) {
			is_char = 0;
			return typed_char;
		}
	}
	must_exit = 1;
	return 0;
}





void beep() {
	
	if (sound) {
		if (has_sound_card) {
			MessageBeep(1);
		}
		else {
			Beep(550,50); // does this need Win32 specific code?
		}
	}
}
