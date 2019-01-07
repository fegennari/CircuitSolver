#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.simpl-dix.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// This file, C.S.simpl-dix.cpp is copyright © 2001 EECS department, UC Berkeley


// Circuit Solver Interface to simpl-dix
// By Frank Gennari 1/20/01
double const short_pause_length = 0.5, long_pause_length  = 2, cross_section_top_ratio = 0.7;
int const fill_scrollbars = 1, DEBUG = 0;
int const pattern_size = 20, pattern_spacing = 32, scrollbar_width = 24, button_width = 60;
int const border_width = 20, menubar_width = 27, window_thickness = 9, font_width = 8;
int const pattern_area_width = 120, layout_top = 2*border_width;
int const line_length = 80;
int const background_color = BLACK, text_color = ORANGE, outline_color = ORANGE, highlight_color = WHITE;

int const scm_len = 16;
char const sel_com_mes[scm_len+2] = " SELECT COMMAND";


int x_size(1024), y_size(824), click(0), max_patterns_on_screen(20);
double xscale, yscale;
point pt;


extern int DW_exists, graph_enable;
extern HINSTANCE thisInstance;
extern HDC DWContext;
extern HWND DrawingWindow;



struct pattern_info {

	int lpos, rpos, lpat, rpat;
	int *colors;
	char **names;
};


struct cif_object {
	
	int x1, x2, y1, y2, type, pattern, color;
};


struct cif_data {

	int num_objects;
	cif_object *object;
};


struct dix_data {

	cif_data layout, cross_section;
	pattern_info patterns;
};



void  call_simpl_dix();
point process_click(int num_buts, char **b_text, char *m_text, int m_len, dix_data &data, int &selection, int &button);
void  init_window(int num_buttons, char **button_text, dix_data &data);
void  paint_window(int num_buttons, char **button_text, char *window_text, int text_length, dix_data &data);
void  paint_layout(cif_data &layout);
void  paint_cross_section(cif_data &cross_section);
void  paint_cif(cif_data &cdata, int section);
void  paint_patterns(pattern_info &patterns);
void  paint_left_patterns(int *colors, char **names, int num_patterns, int lpos);
void  paint_right_patterns(int *colors, char **names, int num_patterns, int rpos);
void  paint_scrollbars(int lpat, int rpat, int lpos, int rpos);
void  paint_buttons(int num_buttons, char **button_text, int invert_button_id);
void  paint_text(char *window_text, int text_length, int flash);
void  draw_rect(int x1, int y1, int x2, int y2, bool fill, int section, int color);
void  draw_line(int x1, int y1, int x2, int y2, int section, int color);
point get_win_click();
int   get_button(point local, int num_buttons);
int   which_section(point global);
point get_section_offset(int selection);
point transform_pointG2L(point global, int selection);
point transform_pointL2G(point local, int selection);
void  do_scroll(int &pos, int pat, int localY);
int   get_YNC(char *question);
int   get_YN(char *question);
int   try_again();
void  compress_pi(pattern_info &patterns, int lpos, int rpos, int lpat, int rpat, int *colors, char **names);
void  compress_object(cif_object &obj, int x1, int x2, int y1, int y2, int type, int pattern, int color);
cif_object convert_to_cif_box(cif_object &obj);
cif_object convert_from_cif_box(cif_object &obj);
void  scale_obj(cif_object &obj);
void  Xpause(int length);
int   do_exit();
LONG CALLBACK DoSimplDix (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam);

int  call_edit_coms(dix_data &data);
int  call_ana_tools(dix_data &data);
int  do_pisces(dix_data &data);
int  run_simpl2(/*dix_data &data*/);


void set_window_for_drawing(HWND Window, HDC& TheContext);
void close_drawing_window();

void draw_string(char *display);






