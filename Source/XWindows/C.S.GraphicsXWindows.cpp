#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S. GraphicsXWindows.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Circuit Solver Graphics for XWindows
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


int CurrentColor = 10, found_click(0);
Point pt, current;

Window DrawingWindow, MenuWindow, ConsoleWindow;

Colormap DWColormap, MBColormap;


extern int must_exit, DWScreen, DWDepth, MBScreen, MBDepth;
extern Display *DWDisp, *MenubarDisp; 
extern GC DWContext, MBContext;


void exit_error();
 



// can I do this?
void SelectConsoleWindow(void)  { 

	XSetInputFocus(ConsoleDisp, RootWindow(ConsoleDisp, 0), RevertToNone, CurrentTime);
	XMapRaised(DWDisp, DrawingWindow);
}


// ?
void SelectDrawingWindow(void)  { 

	XSetInputFocus(DWDisp, RootWindow(DWDisp, 0), RevertToNone, CurrentTime);
	XMapRaised(DWDisp, DrawingWindow);
}



// ?
void SetDrawingColorx(int WhichColorx) { 

	XColor color;
  	  	
  	CurrentColor = WhichColorx;

	if (WhichColorx <= 0 || WhichColorx > num_colors) {
		internal_error();
  		cout << "Error: Invalid Color: " << WhichColorx << endl;
  		return;
	}
	if (SWAP_BW) {
		if (WhichColorx == BLACK) {
			WhichColorx = WHITE;
		}
		else if (WhichColorx == WHITE) {
			WhichColorx = BLACK;
		}
	}
  	color.red   = colorRGB[WhichColorx][0];
  	color.green = colorRGB[WhichColorx][1];
  	color.blue  = colorRGB[WhichColorx][2];

	if (INVERT_COLORS) {
		color.red   ^= 255;
		color.green ^= 255;
		color.blue  ^= 255;
	}
	XAllocColor(DWDisp, DWColormap, &color);
	XSetForeground(DWDisp, DWContext, color.pixel);

    /*Color  color;
  	Colormap color_map = DefaultColormap(DWDisp, DefaultScreen(DWDisp));
  	char red(0), green(0), blue(0), color_def[16];
  	
  	WhichColorx = min(WhichColorx, NUM_COLORS - 1);
  	WhichColorx = max(WhichColorx, 0);
  	
  	CurrentColor = WhichColorx;
  	
	if (WhichColorx > 0 && WhichColorx <= num_colors) {
  		red = colorRGB[WhichColorx][0];
  		green = colorRGB[WhichColorx][1];
  		blue = colorRGB[WhichColorx][2];
  	}
  	else {
  		cout << "Error: Invalid Color: " << WhichColorx << endl;
  		return;
	}	
	sprintf(color_def, "#%02x%02x%02x", red, green, blue);
	XParseColor(DWDisp, color_map, color_def, &color);
	XStoreColor(DWDisp, color_map, &color);
	
	//XFlush(DWDisp); // ?
	
	XSetForeground(DWDisp, DWContext, color);
	XSetBackground(DWDisp, DWContext, color.pixel);
	XSetFunction(DWDisp, DWContext, GXcopy); // ?
	XSetFillStyle(DWDisp, DWContext, FillSolid); // ?*/
}



// ?
Point GetMouseLocation(void)  {

	Point pt;
	XEvent event;
		
	pt.h = pt.v = 0; 
		
	XNextEvent(DWDisp, &event);
	
	if (XCheckWindowEvent(DWDisp, DrawingWindow, PointerMotionMask, &event)) {
		pt.h = event.xbutton.x;
		pt.v = event.xbutton.y;
	}	
	return pt;
}



void GetMouseLocation(int &x, int &y)  {

	Point  pt = GetMouseLocation();
	
	x = pt.h;
	y = pt.v;
}


// ?
int FoundAClick(Point *pt)  {
	
	XEvent event;
		
	SelectDrawingWindow();
	found_click = 0;
	pt.h = pt.v = 0;
	
	XNextEvent(DWDisp, &event);
	
	if (XCheckWindowEvent(DWDisp, DrawingWindow, ButtonPressMask, &event) && event.type == ButtonPress) {
		pt.h = event.xbutton.x;
		pt.v = event.xbutton.y;
		return 1;
	}	
	return 0;
}


// ?
Point GetClick(void)  {
	
	Point pt;
	XEvent event;
	
	SelectDrawingWindow();
		
	//do {
		//XNextEvent(DWDisp, &event);
	//} while (!XCheckWindowEvent(DWDisp, DrawingWindow, ButtonPressMask, &event);
	
	event = GetEvent(ButtonPressMask);
	
	if (event.xkey.window == DrawingWindow) {
		ckick_type = 1;
		pt.h = event.xbutton.x;
		pt.v = event.xbutton.y;
	}	
	return pt; 
} 



