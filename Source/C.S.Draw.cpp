#include "CircuitSolver.h"
#include "Draw.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Draw.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Drawing
// By Frank Gennari
double const sqrt_2 = sqrt(2.0);
double const offset = 0.96;
//double const offset = (1/sqrt_2 + 1/2.95);
int const scroll_time = 10000;
string const ff_short_names[num_ffs+1] = {" FF", " RS L", " RS FF", "D FF", " TD FF", " JK FF", " MS FF", " T FF"};


int o_size, text_size(12), need_node(1), drawing_color(BLACK), dont_whiteout_double(0), whiteout_text_bkg(1), looks_like_IC(1);
double DW_ratio = 1.0;

extern int graph_enable, x_limit, y_limit, object_size, num_buttons, EDGE, clock_speed, pen_style, line_weight;
extern int sch_origin_x, sch_origin_y, sch_origin_grid_x, sch_origin_grid_y;
extern int x_blocks, y_blocks, x_screen, y_screen;
extern double supply_voltage;


void draw_fourth_input(int box_num);


point get_center(int box_num);
int is_in_window(point P);
int is_in_window(Point P);
int clip_line_to_box(int &lx1, int &ly1, int &lx2, int &ly2, int bx1, int by1, int bx2, int by2);
int clip_text(point &c, int bx1, int by1, int bx2, int by2, int shift_dist);

void label_box(int location, char label);
void label_io(int location, int ID, int use_alpha, char name_char);
string get_name_of_ff(int id);
string shift_chars(string text);

void set_text_size(int size);

double get_angle(double x, double y);





void draw_grid(int grid_color) {
	
	unsigned int i;
	
	if (!graph_enable) {
		Window_setup(0);
		set_text_size(0);
		graph_enable = 1;
	}
	if (grid_color == 0 || (x_blocks == 0 && y_blocks == 0))
		return;

	if (x_screen == 0)
		x_screen = x_blocks;
	if (y_screen == 0)
		y_screen = y_blocks;

	SetDrawingColorx(grid_color);
		
	MoveTo(1, 1);
	LineTo((x_limit - 2), 1);
	LineTo((x_limit - 2), (y_limit - 2));
	LineTo(1, (y_limit - 2));
	LineTo(1, 1);
	
	if (x_screen >= x_limit/2 || y_screen >= y_limit/2) {
		return; // resolution too high, will color entire window with grid
	}
	for (i = 1; i <= (unsigned)(y_screen - 1); ++i) {
		LineDraw(1, ((i*(y_limit - 2))/y_screen), (x_limit - 2), ((i*(y_limit - 2))/y_screen));
	}
	for (i = 1; i <= (unsigned)(x_screen - 1); ++i) {
		LineDraw(((i*(x_limit - 2))/x_screen), 1, ((i*(x_limit - 2))/x_screen), (y_limit - 2));
	}
}




void draw_text(string text, int box_num, int t_size) {
	
	char display[MAX_CHARACTERS + 1] = {0};
	
	if (x_blocks == 0 || y_blocks == 0)
		return;

	string_to_char(display, text, MAX_CHARACTERS);
	
	char *display2 = convert_to_constparam255(display);
		
	if (t_size > 0) {
		text_size = t_size;
		set_text_size(t_size);
	}
	else 
		set_text_size(-t_size);
		
	print_to_window(display2, box_num);	
	free(display2);

	if (t_size > 0) {
		text_size = 12;
	}
	set_text_size(12);
}




void draw_double(double num, char label, int box_num) {

	char display[MAX_CHARACTERS + 1] = {0}, display2[MAX_CHARACTERS + 1] = {0};
	int count;
	point c;
		
	strcpy(display, double_to_char(num));
	
	count = shift_and_remove_zeros(display, display2);
	
	if(count > MAX_CHARACTERS - 5) {
		internal_error();
		cerr << "Error in double to string conversion!" << endl;
		return;
	}
	if (label == 'H') {
		display2[count + 1] = ' ';
		display2[count + 2] = 'M';
		display2[count + 3] = 'H';
		display2[count + 4] = 'z';
		display2[count + 5] = 0;
	}
	else if (label == 'X')
		display2[count + 1] = 0;
	
	else if (label == 'x')
		display2[count - 1] = 0;
	
	else {
		display2[count + 1] = ' ';
		display2[count + 2] = label;
		display2[count + 3] = 0;
	}
	char *display3 = convert_to_constparam255(display2);
	
	c = get_center(box_num);
	
	set_text_size(12);
	text_size = 12;
	
	if (text_size != 0 && !dont_whiteout_double) {
		SetDrawingColorx(WHITE);
		RectangleDraw((c.x - 64), (c.y - 16), (c.x + 64), (c.y + 16));
	}	
	print_to_window(display3, box_num);
	free(display3);
}




void draw_char(char x, int box_num, int color, int size) {
	
	unsigned char ch[2] = {0};

	if (x_screen == 0 || y_screen == 0)
		return;
	
	ch[0] = 1;
	ch[1] = x;
	
	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x += o_size/5;
	
	set_text_size(size);
	
	if (color != -1)
		SetDrawingColorx(color);
		
	MoveTo((c.x - x_limit/(4*x_screen)), (c.y + y_limit/(3*y_screen)));
	draw_string2(ch);
	set_text_size(12);	
}




void label_box(int location, char label) {
	
	draw_char(label, location, -1, 12);	
}




void label_io(int location, int ID, int use_alpha, char name_char) {

	unsigned i, ndigits;
	unsigned char labelx[12] = {0};

	if (x_blocks == 0 || y_blocks == 0 || x_screen == 0 || y_screen == 0)
		return;

	if (use_alpha == 1) // input
		draw_char((char)('A' + (ID%MAX_INPUT_LETTER)), location, 10, 12);
	else {
		if (use_alpha == 0) { // input or output
			if (ID == 0 || ID == 1) {
				ndigits = 1;
			}
			else {
				ndigits = (unsigned)ceil(log((double)ID)/log(10.0));
			}
			labelx[0] = (char)(ndigits+1);
			labelx[1] = name_char;

			for (i = 1; i <= ndigits; ++i) {
				labelx[ndigits-i+2] = (char)('0' + (ID%10));
				ID /= 10;
			}
		}
		else { // supply voltage
			if (supply_voltage > 0)
				labelx[1] = '+';
			else if (supply_voltage < 0)
				labelx[1] = '-';
			else
				labelx[1] = ' ';
			if (supply_voltage - int(supply_voltage) == 0 && fabs(supply_voltage) < 10) {
				labelx[0] = 3;
				labelx[2] = (char)('0' + abs(roundx(supply_voltage)));
				labelx[3] = 'V';
			}
			else {
				labelx[0] = 2;
				labelx[2] = 'V';
			}
		}
		point c = get_center(location);
	
		if (location > num_menu_boxes) {
			MoveTo((c.x - x_limit/(4*x_screen)), (c.y + y_limit/(3*y_screen)));
			SetDrawingColorx(BLACK);
			draw_string2(labelx);
		}
	}
}




void draw_box_outline(int color, int box1, int box2) {

	point c1 = get_center(box1), c2 = get_center(box2);
	
	SetDrawingColorx(color);
	RectangleFrame(c1.x, c1.y, c2.x, c2.y);
}





void color_box(int box_num, int color) { // not used yet

	point c = get_center(box_num);
	
	draw_grid(10);
	
	SetDrawingColorx(color);
	
	if (box_num >= 0)
		RectangleDraw((c.x - x_limit/x_screen/2), (c.y - y_limit/y_screen/2), (c.x + x_limit/x_screen/2), (c.y + y_limit/y_screen/2));
}




void number_all_boxes(int viewable_only) { // not used
	
	if (viewable_only) { // *** might need x/y offset ***
		for (int i = 0; i < x_screen*y_screen; ++i) {		
			draw_char((char)('0' + (i%10)), i%10, 10, 12);
		}
	}
	else {
		for (int i = 0; i < x_blocks*y_blocks; ++i) {		
			draw_char((char)('0' + (i%10)), i%10, 10, 12);
		}
	}
}




void print_to_window(char *display, int box_num) {

	int size, x1, x2, y1, y2;
	point c;
	
	size = (int)(strlen(display)*0.6667*text_size);
	
	if (box_num < 0) {
		if (box_num == reserved_pos_value)
			box_num = 0;
			
		c = get_center(-box_num);
		c.x = c.x - size/2;
		c.y = c.y - 10;
	}
	else {
		c = get_center(box_num);
		c.x = c.x - 2*o_size + 12;
		c.y = c.y - 1.25*o_size - 5;
	}	
	if (o_size < 0 && box_num > 0) {
		c.x -= 4;
		c.x += o_size/5;
	}
	c.x = min(c.x, (double)x_limit);
	c.y = min(c.y, (double)y_limit);

	if (text_size == 10) {
		size = (int)(size*1.2);		 
	}
	c.x += 4;
	c.y += 15;
	
	if(clip_text(c, 0, 0, x_limit, y_limit, 20)) {	
		if (whiteout_text_bkg) {
			x1 = max((int)c.x, 1);
			x2 = min((int)(c.x + .89*size - 5), x_limit - 2);
			y1 = max((int)(c.y - 10), 1);
			y2 = min((int)(c.y + 1), y_limit - 2);
			SetDrawingColorx(WHITE);
			RectangleDraw(x1, y1, x2, y2);
		}
		SetDrawingColorx(drawing_color);		
		MoveTo(c.x, c.y);	
		draw_string2(display);
	}
}