void call_simpl_dix() { // ***

	int selection, button(1), lpos(0), rpos(0);
	point local, global;
	pattern_info patterns;
	cif_data layout, cross_section;
	dix_data data;
	
	int const num_buttons(5), Lpatterns(14), Rpatterns(17); // ###
	char *button_text[num_buttons] = {"RUN/ /SIMPL-2", " /PISCES", "CALL/ANALYSIS/TOOLS", "CALL/EDIT/COMMANDS", " /ABORT"};
	
	int colors[Lpatterns + Rpatterns] = {WHITE, BLACK, GREEN, YELLOW, ORANGE, BLUE,
										 RED, TAN, BLACK, ALMOND, LTBLUE, PURPLE, PINK, 
										 DKGREEN, /**/ BLACK, GREEN, GREEN, GREEN, GREEN,
										 GREEN, GREEN, GREEN, GREEN, RED, RED, RED, RED,
										 RED, RED, RED, RED}; // ###
	
	char *names[Lpatterns + Rpatterns] = {" AIR", " CONT", " ACTV", " NWEL", " PSD", " MTL",
										  " POLY", " OXID", " NTRD", " PSG", " METL", " RST",
										  " ERST", " SP1", /**/ " NP", " N12", " N13",
										  " N14", " N15", " N16", " N17", " N18", " N19",
										  " P12", " P13", " P14", " P15", " P16", " P17",
										  " P18", " P19"}; // ###
	
	global.x = global.y = 0;
	
	cout << "Enter window dimensions: ";
	x_size = in();
	y_size = in();
	
	max_patterns_on_screen = (y_size-menubar_width-button_width)/pattern_spacing;
	xscale = (x_size-window_thickness-2*(scrollbar_width+pattern_area_width))/737.0;
	yscale = (y_size-menubar_width-button_width)/747.0;
		
	layout.num_objects = 1; // ###
	cross_section.num_objects = 0; // ###	
	layout.object = new cif_object[layout.num_objects];
	cross_section.object = new cif_object[cross_section.num_objects];
	
	//compress_object(layout.object[0], 100, 300, 100, 250, 0, 0, RED); // ###
	//compress_object(layout.object[1], 200, 550, 150, 350, 0, 0, BLUE); // ###
	//compress_object(cross_section.object[0], 0, 720, 50, 50, 1, 0, GREEN); // ###
	
	compress_object(layout.object[0], 130, 600, 10, 380, 0, 0, DKGREEN);
	scale_obj(layout.object[0]);
	
	compress_pi(patterns, lpos, rpos, Lpatterns, Rpatterns, colors, names);
	
	data.patterns = patterns;
	data.layout = layout;
	data.cross_section = cross_section;
		
	if (DW_exists)
		close_drawing_window();	
	DW_exists = 1;

	cout << "\nRunning simpl-dix interface..." << endl;
	init_window(num_buttons, button_text, data);
	//Xpause(0);
	paint_text((char *)sel_com_mes, 14, 1);
	
	do {
		local = process_click(num_buttons, button_text, (char *)sel_com_mes, scm_len, data, selection, button);
		
		if (click == 2)
			continue;
		
		switch (selection) {
			case 0:
				switch (button) {
					case 0:
						button = do_exit();
						break;
					case 1:
						button = call_edit_coms(data);
						break;
					case 2:
						button = call_ana_tools(data);
						break;
					case 3:
						paint_buttons(num_buttons, button_text, 4);
						button = do_pisces(data);
						break;
					case 4:
						paint_buttons(num_buttons, button_text, 5);
						button = run_simpl2(/*data*/);
						break;
				}
				if (button != 0) {
					paint_buttons(num_buttons, button_text, 0);
					//Xpause(0);
					paint_text((char *)sel_com_mes, 14, 1);
				}
				break;
				
			case 1: // Layout
				
				break;
				
			case 2: // Cross Section
				
				break;
				
			case 3: // L scrollbar
				do_scroll(lpos, Lpatterns, local.y);
				patterns.lpos = lpos;
				break;
				
			case 4: // R scrollbar
				do_scroll(rpos, Rpatterns, local.y);
				patterns.rpos = rpos;
				break;
				
			case 5: // L patterns
				break;
				
			case 6: // R patterns
				break;
				
			case 7: // Other
				break;
		}	
	} while (button != 0);
	
	close_drawing_window();
	delete [] layout.object;
	delete [] cross_section.object;
	graph_enable = 0;
}




