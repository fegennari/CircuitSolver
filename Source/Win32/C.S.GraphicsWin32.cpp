#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.GraphicsWin32.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Circuit Solver Graphics for Win32
unsigned char const colorRGB[NUM_COLORS+1][3] = {
			{(unsigned char)0,	(unsigned char)0,	(unsigned char)0},
			{(unsigned char)255,(unsigned char)0,	(unsigned char)0}, 
			{(unsigned char)0,	(unsigned char)255,	(unsigned char)0},
			{(unsigned char)0,	(unsigned char)0,	(unsigned char)255}, 
			{(unsigned char)255,(unsigned char)255,	(unsigned char)0}, 
			{(unsigned char)255,(unsigned char)23,	(unsigned char)89}, 
			{(unsigned char)255,(unsigned char)0,	(unsigned char)255}, 
			{(unsigned char)255,(unsigned char)94,	(unsigned char)23}, 
			{(unsigned char)86,	(unsigned char)243,	(unsigned char)255},
			{(unsigned char)255,(unsigned char)255,	(unsigned char)255},
			{(unsigned char)0,	(unsigned char)0,	(unsigned char)0}, 
			{(unsigned char)148,(unsigned char)148,	(unsigned char)148}, 
			{(unsigned char)84,	(unsigned char)145,	(unsigned char)185}, 
			{(unsigned char)249,(unsigned char)238,	(unsigned char)178}, 
			{(unsigned char)181,(unsigned char)182,	(unsigned char)184}, 
			{(unsigned char)214,(unsigned char)186,	(unsigned char)156}, 
			{(unsigned char)14,	(unsigned char)125,	(unsigned char)0},
			{(unsigned char)255,(unsigned char)150,	(unsigned char)0}};

int const INVERT_COLORS = 0;
int const SWAP_BW       = 0;


HWND DrawingWindow, MenuWindow, ConsoleWindow = NULL;
HBRUSH currBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
HPEN currPen = (HPEN)GetStockObject(BLACK_PEN);
HBRUSH coloredBrushes[NUM_COLORS+1] = {NULL};
HPEN coloredPens[NUM_COLORS+1] = {NULL};

int found_click(0), pen_style(PS_SOLID), line_weight(1);
Point pt;

extern char typed_char;
extern int must_exit, DW_exists, is_char, click_type, x_limit, y_limit;
extern HDC DWContext;
extern HACCEL AccelTable;



int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2);
int clip_rect_by_DW(int &x1, int &y1, int &x2, int &y2, int frame);

void exit_error();
 




void SelectConsoleWindow(void)  { 

	BringWindowToTop(ConsoleWindow);
	SetFocus(ConsoleWindow);
}



void SelectDrawingWindow(void)  { 

	BringWindowToTop(DrawingWindow);
	SetFocus(DrawingWindow);
}




void SetDrawingColorx(int WhichColorx) {

  	COLORREF  color;
	HPEN last_pen;
  	char red(0), green(0), blue(0);
	int delete_now(0);
  	static int delete_this_pen(0), last_color(-1);

	if (WhichColorx == last_color && WhichColorx != -1 && line_weight == 1) {
		return;
	}
	if (WhichColorx <= 0 || WhichColorx > NUM_COLORS) {
  		internal_error();
  		cerr << "Error: Invalid Color Requested: " << WhichColorx << endl;
  		return;
  	}
	last_color = WhichColorx;

	if (SWAP_BW) {
		if (WhichColorx == BLACK) {
			WhichColorx = WHITE;
		}
		else if (WhichColorx == WHITE) {
			WhichColorx = BLACK;
		}
	}	
  	red   = colorRGB[WhichColorx][0];
  	green = colorRGB[WhichColorx][1];
  	blue  = colorRGB[WhichColorx][2];

	if (INVERT_COLORS) {
		red   ^= 255;
		green ^= 255;
		blue  ^= 255;
	}
	color = RGB(red, green, blue);
	
	SetTextColor(DWContext, color);
	SetBkColor(DWContext, color);
	
	if (coloredBrushes[WhichColorx] == NULL) {
		coloredBrushes[WhichColorx] = CreateSolidBrush(color);
		if (coloredBrushes[WhichColorx] == NULL)
			exit_error();
	}
	currBrush = coloredBrushes[WhichColorx];	
	
	if (delete_this_pen) {
		delete_now = 1;
		delete_this_pen = 0;
	}
	if (pen_style != PS_SOLID || line_weight != 1) {
		line_weight = max(line_weight, 1);
		currPen = CreatePen(pen_style, line_weight, color);
		if (currPen == NULL) {
			cerr << "Error creating weighted pen." << endl;
			exit_error();
		}
		delete_this_pen = 1;
	}	
	else if (coloredPens[WhichColorx] == NULL) {
		currPen = coloredPens[WhichColorx] = CreatePen(PS_SOLID, 1, color);
		if (coloredPens[WhichColorx] == NULL)
			exit_error();
	}
	else
		currPen = coloredPens[WhichColorx];

	SelectObject(DWContext, currBrush);
	last_pen = (HPEN)SelectObject(DWContext, currPen);

	if (delete_now) {
		DeleteObject(last_pen);
	}
}