void draw_custom_ff(int id, int box_num) {
	
	connections x;
	
	x.in1  = 1;
	x.in2  = 2;
	x.in3  = 3;
	x.out1 = 4;
	x.out2 = 0;
	x.out3 = 5;
	
	draw_FF(box_num, id);
		
	switch (id) {
		
	case RSL_ID:
		x.in2  = 0;
		x.in3  = 2;
		x.out1 = 3;
		x.out3 = 4;
		label_ff('R', 'x', 'S', 'Q', 'x', 'q', id);
		break;
	
	case RSFF_ID:
		label_ff('R', 'c', 'S', 'Q', 'x', 'q', id);
		break;
		
	case DFF_ID:
		x.in3  = 0;
		x.out1 = 3;
		x.out3 = 4;
		label_ff('D', 'c', 'x', 'Q', 'x', 'q', id);
		break;
		
	case TDFF_ID:
		x.in3  = 0;
		x.out1 = 3;
		x.out3 = 4;
		label_ff('D', 'E', 'x', 'Q', 'x', 'q', id);
		break;
		
	case JKFF_ID:
		label_ff('J', 'c', 'K', 'Q', 'x', 'q', id);
		break;
		
	case MSFF_ID:
		label_ff('J', 'c', 'K', 'Q', 'x', 'q', id);
		break;
		
	case TFF_ID:
		x.in3  = 0;
		x.out1 = 3;
		x.out3 = 4;
		label_ff('T', 'c', 'x', 'Q', 'x', 'q', id);
		break;

	default:
		return;
	}
	draw_text(get_name_of_ff(id), id, 12);
	number_pins(x, box_num);
}




void draw_FF_name(int id, int box_num) {
	
	if (id <= num_ffs)
		draw_text(ff_short_names[id], -(box_num - x_blocks), 10);
	
	else if (id > num_ffs && id <= 2*num_ffs) 
		draw_FF_name(id - num_ffs, box_num);
}




void number_pins(const connections& x, int box_num) {

	point c = get_center(box_num);
	
	c.y += o_size/8;
	
	SetDrawingColorx(drawing_color);
	
	if (x.in1 != 0) {
		MoveTo((c.x - o_size - 24), (c.y - o_size/2));
		DrawInt(x.in1);
	}
	if (x.in2 != 0) {
		MoveTo((c.x - o_size - 24), c.y);
		DrawInt(x.in2);
	}
	if (x.in3 != 0) {
		MoveTo((c.x - o_size - 24), (c.y + o_size/2));
		DrawInt(x.in3);
	}
	if (x.out1 != 0) {
		MoveTo((c.x + o_size + 2), (c.y - o_size/2));
		DrawInt(x.out1);
	}
	if (x.out2 != 0) {
		MoveTo((c.x + o_size + 2), c.y);
		DrawInt(x.out2);
	}
	if (x.out3 != 0) {
		MoveTo((c.x + o_size + 2), (c.y + o_size/2));
		DrawInt(x.out3);
	}
}




void label_ff(char A, char B, char C, char D, char E, char F, int box_num) {
	
	char x[3];
	
	x[0] = 1;	
	point c = get_center(box_num);	
	c.y += o_size/8;
	
	SetDrawingColorx(drawing_color);
	
	if (A != 'x') {
		MoveTo((c.x - o_size/12 - 24), (c.y - o_size/2));
		x[1] = A;
		draw_string2(x);
	}
	if (B != 'x') {
		if (B == 'c') {
			MoveTo((c.x - o_size/15 - 12), c.y);
			x[1] = 'C';
			draw_string2(x);
			MoveTo((c.x - o_size/15 - 4), c.y);
			x[1] = 'K';
			draw_string2(x);
			draw_CK(box_num);
		}
		else {
			x[1] = B;
			if (B == 'E') 
				MoveTo((c.x - o_size/12 - 24), c.y);
			else
				MoveTo((c.x - o_size/12 - 24), c.y);
			draw_string2(x);
			
		}
	}
	if (C != 'x') {
		MoveTo((c.x - o_size/12 - 24), (c.y + o_size/2));
		x[1] = C;
		draw_string2(x);
	}
	if (D != 'x') {
		if (D == 'q') {
			MoveTo((c.x + o_size/1.5), (c.y - o_size/2));
			x[1] = 'Q';
			draw_string2(x);
			MoveTo((c.x + o_size/1.5), (c.y - o_size/2 - 12));
			x[1] = '_';
			draw_string2(x);
		}
		else {
			MoveTo((c.x + o_size/1.5), (c.y - o_size/2));
			x[1] = D;
			draw_string2(x);
		}
	}
	if (E != 'x') {
		if (E == 'q') {
			MoveTo((c.x + o_size/1.5), c.y);
			x[1] = 'Q';
			draw_string2(x);
			MoveTo((c.x + o_size/1.5), (c.y - 12));
			x[1] = '_';
			draw_string2(x);
		}
		else {
			MoveTo((c.x + o_size/1.5), c.y);
			x[1] = E;
			draw_string2(x);
		}
	}
	if (F != 'x') {
		if (F == 'q') {
			MoveTo((c.x + o_size/1.5), (c.y + o_size/2));
			x[1] = 'Q';
			draw_string2(x);
			MoveTo((c.x + o_size/1.5), (c.y + o_size/2 - 12));
			x[1] = '_';
			draw_string2(x);
		}
		else {
			MoveTo((c.x + o_size/1.5), (c.y + o_size/2));
			x[1] = F;
			draw_string2(x);
		}
	}
}




void draw_selection_window(int count) {

	init_menu(MAX_BOXES, 10, 0);	
	draw_text(" BACK ", reserved_pos_value, 12);
	draw_LARROW(0);
		
	if (count == (MAX_BOXES - 2)) {	
		draw_text(" MORE ", -(count + 1), 12);
		draw_RARROW(count + 1);
	}
	else 
		draw_text(" RETURN TO MENU ", -(count + 1), 12);
}



void scroll_text(string text, int scroll_length) {
	
	Point P;
	char temp;
	int i, j, len = max(scroll_length, (int)text.length()), dlen;
	string scrolled_text(len, ' ');
	string output_text(scroll_length, ' ');
	
	len = min(MAX_CHARACTERS, len);
	dlen = len - text.length();
	
	draw_status_bar();
	
	for (i = dlen; i < len; ++i) {
		scrolled_text[i] = text[i - dlen];
	}
	for (i = 0; i < scroll_time; ++i) {
		for (j = 0; j < clock_speed/15; ++j) {
			if (interrupt() || FoundAClick(&P))
				return;
		}
		for (j = 0; j < scroll_length; ++j) {
			output_text[j] = scrolled_text[j];
		}
		SetDrawingColorx(WHITE);
		RectangleDraw(((x_limit*(num_menu_boxes + 2))/x_screen - 9), y_limit/y_screen - 2, (x_limit - 2), y_limit/y_screen);
		draw_status(output_text);
		temp = scrolled_text[0];
		
		for (j = 1; j < len; ++j) {
			scrolled_text[j-1] = scrolled_text[j];	
		}
		scrolled_text[j-1] = temp; 
	}
}




void draw_status_bar() {
	
	if (x_screen == 0 || y_screen == 0)
		return;

	SetDrawingColorx(WHITE);
	RectangleDraw(((x_limit*(num_menu_boxes + 2))/x_screen - 10), 0, (x_limit - 1), y_limit/y_screen + 1);
	
	SetDrawingColorx(BLACK);
	RectangleFrame(((x_limit*(num_menu_boxes + 2))/x_screen - 10), 0, (x_limit - 1), y_limit/y_screen + 1);
}




void draw_status(const string status) {

	char display[MAX_CHARACTERS + 1] = {0};
	string status2;

	status2 = shift_chars(status);	
	string_to_char(display, status2, MAX_CHARACTERS);	
	draw_status(display);
}




void draw_status(const char *status) {
	
	if (x_blocks == 0 || y_blocks == 0)
		return;

	char *status2 = convert_to_constparam255(status);	// left over from MacOS->Win32 port
	print_to_window(status2, -((int)(num_menu_boxes + strlen(status)/4 + 2)));

	free(status2);
}




