#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Window.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S. Window.cc (Any Window Environment)
// By Frank Gennari
int const realloc_with_aspect_ratio = 0;


int object_size, ar_changed(0), sch_origin_grid_x(0), sch_origin_grid_y(0);


extern int grid_color, saveBOXES, x_limit, y_limit, o_size, num_buttons, max_total_boxes, click_type;
extern int sch_origin_x, sch_origin_y;
extern int x_blocks, y_blocks, x_screen, y_screen;
extern double DW_ratio;
extern box new_num;


point get_center(int box_num);
int get_scrolled_window_click();
int get_valid_position();
int in_selection_area(int location, int position1, int position2);
int is_in_window(point P);
int is_in_window(Point P);
int is_in_window(int location);
int search_for_box(int x, int y);
void init_buttons(int is_schematic_window);
int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2);
int clip_rect_by_DW(int &x1, int &y1, int &x2, int &y2, int frame);
int clip_rect_by_box(int &x1, int &y1, int &x2, int &y2, int bx1, int by1, int bx2, int by2, int frame);
int point_in_box(int x, int y, int bx1, int by1, int bx2, int by2);
int clip_text(point &c, int bx1, int by1, int bx2, int by2, int shift_dist);
void clipping_check();

void draw_grid(int grid_color);
void draw_special();




point get_center(int box_num) {

	point center;
	int x_position, y_position;

	if (x_blocks == 0 || y_blocks == 0) {
		center.x = center.y = 0;
		return center;
	}
	if (box_num < 0)
		box_num = -box_num;
	
	x_position = box_num%x_blocks - sch_origin_grid_x; // in grid units	
	y_position = box_num/x_blocks - sch_origin_grid_y; 

	center.x = (2*x_position + 1)*x_limit/((double)(2*x_blocks)); // convert to pixels
	center.y = (2*y_position + 1)*y_limit/((double)(2*y_blocks));

	if (o_size < 0)
		center.x -= o_size/5;
			
	return center;
}




int get_scrolled_window_click() {

	int value;
	
	sch_origin_grid_x = sch_origin_x;
	sch_origin_grid_y = sch_origin_y;
	
	value = get_window_click();
		
	sch_origin_grid_x = 0;
	sch_origin_grid_y = 0;
	
	return value;
}




int get_valid_position() {
	
	int position;
	
	do {
		position = get_scrolled_window_click();
		
		if (position == CANCEL) {
			cout << "\nCANCEL" << endl;
			return -1;
		}
	} while (position <= num_menu_boxes);
	
	return position;
}




int box_clicked() {
	
	Point pt;

	while (1) {
		pt = GetDWClick();
		if (click_type == 0)
			return which_box(pt);
	}
	return -1;
}




int in_selection_area(int location, int position1, int position2) {

	int x1, x2, y1, y2, px, py;
	
	if (position2 == -1) {
		return (location == position1);
	}
	x1 = min(position1%x_blocks, position2%x_blocks);
	x2 = max(position1%x_blocks, position2%x_blocks);
	y1 = min(position1/x_blocks, position2/x_blocks);
	y2 = max(position1/x_blocks, position2/x_blocks);
	
	px = location%x_blocks;
	py = location/x_blocks;
	
	return (px >= x1 && px <= x2 && py >= y1 && py <= y2);
}




int is_in_window(point P) {

	return (P.x >= 0 && P.x <= x_limit && P.y >= 0 && P.y <= y_limit);
}




int is_in_window(Point P) {

	return (P.h >= 0 && P.h <= x_limit && P.v >= 0 && P.v <= y_limit);
}




int is_in_window(int location) {

	return (location >= x_blocks && location < x_blocks*y_blocks); // *** no sure about this, might need to check x-value ***
}





int which_box(Point click) {	
		
	if (click.h < 0 && click.v < 0)
		return box_clicked();
	
	return search_for_box(click.h, click.v);
}




int which_box(point click) {	
			
	if (click.x < 0 && click.y < 0)
		return box_clicked();
	
	return search_for_box((int)click.x, (int)click.y);
}




int which_box_g(point click) {	
	
	return search_for_box((int)click.x, (int)click.y);
}




int search_for_box(int x, int y) {

	int i(0), j(0);

	if (y > 0)
		i = y_screen*y/(y_limit - 2);
	
	if (i >= y_screen - 1)
		i = y_screen - 1;		
		
	if(x > 0)
		j = x_screen*x/(x_limit - 2);
	
	if (j >= x_screen - 1)
		j = x_screen - 1;	
		
	return (int)(x_blocks*(i + sch_origin_grid_y) + j + sch_origin_grid_x);
}