Point GetMouseLocation(void)  {
	
	tagPOINT PT;
	tagRECT r;

	GetCursorPos(&PT); 
	GetWindowRect(DrawingWindow, &r); 
	
	// Convert from desktop to windows coords
	pt.h = PT.x - r.left - window_nonclient_width/2; 
	pt.v = PT.y - r.top - window_nonclient_height + window_nonclient_width/2 - menuwidth;
	
	return pt;
}



void GetMouseLocation(int &x, int &y)  {

	pt = GetMouseLocation();
	x = pt.h;
	y = pt.v;
}



int FoundAClick(Point *PT)  {
	
	BOOL SeeMessage;
	MSG theMessage;
			
	if (DW_exists)
		SelectDrawingWindow();
		
	found_click = 0;
	
	SeeMessage = PeekMessage(&theMessage, NULL, 0, 0, 1); // Get next message
	
	if (SeeMessage) {
		TranslateMessage (&theMessage); // Convert virtual keys to characters
		DispatchMessage (&theMessage); // Send message to window procedure
		SeeMessage = found_click;
		found_click = 0;
		*PT = pt;
		return SeeMessage;
	}
	return 0;
}



Point GetClick(void)  {
	
	BOOL ContinueFlag = TRUE, translated;
	MSG theMessage; // Next message to process
	
	SelectDrawingWindow();
		
	while(ContinueFlag) {
		ContinueFlag = GetMessage(&theMessage, NULL, 0, 0); // Get next message
		translated = TranslateAccelerator(DrawingWindow, AccelTable, &theMessage); // Check for keyboard accelerator
		
		if (!translated) { // Was the message an accelerator?
			TranslateMessage (&theMessage); // Convert virtual keys to characters
			DispatchMessage (&theMessage); // Send message to window procedure
		}
		else {
			found_click = 0;
			return pt;
		}
		if (found_click || (is_char && typed_char >= 37 && typed_char <= 40)) {
			found_click = 0;
			return pt;
		}
	}
	must_exit = 1;
	
	return pt; 
} 



void GetClick(int &x, int &y)  {

	while (!FoundAClick(&pt)) {};
  
	x = pt.h;
	y = pt.v;
}




void MoveTo(int x, int y) {
	
	if(x < 0) x = -1;
	if(y < 0) y = -1;
	
	MoveToEx(DWContext, x, y, NULL);
}




void LineTo(int x, int y) {
	
	if(x < 0) x = -1;
	if(y < 0) y = -1;
	
	LineTo(DWContext, x, y);
}



void LineDraw(int x1, int y1, int x2, int y2) {

	if(!clip_line_to_box(x1, y1, x2, y2, 0, 0, x_limit, y_limit))
		return;

	MoveTo(x1,y1);
	LineTo(x2,y2);	
}




// top left and bottom right x and y coords 
void RectangleDraw(int x1, int y1 , int x2, int y2) {

	tagRECT r;
	
	if(!clip_rect_by_DW(x1, y1, x2, y2, 0))
		return;
	
	SetRect(&r, x1,y1,x2,y2);
	FillRect(DWContext, &r, currBrush); 
}



		
// top left and bottom right x and y coords 
void RectangleFrame(int x1, int y1 , int x2, int y2) {
		
	if(!clip_rect_by_DW(x1, y1, x2, y2, 1))
		return;
	
	MoveTo(x1, y1);
	LineTo(x2, y1);
	LineTo(x2, y2);
	LineTo(x1, y2);
	LineTo(x1, y1);
}