void draw_NO(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));

	SetDrawingColorx(RED);
	draw_RED_CIRCLE(box_num);
	LineDraw((c.x - 1.05*o_size/sqrt_2 + sign_os), (c.y + 1.05*o_size/sqrt_2), (c.x + 1.05*o_size/sqrt_2 - sign_os), (c.y - 1.05*o_size/sqrt_2));
	LineDraw((c.x - 1.05*o_size/sqrt_2 + sign_os), (c.y + 1.05*o_size/sqrt_2 - sign_os), (c.x + 1.05*o_size/sqrt_2 - sign_os), (c.y - 1.05*o_size/sqrt_2 - sign_os));
}




void draw_RED_CIRCLE(int box_num) {

	point c = get_center(box_num);

	SetDrawingColorx(RED);
	CircleFrame(c.x, c.y, (int)(1.05*o_size));
	CircleFrame(c.x, c.y, (int)(1.05*o_size + sign(o_size)));
}




void draw_blue_x(int location) {

	draw_char('X', location, 3, 16);
}




void draw_SQUAREWAVE(int box_num) {

	point c = get_center(box_num);
	
	c.x -= o_size/5;
	
	SetDrawingColorx(BLUE);
	MoveTo((c.x - o_size/3), (c.y + o_size/10));
	LineTo((c.x - o_size/7), (c.y + o_size/10));
	LineTo((c.x - o_size/7), (c.y - o_size/5));
	LineTo((c.x + o_size/7), (c.y - o_size/5));
	LineTo((c.x + o_size/7), (c.y + o_size/10));
	LineTo((c.x + o_size/3), (c.y + o_size/10));
}




void draw_SINEWAVE(int box_num) {
	
	point c = get_center(box_num);
		
	SetDrawingColorx(PINK);
	MoveTo((c.x - o_size/3.5), c.y);
	
	for (int x = 0; x < 20; ++x) 
		LineTo((c.x + (x - 10)*o_size/35), (c.y - o_size/4.5*(sin(x*pi/10))));
}




void draw_STATE_SELECTION(int box_num) { 

	point c = get_center(box_num);
	
	draw_CUSTOMWIRE(box_num);
	
	SetDrawingColorx(LTGREY);
	set_text_size((int)(180/sqrt((double)(x_screen*y_screen))));
	
	MoveTo((c.x - 2*o_size), (c.y - o_size/8));
	DrawInt(1);
	
	MoveTo((c.x - o_size), (c.y + o_size/1.5));
	DrawInt(0);
	
	set_text_size(12);
}




void draw_NOT0(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));

	SetDrawingColorx(WHITE);
	CircleDraw((c.x + 1.167*o_size - sign_os*2), c.y, (o_size/8 + sign_os));
	SetDrawingColorx(drawing_color);
	CircleFrame((c.x + 1.167*o_size - sign_os*2), c.y, (o_size/8 + sign_os));
}




void draw_BUFFER(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	MoveTo((c.x + o_size), c.y);
	LineTo((c.x - o_size/sqrt_2), (c.y + o_size/sqrt_2));
	LineTo((c.x - o_size/sqrt_2), (c.y - o_size/sqrt_2));
	LineTo((c.x + o_size), c.y);	
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/sqrt_2), c.y);
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y); 
}




void draw_NOT(int box_num) {

	point c = get_center(box_num);
	
	draw_BUFFER(box_num);
	SetDrawingColorx(WHITE);
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y);
	draw_NOT0(box_num);
}




void draw_AND(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/sqrt_2), (c.y + o_size/2));
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/sqrt_2), (c.y - o_size/2));
	LineDraw((c.x + o_size - 1 + sign_os), c.y, (c.x + 1.2*o_size), c.y);
	MoveTo((c.x + o_size/(2*sqrt_2)), (c.y + o_size/sqrt_2 - sign_os));
	LineTo((c.x - o_size/sqrt_2), (c.y + o_size/sqrt_2 - sign_os));
	if (o_size < 0) {
		LineTo((c.x - o_size/sqrt_2), (c.y - o_size/sqrt_2 + sign_os));
		LineTo((c.x + o_size/(2*sqrt_2)), (c.y - o_size/sqrt_2 + sign_os));
	}
	else {
		LineTo((c.x - o_size/sqrt_2), (c.y - o_size/sqrt_2));
		LineTo((c.x + o_size/(2*sqrt_2)), (c.y - o_size/sqrt_2));
	}
	ArcFrame((c.x + o_size/(2*sqrt_2)), c.y, (int)(o_size/sqrt_2), 0, (sign_os*180)); 
}




void draw_NAND(int box_num) {

	point c = get_center(box_num);
	
	draw_AND(box_num);
	SetDrawingColorx(WHITE);
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y);
	draw_NOT0(box_num);
}




void draw_OR(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));
	
	SetDrawingColorx(drawing_color);
	
	if (o_size < 0)
		c.x -= 1;
	
	ArcFrame((c.x - o_size), c.y, (int)(o_size/sqrt_2), (sign_os*30), (sign_os*125));
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/1.75 - sign_os), (c.y + o_size/2));
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/1.75), (c.y - o_size/2));
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y);
	LineDraw((c.x - o_size/1.5), (c.y + o_size/1.68), (c.x - o_size/6 + sign_os*2), (c.y + o_size/1.68));
	LineDraw((c.x - o_size/1.5), (c.y - o_size/1.68), (c.x - o_size/6 + sign_os*2), (c.y - o_size/1.68));  
	ArcFrame((int)(c.x - o_size/2.5), (int)(c.y + 1.225*abs(o_size) + 1), (int)(1.9*o_size), sign_os*10, sign_os*39);
	ArcFrame((int)(c.x - o_size/2.5), (int)(c.y - 1.225*abs(o_size)), (int)(1.9*o_size), sign_os*170, -sign_os*40);
}




void draw_NOR(int box_num) {

	point c = get_center(box_num);
	
	draw_OR(box_num);
	SetDrawingColorx(WHITE);
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y);
	draw_NOT0(box_num);
}




void draw_XOR(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_OR(box_num);
	SetDrawingColorx(WHITE);
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/1.65 - sign_os), (c.y + o_size/2));
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/1.65), (c.y - o_size/2));
	SetDrawingColorx(drawing_color);
	ArcFrame((c.x - o_size - o_size/5), c.y, (int)(o_size/sqrt_2), (sign_os*30), (sign_os*125));
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - 0.776*o_size - sign_os), (c.y + o_size/2));
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - 0.776*o_size), (c.y - o_size/2));
}




void draw_XNOR(int box_num) {

	point c = get_center(box_num);
	
	draw_XOR(box_num);
	SetDrawingColorx(WHITE);
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y);
	draw_NOT0(box_num);
} 




void draw_3AND(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_AND(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/sqrt_2), c.y);
}





void draw_3NAND(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_NAND(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/sqrt_2), c.y);
}





void draw_3OR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_OR(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - 0.293*o_size - sign(o_size)*2), c.y);
}





void draw_3NOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_NOR(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - 0.293*o_size - sign(o_size)*2), c.y);
}





void draw_3XOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_XOR(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - 0.493*o_size - sign(o_size)), c.y);
}




void draw_3XNOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	draw_XNOR(box_num);
	LineDraw((c.x - o_size/offset), c.y, (c.x - 0.493*o_size - sign(o_size)), c.y);
}




void draw_4AND(int box_num) {

	draw_3AND(box_num);
	draw_fourth_input(box_num);
}




void draw_4NAND(int box_num) {

	draw_3NAND(box_num);
	draw_fourth_input(box_num);
}




void draw_4OR(int box_num) {

	draw_3OR(box_num);
	draw_fourth_input(box_num);
}




void draw_4NOR(int box_num) {

	draw_3NOR(box_num);
	draw_fourth_input(box_num);
}




void draw_4XOR(int box_num) {

	draw_3XOR(box_num);
	draw_fourth_input(box_num);
}




void draw_4XNOR(int box_num) {

	draw_3XNOR(box_num);
	draw_fourth_input(box_num);
}




void draw_fourth_input(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y - o_size), (c.x - 0.71*o_size), (c.y - o_size));
	LineDraw((c.x - 0.71*o_size), (c.y - 1.125*o_size), (c.x - 0.71*o_size), (c.y - 0.58*o_size));
}




void draw_TRI_STATE_BUFFER(int box_num) {
	
	draw_BUFFER(box_num);
	draw_TRI_STATE(box_num);
}




void draw_TRI_STATE_INVERTER(int box_num) {
	
	draw_NOT(box_num);
	draw_TRI_STATE(box_num);
}




void draw_TRI_STATE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y - o_size), c.x, (c.y - o_size)); // Control
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/2.5));
	
	SetDrawingColorx(WHITE);
	CircleDraw(c.x, (c.y - o_size/2), (o_size/8 + sign(o_size)));
	SetDrawingColorx(drawing_color);
	CircleFrame(c.x, (c.y - o_size/2), (o_size/8 + sign(o_size)));
}




