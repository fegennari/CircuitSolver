#include "CircuitSolver.h"
#include "Logic.h"
#include "Undo.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Window.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S. Window Ops.cc - Experimental Window Resizing/Scrolling Code
// By Frank Gennari
int const SCROLL_DEBUG = 0;
double const pan_amount = 0.4;

int sch_origin_x(0), sch_origin_y(0), sch_viewable_x_size(0), sch_viewable_y_size(0);
int x_blocks(0), y_blocks(0), x_screen(0), y_screen(0), x_screen_real(0), y_screen_real(0); // x/y_blocks replace box num


extern int grid_color, needs_redraw, num_buttons, cScale, o_size, object_size, is_ic;
extern box new_num;



void resize_DW(component *&head, string name, int pins, string description, int new_num_boxes);
void resize_comps(component *&head, string name, int pins, string description, int size_factor);
void position_transform(component *head, int size_factor, int x_old, int x_new);
void component_transform(component *comp, int size_factor, int x_old, int x_new);
int  point_transform(int location, int size_factor, int x_old, int x_new);
void set_grid_res(component *&head, string name, int pins, string description, int grid_factor);
void zoom_DW(component *&head, string name, int pins, string description, int zoom_factor);
void reset_DW_defaults(component *&head, string name, int pins, string description);
void scroll_DW(component *head, string name, int pins, string description, int scroll_type, int scroll_param);
void window_resize(component *head, string name, int pins, string description);
void reset_origin();
void fit_circuit_to_screen();
int  get_window_param();


void draw_special();



// zoom in/out - resize
void resize_DW(component *&head, string name, int pins, string description, int new_num_boxes) {

	int x1(x_blocks)/*, y1(y_blocks)*/;
	
	if (new_num_boxes < 100)
		return;
	
	new_num.h_boxes = new_num.v_boxes = 0;
	
	if (needs_redraw != -1)
		whiteout();
		
	set_buttons(new_num_boxes, grid_color, 1);	
	position_transform(head, 1, x1, x_blocks);
	ur_action_transform(1, x1, x_blocks);
	
	if (needs_redraw != -1) {
		draw_circuit(head, name, pins, description, 0);
		draw_special();
	}
	needs_redraw = 0;
	new_num.h_boxes = x_blocks;
	new_num.v_boxes = y_blocks;
	num_buttons = x_blocks*y_blocks;
}




void resize_comps(component *&head, string name, int pins, string description, int size_factor) {

	if (size_factor == 0 || cScale < -size_factor)
		return;

	if (size_factor > 1) {
		object_size *= size_factor;
		cScale *= size_factor;
	}
	else if (size_factor < 1) {
		object_size /= -size_factor;
		cScale /= -size_factor;
	}
	else
		return;
	
	position_transform(head, size_factor, x_blocks, x_blocks);
	ur_action_transform(size_factor, x_blocks, x_blocks);
		
	init_buttons(1);
	draw_circuit(head, name, pins, description, 0); 
	draw_special();
		
	needs_redraw = 0;
}




void position_transform(component *head, int size_factor, int x_old, int x_new) {

	while (head != 0) {
		component_transform(head, size_factor, x_old, x_new);
		head = head->next;
	}
}


void component_transform(component *comp, int size_factor, int x_old, int x_new) {

	comp->location = point_transform(comp->location, size_factor, x_old, x_new);
	
	if (get_order(comp->id) == wirex) 
		comp->id = Wire_base_id + point_transform((comp->id - Wire_base_id), size_factor, x_old, x_new);
}


int  point_transform(int location, int size_factor, int x_old, int x_new) {

	if (x_old != x_new)
		location = transform_DW_point(location, x_new, x_old);
		
	if (size_factor != 1)
		location = scale_DW_point(location, size_factor);
		
	return location;
}




void set_grid_res(component *&head, string name, int pins, string description, int grid_factor) {

	int new_num_boxes;
	
	if (grid_factor > 1)
		new_num_boxes = (x_blocks*grid_factor)*(y_blocks*grid_factor);
	else if (grid_factor < -1)
		new_num_boxes = (x_blocks/grid_factor)*(y_blocks/grid_factor);
	else
		return;
		
	if (cScale < -grid_factor || new_num_boxes < 100)
		return;

	needs_redraw = -1;
	
	resize_DW(head, name, pins, description, new_num_boxes);
	resize_comps(head, name, pins, description, grid_factor);
}