point process_click(int num_buts, char **b_text, char *m_text, int m_len, dix_data &data, int &selection, int &button) {

	point global, local;
	
	click = 0;
		
	global = get_win_click();
	
	if (click == 2) {
		paint_window(num_buts, b_text, m_text, m_len, data);
		return global;
	}
	if (click == 4) {
		selection = 0;
		button = 0;
		return global;
	}	
	selection = which_section(global);
	local = transform_pointG2L(global, selection);
	
	if (DEBUG)
		cout << "Selection = " << selection << ", local point = (" << local.x << ", " << local.y << ")" << endl;
	
	if (selection == 0) {
		button = get_button(global, num_buts);
		if (DEBUG)
			cout << ", button ID = " << button << endl;
	}
	return local;
}




void  init_window(int num_buttons, char **button_text, dix_data &data) {

	WNDCLASS DIX_Window;
  	
  	if (thisInstance == NULL)
		thisInstance = GetModuleHandle(0);
	
	HICON progIcon = LoadIcon(NULL, IDI_APPLICATION);  // Program's screen icon
	HCURSOR arrowCursor = LoadCursor(NULL, IDC_ARROW); // Default cursor
	HBRUSH bkBrush = HBRUSH(COLOR_WINDOW + 1);         // Brush for painting window background
	
	DIX_Window.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	DIX_Window.lpfnWndProc = DoSimplDix;
	DIX_Window.cbClsExtra = 0;
	DIX_Window.cbWndExtra = 0;
	DIX_Window.hInstance = thisInstance;
	DIX_Window.hIcon = progIcon;
	DIX_Window.hCursor = arrowCursor;
	DIX_Window.hbrBackground = bkBrush;
	DIX_Window.lpszMenuName = NULL;
	DIX_Window.lpszClassName = "Win SIMPL-DIX";
  
    RegisterClass (&DIX_Window);
    
	DrawingWindow = CreateWindow ("Win SIMPL-DIX",
		"Win32 SIMPL-DIX: UC-Berkeley 1/20/01"
		,     // Set window title
		WS_OVERLAPPEDWINDOW, // Use standard window style
		0,                   // Initial x
		0, // and y position
		x_size, // Initial width
		y_size, // and height
		NULL,                // No parent window
		NULL,                // Use menu from window class
		thisInstance,        // Use current program instance
		NULL);               // No special creation parameters
    
    if (DrawingWindow == NULL)
    	exit_error();
                
	ShowWindow(DrawingWindow, SW_SHOWNORMAL);
	UpdateWindow(DrawingWindow);
		
	set_window_for_drawing(DrawingWindow, DWContext); 
	SelectDrawingWindow();
	paint_window(num_buttons, button_text, " WELCOME TO SIMPL-DIX", 20, data);
}




void  paint_window(int num_buttons, char **button_text, char *window_text, int text_length, dix_data &data) {
	
	SetDrawingColorx(background_color);
	RectangleDraw(0, 0, x_size-window_thickness, y_size-menubar_width);
	
	paint_text(window_text, text_length, 0);
	paint_patterns(data.patterns);
	paint_scrollbars(data.patterns.lpat, data.patterns.rpat, data.patterns.lpos, data.patterns.rpos);
	paint_layout(data.layout);
	paint_cross_section(data.cross_section);
	paint_buttons(num_buttons, button_text, 0);
}




