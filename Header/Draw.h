#ifndef _DRAW_H_
#define _DRAW_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Draw.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Draw Header
// By Frank Gennari
void draw_grid(int grid_color);

void draw_text(string text, int box_num, int t_size);
void draw_double(double num, char label, int box_num);
void draw_char(char x, int box_num, int color, int size);
void draw_box_outline(int color, int box1, int box2);
void color_box(int box_num, int color);
void number_all_boxes();
void print_to_window(char *display, int box_num);
void draw_custom_ff(int id, int box_num);
void draw_FF_name(int id, int box_num);
void number_pins(const connections& x, int box_num);
void label_ff(char A, char B, char C, char D, char E, char F, int box_num);
void draw_selection_window(int count);

void scroll_text(string text, int scroll_length);
void draw_status_bar();
void draw_status(const char *status);
void draw_status(const string status);

void draw_NO(int box_num);
void draw_RED_CIRCLE(int box_num);
void draw_blue_x(int location);
void draw_SQUAREWAVE(int box_num);
void draw_SINEWAVE(int box_num);
void draw_STATE_SELECTION(int box_num);
void draw_NOT0(int box_num);
void draw_BUFFER(int box_num);
void draw_NOT(int box_num);
void draw_AND(int box_num);
void draw_NAND(int box_num);
void draw_OR(int box_num);
void draw_NOR(int box_num);
void draw_XOR(int box_num);
void draw_XNOR(int box_num);
void draw_3AND(int box_num);
void draw_3NAND(int box_num);
void draw_3OR(int box_num);
void draw_3NOR(int box_num);
void draw_3XOR(int box_num);
void draw_3XNOR(int box_num);
void draw_4AND(int box_num);
void draw_4NAND(int box_num);
void draw_4OR(int box_num);
void draw_4NOR(int box_num);
void draw_4XOR(int box_num);
void draw_4XNOR(int box_num);
void draw_TRI_STATE_BUFFER(int box_num);
void draw_TRI_STATE_INVERTER(int box_num);
void draw_TRI_STATE(int box_num);
void draw_MUX(int box_num);
void draw_ADDER(int box_num); 
void draw_IC(int box_num);
void draw_IC_PINS(int box_num, int size, int num_pins, int add_nodes);
void draw_LARGE_IC(int box_num, int size, int num_pins);
void draw_FULL_IC(int box_num, int num_pins);
void draw_IC_ON_SCREEN(string name, int pins, string description);
void draw_OPAMP(int box_num);
void draw_RESISTOR(int box_num);
void draw_RESISTOR2(int box_num);
void draw_TRANSISTOR(int box_num);
void draw_NPN(int box_num);
void draw_PNP(int box_num);
void draw_NFET(int box_num);
void draw_PFET(int box_num);
void draw_FET(int box_num);
void draw_CAPACITOR(int box_num);
void draw_CAPACITOR2(int box_num);
void draw_INDUCTOR(int box_num);
void draw_INDUCTOR2(int box_num);
void draw_INDUCTOR_BASE(int cx, int cy, int cx2, int cy2);
void draw_INDUCTOR2_BASE(int cx, int cy, int cx2, int cy2);
void draw_XFMR(int box_num);
void draw_XFMR2(int box_num);
void draw_VARIABLE(int box_num);
void draw_CLABEL(int box_num);
void draw_DIODE(int box_num);
void draw_DIODE2(int box_num);
void draw_CUSTOMWIRE(int box_num);
void draw_LINE(int L1, int L2);
void draw_WIRE(point c1, point c2);
void draw_POWER(int box_num);
void draw_GROUND(int box_num);
void draw_INPUT(int box_num);
void draw_OUTPUT(int box_num);
void draw_BATTERY(int box_num);
void draw_V_SOURCE(int box_num);
void draw_I_SOURCE(int box_num);
void draw_DEPENDENT_SOURCE(int box_num);
void draw_BATTERY2(int box_num);
void draw_V_SOURCE2(int box_num);
void draw_I_SOURCE2(int box_num);
void draw_DEPENDENT_SOURCE2(int box_num);
void draw_VCV_SOURCE(int box_num);
void draw_CCV_SOURCE(int box_num);
void draw_VCC_SOURCE(int box_num);
void draw_CCC_SOURCE(int box_num);
void draw_VCV_SOURCE2(int box_num);
void draw_CCV_SOURCE2(int box_num);
void draw_VCC_SOURCE2(int box_num);
void draw_CCC_SOURCE2(int box_num);
void draw_CONTROL_V(int box_num);
void draw_CONTROL_I(int box_num);
void draw_CONTROL_V2(int box_num);
void draw_CONTROL_I2(int box_num);
void draw_SIGNS(int box_num);
void draw_CURRENT(int box_num);
void draw_SIGNS2(int box_num);
void draw_CURRENT2(int box_num);
void draw_FF(int box_num, int id);
void draw_ckFF(int box_num, int id);
void draw_CK(int box_num);
void draw_CK0(int box_num);
void draw_MODULE(int box_num, int id);
void draw_DELAY(int box_num);
void draw_DELAY2(int box_num);
void draw_TLINE(int box_num);
void draw_TLINE2(int box_num);
void draw_NODE(int box_num);
void draw_NODES(const connections& x);
void draw_BULLET(int box_num);
void draw_FBULLET(int box_num);
void draw_RARROW(int box_num);
void draw_LARROW(int box_num);
void draw_REVERSE();
void draw_ARROW(double x1, double y1, double x2, double y2, double ah_len, double ah_angle);
void draw_path(int num_points, int show_nodes, int *point_array);
void draw_special();
void draw_cancel_button();


#endif
