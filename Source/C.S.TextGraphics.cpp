#include "CircuitSolver.h"
#include "Logic.h"
#include "CompArray.h"
#include "TextGraphics.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Text Graphics.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Text Based Graphics
// By Frank Gennari
extern int x_blocks, y_blocks;



void write_circuit_as_text(component *head);
int write_cir_as_text(component *head);
void add_wire_to_matrix(char *circuit_matrix, unsigned int start, unsigned int end, unsigned int size);
void add_comp_to_matrix(char *circuit_matrix, int ID, char flags, unsigned int loc, unsigned int size);
void add_ic_to_matrix(char *circuit_matrix, int ID, unsigned int loc, unsigned int size);
void add_ff_to_matrix(char *circuit_matrix, int ID, char flags, unsigned int loc, unsigned int size);
void cond_add_char(char *circuit_matrix, char text_char, int loc, unsigned int size);
int get_tr_text_id(int comp_id);


transistor Qnum_find(transistor &tr, int num);



void write_circuit_as_text(component *head) {

	while (!write_cir_as_text(head)) {	
		cout << "\nSave circuit text?  ";
		if (!decision())
			return;
	}
}




int write_cir_as_text(component *head) {

	unsigned int i, j, size, loc;
	int ID;
	ofstream outfile;
	char filename[MAX_SAVE_CHARACTERS + 1] = {0}, *circuit_matrix = NULL;
	
	if (head == 0) {
		beep();
		cout << "\nThere is no circuit to save!" << endl;
		return 1;
	}
	cout << "\nSave circuit text as(max 31 characters):  ";
	
	cin >> ws;					
	strcpy(filename, xin(MAX_SAVE_CHARACTERS));
	
	if (!overwrite_file(filename)) 
		return 0;
	
	outfile.open(filename, ios::out | ios::trunc);
	
	if (outfile.fail() || !filestream_check(outfile)) {
		beep();
		cerr << "\nError: Circuit could not be saved as " << filename << "!" << endl;
		return 0;
	}
	size = x_blocks*y_blocks;
	
	circuit_matrix = memAlloc_init(circuit_matrix, size, ' ');
	
	// add to matrix
	while (head != 0) {
		ID  = head->id;
		loc = head->location;
		
		if (get_order(ID) == wirex) {
			add_wire_to_matrix(circuit_matrix, loc, (ID - Wire_base_id), size);
		}
		else if (get_order(ID) == transistorx) {
			add_comp_to_matrix(circuit_matrix, get_tr_text_id(ID), head->flags, loc, size);
		}
		else if (get_order(ID) == icx) {
			add_ic_to_matrix(circuit_matrix, ID, loc, size);
		}
		else if (get_order(ID) == flipflopx) {
			add_ff_to_matrix(circuit_matrix, ID, head->flags, loc, size);
		}
		else if (ID < MAX_USED_ID) {
			add_comp_to_matrix(circuit_matrix, ID, head->flags, loc, size);
		}
		head = head->next;
	}
	for (i = 0; (int)i < y_blocks; ++i) {
		for (j = 0; (int)j < x_blocks; ++j) {
			outfile << circuit_matrix[i*x_blocks + j];
		}
		outfile << endl;

		if (!filestream_check(outfile)) {
			beep();
			delete [] circuit_matrix;
			return 0;
		}
	}
	outfile.close();
	delete [] circuit_matrix;
	cout << "\nThe current Circuit Solver circuit text has been saved as: " << filename << "." << endl;
	
	return 1;
}




void add_wire_to_matrix(char *circuit_matrix, unsigned int start, unsigned int end, unsigned int size) {

	unsigned int i, x1, y1, x2, y2, loc;

	if (((int)start > x_blocks && end > size) || circuit_matrix == NULL)
		return;

	x1 = start%x_blocks;
	x2 = end%x_blocks;
	y1 = start/x_blocks;	
	y2 = end/x_blocks;
	
	if (x1 == x2) { // vertical
		if (y1 > y2)
			swap(y1, y2);
		for (i = y1+1; (int)i < min((int)y2, y_blocks); ++i) {
			loc = i*x_blocks + x1;
			if (circuit_matrix[loc] == text_HLINE)
				circuit_matrix[loc] = text_INTER;
			else if (circuit_matrix[loc] != text_NODE)
				circuit_matrix[loc] = text_VLINE;
		}
	}
	else if (y1 == y2) { // horizontal
		if (x1 > x2)
			swap(x1, x2);
		for (i = x1+1; (int)i < min((int)x2, x_blocks); ++i) {
			loc = y1*x_blocks + i;
			if (circuit_matrix[loc] == text_VLINE)
				circuit_matrix[loc] = text_INTER;
			else if (circuit_matrix[loc] != text_NODE)
				circuit_matrix[loc] = text_HLINE;
		}
	}
	else { // diagonal wire - split into horizontal and vertical components
		loc = x1 + x_blocks*y2; // midpoint
		//loc = x2 + x_blocks*y1; // midpoint
		add_wire_to_matrix(circuit_matrix, start, loc, size);
		add_wire_to_matrix(circuit_matrix, loc, end, size);
		return;
	}
	if (start < size)
		circuit_matrix[start] = text_NODE;
	if (end < size)
		circuit_matrix[end]   = text_NODE;
}