void  paint_layout(cif_data &layout) { 

	SetDrawingColorx(background_color);
	RectangleDraw(scrollbar_width+pattern_area_width, 3*border_width, x_size-window_thickness-scrollbar_width-pattern_area_width, cross_section_top_ratio*(y_size-menubar_width-button_width));
	paint_cif(layout, 1);
}




void  paint_cross_section(cif_data &cross_section) { 

	SetDrawingColorx(background_color);
	RectangleDraw(scrollbar_width+pattern_area_width, cross_section_top_ratio*(y_size-menubar_width-button_width), x_size-window_thickness-scrollbar_width-pattern_area_width, y_size-menubar_width-button_width);
	paint_cif(cross_section, 2);
}




void  paint_cif(cif_data &cdata, int section) { // ***

	cif_object obj;
	
	for (unsigned int i = 0; i < cdata.num_objects; ++i) {
		obj = cdata.object[i];
		
		switch (obj.type) {
			case 0: // box
				draw_rect(obj.x1, obj.y1, obj.x2, obj.y2, (obj.type == 0), section, obj.color);
				break;
			case 1: // wire
				draw_line(obj.x1, obj.y1, obj.x2, obj.y2, section, obj.color);
				break;
			case 2: // round flash
				
				break;
			case 3: // polygon
				
				break;
		}
	}
}




void  paint_patterns(pattern_info &patterns) {
	
	paint_left_patterns(patterns.colors, patterns.names, patterns.lpat, patterns.lpos);
	paint_right_patterns((int*)(patterns.colors + patterns.lpat), (char**)(patterns.names + patterns.lpat), patterns.rpat, patterns.rpos);
}




void  paint_left_patterns(int *colors, char **names, int num_patterns, int lpos) {

	SetDrawingColorx(background_color);
	RectangleDraw(scrollbar_width, 0, pattern_area_width+scrollbar_width, y_size-menubar_width-button_width);

	for (unsigned int i = lpos; i < min(num_patterns, lpos+max_patterns_on_screen); ++i) {
		SetDrawingColorx(colors[i]);
		RectangleDraw(1.5*scrollbar_width, (i+0.5)*pattern_spacing, 1.5*scrollbar_width+pattern_size, (i+0.5)*pattern_spacing+pattern_size);
		SetDrawingColorx(outline_color);
		RectangleFrame(1.5*scrollbar_width, (i+0.5)*pattern_spacing, 1.5*scrollbar_width+pattern_size, (i+0.5)*pattern_spacing+pattern_size);	
		RectangleDraw(1.5*scrollbar_width+1.3*pattern_size, (i+0.6)*pattern_spacing, pattern_area_width+font_width, (i+1.1)*pattern_spacing);
		SetDrawingColorx(background_color);
		MoveTo(1.5*scrollbar_width+1.4*pattern_size, (i+0.5)*pattern_spacing+0.75*pattern_size);
		draw_string(names[i]);
	}
}




void  paint_right_patterns(int *colors, char **names, int num_patterns, int rpos) {

	SetDrawingColorx(background_color);
	RectangleDraw(x_size-window_thickness-pattern_area_width-scrollbar_width, 0, x_size-window_thickness-scrollbar_width, y_size-menubar_width-button_width);
	
	for (unsigned int i = rpos; i < min(num_patterns, rpos+max_patterns_on_screen); ++i) {
		SetDrawingColorx(colors[i]);
		RectangleDraw(x_size-window_thickness-1.5*scrollbar_width-pattern_size, (i+0.5)*pattern_spacing, x_size-window_thickness-1.5*scrollbar_width, (i+0.5)*pattern_spacing+pattern_size);
		SetDrawingColorx(outline_color);
		RectangleFrame(x_size-window_thickness-1.5*scrollbar_width-pattern_size, (i+0.5)*pattern_spacing, x_size-window_thickness-1.5*scrollbar_width, (i+0.5)*pattern_spacing+pattern_size);	
		RectangleDraw(x_size-window_thickness-pattern_area_width-font_width, (i+0.6)*pattern_spacing, x_size-window_thickness-1.5*scrollbar_width-1.2*pattern_size, (i+1.1)*pattern_spacing);
		SetDrawingColorx(background_color);
		MoveTo(x_size-window_thickness-1.4*scrollbar_width-pattern_size-0.57*pattern_area_width, (i+0.5)*pattern_spacing+0.75*pattern_size);
		draw_string(names[i]);
	}
}