void set_buttons(int boxes, int grid_color, int is_schematic_window) {
	
	double aspect_ratio = window_ratio, temp;
	
	if ((!ar_changed || !realloc_with_aspect_ratio) && is_schematic_window && saveBOXES && new_num.v_boxes > 2 && new_num.h_boxes > 6 && new_num.h_boxes*new_num.v_boxes < max_total_boxes) {
		x_blocks = new_num.h_boxes;
		y_blocks = new_num.v_boxes;
	}
	else if (boxes == 0) {
		x_blocks = 0;
		y_blocks = 0;
	}
	else {
		if (realloc_with_aspect_ratio) {
			aspect_ratio *= DW_ratio;
		}
		ar_changed = 0;
		temp     = ceil(sqrt(abs(boxes)/aspect_ratio));
		y_blocks = (int)temp;
		x_blocks = (int)ceil(abs(boxes)/temp);
	}
	if (!is_schematic_window) {
		x_screen = x_blocks;
		y_screen = y_blocks;
	}
	if (grid_color > 0)	{
		draw_grid(grid_color);
	}
}




void init_buttons(int is_schematic_window) {

	whiteout();
	set_buttons(num_buttons, grid_color, is_schematic_window);
	draw_special();
}





void init_menu(int number, int color, int is_schematic_window) {

	if (number < 1)
		number = x_screen*y_screen;

	whiteout();	
	set_buttons(number, color, is_schematic_window);	
	o_size = (int)(object_size/sqrt(number + 3.0));
}




int transform_DW_point(int position, int new_x, int old_x) {
	
	return position%old_x + new_x*(position/old_x);
}


int scale_DW_point(int position, int scale_factor) {

	if (scale_factor > 1)
		return position*scale_factor;
	else if (scale_factor < 1)
		return -position/scale_factor;
		
	return position;
}




int is_vertical (int position1, int position2) {

	return (position1%x_blocks == position2%x_blocks);
}


int is_horizontal (int position1, int position2) {

	return (position1/x_blocks == position2/x_blocks);
}


int is_diagonal (int position1, int position2) {

	return (!is_horizontal(position1, position2) && !is_vertical(position1, position2));
}




int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2) {

	int lx2_t, lx1_t;
	double slope;

	if ((ly1 >= by2 && ly2 >= by2) || (ly1 <= by1 && ly2 <= by1) ||
		(lx1 >= bx2 && lx2 >= bx2) || (lx1 <= bx1 && lx2 <= bx1))
		return 0; // completely out of box
	
	if (ly1 >= by1 && ly2 >= by1 && ly1 <= by2 && ly2 <= by2 &&
		lx1 >= bx1 && lx2 >= bx1 && lx1 <= bx2 && lx2 <= bx2)
		return 1; // completely in box

	if (ly1 > ly2) {
		swap(ly1, ly2);
		swap(lx1, lx2);
	} // ly2 > ly1	
	if (lx1 != lx2 && ly1 != ly2) {
		slope = ((double)(ly2 - ly1))/((double)(lx2 - lx1));
		lx1_t = lx1;
		lx2_t = lx2;
				
		if (ly2 > by2) {
			lx2 -= (int)((ly2 - by2)/slope + 0.5);
		}
		if (ly1 < by1) {
			lx1 += (int)((by1 - ly1)/slope + 0.5);
		}
		if (lx2 > bx2) {
			ly2 -= (int)((lx2_t - bx2)*slope + 0.5);
		}
		if (lx1 > bx2) { // x1 > x2
			ly1 -= (int)((lx1_t - bx2)*slope + 0.5);
		}
		if (lx1 < bx1) {
			ly1 += (int)((bx1 - lx1_t)*slope + 0.5);
		}
		if (lx2 < bx1) { // x1 > x2
			ly2 += (int)((bx1 - lx2_t)*slope + 0.5);
		}
	}
	lx1 = max(bx1, min(bx2, lx1));
	lx2 = max(bx1, min(bx2, lx2));
	ly1 = max(by1, min(by2, ly1));
	ly2 = max(by1, min(by2, ly2));

	return (lx1 != lx2 || ly1 != ly2);
}




int clip_rect_by_DW(int &x1, int &y1, int &x2, int &y2, int frame) {

	return clip_rect_by_box(x1, y1, x2, y2, 0, 0, x_limit, y_limit, frame);
}




int clip_rect_by_box(int &x1, int &y1, int &x2, int &y2, int bx1, int by1, int bx2, int by2, int frame) {

	if (x1 > x2)
		swap(x1, x2);
	
	if (y1 > y2)
		swap(y1, y2);
		
	if (x2 < bx1 || y2 < by1 || x1 > bx2 || y1 > by2)
		return 0;
		
	if (x1 < bx1) x1 = bx1 - frame;
	if (y1 < by1) y1 = by1 - frame;
	if (x2 > bx2) x2 = bx2 + frame;
	if (y2 > by2) y2 = by2 + frame;

	return (x1 != x2 || y1 != y2);
}