void draw_MUX(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/3), (c.y + o_size/2)); // Inputs
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/3), (c.y - o_size/2));
	
	LineDraw((c.x - o_size/offset), (c.y - o_size - sign_os), c.x, (c.y - o_size - sign_os)); // Control
	LineDraw(c.x, (c.y - o_size - sign_os), c.x, (c.y - o_size/1.4));
	
	LineDraw((c.x + o_size/3), c.y, (c.x + 1.2*o_size), c.y); // Output
	
	LineDraw((c.x - o_size/3), (c.y - o_size/1.1), (c.x - o_size/3), (c.y + o_size/1.1)); // Mux
	LineDraw((c.x + o_size/3), (c.y - o_size/2), (c.x + o_size/3), (c.y + o_size/2));
	LineDraw((c.x - o_size/3), (c.y - o_size/1.1), (c.x + o_size/3), (c.y - o_size/2));
	LineDraw((c.x - o_size/3), (c.y + o_size/1.1), (c.x + o_size/3), (c.y + o_size/2));
	
	CircleFrame((int)(c.x - o_size/8), (int)(c.y - o_size/2), (int)(o_size/16 + sign_os));
	LineDraw((c.x - o_size/8), (c.y + o_size/2.4), (c.x - o_size/8), (c.y + o_size/1.7));
}




void draw_ADDER(int box_num) {

	point c = get_center(box_num);
	int sign_os(sign(o_size));
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/3), (c.y + o_size/2)); // Inputs
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/3), (c.y - o_size/2));
	
	LineDraw((c.x - o_size/offset), (c.y - o_size - sign_os), c.x, (c.y - o_size - sign_os)); // Cin
	LineDraw(c.x, (c.y - o_size - sign_os), c.x, (c.y - o_size/1.4));
	
	LineDraw((c.x + o_size/sqrt_2 + o_size/2), (c.y + o_size + sign_os), c.x, (c.y + o_size + sign_os)); // Cout
	LineDraw(c.x, (c.y + o_size + sign_os), c.x, (c.y + o_size/1.4));
	
	LineDraw((c.x + o_size/3), c.y, (c.x + 1.2*o_size), c.y); // Output
	 
	LineDraw((c.x - o_size/3), (c.y - o_size/1.1), (c.x - o_size/3), (c.y - o_size/3.5));	// Adder
	LineDraw((c.x - o_size/3), (c.y + o_size/3.5), (c.x - o_size/3), (c.y + o_size/1.1));
	
	LineDraw((c.x - o_size/3), (c.y - o_size/3.5), (c.x - o_size/12), c.y);
	LineDraw((c.x - o_size/12), c.y, (c.x - o_size/3), (c.y + o_size/3.5));
	
	LineDraw((c.x + o_size/3), (c.y - o_size/2), (c.x + o_size/3), (c.y + o_size/2));
	LineDraw((c.x - o_size/3), (c.y - o_size/1.1), (c.x + o_size/3), (c.y - o_size/2));
	LineDraw((c.x - o_size/3), (c.y + o_size/1.1), (c.x + o_size/3), (c.y + o_size/2));
	
	LineDraw((c.x + o_size/8), (c.y - o_size/8), (c.x + o_size/8), (c.y + o_size/8)); // |
	LineDraw(c.x, c.y, (c.x + o_size/4), c.y); // -
}




void draw_IC(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	RectangleFrame((c.x - 1.5*o_size), (c.y - 0.6*o_size), (c.x + 1.5*o_size), (c.y + 0.6*o_size));
	
	if (looks_like_IC)
		ArcFrame((c.x - 1.5*o_size), c.y, (o_size/5), 0, sign(o_size)*180);
}




void draw_IC_PINS(int box_num, int size, int num_pins, int add_nodes) {
	
	int count(1), count2(0), original_num(num_pins), x(0), cx_ps, node_loc, last_node_loc(-1);
	double pin_size(0);
	point c = get_center(box_num);
	Point node;
	
	if (num_pins < 0) 
		num_pins = -num_pins;
	
	if ((num_pins - 2)%4) 
		c.x -= size/2.82;	
	else 
		x = 1;
	
	SetDrawingColorx(drawing_color);

	for (num_pins /= 2; num_pins > 0; --num_pins) {	
		if (count%2) 
			pin_size = -pin_size;

		cx_ps = (int)(c.x + pin_size);
			
		LineDraw((cx_ps + 2), (int)(c.y - .5*size), (cx_ps + 2), (int)(c.y - .65*size));
		LineDraw((cx_ps + 2), (int)(c.y + .5*size), (cx_ps + 2), (int)(c.y + .65*size));
		
		if (add_nodes) {
			node.h = (cx_ps + 2);
			node.v = (int)(c.y - .65*size);
			
			if (is_in_window(node)) {
				node_loc = which_box(node);
				if (node_loc >= 0 && node_loc != last_node_loc)
					draw_NODE(node_loc);
				last_node_loc = node_loc;
			}
			node.v = (int)(c.y + .65*size);
			
			if (is_in_window(node))
				draw_NODE(which_box(node));
		}
		if (original_num < 0) {
			MoveTo((cx_ps - 8), (int)(c.y - .59*size));
			DrawInt((int)(-3*original_num/4 + count2 + 1));
		
			MoveTo((cx_ps - 8), (int)(c.y + .62*size + 5));
			DrawInt((int)(-original_num/4 - count2 + x));
		}
		if (count%2) {
			pin_size = -pin_size;
			pin_size += size/2.83;
			++count2;
		}
		++count;	
		count2 = -count2;
	}
}




void draw_LARGE_IC(int box_num, int size, int num_pins) {
	
	int ic_size;
	point c = get_center(box_num);
	
	ic_size = size*num_pins/15; 
	
	if ((num_pins - 2)%4) 
		c.x -= ic_size/9;
	else
		c.x += ic_size/12;
			
	SetDrawingColorx(drawing_color);
	RectangleFrame((c.x - 1.4*ic_size), (c.y - .5*size + sign(o_size)), (c.x + 1.36*ic_size), (c.y + .5*size));
	
	if (looks_like_IC) {
		ArcFrame((c.x - 1.4*ic_size), c.y, (o_size/6), 0, sign(o_size)*180);
	}
}




void draw_FULL_IC(int box_num, int num_pins) {

	int temp_size(o_size);
	
	o_size = (int)(o_size*1.55); // size of IC
	draw_LARGE_IC(box_num, (int)(1.06*o_size), num_pins); // size = width
	o_size = temp_size;
	draw_IC_PINS(box_num, (int)(1.608*o_size), num_pins, 1);
}




void draw_IC_ON_SCREEN(string name, int pins, string description) {
	
	int sign_os(sign(o_size));
	int num_boxes(default_num_buttons);

	if (y_blocks > 0) {
		num_boxes = x_blocks*y_blocks;
		if (!(y_blocks%2))
			num_boxes += x_blocks;
	}
	if (num_boxes == default_num_buttons)
		num_boxes -= 7; // Simple fix for when I changed default_num_buttons from 1000 to 1007.
			
	o_size = (int)(30*object_size/sqrt(x_screen*y_screen + 4.0));
			
	draw_LARGE_IC(num_boxes/2, (int)(1.25*o_size), pins);
	draw_IC_PINS(num_boxes/2, (int)(1.25*o_size), -pins, 0);
	
	o_size = o_size/30;
	
	if (description != " ")
		draw_text(" Modify", 2*x_screen + 1, 12);

	draw_text(description, (int)(3.76*x_screen), 12);
	draw_text(name, (int)(1.76*x_screen), 12);
	
	o_size  = (int)(sign_os*6*object_size/sqrt((double)num_buttons));	
}






void draw_OPAMP(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
		
	SetDrawingColorx(drawing_color);
	MoveTo((c.x + o_size), c.y);
	LineTo((c.x - o_size/sqrt_2), (c.y + o_size/sqrt_2));
	LineTo((c.x - o_size/sqrt_2), (c.y - o_size/sqrt_2));
	LineTo((c.x + o_size), c.y);
	LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - o_size/sqrt_2), (c.y + o_size/2));
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - o_size/sqrt_2), (c.y - o_size/2));
	LineDraw((c.x + o_size), c.y, (c.x + 1.2*o_size), c.y); 
	
	LineDraw((c.x - o_size/2), (c.y + o_size/2), (c.x - o_size/2), (c.y + o_size/4));
	LineDraw((c.x - o_size/1.6), (c.y + 0.375*o_size), (c.x - o_size/2.8), (c.y + 0.375*o_size));
	LineDraw((c.x - o_size/1.6), (c.y - 0.375*o_size), (c.x - o_size/2.8), (c.y - 0.375*o_size));
}