void  paint_scrollbars(int lpat, int rpat, int lpos, int rpos) { // ***

	int scrollbar_bot = y_size-button_width-menubar_width-5;
	
	SetDrawingColorx(background_color);
	RectangleDraw(0, 0, scrollbar_width, y_size-menubar_width-button_width);
	RectangleDraw(x_size-window_thickness-scrollbar_width, 0, x_size-window_thickness, y_size-menubar_width-button_width);

	SetDrawingColorx(outline_color);
	
	if (lpat <= max_patterns_on_screen)
		RectangleFrame(2, 2, scrollbar_width, scrollbar_bot);
	else // need to use max_patterns_on_screen
		RectangleFrame(2, 2+(lpos/lpat)*scrollbar_bot, scrollbar_width, ((lpat-lpos)/lpat)*scrollbar_bot);
	
	if (rpat <= max_patterns_on_screen)
		RectangleFrame(x_size-window_thickness-scrollbar_width, 2, x_size-window_thickness-2, scrollbar_bot);
	else 
		RectangleFrame(x_size-window_thickness-scrollbar_width, 2+(rpos/rpat)*scrollbar_bot, x_size-window_thickness-2, ((rpat-rpos)/rpat)*scrollbar_bot);
	
	if (fill_scrollbars) {
		RectangleDraw(2, 2, scrollbar_width, scrollbar_bot);
		RectangleDraw(x_size-window_thickness-scrollbar_width, 2, x_size-window_thickness-2, scrollbar_bot);
	}
}




void  paint_buttons(int num_buttons, char **button_text, int invert_button_id) { 

	int linenum, button_top(y_size-button_width-menubar_width), tcolor;
	int invert_id = num_buttons - invert_button_id + 1;
	unsigned int i, j, k;
	char line[line_length];
	
	line[0] = ' ';
	
	SetDrawingColorx(background_color);
	RectangleDraw(0, y_size-menubar_width-button_width, x_size-window_thickness, y_size-menubar_width);

	SetDrawingColorx(outline_color);
	RectangleFrame(4, button_top+1, x_size-window_thickness-4, y_size-menubar_width-4);
	
	SetDrawingColorx(highlight_color);
	RectangleFrame(1, button_top-1, x_size-window_thickness-1, y_size-menubar_width-1);
	RectangleFrame(2, button_top-2, x_size-window_thickness-2, y_size-menubar_width-2);

	for (i = 0; i < num_buttons; ++i) {
		if (i == invert_id - 1) {
			SetDrawingColorx(text_color);
			RectangleDraw(i*(x_size-window_thickness)/num_buttons+2+2*(i==0), button_top+1, (i+1)*(x_size-window_thickness)/num_buttons-2*(i==num_buttons-1), y_size-menubar_width-4);
			tcolor = background_color;
		}
		else
			tcolor = text_color;
				
		if (i != 0) {
			SetDrawingColorx(outline_color);
			LineDraw(i*(x_size-window_thickness)/num_buttons-1, button_top+1, i*(x_size-window_thickness)/num_buttons-1, y_size-menubar_width-4);
			LineDraw(i*(x_size-window_thickness)/num_buttons+2, button_top+1, i*(x_size-window_thickness)/num_buttons+2, y_size-menubar_width-4);
		}
		for (j = 0, k = 1, linenum = 1; k < line_length; ++j) {
			if (button_text[i][j] == '/' || button_text[i][j] == '\0') {
				line[k] = '\0';
				SetDrawingColorx(tcolor);
				MoveTo((i+0.5)*(x_size-window_thickness)/num_buttons-k*font_width/2, button_top+linenum*button_width/4+4);
				draw_string(line);
				if (button_text[i][j] == '\0')
					break;
				k = 1;
				++linenum;
			}
			else
				line[k++] = button_text[i][j];
		}
	}
}




