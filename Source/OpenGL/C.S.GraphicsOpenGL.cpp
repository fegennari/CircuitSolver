#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.GraphicsOpenGL.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Circuit Solver Graphics for OpenGL
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


int found_click(0), line_weight(1);
Point pt;

extern int x_limit, y_limit;



int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2);
int clip_rect_by_DW(int &x1, int &y1, int &x2, int &y2, int frame);
 



// ???
void SelectConsoleWindow(void)  { 

}


// ???
void SelectDrawingWindow(void)  { 

}




void SetDrawingColorx(int WhichColorx) { 

  	unsigned char red(0.0), green(0.0), blue(0.0);
  	static int last_color(-1);

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
	glColor2f(((double)red)/255.0, ((double)green)/255.0, ((double)blue)/255.0);
}



// ???
Point GetMouseLocation(void)  {
	
	return pt;
}


// ???
void GetMouseLocation(int &x, int &y)  {

	x = 0;
	y = 0;
}


// ???
int FoundAClick(Point *PT)  {
	
	return 0;
}


// ???
Point GetClick(void)  {
	
	return pt; 
} 


// ???
void GetClick(int &x, int &y)  {

}




void MoveTo(int x, int y) {
	
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);
	glVertex2f((float)x, (float)y);
}




void LineTo(int x, int y) {
	
	glVertex2f((float)x, (float)y);
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);
	glVertex2f((float)x, (float)y);
}



void LineDraw(int x1, int y1, int x2, int y2) {

	if(!clip_line_to_box(x1, y1, x2, y2, 0, 0, x_limit, y_limit))
		return;

	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);
	glVertex2f((float)x1, (float)y1);
	glVertex2f((float)x2, (float)y2);
	glEnd();
}




// top left and bottom right x and y coords 
void RectangleDraw(int x1, int y1 , int x2, int y2) {

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGONS);
	glVertex2f((float)x1, (float)y1);
	glVertex2f((float)x1, (float)y2);
	glVertex2f((float)x2, (float)y2);
	glVertex2f((float)x2, (float)y1);
	glVertex2f((float)x1, (float)y1);
	glEnd();
}



		
// top left and bottom right x and y coords 
void RectangleFrame(int x1, int y1 , int x2, int y2) {
		
	if(!clip_rect_by_DW(x1, y1, x2, y2, 1))
		return;
	
	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_LINES);
	glVertex2f((float)x1, (float)y1);
	glVertex2f((float)x1, (float)y2);
	glVertex2f((float)x1, (float)y2);
	glVertex2f((float)x2, (float)y2);
	glVertex2f((float)x2, (float)y2);
	glVertex2f((float)x2, (float)y1);
	glVertex2f((float)x2, (float)y1);
	glVertex2f((float)x1, (float)y1);
	glEnd();
}



// ???
// given x ,y points, and radius - draw filled circle 
void CircleDraw(int x, int y, int rad) {

}



// ???
// given x ,y points, and radius - draw wire frame circle 
void CircleFrame(int x, int y, int rad) {
	
}



// ???
void ArcFrame( int x, int y, int radius, int startDegrees, int NumDegrees) {
	
}




void ColorPixel(int color, Point p) {	// Not used yet.
	
	if(pt.h < 0 || pt.v < 0 || pt.h > x_limit || pt.v > y_limit)
		return;
	
	SetDrawingColorx(color);
	
	glBegin(GL_POINTS);
	glVertex2f((float)pt.h, (float)pt.v);
	glEnd();
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