void draw_RESISTOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0) 
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/sqrt_2), c.y);
	LineDraw((c.x + o_size/1.2), c.y, (c.x + 1.2*o_size), c.y);
	MoveTo((c.x - o_size/sqrt_2), c.y);
	LineTo((c.x - o_size/1.8), (c.y - o_size/3));
	LineTo((c.x - o_size/3), (c.y + o_size/3));
	LineTo((c.x - o_size/5), (c.y - o_size/3)); 
	LineTo(c.x, (c.y + o_size/3));
	LineTo((c.x + o_size/5), (c.y - o_size/3));
	LineTo((c.x + o_size/3), (c.y + o_size/3));
	LineTo((c.x + o_size/1.8), (c.y - o_size/3));
	LineTo((c.x + o_size/1.45), (c.y + o_size/3));
	LineTo((c.x + o_size/1.2), c.y);
}




void draw_RESISTOR2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
			
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/1.3));
	LineDraw(c.x, (c.y + o_size/1.3), c.x, (c.y + o_size/offset));
	MoveTo(c.x, (c.y - o_size/1.3));
	LineTo((c.x - o_size/3), (c.y - o_size/1.6));
	LineTo((c.x + o_size/3), (c.y - o_size/2.4));
	LineTo((c.x - o_size/3), (c.y - o_size/4.4)); 
	LineTo((c.x + o_size/3), (c.y - o_size/8));
	LineTo((c.x - o_size/3), (c.y + o_size/5.8));
	LineTo((c.x + o_size/3), (c.y + o_size/3.4));
	LineTo((c.x - o_size/3), (c.y + o_size/2));
	LineTo((c.x + o_size/3), (c.y + o_size/1.5));
	LineTo(c.x, (c.y + o_size/1.3));
}




void draw_TRANSISTOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0) 
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/12), c.y);
	LineDraw((c.x - o_size/12), (c.y + o_size/2.25), (c.x - o_size/12), (c.y - o_size/2.25));
	LineDraw((c.x - o_size/12), (c.y + o_size/5), (c.x + o_size/2), (c.y + o_size/2));
	LineDraw((c.x - o_size/12), (c.y - o_size/5), (c.x + o_size/2), (c.y - o_size/2));
	LineDraw((c.x + o_size/2), (c.y + o_size/2), (c.x + 1.2*o_size), (c.y + o_size/2));
	LineDraw((c.x + o_size/2), (c.y - o_size/2), (c.x + 1.2*o_size), (c.y - o_size/2));
}




void draw_NPN(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	draw_TRANSISTOR(box_num);
	
	SetDrawingColorx(WHITE);
	RectangleDraw((c.x + o_size/4.5), (c.y + o_size/3.5), (c.x + o_size/2), (c.y + o_size/1.8));
	
	SetDrawingColorx(drawing_color);
	MoveTo((c.x + o_size/6), (c.y + o_size/2));
	LineTo((c.x + o_size/4), (c.y + o_size/3.8));
	LineTo((c.x + o_size/2), (c.y + o_size/2));
	LineTo((c.x + o_size/6), (c.y + o_size/2));
}




void draw_PNP(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	draw_TRANSISTOR(box_num);
	
	SetDrawingColorx(WHITE);
	RectangleDraw((c.x + o_size/4.5), (c.y - o_size/3.5), (c.x + o_size/2.2), (c.y - o_size/1.8));
	
	SetDrawingColorx(drawing_color);
	MoveTo((c.x + o_size/5), (c.y - o_size/3.1));
	LineTo((c.x + o_size/2), (c.y - o_size/3.1));
	LineTo((c.x + o_size/2.5), (c.y - o_size/1.8));
	LineTo((c.x + o_size/5), (c.y - o_size/3.1));
}




void draw_NFET(int box_num) {
	
	draw_FET(box_num);
}




void draw_PFET(int box_num) {

	point c = get_center(box_num);
	
	draw_FET(box_num);
	
	SetDrawingColorx(WHITE);
	CircleDraw((c.x - o_size/6), c.y, (o_size/8 + sign(o_size)));
	
	SetDrawingColorx(drawing_color);
	CircleFrame((c.x - o_size/6), c.y, (o_size/8 + sign(o_size)));	
}




void draw_FET(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/12), c.y);
	LineDraw((c.x + o_size/6), (c.y - o_size/2), (c.x + 1.2*o_size), (c.y - o_size/2));
	LineDraw((c.x + o_size/6), (c.y + o_size/2), (c.x + 1.2*o_size), (c.y + o_size/2));
	LineDraw((c.x - o_size/12), (c.y + o_size/2), (c.x - o_size/12), (c.y - o_size/2));
	LineDraw((c.x + o_size/6), (c.y + o_size/2), (c.x + o_size/6), (c.y - o_size/2));
}





void draw_CAPACITOR(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/12), c.y);
	LineDraw((c.x + o_size/12), c.y, (c.x + 1.2*o_size), c.y);
	LineDraw((c.x - o_size/12), (c.y + o_size/2.5), (c.x - o_size/12), (c.y - o_size/2.5));
	LineDraw((c.x + o_size/12), (c.y + o_size/2.5), (c.x + o_size/12), (c.y - o_size/2.5));
}




void draw_CAPACITOR2(int box_num) {

	point c = get_center(box_num);
	
	c.x += 0.111*o_size;
	
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/12));
	LineDraw(c.x, (c.y + o_size/12), c.x, (c.y + o_size/offset));
	LineDraw((c.x - o_size/2.5), (c.y - o_size/12), (c.x + o_size/2.5), (c.y - o_size/12));
	LineDraw((c.x - o_size/2.5), (c.y + o_size/12), (c.x + o_size/2.5), (c.y + o_size/12));
}




void draw_INDUCTOR(int box_num) {
	
	point c = get_center(box_num);
		
	SetDrawingColorx(drawing_color);
	draw_INDUCTOR_BASE((int)c.x, (int)c.y, (int)c.x, (int)c.y);
}




void draw_INDUCTOR2(int box_num) {
	
	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	draw_INDUCTOR2_BASE((int)(c.x + 0.111*o_size), (int)c.y, (int)(c.x + 0.111*o_size), (int)(c.y - 0.167*o_size));
}




void draw_INDUCTOR_BASE(int cx, int cy, int cx2, int cy2) {

	LineDraw((int)(cx - o_size/offset), cy, (int)(cx - 0.665*o_size), cy);
	LineDraw((int)(cx + o_size/1.17), cy, (int)(cx + 1.2*o_size), cy);
	
	ArcFrame((int)(cx2 - o_size/2.7), (cy2 - o_size/7), (o_size/3), 245, 240);
	ArcFrame((cx2 + o_size/8), (cy2 - o_size/7), (o_size/3), 225, 255);
	ArcFrame((int)(cx2 + o_size/1.65), (cy2 - o_size/7), (o_size/3), 225, 250);	
}




void draw_INDUCTOR2_BASE(int cx, int cy, int cx2, int cy2) {

	LineDraw(cx, (cy - o_size), cx, (int)(cy - o_size/1.24));
	LineDraw(cx, (int)(cy + o_size/1.28), cx, (int)(cy + o_size/offset));
		
	ArcFrame((cx2 - o_size/7), (int)(cy2 - o_size/2.65), (o_size/3), 322, 235);
	ArcFrame((cx2 - o_size/7), (cy2 + o_size/8), (o_size/3), 325, 254);
	ArcFrame((cx2 - o_size/7), (int)(cy2 + o_size/1.65), (o_size/3), 340, 235);	
}




void draw_XFMR(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	draw_INDUCTOR2_BASE((int)(c.x + .65*o_size), (int)c.y, (int)(c.x + 1.02*o_size), (int)(c.y - 0.167*o_size));
	draw_INDUCTOR2_BASE((int)(c.x - .5*o_size), (int)c.y, (int)(c.x - .13*o_size), (int)(c.y - 0.167*o_size));
	
	c.x += .34*o_size;
			
	LineDraw((c.x - o_size/6), (c.y - o_size/1.1), (c.x - o_size/6), (c.y + o_size/1.1));
	LineDraw((c.x + o_size/6), (c.y - o_size/1.1), (c.x + o_size/6), (c.y + o_size/1.1));
}



void draw_XFMR2(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	draw_INDUCTOR_BASE((int)c.x, (int)(c.y - 0.5*o_size), (int)(c.x), (int)(c.y - 0.2*o_size));
	draw_INDUCTOR_BASE((int)c.x, (int)(c.y + 0.5*o_size), (int)(c.x), (int)(c.y + 0.8*o_size));
	
	c.y += 0.22*o_size;
				
	LineDraw((c.x - o_size/1.1), (c.y - o_size/6), (c.x + o_size/1.1), (c.y - o_size/6));
	LineDraw((c.x - o_size/1.1), (c.y + o_size/6), (c.x + o_size/1.1), (c.y + o_size/6));
}




void draw_VARIABLE(int box_num) {

	point c = get_center(box_num);
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/1.2), (c.y + o_size/2.4), (c.x + o_size/1.2), (c.y - o_size/2.4));
	LineDraw((c.x + o_size/1.8), (c.y - o_size/2.3), (c.x + o_size/1.2), (c.y - o_size/2.3));
	LineDraw((c.x + o_size/1.4), (c.y - o_size/6), (c.x + o_size/1.2), (c.y - o_size/2.2));
}