// given x ,y points, and radius - draw filled circle 
void CircleDraw(int x, int y, int rad) {

	if (rad < 0)
		rad = -rad;
		
	if(x + rad < 0 || y + rad < 0)
		return;
	
	Ellipse(DWContext, x-rad,y-rad,x+rad,y+rad); 
}




// given x ,y points, and radius - draw wire frame circle 
void CircleFrame(int x, int y, int rad) {
	
	if (rad < 0)
		rad = -rad;
		
	if(x + rad < 0 || y + rad < 0)
		return;
	
	
	SelectObject(DWContext, GetStockObject(NULL_BRUSH)); // Transparent brush
	Ellipse(DWContext, x-rad,y-rad,x+rad,y+rad);
	SelectObject(DWContext, currBrush);
}




void ArcFrame( int x, int y, int radius, int startDegrees, int NumDegrees) {
	
	int x1, x2, y1, y2;
	int endDegrees = startDegrees + NumDegrees;
	
	if (radius < 0)
		radius = -radius;
		
	if(x + radius < 0 || y + radius < 0)
		return;
	
	if (NumDegrees < 0) 
		swap(startDegrees, endDegrees); // Horrible Arc Conversion Mac->Windows
	
	while (startDegrees < 0)
		startDegrees += 360;
	while (startDegrees >= 360)
		startDegrees -= 360;
	
	while (endDegrees < 0)
		endDegrees += 360;
	while (endDegrees >= 360)
		endDegrees -= 360;
	
	x1 = (int)fabs(radius*sin(pi*endDegrees/180));
	y1 = (int)fabs(radius*cos(pi*endDegrees/180));
	x2 = (int)fabs(radius*sin(pi*startDegrees/180));
	y2 = (int)fabs(radius*cos(pi*startDegrees/180));
	
	if (endDegrees > 180)
		x1 = -x1;	
	if (startDegrees > 180)
		x2 = -x2;		
	if (endDegrees > 90 && endDegrees < 270)
		y1 = -y1;		
	if (startDegrees > 90 && startDegrees < 270)
		y2 = -y2;
				
    Arc (DWContext, x-radius, y-radius, x+radius, y+radius, 
         x+x1, y-y1, x+x2, y-y2);               
	
	//AngleArc(DWContext, x, y, radius, (float)startDegrees, (float)NumDegrees); // Doesn't work
}




void ColorPixel(int color, Point p) {	// Not used yet.
	
	if(pt.h < 0 || pt.v < 0 || pt.h > x_limit || pt.v > y_limit)
		return;
	
	SetDrawingColorx(color);
	LineDraw(p.h, p.v, p.h+1, p.v+1);
}




void NumToStr(int n, char str[]) {	// Max value of int is 9999.

	int d1,d2,d3,d4;
	int foundNonZero = 0;

	if (n > 10000) { // error
		n %= 1000;
	}
	d1 = n / 1000;
	d2 = n/100 %10;	
	d3 = n/10 % 10;	
	d4 = n % 10;
	
	str[0] = 4; // This must be the length of the string since
				// it must be a Pascal string.
	if (d1!=0 ) {
		str[1] = (char)(d1 + '0');
		foundNonZero = 1;
	}
	else
		str[1] =' ';
	
	if (d2 != 0 || foundNonZero == 1) {
		str[2] = (char)(d2 + '0');
		foundNonZero = 1;
	}
	else
		str[2] =' ';
	
	if (d3 != 0 || foundNonZero == 1) {
		str[3] = (char)(d3 + '0');
		foundNonZero = 1;
	}
	else
		str[3] =' ';
	
	str[4] = (char)(d4 + '0');
	str[5] = '\0';
}




void DrawInt(int n) {	// Max value of int is 9999.

	char str[6];
	
	NumToStr(n, str);
	draw_string2((unsigned char *)str);
}