void  paint_text(char *window_text, int text_length, int flash) { 

	SetDrawingColorx(background_color);
	RectangleDraw(0.25*x_size, 0, 0.75*x_size, 2*border_width);

	SetDrawingColorx(text_color);
	MoveTo((x_size-window_thickness-font_width*text_length)/2, border_width);
	draw_string(window_text);
	
	if (flash) {
		SetDrawingColorx(highlight_color);
		RectangleFrame(0.25*x_size+2, -1, 0.75*x_size-2, 2*border_width-2);
		RectangleFrame(0.25*x_size+1, -1, 0.75*x_size-1, 2*border_width-1);
		wait(0.5);
		SetDrawingColorx(background_color);
		RectangleFrame(0.25*x_size+2, -1, 0.75*x_size-2, 2*border_width-2);
		RectangleFrame(0.25*x_size+1, -1, 0.75*x_size-1, 2*border_width-1);
	}
}




void  draw_rect(int x1, int y1, int x2, int y2, bool fill, int section, int color) {

	point offset = get_section_offset(section);
	
	SetDrawingColorx(color);
	
	if (fill)
		RectangleDraw(x1 + offset.x, y1 + offset.y, x2 + offset.x, y2 + offset.y);
	else
		RectangleFrame(x1 + offset.x, y1 + offset.y, x2 + offset.x, y2 + offset.y);
}




void  draw_line(int x1, int y1, int x2, int y2, int section, int color) {

	point offset = get_section_offset(section);
	
	SetDrawingColorx(color);
	LineDraw(x1 + offset.x, y1 + offset.y, x2 + offset.x, y2 + offset.y);
}




point get_win_click() {

	BOOL ContinueFlag = TRUE;
	MSG theMessage; // Next message to process
		
	while(ContinueFlag) {
		ContinueFlag = GetMessage(&theMessage, NULL, 0, 0); // Get next message
		TranslateMessage (&theMessage); // Convert virtual keys to characters
		DispatchMessage (&theMessage); // Send message to window procedure
		if (click) {
			return pt;
		}
	}
	click = 0;
	
	return pt;
}




int   get_button(point local, int num_buttons) { 

	return num_buttons-1 - (int)(local.x*num_buttons/(x_size-window_thickness));
}



// 0 = buttons, 1 = layout, 2 = cross section, 3 = L SB, 4 = R SB, 5 = L pat, 6 = R pat, 7 = else
int   which_section(point global) {

	if (global.y > y_size-menubar_width-button_width) // buttons
		return 0;

	if (global.x < scrollbar_width) // left scrollbar
		return 3;
	
	else if (global.x < scrollbar_width+pattern_area_width) // left patterns
		return 5;
	
	else if (global.x < x_size-window_thickness-scrollbar_width-pattern_area_width) { // layout/cross section
		if (global.y > layout_top && global.y < cross_section_top_ratio*(y_size-menubar_width-button_width))
			return 1; // layout
		else if (global.y >= cross_section_top_ratio*(y_size-menubar_width-button_width))
			return 2; // cross section
	}
	else if (global.x < x_size-window_thickness-scrollbar_width) // right patterns
		return 6;
	
	else // right scrollbar
		return 4;
		
	return 7;
}