void add_comp_to_matrix(char *circuit_matrix, int ID, char flags, unsigned int loc, unsigned int size) {

	unsigned int i;
	int x1, x_offset, y_offset, xval, yval, loc2;
	string tcomp;
	
	if (flags & ROTATED) {
		tcomp    = text_comps_rot[ID];
		x_offset = text_comp_x_offset_rot[ID];
		y_offset = text_comp_y_offset_rot[ID];
	}
	else {
		tcomp    = text_comps[ID];
		x_offset = text_comp_x_offset[ID];
		y_offset = text_comp_y_offset[ID];
	}
	if (ID > MAX_TEXT_ID || circuit_matrix == NULL)
		return;
		
	xval = max(0, min((int)(loc%x_blocks) - x_offset, x_blocks-1));
	yval = max(0, min((int)(loc/x_blocks) - y_offset, y_blocks-1));
	x1   = xval;	
	
	for (i = 0; i < tcomp.size(); ++i) {
		if (tcomp[i] == text_SEPAR) { // new line
			xval = x1;
			++yval;
		}
		else {
			loc2 = yval*x_blocks + xval;
			cond_add_char(circuit_matrix, tcomp[i], loc2, size);
			++xval;
		}
	}
}




void add_ic_to_matrix(char *circuit_matrix, int ID, unsigned int loc, unsigned int size) {

	unsigned int i, xlen;
	int x_offset_l, y_offset, y_offset2;
	IC ic;
		
	find_ic(ic, (ID - IC_base_id));
	
	xlen       = ic.pins >> 1;
	x_offset_l = xlen >> 1;
	y_offset   = text_comp_y_offset[IC_TEXT_ID];
	
	if (((int)loc)/x_blocks - y_offset >= y_blocks) { // out of window
		cout << "IC off screen." << endl;
		return;
	}
	y_offset2  = (y_offset >> 1)*x_blocks;
	y_offset  *= x_blocks;
	loc       -= x_offset_l;
	
	for (i = 0; i < xlen; ++i) {
		if (i == 0)
			cond_add_char(circuit_matrix, text_IC_S, loc, size);
		else if (i == xlen-1)
			cond_add_char(circuit_matrix, text_IC_E, loc + i, size);
			
		cond_add_char(circuit_matrix, text_IC_M, (loc + i + y_offset2), size);
		cond_add_char(circuit_matrix, text_VPIN, (loc + i + y_offset),  size);
		cond_add_char(circuit_matrix, text_IC_M, (loc + i - y_offset2), size);
		cond_add_char(circuit_matrix, text_VPIN, (loc + i - y_offset),  size);
	}	
}



void add_ff_to_matrix(char *circuit_matrix, int ID, char flags, unsigned int loc, unsigned int size) {

	connections x = get_connections(loc, ID, flags);

	add_comp_to_matrix(circuit_matrix, FF_TEXT_ID, flags, loc, size);
	
	cond_add_char(circuit_matrix, text_HPIN, x.in1,  size);
	cond_add_char(circuit_matrix, text_HPIN, x.in2,  size);
	cond_add_char(circuit_matrix, text_HPIN, x.in3,  size);
	cond_add_char(circuit_matrix, text_HPIN, x.out1, size);
	cond_add_char(circuit_matrix, text_HPIN, x.out2, size);
	cond_add_char(circuit_matrix, text_HPIN, x.out3, size);
}




void cond_add_char(char *circuit_matrix, char text_char, int loc, unsigned int size) {

	if (loc > 0 && loc < (int)size && circuit_matrix != NULL && circuit_matrix[loc] == text_NULL && text_char != ' ') {
		circuit_matrix[loc] = text_char;
	}
}





int get_tr_text_id(int comp_id) {

	transistor tr;
	
	tr = Qnum_find(tr, (comp_id - Q_base_id));
		
	switch (tr.z) {
		case 'n':
		case 'N':
			return NPN_TEXT_ID;
			
		case 'p':
		case 'P':
			return PNP_TEXT_ID;
			
		case 'f':
		case 'F':
			return NFET_ID;
			
		case 'g':
		case 'G':
			return PFET_ID;
			
		default:
			return TRANSISTOR_ID;
	}
	return TRANSISTOR_ID;
}