void GetClick(int &x, int &y)  {

	Point  pt;

	while (!FoundAClick(&pt)) {};
  
	x = pt.h;
	y = pt.v;
}




void MoveTo(int x, int y) {
	
	current.h = x;
	current.v = y;
}




void LineTo(int x, int y) {
	
	LineDraw(current.h, current.v, x, y);
	
	current.h = x;
	current.v = y;
}


// ?
void LineDraw(int x1, int y1, int x2, int y2) {
	
	XDrawLine(DWDisp, DrawingWindow, DWContext, x1, y1, x2, y2);
}



// ?
// top left and bottom right x and y coords 
void RectangleDraw(int x1, int y1 , int x2, int y2) {
	
	if(x1 > x2)
		swap(x1, x2);
	
	if (y1 > y2)
		swap(y1, y2);
	
	//XSetFillStyle(DWDisp, DWContext, FillSolid); // ?
	//XSetFunction(DWDisp, DWContext, GXcopy); // ?
    XFillRectangle(DWDisp, DrawingWindow, DWContext, x1, y1, x2-x1, y2-y1);
}



// ?		
// top left and bottom right x and y coords 
void RectangleFrame(int x1, int y1 , int x2, int y2) {
	
	if(x1 > x2)
		swap(x1, x2);
	
	if (y1 > y2)
		swap(y1, y2);
	
	/*MoveTo(x1, y1);
	LineTo(x2, y1);
	LineTo(x2, y2);
	LineTo(x1, y2);
	LineTo(x1, y1);*/
	
	XDrawRectangle(DWDisp, DrawingWindow, DWContext, x1, y1, x2-x1, y2-y1);
}



// ?
// given x ,y points, and radius - draw filled circle 
void CircleDraw(int x, int y, int rad) {

	if (rad < 0)
		rad = -rad;
	
	XFillCircle(DWDisp, DrawingWindow, DWContext, x, y, rad); // ?
}



// ?
// given x ,y points, and radius - draw wire frame circle 
void CircleFrame(int x, int y, int rad) {

	if (rad < 0)
		rad = -rad;
	
	XDrawCircle(DWDisp, DrawingWindow, DWContext, x, y, rad); // ?
}



// ?
void ArcFrame( int x, int y, int radius, int startDegrees, int NumDegrees) {
	
	int EndDegrees = startDegrees + NumDegrees;
	
	XDrawArc(DWDisp, DrawingWindow, DWContext, x-radius, y-radius, x+radius,
			y+radius, startDegrees*RTOD*64*pi/180, EndDegrees*RTOD*64*pi/180); // ?
	
	/*int x1, x2, y1, y2;
	
	if (radius < 0)
		radius = -radius;
		
	int endDegrees = startDegrees + NumDegrees;
	
	if (NumDegrees < 0) 
		swap(startDegrees, endDegrees); // Horrible Arc Conversion Mac->XWindows
	
	while (startDegrees < 0)
		startDegrees += 360;
	while (startDegrees >= 360)
		startDegrees -= 360;
	
	while (endDegrees < 0)
		endDegrees += 360;
	while (endDegrees >= 360)
		endDegrees -= 360;
	
	x1 = fabs(radius*sin(pi*endDegrees/180));
	y1 = fabs(radius*cos(pi*endDegrees/180));
	x2 = fabs(radius*sin(pi*startDegrees/180));
	y2 = fabs(radius*cos(pi*startDegrees/180));
	
	if (endDegrees > 180)
		x1 = -x1;
	
	if (startDegrees > 180)
		x2 = -x2;
		
	if (endDegrees > 90 && endDegrees < 270)
		y1 = -y1;
		
	if (startDegrees > 90 && startDegrees < 270)
		y2 = -y2;*/
		
	//SelectObject(DWContext, currPen);			
    //Arc (DWContext, x-radius, y-radius, x+radius, y+radius, x+x1, y-y1, x+x2, y-y2);               	
}



// ?
void ColorPixel(int color, Point p) {	// Not used yet.
	
	SetDrawingColorx(color);
	//LineDraw(p.h, p.v, p.h + 1, p.v);
	
	XDrawPoint(DWDisp, DrawingWindow, DWContext, (int)p.h, (int)p.v);
}




void NumToStr(int n, char str[]) {	// Max value of int is 9999.

	int d1,d2,d3,d4;
	int foundNonZero = 0;
	
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
	else {
		str[2] =' ';
	}
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