point get_section_offset(int selection) {

	point origin;

	origin.x = 0;
	origin.y = 0;

	switch (selection) {
		case 0: // buttons
			origin.y = y_size-menubar_width-button_width;
			break;
			
		case 1: // layout
			origin.x = scrollbar_width+pattern_area_width;
			origin.y = layout_top;
			break;
			
		case 2: // cross section
			origin.x = scrollbar_width+pattern_area_width;
			origin.y = (int)(cross_section_top_ratio*(y_size-menubar_width-button_width));
			break;
			
		case 3: // left scrollbar
			break;
			
		case 4: // right scrollbar
			origin.x = x_size-window_thickness-scrollbar_width;
			break;
			
		case 5: // left patterns
			origin.x = scrollbar_width;
			break;
			
		case 6: // right patterns
			origin.x = x_size-window_thickness-scrollbar_width-pattern_area_width;
			break;
			
		case 7: // other
			break;
	}
	return origin;
}




point transform_pointG2L(point global, int selection) {

	point local, origin;
	
	origin = get_section_offset(selection);
				
	local.x = global.x - origin.x;
	local.y = global.y - origin.y;
	
	return local;
}




point transform_pointL2G(point local, int selection) {

	point global, origin;
	
	origin = get_section_offset(selection);
				
	global.x = local.x + origin.x;
	global.y = local.y + origin.y;
	
	return global;
}




void  do_scroll(int &pos, int pat, int localY) { // ***
	
	if (pat > max_patterns_on_screen)
		pos = pat*localY/(y_size-button_width-menubar_width-5);
}




int   get_YNC(char *question) { // Y = 1, N = 0, C = 2

	UINT msgStyle = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNOCANCEL | MB_SETFOREGROUND; // dialog box params 
	int msgResult = MessageBox(DrawingWindow, question, "Question Y/N/C?", msgStyle);

	if (msgResult == 6)
		return 1;
	if (msgResult == 7)
		return 0;
		
	return 2;
}




int   get_YN(char *question) { // Y = 1, N = 0

	UINT msgStyle = MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO | MB_SETFOREGROUND; // dialog box params
	
	int msgResult = MessageBox(DrawingWindow, question, "Question Y/N?", msgStyle); // Display message box
			
	if (msgResult == 6)
		return 0;
	else if (msgResult == 7)
		return 1;

	return msgResult;
}




int   try_again() {

	return get_YN("Try again?");
}




void  compress_pi(pattern_info &patterns, int lpos, int rpos, int lpat, int rpat, int *colors, char **names) {

	patterns.lpos = lpos;
	patterns.rpos = rpos;
	patterns.lpat = lpat;
	patterns.rpat = rpat;
	patterns.colors = colors;
	patterns.names = names;
}




void  compress_object(cif_object &obj, int x1, int x2, int y1, int y2, int type, int pattern, int color) {

	obj.x1 = x1;
	obj.x2 = x2;
	obj.y1 = y1;
	obj.y2 = y2;
	obj.type = type;
	obj.pattern = pattern;
	obj.color = color;
}




cif_object convert_to_cif_box(cif_object &obj) {

	cif_object obj2;

	obj2.x1 = (obj.x1 + obj.x2)/2; // ul.x -> center.x
	obj2.y1 = (obj.y1 + obj.y2)/2; // ul.y -> center.y 
	obj2.x2 = abs(obj.x1 - obj.x2); // lr.x -> length
	obj2.y2 = abs(obj.y1 - obj.y2); // lr.x -> width
	obj2.type = obj.type;
	obj2.pattern = obj.pattern;
	obj2.color = obj.color;
	
	return obj2;
}





cif_object convert_from_cif_box(cif_object &obj) {

	cif_object obj2;

	obj2.x1 = obj.x1 - obj.x2/2; // center.x -> ul.x
	obj2.y1 = obj.y1 - obj.y2/2; // center.x -> ul.x
	obj2.x2 = obj.x1 + obj.x2/2; // length -> lr.x
	obj2.y2 = obj.y1 + obj.y2/2; // width -> lr.x
	obj2.type = obj.type;
	obj2.pattern = obj.pattern;
	obj2.color = obj.color;
	
	return obj2;
}