int point_in_box(int x, int y, int bx1, int by1, int bx2, int by2) {

	return (x >= bx1 && x <= bx2 && y >= by1 && y <= by2);
}




int clip_text(point &c, int bx1, int by1, int bx2, int by2, int shift_dist) {

	if (c.x < bx1 && c.x > bx1 - shift_dist)
		c.x = bx1;

	if (c.y < by1 && c.y > by1 - shift_dist)
		c.y = by1;

	if (c.x > bx2 && c.x < bx2 + shift_dist)
		c.x = bx2;

	if (c.y > by2 && c.y < by2 + shift_dist)
		c.y = by2;

	return point_in_box((int)c.x, (int)c.y, bx1, by1, bx2, by2);
}




void clipping_check() {

	char str[MAX_CHARACTERS], ch;
	int bx1(0), by1(0), bx2(x_limit), by2(y_limit), color;
	Point pt1, pt2;
	point c;

	Window_setup(1);
	set_buttons(1, NO_COLOR, 1);

	while (1) {
		cout << "Enter a command: Clip (B)ox, (P)oint, (L)ine, (R)ectangle, (T)ext, (C)lear, (Q)uit:  ";
		ch = inc();

		switch (ch) {
		case 'b':
		case 'B':
			cout << "Click on one corner of the clipping box." << endl;
			pt1 = GetClick();
			cout << "Click on the other corner of the clipping box." << endl;
			pt2 = GetClick();
			bx1 = min(pt1.h, pt2.h);
			bx2 = max(pt1.h, pt2.h);
			by1 = min(pt1.v, pt2.v);
			by2 = max(pt1.v, pt2.v);
			SetDrawingColorx(BLACK);
			RectangleFrame(bx1, by1, bx2, by2);
			break;

		case 'p':
		case 'P':
			cout << "Click on the location of the point." << endl;
			pt1 = GetClick();
			if (point_in_box(pt1.h, pt1.v, bx1, by1, bx2, by2)) {
				color = BLUE;
				cout << "Point is inside of clipping box." << endl;
			}
			else {
				color = RED;
				cout << "Point is outside of clipping box." << endl;
			}
			ColorPixel(color, pt1);
			break;

		case 'l':
		case 'L':
			cout << "Click on the starting point of the line." << endl;
			pt1 = GetClick();
			cout << "Click on the ending point of the line." << endl;
			pt2 = GetClick();
			SetDrawingColorx(RED);
			LineDraw(pt1.h, pt1.v, pt2.h, pt2.v);
			if (clip_line_to_box(pt1.h, pt1.v, pt2.h, pt2.v, bx1, by1, bx2, by2)) {
				SetDrawingColorx(BLUE);
				LineDraw(pt1.h, pt1.v, pt2.h, pt2.v);
			}
			else {
				cout << "Line is outside of clipping box." << endl;
			}
			break;

		case 'r':
		case 'R':
			cout << "Click on one corner of the rectnagle." << endl;
			pt1 = GetClick();
			cout << "Click on the other corner of the rectangle." << endl;
			pt2 = GetClick();
			SetDrawingColorx(RED);
			RectangleFrame(pt1.h, pt1.v, pt2.h, pt2.v);
			if (clip_rect_by_box(pt1.h, pt1.v, pt2.h, pt2.v, bx1, by1, bx2, by2, 0)) {
				SetDrawingColorx(BLUE);
				RectangleFrame(pt1.h, pt1.v, pt2.h, pt2.v);
			}
			else {
				cout << "Rectangle is of zero size or outside of clipping box." << endl;
			}
			break;

		case 't':
		case 'T':
			cout << "Enter the text string beginning with a space:  ";
			strcpy(str, xin(MAX_CHARACTERS));
			cout << "Click on the location of the text." << endl;
			pt1 = GetClick();
			c.x = pt1.h;
			c.y = pt1.v;
			MoveTo(c.x, c.y);
			SetDrawingColorx(RED);
			draw_string(str);
			if (clip_text(c, bx1, by1, bx2, by2, 20)) {
				MoveTo(c.x, c.y);
				SetDrawingColorx(BLUE);
				draw_string(str);
			}
			else {
				cout << "Text outside of clipping box." << endl;
			}
			break;

		case 'c':
		case 'C':
			whiteout();
			SetDrawingColorx(BLACK);
			RectangleFrame(bx1, by1, bx2, by2);
			break;

		case 'q':
		case 'Q':
			return;

		default:
			cerr << "Illegal selection!" << endl;
		}
	}
}









