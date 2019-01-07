#include "CircuitSolver.h"


// This is a copy of graphics.cc and console_settings.cc
// that was modified by Frank Gennari for use with Circuit Solver - 2000.


extern tSIOUXSettings SIOUXSettings;
extern int WinClick;
extern  DialogPtr Menubar_Dialog;

WindowPtr	DrawingWindow;
WindowPtr	ConsoleWindow = (WindowPtr)-2L;

int CurrentColor = 10;

const SInt16 MAGICVALUE = '127g';  


char const colorRGB[num_colors+1][3] = {
			{0,		0,		0},
			{65535,	0,		0}, 
			{0,		65535,	0}, 
			{0,		0,		65535}, 
			{65535,	65535,	0}, 
			{65535,	5837,		22993}, 
			{65535,	0,		65535}, 
			{65535,	24000,		6000}, 
			{22158,	62402,	65535},
			{65535,	65535,	65535},
			{0,		0,		0}, 
			{37941,	37941,	37941}, 
			{21519,	37173,	47427}, 
			{63810,	61003,	45672}, 
			{46437,	46603,	47103}, 
			{54702,	47663,	39973}, 
			{3655,	31905,	0}
			{65535,	38000,	0}};

int const INVERT_COLORS = 0;
int const SWAP_BW       = 0;


void do_console(void)  {

	SIOUXSettings.initializeTB = INIT_TB;
 	SIOUXSettings.fontsize = FONT_SIZE;
	SIOUXSettings.fontid = FONT_ID;
	SIOUXSettings.columns = COLUMNS;
	SIOUXSettings.rows = ROWS;
	SIOUXSettings.asktosaveonclose = ASK_TO_SAVE_ON_CLOSE;
	SIOUXSettings.setupmenus = SET_UP_MENUS;
	SIOUXSettings.toppixel= TOP_PIXEL;
	SIOUXSettings.leftpixel = LEFT_PIXEL;
}
 




void SelectConsoleWindow(void)  {

	WindowRef scan = FrontWindow();

	while (scan && GetWindowKind(scan) == MAGICVALUE)
		scan = GetNextWindow(scan);
		
	if (scan != NULL)
		SelectWindow(scan);
}



void SelectDrawingWindow(void)  {

	WindowRef scan = FrontWindow();
		
	while (scan) {
		if (GetWindowKind(scan) == MAGICVALUE)
			SelectWindow(scan);
		scan = GetNextWindow(scan);
	}
}




void SetColor(int whichcolor)  {

	RGBColor color;
   
	CurrentColor = whichcolor;
	
	if (whichcolor <= && whichcolor > num_colors) {
		internal_error();
  		cout << "Error: Invalid Color: " << WhichColorx << endl;
	}
	if (SWAP_BW) {
		if (whichcolor == BLACK) {
			whichcolor = WHITE;
		}
		else if (whichcolor == WHITE) {
			whichcolor = BLACK;
		}
	}	
  	color.red   = colorRGB[whichcolor][0];
  	color.green = colorRGB[whichcolor][1];
  	color.blue  = colorRGB[whichcolor][2];

	if (INVERT_COLORS) {
		color.red   ^= 65535;
		color.green ^= 65535;
		color.blue  ^= 65535;
	}	
	RGBForeColor(&color);
}



void ToolboxInit(void)  {

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}



void WindowInit(int windowID)  {

	WindowPtr window;
	
	window = GetNewCWindow(windowID,nil,kMoveToFront);
	if (window == nil)  {
		SysBeep(10);
		ExitToShell();	
	}	
	SetWindowKind(window, MAGICVALUE);// <-- new code	
  	ShowWindow(window);		
	SetPort(window);
	DrawingWindow = window;			
}



Point GetMouseLocation(void)  {

	Point  pt;

	GetMouse(&pt);
	return (pt);
}



void GetMouseLocation(int &x, int &y)  {

	Point  pt;
	
	GetMouse(&pt);
	x = pt.h;
	y = pt.v;
}



