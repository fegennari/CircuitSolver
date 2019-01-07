#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.GraphicsAll.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Circuit Solver Graphics for All OS
char const colorRGB[NUM_COLORS+1][3] = {0};

int const INVERT_COLORS = 0;
int const SWAP_BW       = 0;


int found_click(0), line_weight(1);
Point pt;

extern int x_limit, y_limit;



int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2);
int clip_rect_by_DW(int &x1, int &y1, int &x2, int &y2, int frame);
 




void SelectConsoleWindow(void)  { 

}



void SelectDrawingWindow(void)  { 

}




void SetDrawingColorx(int WhichColorx) { 

}




Point GetMouseLocation(void)  {
	
	return pt;
}



void GetMouseLocation(int &x, int &y)  {

}



int FoundAClick(Point *PT)  {
	
	return 0;
}



Point GetClick(void)  {
	
	return pt; 
} 



void GetClick(int &x, int &y)  {

}




void MoveTo(int x, int y) {
	
}




void LineTo(int x, int y) {
	
}



void LineDraw(int x1, int y1, int x2, int y2) {

	if(!clip_line_to_box(x1, y1, x2, y2, 0, 0, x_limit, y_limit))
		return;

	MoveTo(x1,y1);
	LineTo(x2,y2);	
}




// top left and bottom right x and y coords 
void RectangleDraw(int x1, int y1 , int x2, int y2) {

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

}




// given x ,y points, and radius - draw wire frame circle 
void CircleFrame(int x, int y, int rad) {
	
}




void ArcFrame( int x, int y, int radius, int startDegrees, int NumDegrees) {
	
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
	
	d1 = n / 1000;
	d2 = n/100 %10;	
	d3 = n/10 % 10;	
	d4 = n % 10;
	
	str[0] = 4; // This must be the length of the string since
				// it must be a Pascal string.
	if (d1!=0 ) {
		str[1] = d1 + 48;
		foundNonZero = 1;
	}
	else
		str[1] =' ';
	
	if (d2 != 0 || foundNonZero == 1) {
		str[2] = d2 + 48;
		foundNonZero = 1;
	}
	else
		str[2] =' ';
	
	if (d3 != 0 || foundNonZero == 1) {
		str[3] = d3 + 48;
		foundNonZero = 1;
	}
	else
		str[3] =' ';
	
	str[4] = d4 + 48;
	str[5] = '\0';
}




void DrawInt(int n) {	// Max value of int is 9999.

	char str[6];
	
	NumToStr(n, str);
	draw_string2((unsigned char *)str);
}