void  scale_obj(cif_object &obj) {

	obj.x1 *= xscale;
	obj.x2 *= xscale;
	obj.y1 *= yscale;
	obj.y2 *= yscale;
}




void  Xpause(int length) { 

	if (length == 0)
		wait(short_pause_length);
	else if (length == 1)
		wait(long_pause_length);
	else
		wait(abs(length));
}




int   do_exit() {

	int decision = get_YNC("SAVE PATTERN DIEFINITIONS?");
	
	if (decision == 1)
		cout << "\nSaving Pattern Definitions..." << endl;
		
	else if (decision == 2)
		return 1;
		
	return 0;
}




LONG CALLBACK DoSimplDix (HWND thisWindow, UINT msgCode, WPARAM wParam, LPARAM lParam) {

	LONG result = 0; // Function result
	
	switch (msgCode) { // Dispatch on message code
	
		case WM_CLOSE:
			click = 4;
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			click = 1;
			break;
		
		case WM_SIZE: // ???
		case WM_MOVE:
			click = 2;
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam);
			break;
			
		case WM_PAINT:
			click = 2;
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam);
			break;
					
		default:
			click = 0;
			result = DefWindowProc (thisWindow, msgCode, wParam, lParam); // Pass message to Windows for default processing
			break;
	} 
	return result;
}




// ****** Other Routines ******




int  call_edit_coms(dix_data &data) {

	int const num_buttons = 5;	
	int selection, button;
	point local;
	char *button_text[num_buttons] = {"EDIT/LAYOUT", "EDIT/PROFILE", "EDIT/PATTERN", " /RETURN", " /ABORT"};
	
	paint_buttons(num_buttons, button_text, 0);
	
	do {
		local = process_click(num_buttons, button_text, (char *)sel_com_mes, scm_len, data, selection, button);

		if (selection == 0 && button == 0)
			if (do_exit() == 0)
				return 0;
	} while (button != 1);
		
	return 1;
}




int  call_ana_tools(dix_data &data) {

	int const num_buttons = 6;
	int selection, button;
	point local;
	char *button_text[num_buttons] = {"CRITIC/(PROFILE/ANALYSIS)", "HUNCH/(MASK/ANALYSIS)", "WORST/(MISALIGN/MASK)", "SPLAT/(AERIAL/IMAGE)", " /RETURN", " /ABORT"};
	
	paint_buttons(num_buttons, button_text, 0);
	
	do {
		local = process_click(num_buttons, button_text, (char *)sel_com_mes, scm_len, data, selection, button);

		if (selection == 0 && button == 0)
			if (do_exit() == 0)
				return 0;
	} while (button != 1);
		
	return 1;
}




int  do_pisces(dix_data &data) {

	int const num_buttons = 6;
	int selection, button, decision;
	point local;
	char *button_text[num_buttons] = {" /BIPOLAR", " /MOSFET", " /GENERAL", " /EDIT", " /RETURN", " /ABORT"};
	
	decision = get_YN("CROSS SECTION?");
	
	paint_buttons(num_buttons, button_text, 0);	
	
	do {
		local = process_click(num_buttons, button_text, (char *)sel_com_mes, scm_len, data, selection, button);

		if (selection == 0 && button == 0)
			if (do_exit() == 0)
				return 0;
	} while (button != 1);
		
	return 1;
}




int  run_simpl2(/*dix_data &data*/) {

	int const num_buttons = 5;
	int decision;
	char *button_text[num_buttons] = {"RUN/ /SIMPL-2", " /PISCES", "CALL/ANALYSIS/TOOLS", "CALL/EDIT/COMMANDS", " /ABORT"};
	
	decision = get_YN("START A NEW SIMULATION?");
	
	Xpause(0);
	paint_text(" CANNOT RUN SIMPL", 16, 1);
	
	return 1;	
}