void draw_CLABEL(int box_num) { // Not Finished

	point c = get_center(box_num);
		
	SetDrawingColorx(drawing_color);
	RectangleDraw((c.x-1), (c.y-1), (c.x+1), (c.y+1)); // temporary
}




void draw_DIODE(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/2), c.y);
	LineDraw((c.x + o_size/2), c.y, (c.x + 1.2*o_size), c.y);
	LineDraw((c.x - o_size/2), (c.y + o_size/2.5), (c.x - o_size/2), (c.y - o_size/2.5));
	LineDraw((c.x + o_size/2), (c.y + o_size/2.5), (c.x + o_size/2), (c.y - o_size/2.5));
	LineDraw((c.x - o_size/2), (c.y + o_size/2.5), (c.x + o_size/2), c.y);
	LineDraw((c.x - o_size/2), (c.y - o_size/2.5), (c.x + o_size/2), c.y);
}




void draw_DIODE2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
	
	if (o_size > 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/2));
	LineDraw(c.x, (c.y + o_size/2), c.x, (c.y + o_size/offset));
	LineDraw((c.x - o_size/2.5), (c.y - o_size/2), (c.x + o_size/2.5), (c.y - o_size/2));
	LineDraw((c.x - o_size/2.5), (c.y + o_size/2), (c.x + o_size/2.5), (c.y + o_size/2));
	LineDraw((c.x + o_size/2.5), (c.y - o_size/2), c.x, (c.y + o_size/2));
	LineDraw((c.x - o_size/2.5), (c.y - o_size/2), c.x, (c.y + o_size/2));
}




void draw_CUSTOMWIRE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), (c.y + o_size/offset), (c.x + o_size), (c.y - o_size));
}




void draw_LINE(int L1, int L2) {

	int x1, y1, x2, y2;
	point c1 = get_center(L1 - Wire_base_id);
	point c2 = get_center(L2);
		
	x1 = (int)c1.x;
	y1 = (int)c1.y;
	x2 = (int)c2.x;
	y2 = (int)c2.y;	
	x1 += o_size/12;
	x2 += o_size/12;

	if (x1 == x2) {
		--y2;
	}
	else if (y1 == y2) {
		--x2;
	}
	if (!clip_line_to_box(x1, y1, x2, y2, 0, 0, x_limit, y_limit))
		return;
	
	SetDrawingColorx(drawing_color);
	LineDraw(x1, y1, x2, y2);
}




void draw_WIRE(point c1, point c2) {

	SetDrawingColorx(drawing_color);
	LineDraw((c1.x + o_size/12), c1.y, (c2.x + o_size/12), c2.y);
}




void draw_POWER(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x + o_size/12), (c.y - o_size/2.5), (c.x + o_size/12), c.y);
	MoveTo((c.x + o_size/12), (c.y - 0.956*o_size));
	LineTo((c.x + 0.333*o_size), (c.y - o_size/2.5));
	LineTo((c.x - 0.167*o_size), (c.y - o_size/2.5));
	LineTo((c.x + o_size/12), (c.y - 0.956*o_size));
}




void draw_GROUND(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x + o_size/12), c.y, (c.x + o_size/12), (c.y + o_size/2.5));
	LineDraw((c.x - 0.417*o_size), (c.y + o_size/2.5), (c.x + 0.417*o_size), (c.y + o_size/2.5));
	LineDraw((c.x - 0.25*o_size), (c.y + o_size/1.7), (c.x + 0.25*o_size), (c.y + o_size/1.7));
	LineDraw((c.x - 0.083*o_size), (c.y + o_size/1.35), (c.x + 0.083*o_size), (c.y + o_size/1.35));
}




void draw_INPUT(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - abs(o_size)/2.5), c.y, (c.x + o_size/12), c.y);
	CircleFrame((c.x - 0.567*abs(o_size)), c.y, (abs(o_size)/6));
}




void draw_OUTPUT(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x + abs(o_size)/12), c.y, (c.x + abs(o_size)/2.5), c.y);
	CircleFrame((c.x + 0.567*abs(o_size)), c.y, (abs(o_size)/6));
}




void draw_BATTERY(int box_num) {

	point c = get_center(box_num);
	
	if (o_size < 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/8), c.y);
	LineDraw((c.x + o_size/8), c.y, (c.x + 1.2*o_size), c.y);
	LineDraw((c.x - o_size/8), (c.y + o_size/3.8), (c.x - o_size/8), (c.y - o_size/3.8));
	LineDraw((c.x + o_size/8), (c.y + o_size/1.6), (c.x + o_size/8), (c.y - o_size/1.6));
	
	LineDraw((c.x + o_size/2), (c.y - o_size/2), (c.x + o_size/2), (c.y - o_size/4.4));
	LineDraw((c.x + o_size/1.6), (c.y - 0.375*o_size), (c.x + o_size/2.8), (c.y - 0.375*o_size));
	LineDraw((c.x - o_size/1.6), (c.y - 0.375*o_size), (c.x - o_size/2.8), (c.y - 0.375*o_size));
}




void draw_V_SOURCE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	CircleFrame(c.x, c.y, (int)(o_size/1.5));
	
	if (o_size < 0)
		c.x -= 1;
	
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/1.5), c.y);
	LineDraw((c.x + o_size/1.6), c.y, (c.x + 1.2*o_size), c.y);
	
	draw_SIGNS(box_num);
}




void draw_I_SOURCE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	CircleFrame(c.x, c.y, (int)(o_size/1.5));
	
	if (o_size < 0)
		c.x -= 1;
	
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/1.5), c.y);
	LineDraw((c.x + o_size/1.6), c.y, (c.x + 1.2*o_size), c.y);
	
	draw_CURRENT(box_num);
}




void draw_DEPENDENT_SOURCE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset + sign(o_size)), c.y, (c.x - o_size/1.2), c.y);
	LineDraw((c.x + o_size/1.2), c.y, (c.x + 1.217*o_size), c.y);
	
	MoveTo(c.x, (c.y - o_size/2.2));
	LineTo((c.x + o_size/1.2), c.y);
	LineTo(c.x , (c.y + o_size/2.2));
	LineTo((c.x - o_size/1.2), c.y);
	LineTo(c.x, (c.y - o_size/2.2));
}




void draw_BATTERY2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
	
	if (o_size > 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/8));
	LineDraw(c.x, (c.y + o_size/8), c.x, (c.y + o_size/offset));
	LineDraw((c.x - o_size/1.6), (c.y - o_size/8), (c.x + o_size/1.6), (c.y - o_size/8));
	LineDraw((c.x - o_size/3.8), (c.y + o_size/8), (c.x + o_size/3.8), (c.y + o_size/8));
	
	LineDraw((c.x + o_size/2), (c.y - o_size/2), (c.x + o_size/2), (c.y - o_size/4.4));
	LineDraw((c.x + o_size/1.6), (c.y - 0.375*o_size), (c.x + o_size/2.8), (c.y - 0.375*o_size));
	LineDraw((c.x + o_size/1.6), (c.y + 0.375*o_size), (c.x + o_size/2.8), (c.y + 0.375*o_size));
}




void draw_V_SOURCE2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
	
	if (o_size > 0)
		c.x -= 1;
	
	SetDrawingColorx(drawing_color);
	CircleFrame(c.x, c.y, (int)(o_size/1.5));
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/1.55));
	LineDraw(c.x, (c.y + o_size/1.55), c.x, (c.y + o_size/offset));
	
	draw_SIGNS2(box_num);
}




void draw_I_SOURCE2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
	
	if (o_size > 0)
		c.x -= 1; 
	
	SetDrawingColorx(drawing_color);
	CircleFrame(c.x, c.y, (int)(o_size/1.5));
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/1.55));
	LineDraw(c.x, (c.y + o_size/1.55), c.x, (c.y + o_size/offset));
	
	draw_CURRENT2(box_num);
}




void draw_DEPENDENT_SOURCE2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
	
	if (o_size > 0)
		c.x -= o_size/20;
	
	SetDrawingColorx(drawing_color);
	
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/1.2));
	LineDraw(c.x, (c.y + o_size/1.2), c.x, (c.y + o_size/offset));
	
	MoveTo((c.x - o_size/2.2), c.y);
	LineTo(c.x, (c.y + o_size/1.2));
	LineTo((c.x + o_size/2.2) , c.y);
	LineTo(c.x, (c.y - o_size/1.2));
	LineTo((c.x - o_size/2.2), c.y);
}




void draw_VCV_SOURCE(int box_num) {

	draw_DEPENDENT_SOURCE(box_num);
	draw_SIGNS(box_num);
	draw_CONTROL_V(box_num);
}




void draw_CCV_SOURCE(int box_num) {
	
	draw_DEPENDENT_SOURCE(box_num);
	draw_SIGNS(box_num);
	draw_CONTROL_I(box_num);
}