void zoom_DW(component *&head, string name, int pins, string description, int zoom_factor) {

	int new_size;
	
	if (is_ic) {
		cout << "Currently cannot zoom in IC/subcircuit drawing window." << endl;
		return;
	}
	if (zoom_factor > 1) {
		x_screen_real = min(x_blocks, (int)(x_screen_real*zoom_factor));
		y_screen_real = min(10*y_blocks, (int)(y_screen_real*zoom_factor));
	}
	else if (zoom_factor < -1) {
		x_screen_real = min(x_blocks, (int)(-x_screen_real/zoom_factor));
		y_screen_real = min(y_blocks, (int)(-y_screen_real/zoom_factor));
	}
	return; // TEMPORARY

	if (zoom_factor > 1)
		new_size = zoom_factor*x_screen_real*y_screen_real;
	else if (zoom_factor < -1)
		new_size = x_screen_real*y_screen_real/(-zoom_factor);

	// *** should not change data structures ***
	resize_DW(head, name, pins, description, new_size);
}




void reset_DW_defaults(component *&head, string name, int pins, string description) {

	needs_redraw = -1;

	resize_DW(head, name, pins, description, default_num_buttons);
	resize_comps(head, name, pins, description, -cScale);
	fit_circuit_to_screen();
}




void scroll_DW(component *head, string name, int pins, string description, int scroll_type, int scroll_param) {
	
	int old_x(sch_origin_x), old_y(sch_origin_y);
		
	if (scroll_type == 0 || scroll_param == 0)
		return;
	
	if (is_ic) {
		cout << "Currently cannot scroll in IC/subcircuit drawing window." << endl;
		return;
	}
	if (scroll_type == 1) {	
		switch (scroll_param) {
		
			case 1: // left
				sch_origin_x -= roundx(pan_amount*x_screen_real);
				sch_origin_x = max(sch_origin_x, 0);
				if (SCROLL_DEBUG)
					cout << "Origin X1 = " << sch_origin_x << endl;
				break;
				
			case 2: // up
				sch_origin_y -= roundx(pan_amount*y_screen_real);
				sch_origin_y = max(sch_origin_y, 0);
				if (SCROLL_DEBUG)
					cout << "Origin Y1 = " << sch_origin_y << endl;
				break;
				
			case 3: // right
				sch_origin_x += roundx(pan_amount*x_screen_real);
				sch_origin_x = min(sch_origin_x, x_screen_real - x_blocks);
				if (SCROLL_DEBUG)
					cout << "Origin X2 = " << sch_origin_x << endl;
				break;
				
			case 4: // down
				sch_origin_y += roundx(pan_amount*y_screen_real);
				sch_origin_y = min(sch_origin_y, y_screen_real - y_blocks);
				if (SCROLL_DEBUG)
					cout << "Origin Y2 = " << sch_origin_y << endl;
				break;
				
			default: // should not happen
				return;
		}
	}
	if (needs_redraw || sch_origin_x != old_x || sch_origin_y != old_y) {
		init_buttons(1);
		draw_circuit(head, name, pins, description, 0);
		needs_redraw = 0;
	}
}




void window_resize(component *head, string name, int pins, string description) {

	// write

	draw_circuit(head, name, pins, description, 1);
	needs_redraw = 0;
}




void reset_origin() {

	sch_origin_x = 0;
	sch_origin_y = 0;
}




void fit_circuit_to_screen() {

	reset_origin();

	x_screen_real = x_screen = x_blocks;
	y_screen_real = y_screen = y_blocks;

	/*
	int new_size;
	double zoom;

	zoom = max(x_screen_real/x_blocks, y_screen_real/y_blocks);
	new_size = zoom*zoom*x_screen_real*y_screen_real; // should be equal to x_blocks*y_blocks
	
	// should not change data structures
	resize_DW(head, name, pins, description, new_size);
	*/
}




int get_window_param() {

	cout << "\nEnter the new window parameter (positive integer for increase, negative integer for decrease): ";
	return in();
}