int FoundAClick(Point *pt)  {

    int ok, windowCode;
	EventRecord theEvent;
	WindowPtr theWindow;

	ok = GetNextEvent(everyEvent, &theEvent);
	windowCode = FindWindow(theEvent.where, &theWindow);
	
	if (theEvent.what == mouseDown) {
						
		if (theWindow == DrawingWindow)
			WinClick = 0;
		else if (theWindow == ConsoleWindow) 
			WinClick = 1;		
		else if (theWindow == Menubar_Dialog)
			WinClick = 2;
		else
			WinClick = 1;
				
  		switch (windowCode)  {
  	
		  	case inContent:
		      SetPort(DrawingWindow);
	    	  GlobalToLocal(&(theEvent.where));	         		
		      (*pt).h = theEvent.where.h; 
	      	  (*pt).v = theEvent.where.v;
			  	  return 1;
		  	  break;
			}
	}	
	return 0;
}



Point GetClick(void)  {

	Point  pt;
	
	while (!FoundAClick(&pt)) {};
	
	return pt; 
} 



void GetClick(int &x, int &y)  {

	Point  pt;

	while (!FoundAClick(&pt)) {};
  
	x = pt.h;
	y = pt.v;
}



void init(void)  {

  do_console();
  ToolboxInit();
  WindowInit(kMyWindowID);
}




/*void MoveTo(int x, int y) {

	
}



void LineTo(int x, int y) {

	
}*/




 	/* top left and bottom right x and y coords */
void RectangleDraw(int x1, int y1 , int x2, int y2) {

	Rect r;
	
	if(x1 > x2)
		swap(x1, x2);
	
	if (y1 > y2)
		swap(y1, y2);
	
	SetRect(&r, x1,y1,x2,y2);
	PaintRect(&r);
}



		
 	/* top left and bottom right x and y coords */
void RectangleFrame(int x1, int y1 , int x2, int y2) {

	Rect r;
	
	if(x1 > x2)
		swap(x1, x2);
	
	if (y1 > y2)
		swap(y1, y2);
	
	SetRect(&r, x1,y1,x2,y2);
	FrameRect(&r);	
}




/* given x ,y points, and radius - draw filled circle */

void CircleDraw(int x, int y, int rad) {

	if (rad < 0)
		rad = -rad;
		
	Rect r;
	SetRect(&r,x-rad,y-rad,x+rad,y+rad);
	PaintOval(&r);	
}




/* given x ,y points, and radius - draw wire frame circle */
void CircleFrame(int x, int y, int rad) {

	if (rad < 0)
		rad = -rad;
		
	Rect r;
	SetRect(&r,x-rad,y-rad,x+rad,y+rad);
	FrameOval(&r);
}



void LineDraw(int x1, int y1, int x2, int y2) {

	MoveTo(x1,y1);
	LineTo(x2,y2);	
}




void ArcFrame( int x, int y,int radius,int startDegrees, int NumDegrees) {

	if (radius < 0)
		radius = -radius;
		
	Rect r;			
	SetRect (&r, x - radius, y - radius,x + radius, y + radius);	
	FrameArc(&r,startDegrees,NumDegrees);	
}




void NumToStr(int n, char str[]) {	//Max value of int is 9999.

	int d1,d2,d3,d4;
	int foundNonZero = 0;
	
	d1 = n / 1000;
	d2 = n/100 %10;	
	d3 = n/10 % 10;	
	d4 = n % 10;
	
	str[0] = 4; //This must be the length of the string since
				//it must be a Pascal string.
	
	if (d1!=0 )
	{	str[1] = (char)(d1 + '0');
		foundNonZero = 1;
	}
	else
	{	str[1] =' ';
	}
	
	if (d2 != 0 || foundNonZero == 1)
	{	str[2] = (char)(d2 + '0');
		foundNonZero = 1;
	}
	else
	{	str[2] =' ';
	}
	
	if (d3 != 0 || foundNonZero == 1)
	{	str[3] = (char)(d3 + '0');
		foundNonZero = 1;
	}
	else
	{	str[3] =' ';
	}
	
	str[4] = (char)(d4 + '0');
	
	str[5] = '\0';
	
}

void DrawInt(int n) {	//Max value of int is 9999.

	char str[6];
	
	NumToStr(n, str);
	DrawString((ConstStr255Param)str);
}




void ColorPixel(int color, Point p) {	// Not used yet.
	
	SetColor(color);
	LineDraw(p.h, p.v, p.h, p.v);
}