void draw_VCC_SOURCE(int box_num) {

	draw_DEPENDENT_SOURCE(box_num);
	draw_CURRENT(box_num);
	draw_CONTROL_V(box_num);
}




void draw_CCC_SOURCE(int box_num) {

	draw_DEPENDENT_SOURCE(box_num);
	draw_CURRENT(box_num);
	draw_CONTROL_I(box_num);
}




void draw_VCV_SOURCE2(int box_num) {

	draw_DEPENDENT_SOURCE2(box_num);
	draw_SIGNS2(box_num);
	draw_CONTROL_V2(box_num);
}




void draw_CCV_SOURCE2(int box_num) {
	
	draw_DEPENDENT_SOURCE2(box_num);
	draw_SIGNS2(box_num);
	draw_CONTROL_I2(box_num);
}




void draw_VCC_SOURCE2(int box_num) {

	draw_DEPENDENT_SOURCE2(box_num);
	draw_CURRENT2(box_num);
	draw_CONTROL_V2(box_num);
}




void draw_CCC_SOURCE2(int box_num) {

	draw_DEPENDENT_SOURCE2(box_num);
	draw_CURRENT2(box_num);
	draw_CONTROL_I2(box_num);
}




void draw_CONTROL_V(int box_num) {

	point c = get_center(box_num);
	
	int o_size2 = o_size;
	o_size = abs(o_size);
	
	if (o_size2 > 0)
		c.x += o_size/7;
	else
		c.x -= o_size/20;
		
	c.y -= o_size/28.5;
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x + o_size/offset), (c.y - o_size/2), (c.x + o_size/1.2 + o_size/8), (c.y - o_size/2));
	LineDraw((c.x - 0.958*o_size), (c.y - o_size/2), (c.x - 1.2*o_size), (c.y - o_size/2));
	
	CircleFrame((int)(c.x + o_size/1.2), (int)(c.y - o_size/2), (int)(o_size/8));
	CircleFrame((int)(c.x - o_size/1.2), (int)(c.y - o_size/2), (int)(o_size/8));
	
	LineDraw((c.x - 0.527*o_size), (c.y - 0.431*o_size), (c.x - 0.527*o_size), (c.y - 0.681*o_size));
	LineDraw((c.x - 0.669*o_size), (c.y - o_size/1.8), (c.x - 0.387*o_size), (c.y - o_size/1.8));
	LineDraw((c.x + 0.669*o_size), (c.y - o_size/1.8), (c.x + 0.387*o_size), (c.y - o_size/1.8));
	
	o_size = o_size2;
}




void draw_CONTROL_I(int box_num) {

	point c = get_center(box_num);
	
	int o_size2 = abs(o_size);
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset + sign(o_size)), (c.y - o_size2/1.95), (c.x + 1.2*o_size), (c.y - o_size2/1.95));
	
	c.y -= o_size2/1.5;
	
	LineDraw((c.x - o_size2/3), c.y, (c.x + o_size2/3), c.y);
	LineDraw((c.x + o_size2/3), c.y, (c.x + o_size2/6 - 1), (c.y + o_size2/6));
	LineDraw((c.x + o_size2/3), c.y, (c.x + o_size2/6), (c.y - o_size2/6));
}




void draw_CONTROL_V2(int box_num) {

	point c = get_center(box_num);
	
	int o_size2 = o_size;
	o_size = abs(o_size);
	
	if (o_size2 > 0)
		c.x += o_size/5;
		
	c.y -= o_size/20 - 1;
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/1.5), (c.y - o_size), (c.x - o_size/1.5), (c.y - 0.792*o_size));
	LineDraw((c.x - o_size/1.5), (c.y + 0.792*o_size), (c.x - o_size/1.5), (c.y + 1.03*o_size));
	
	CircleFrame((int)(c.x - o_size/1.5), (int)(c.y + o_size/1.5), o_size/8);
	CircleFrame((int)(c.x - o_size/1.5), (int)(c.y - o_size/1.5), o_size/8);
	
	LineDraw((c.x - o_size/1.5), (c.y - o_size/1.9), (c.x - o_size/1.5), (c.y - o_size/4.1));
	LineDraw((c.x - 0.542*o_size), (c.y - o_size/2.6), (c.x - 0.792*o_size), (c.y - o_size/2.6));
	LineDraw((c.x - 0.542*o_size), (c.y + o_size/2.6), (c.x - 0.792*o_size), (c.y + o_size/2.6));
	
	o_size = o_size2;
}




void draw_CONTROL_I2(int box_num) {

	point c = get_center(box_num);
	
	int o_size2 = o_size;
	o_size = abs(o_size);
	
	if (o_size2 > 0)
		c.x += o_size/5;
				
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/1.5), (c.y - o_size), (c.x - o_size/1.5), (c.y + o_size/offset));
	
	LineDraw((c.x - o_size/1.2), (c.y - o_size/3), (c.x - o_size/1.2), (c.y + o_size/3 - 1));
	LineDraw((c.x - o_size/1.2), (c.y + o_size/3), (c.x - 0.667*o_size), (c.y + o_size/6));
	LineDraw((c.x - o_size/1.2), (c.y + o_size/3), (c.x - o_size), (c.y + o_size/6));
	
	o_size = o_size2;
}





void draw_SIGNS(int box_num) {

	point c = get_center(box_num);
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x + o_size/2.6), (c.y + o_size/8), (c.x + o_size/2.6), (c.y - o_size/8));
	LineDraw((c.x + o_size/1.9), c.y, (c.x + o_size/4.1), c.y);
	LineDraw((c.x - o_size/1.9), c.y, (c.x - o_size/4.1), c.y);
}




void draw_CURRENT(int box_num) {

	point c = get_center(box_num);
		
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/3), c.y, (c.x + o_size/3 - sign(o_size)), c.y);
	LineDraw((c.x + o_size/3 - sign(o_size)), c.y, (c.x + o_size/6), (c.y + o_size/6));
	LineDraw((c.x + o_size/3), c.y, (c.x + o_size/6), (c.y - o_size/6));
}




void draw_SIGNS2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
		
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size/1.9), c.x, (c.y - o_size/4.1));
	LineDraw((c.x + o_size/8), (c.y - o_size/2.6), (c.x - o_size/8), (c.y - o_size/2.6));
	LineDraw((c.x + o_size/8), (c.y + o_size/2.6), (c.x - o_size/8), (c.y + o_size/2.6));
}




void draw_CURRENT2(int box_num) {

	point c = get_center(box_num);
	
	c.x += o_size/9;
		
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y + o_size/3), c.x, (c.y - o_size/3 + sign(o_size)));
	LineDraw(c.x, (c.y - o_size/3 + sign(o_size)), (c.x + o_size/6), (c.y - o_size/6));
	LineDraw(c.x, (c.y - o_size/3), (c.x - o_size/6), (c.y - o_size/6));
}




void draw_FF(int box_num, int id) {

	point c = get_center(box_num);

	SetDrawingColorx(drawing_color);
		
	LineDraw((c.x - o_size/offset), (c.y - o_size/2), (c.x - 0.667*o_size), (c.y - o_size/2)); // in1
	if (id != RSL_ID && id != (num_ffs + RSL_ID))
		LineDraw((c.x - o_size/offset), c.y, (c.x - 0.667*o_size), c.y); // in2
	if (id != DFF_ID && id != (num_ffs + DFF_ID) && id != TDFF_ID && id != (num_ffs + TDFF_ID) && id != TFF_ID && id != (num_ffs + TFF_ID))
		LineDraw((c.x - o_size/offset), (c.y + o_size/2), (c.x - 0.667*o_size), (c.y + o_size/2)); // in3
	LineDraw((c.x + o_size/1.1), (c.y - o_size/2), (c.x + 1.2*o_size), (c.y - o_size/2)); // out1
	if (id > 2*num_ffs && id != 100)
		LineDraw((c.x + o_size/1.1), c.y, (c.x + 1.2*o_size), c.y); // out2
	LineDraw((c.x + o_size/1.1), (c.y + o_size/2), (c.x + 1.2*o_size), (c.y + o_size/2)); // out3
	
	RectangleFrame((c.x - 0.667*o_size), (c.y - 0.75*o_size), (c.x + o_size/1.1), (c.y + 0.75*o_size));
		
	if (id > 0) {
		draw_CK0(box_num + x_blocks + 3);
	}
	else {
		SetDrawingColorx(WHITE);
		CircleDraw((c.x + o_size), (c.y + o_size/2), (o_size/8 + sign(o_size)));
	
		SetDrawingColorx(drawing_color);
		CircleFrame((c.x + o_size), (c.y + o_size/2), (o_size/8 + sign(o_size)));
	}
}





void draw_ckFF(int box_num, int id) {
	
	SetDrawingColorx(drawing_color);
	
	if (id != RSL_ID && id != TDFF_ID && id != (num_ffs + RSL_ID) && id != (num_ffs + TDFF_ID)) {
		draw_CK(box_num);
	}
	draw_FF(box_num, id);
}




void draw_CK(int box_num) {
	
	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - 0.667*o_size), (c.y - o_size/7), (c.x - o_size/2.6), c.y);
	LineDraw((c.x - 0.667*o_size), (c.y + o_size/7), (c.x - o_size/2.6), c.y);
}




void draw_CK0(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(WHITE);
	CircleDraw((c.x - o_size/1.4), c.y, (o_size/8 + sign(o_size)));
	
	SetDrawingColorx(drawing_color);
	CircleFrame((c.x - o_size/1.4), c.y, (o_size/8 + sign(o_size)));
}




void draw_MODULE(int box_num, int id) {
	
	point c = get_center(box_num);
	
	if (id <= MAX_USED_ID || id >= FF_base_id)
		return;
	
	SetDrawingColorx(drawing_color);
	RectangleFrame((c.x - 2*o_size), (c.y - 1.5*o_size), (c.x + 2*o_size) ,(c.y + 1.5*o_size));
}




void draw_DELAY(int box_num) {

	point c = get_center(box_num);
	char text[2]; 
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - 0.45*o_size), c.y, (c.x - 0.15*o_size), c.y);
	LineDraw((c.x + 0.35*o_size), c.y, (c.x + 0.65*o_size), c.y);
	LineDraw((c.x - 0.15*o_size), c.y, (c.x - 0.35*o_size), (c.y + 0.2*o_size));
	LineDraw((c.x - 0.15*o_size), c.y, (c.x - 0.35*o_size), (c.y - 0.2*o_size));
	RectangleFrame((c.x - 0.15*o_size), (c.y + 0.25*o_size), (c.x + 0.35*o_size), (c.y - 0.25*o_size));

	text[0] = 1;
	text[1] = 'D';

	MoveTo((c.x - 0.04*o_size), (c.y + 0.15*o_size));
	draw_string2(text);
}




void draw_DELAY2(int box_num) {

	point c = get_center(box_num);
	char text[2];
	
	c.x += 0.08*o_size;
	
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - 0.51*o_size), c.x, (c.y - 0.25*o_size));
	LineDraw(c.x, (c.y + 0.25*o_size), c.x, (c.y + 0.53*o_size));
	LineDraw(c.x, (c.y - 0.25*o_size), (c.x + 0.2*o_size), (c.y - 0.45*o_size));
	LineDraw(c.x, (c.y - 0.25*o_size), (c.x - 0.2*o_size), (c.y - 0.45*o_size));
	RectangleFrame((c.x + 0.25*o_size), (c.y - 0.25*o_size), (c.x - 0.25*o_size), (c.y + 0.25*o_size));

	text[0] = 1;
	text[1] = 'D';

	MoveTo((c.x - 0.13*o_size), (c.y + 0.13*o_size));
	draw_string2(text);
}




void draw_TLINE(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw((c.x - o_size/offset), c.y, (c.x - o_size/sqrt_2), c.y);
	LineDraw((c.x + o_size/1.2), c.y, (c.x + 1.2*o_size), c.y);
	RectangleFrame((c.x - o_size/sqrt_2), (c.y - 0.25*o_size), (c.x + o_size/1.2), (c.y + 0.25*o_size));
	draw_GROUND(box_num);
}




void draw_TLINE2(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(drawing_color);
	LineDraw(c.x, (c.y - o_size), c.x, (c.y - o_size/1.3));
	LineDraw(c.x, (c.y + o_size/1.3), c.x, (c.y + o_size/offset));
	RectangleFrame((c.x - 0.25*o_size), (c.y - o_size/1.3), (c.x + 0.25*o_size), (c.y + o_size/1.3));
	draw_GROUND(box_num);
}




void draw_NODE(int box_num) {
	
	if (need_node) {
		point c = get_center(box_num);		
		if (box_num != 0 && box_num != 1 && (box_num - x_blocks*sch_origin_y - sch_origin_x) < x_blocks*y_blocks) {	
			SetDrawingColorx(drawing_color);
			CircleDraw((c.x + o_size/8), c.y , 2);
		}
	}	
}




void draw_NODES(const connections& x) {

	draw_NODE(x.in1);
	draw_NODE(x.in2);
	draw_NODE(x.in3);
	draw_NODE(x.out1);
	draw_NODE(x.out2);
	draw_NODE(x.out3);
}




void draw_BULLET(int box_num) {

	point c = get_center(box_num);
	
	SetDrawingColorx(WHITE);
	CircleDraw(c.x, c.y, 7);
	
	SetDrawingColorx(BLACK);
	CircleFrame(c.x, c.y, 7);
}




void draw_FBULLET(int box_num) {

	point c = get_center(box_num);
	
	draw_BULLET(box_num);
	
	SetDrawingColorx(BLACK);
	CircleDraw(c.x, c.y, 4);
}




void draw_RARROW(int box_num) {

	point c = get_center(box_num);
	
	if (box_num == 0)
		box_num = 1000;
	
	SetDrawingColorx(BLACK);
	MoveTo((c.x + o_size), c.y);
	LineTo((c.x + o_size/2), (c.y + o_size/2)); 
	LineTo((c.x + o_size/2), (c.y + o_size/4));
	LineTo((c.x - o_size), (c.y + o_size/4));
	LineTo((c.x - o_size), (c.y - o_size/4));
	LineTo((c.x + o_size/2), (c.y - o_size/4));
	LineTo((c.x + o_size/2), (c.y - o_size/2));
	LineTo((c.x + o_size), c.y);
}




void draw_LARROW(int box_num) {

	point c = get_center(box_num);
	
	if (box_num == 0)
		box_num = 1000;
	
	SetDrawingColorx(BLACK);
	MoveTo((c.x - o_size), c.y);
	LineTo((c.x - o_size/2), (c.y + o_size/2)); 
	LineTo((c.x - o_size/2), (c.y + o_size/4));
	LineTo((c.x + o_size), (c.y + o_size/4));
	LineTo((c.x + o_size), (c.y - o_size/4));
	LineTo((c.x - o_size/2), (c.y - o_size/4));
	LineTo((c.x - o_size/2), (c.y - o_size/2));
	LineTo((c.x - o_size), c.y);
}




void draw_REVERSE() {
	
	point c = get_center(9);
	
	if (o_size < 0)
		c.x += o_size/5;
	
	SetDrawingColorx(WHITE);
	RectangleDraw((c.x - x_limit/x_screen/2.2), (c.y - y_limit/y_screen/3), (c.x + x_limit/x_screen/2.2), (c.y + y_limit/y_screen/2.2));
	
	SetDrawingColorx(PURPLE);
	
	if (o_size < 0) {
		o_size = -o_size;
		label_box(9, '<');
		o_size = -o_size;
	}
	else
		label_box(9, '>');
}




void draw_ARROW(double x1, double y1, double x2, double y2, double ah_len, double ah_angle) {

	int len;
	double alpha, gamma1, gamma2;

	len    = (int)((ah_len*(x_limit + y_limit))/(x_screen + y_screen));
	alpha  = get_angle((x2 - x1), (y2 - y1));
	gamma1 = pi_over_180*(alpha + ah_angle);
	gamma2 = pi_over_180*(alpha - ah_angle);

	LineDraw(x1, y1, x2, y2);
	LineDraw(x2, y2, (x2 - len*cos(gamma1)), (y2 - len*sin(gamma1)));
	LineDraw(x2, y2, (x2 - len*cos(gamma2)), (y2 - len*sin(gamma2)));
}




void draw_path(int num_points, int show_nodes, int *point_array) {

	point c;

	for (unsigned int i = 1; (int)i < num_points; ++i) {
		c = get_center(point_array[i]);
		if (show_nodes)
			draw_NODE(point_array[i]);
		if (i == 0)
			MoveTo(c.x, c.y);
		else
			LineTo(c.x, c.y);
	}
}




void draw_special() {
		
	set_text_size(12);
	text_size = 12;
	
	SetDrawingColorx(BLUE);	
	label_box(0, 'S');
	label_box(1, 'A');
	label_box(3, 'O');
	label_box(4, 'S');
	label_box(6, 'R');
	label_box(7, 'P');
	label_box(11, 'M');
	
	SetDrawingColorx(RED);
	label_box(2, 'D');
	label_box(5, 'C'); // could also be N (New)
	label_box(8, 'X');
	label_box(14, 'E');
	
	SetDrawingColorx(PURPLE);
	if (o_size <= 0) 
		label_box(9, '<');
	else
		label_box(9, '>');
		
	label_box(12, 'U');
	label_box(13, 'R');
	
	SetDrawingColorx(GREEN);
	label_box(10, '?');
}




void draw_cancel_button() {

	char cancel_text[16] = " Cancel";

	SetDrawingColorx(BLACK);
	
	RectangleFrame(1, 1, cancel_button_length+1, cancel_button_width+1);
	
	MoveTo(4, cancel_button_width/2 + 4);
	draw_string(cancel_text);
}









