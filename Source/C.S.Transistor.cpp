#include "CircuitSolver.h"
#include "Logic.h"
#include "Draw.h"
#include "StringTable.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Transistor.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Transistors
// By Frank Gennari
int const useQStringTable = 1;

string const unknown1 = "???";


int Qdata_size(0), Qdata_capacity(0);
StringTable Q_names(0);


extern transistor Add;
extern transistor *Qdata;	
extern int function_id, num_iterations, storeDATAA, timer, DEMO;
extern string last_qname;




component *pick_transistor(component *head, string name, int pins, string description);
int fill_tr_window(transistor *data, int count);
int show_transistor(transistor *tr, int count);
void show_tr_selection();
void display_info(transistor *tr);
void modify_transistor(transistor *tr);
void rewrite_data(const transistor& tr);
void draw_tr_mod_window(const transistor& temp);
void draw_tr_mod_selection();
int Qsearch(transistor& Model, string Q_select, int use_status_bar);
double get_q_type(int id);
transistor Qnum_find(transistor &tr, int num);
transistor *read_q_data(int screen, int &last_screen, int &last_q);
int count_qdata(transistor *data);
transistor *add_transistor(const transistor& tr, transistor *data, transistor *&tail);
void add_transistor_to_datafile(const transistor& tr);
void draw_transistors(transistor *data, int count);
void Q_values();
void enter_qvalues();
void NewQ();
void copy_transistor(transistor &New, transistor *old);
void copy_transistor(transistor *tr, const transistor& old);
void no_qdatafile();

point get_center(int box_num);






component *pick_transistor(component *head, string name, int pins, string description) {
	
	int Transistor, count, count2, count3(1), screen(1), last_screen, last_q, selection(-1), id;
	transistor *data = NULL, *temp2;
	transistor DATA;
	string find_name;
	
	do {
		destroyL(data);
		data = 0;
		last_q = 0;
		last_screen = 1;
		
		data = read_q_data(screen, last_screen, last_q);
		count = count_qdata(data);
		
		if (count > max_trs)
		cout << "\nThe file " << transistor_datafile << " contains too many transistor entries (max " << max_trs << ")." << endl;
					
		count2 = fill_tr_window(data, count);
		
		do {
			cout << "\nChoose a Transistor." << endl;	
			Transistor = box_clicked();
			
			if (Transistor == 0) { // <-				
				if (screen == 1) {
					destroyL(data);
					return head;
				}
				else
				--screen;
			}
			else if (Transistor == (count + 1)) { // ->				
				if (count == (MAX_BOXES - 2))
					++ screen;
				else {
					destroyL(data);
					return head;					
				}
			}
			else if (Transistor == 1 && screen == 1) { // new
				if (DEMO) 
					demo_error();
				else if (count < max_trs) {
					cout << "\nNEW" << endl;
					Q_values();
					data = read_q_data(screen, last_screen, last_q);
					count = count_qdata(data);
					count2 = fill_tr_window(data, count);
				}
				else
					cout << "\nThe file " << transistor_datafile << " is full. (" << max_trs << " transistors)" << endl;	
			}
			else if (Transistor == 2 && screen == 1) { // find
				cout << "\nEnter the name of the Transistor(case sensitive):  ";
				find_name = xin(MAX_INPUT_CHARACTERS);
				find_name = remove_spaces(find_name, find_name.length());
				id = Qsearch(DATA, find_name, 1);
				if (id > 0) {
					selection = show_transistor(&DATA, count);
					if (selection == 1) {
						destroyL(data);
						if (screen == 1)
							Transistor -= 2;
						return draw_component_at_click((Q_base_id + id), head, name, pins, description);
						}
				}
				count2 = fill_tr_window(data, count);
				
				Transistor = 1;
			}
		} while (Transistor > (count + 1) || (Transistor == (count + 1) && count < (MAX_BOXES - 2)) || (Transistor == 1 && screen == 1));
	
		if (Transistor != 0 && Transistor != (count + 1)) {
		
			temp2 = data;
			count3 = 1;
			
			while (count3 < Transistor && temp2->next != 0) {
			
				++count3;
				temp2 = temp2->next;
			}
			selection = show_transistor(temp2, count); 
		}	
	} while (selection == 0 || selection == -1);
		
	if (selection == 1) {
		destroyL(data);
		if (screen == 1)
			Transistor -= 2;
		return draw_component_at_click((Q_base_id + (screen - 1)*(MAX_BOXES - 2) + Transistor), head, name, pins, description);
	}
	destroyL(data);
	
	return head;
}




int fill_tr_window(transistor *data, int count) {

	transistor *temp = data;
	int count2(1);
	
	draw_selection_window(count);
			
	while (temp != 0) {	
		draw_transistors(temp, count2);		
		++count2;
		temp = temp->next;
	}
	return count2;
}






int show_transistor(transistor *tr, int count) {
	
	int selection;
	
	do {
		init_menu(6, 10, 0);
	
		show_tr_selection();
	
		draw_LARROW(0);
		
		draw_text(shift_chars(tr->name), 1, 12);
		
		cout << "\nClick on a square." << endl;
		
		selection = box_clicked();
		
		switch (selection) {
			
			case 2: // show info
				display_info(tr);
				break;
			
			case 3: // modify
				if (DEMO) {
					demo_error();
					break;	
				}
				modify_transistor(tr);	
				break;	
			
			case 4: // new
				if (DEMO) {
					demo_error();
					break;	
				}
				if (count < max_trs) {
					cout << "\nNEW" << endl;
					Q_values();
				}
				else
					cout << "\nThe file " << transistor_datafile << " is full. (" << max_trs << " transistors)" << endl;
				break;
		}
	} while (selection != 0 && selection != 1);

	return selection;
}





void show_tr_selection() {

	draw_text(" BACK", reserved_pos_value, 12);
	draw_text(" USE TRANSISTOR", -1, 12);
	draw_text(" SPECIFICATIONS", -2, 12);
	draw_text(" MODIFY", -3, 12);
	draw_text(" NEW", -4, 12);
}





void display_info(transistor *tr) {

	cout << "\n" << tr->name << endl;
	
	if (comp_char(tr->z, 'n'))
		cout << "NPN" << endl;
		
	else if (comp_char(tr->z, 'p'))
		cout << "PNP" << endl;
		
	else
		cout << "Unknown Type" << endl;
	
	if (tr->name == "")
		return;
		
	cout << "Maximum collector current = ";
	out(tr->q_icmax);
	cout << "A." << endl << "Maximum collector to base voltage = ";
	out(tr->q_cbmax);
	cout << "V." << endl << "Maximum collector to emitter voltage = ";
	out(tr->q_vcemax);
	cout << "V." << endl << "Maximum base to emitter voltage = ";
	out(tr->q_vbemax);
	cout << "V." << endl << "Beta = ";
	cout << tr->q_beta;
	cout << endl << "Maximum power dissipation = ";
	out(tr->q_pdmax);
	cout << "W." << endl << "Typical frequency = ";
	out(tr->q_tf);
	cout << "MHz." << endl << "Ideal base to emitter voltage = ";
	out(tr->q_vbe);
	cout << "V." << endl << "Ideal collector to emitter voltage during saturation = ";
	out(tr->q_vcesat);
	cout << "V." << endl;
}





void modify_transistor(transistor *tr) {
	
	transistor temp, temp2;
	int selection, is_modified(0);
	point c;
	
	copy_transistor(temp, tr);
	draw_tr_mod_window(temp);
	
	do {	
		cout << "\nClick on the transistor property you wish to change." << endl;
	
		selection = box_clicked();
	
		switch (selection) {
	
			case 0:
				if (!is_modified)
					break;
				copy_transistor(tr, temp); 
				if (storeDATAA) {
					for (int i = 0; i < Qdata_size; ++i) {
						if (tr->name == Qdata[i].name) {
							Qdata[i] = temp;
							rewrite_data(temp);
							return;	
						}
					}
					add_transistor_to_datafile(temp);
				}
				rewrite_data(temp);
				break;
			
			case 2:
				cout << "\nWarning: Changing transistor name can corrupt saved circuits and cause other problems! Proceed?  ";
				if (decision()) {
					cout << "\nEnter new name:  ";
					temp.name = xin(MAX_INPUT_CHARACTERS);
					temp.name = remove_spaces(temp.name, temp.name.length());
					is_modified = 1;
				}
				cout << "\nChange type? (This may also affect saved circuits.)  ";
				if (decision())
					do {
						cout << "\nIs the transistor to be added (n)pn or(p)np? ";
						temp.z = inc();
						is_modified = 1;
					} while(!comp_char(temp.z, 'n') && !comp_char(temp.z, 'p'));
				if (!is_modified)
					break;
				c = get_center(2);
				SetDrawingColorx(WHITE);
				RectangleDraw((c.x - 64), (c.y - 64), (c.x + 64), (c.y + 64));
				
				draw_text(shift_chars(temp.name), 2, 12);
				if (comp_char(temp.z, 'n'))
					draw_text(" NPN", -2, 12);
				else if (comp_char(temp.z, 'p'))
					draw_text(" PNP", -2, 12);
				break;
			
			case 3:
				cout << "\nMaximum collector current = ";
				out(temp.q_icmax);
				cout << "A." << endl;
				cout << "Enter the new maximum collector current(A): ";
				temp.q_icmax = ind(); 
				draw_double(temp.q_icmax, 'A', -3);
				is_modified = 1;
				break;
			
			case 4:
				cout << "\nMaximum collector to base voltage = ";
				out(temp.q_cbmax);
				cout << "V." << endl;			
				cout << "Enter the new maximum collector to base voltage(V): ";
				temp.q_cbmax = ind(); 
				draw_double(temp.q_cbmax, 'V', -4);
				is_modified = 1;
				break;
			
			case 5:
				cout << "\nMaximum collector to emitter voltage = ";
				out(temp.q_vcemax);
				cout << "V." << endl; 
				cout << "Enter the new maximum collector to emitter voltage(V): ";
				temp.q_vcemax = ind();
				draw_double(temp.q_vcemax, 'V', -5);
				is_modified = 1;
				break;
				
			case 6:
				cout << "\nMaximum base to emitter voltage = ";
				out(temp.q_vbemax);
				cout << "V." << endl;
				cout << "Enter the new maximum base to emitter voltage(V): ";
				temp.q_vbemax = ind();
				draw_double(temp.q_vbemax, 'V', -6);
				is_modified = 1;
				break;
			
			case 7:
				cout << "\nBeta = ";
				cout << temp.q_beta;
				cout << endl ;
				cout << "Enter new beta: ";
				temp.q_beta = ind();
				draw_double(temp.q_beta, ' ', -7);
				is_modified = 1;
				break;
			
			case 8:
				cout << "\nMaximum power dissipation = ";
				out(temp.q_pdmax);
				cout << "W." << endl;
				cout << "Enter the new maximum power dissipation(W): ";
				temp.q_pdmax = ind();
				draw_double(temp.q_pdmax, 'W', -8);
				is_modified = 1; 
				break;
			
			case 9:
				cout << "\nTypical frequency = ";
				out(temp.q_tf);
				cout << "MHz." << endl; 
				cout << "Enter the new typical frequency(MHz): ";
				temp.q_tf = ind();
				draw_double(temp.q_tf, 'H', -9);
				is_modified = 1;
				break;
		
			case 10:
				cout << "\nIdeal base to emitter voltage = ";
				out(temp.q_vbe);
				cout << "V." << endl;
				cout << "Enter the new ideal base to emitter voltage(V): ";
				temp.q_vbe = ind(); 
				draw_double(temp.q_vbe, 'V', -10);
				is_modified = 1;
				break;
			
			case 11:
				cout << "\nIdeal collector to emitter voltage during saturation = ";
				out(temp.q_vcesat);
				cout << "V." << endl;
				cout << "Enter the new ideal collector to emitter voltage during saturation(V): ";
				temp.q_vcesat = ind();
				draw_double(temp.q_vcesat, 'V', -11);
				is_modified = 1; 
				break;
		}
	} while (selection > 1);
}





void rewrite_data(const transistor& tr) {
	
	int in_data_folder(1);
	char ifname2[MAX_CHARACTERS + 1] = {0};	
	fstream tfile;
	transistor Q;
	
	strcpy(ifname2, data_folder);
	strcat(ifname2, transistor_datafile);
	
	tfile.open(transistor_datafile, ios::in);
	
	if (tfile.fail()) {
		tfile.clear();
		tfile.open(ifname2, ios::in);
	}
	else
		in_data_folder = 0;
		
	if (!storeDATAA) {		
		if (tfile.fail()) 
			no_qdatafile();
		else {	
			while (tfile >> Q.name >> Q.z >> Q.q_icmax >> Q.q_cbmax >> Q.q_vcemax >> 
				   Q.q_vbemax >> Q.q_beta >> Q.q_pdmax >> Q.q_tf >> Q.q_vbe >> Q.q_vcesat) {	
				
				if (Q.name == tr.name) 
					add_transistor_to_datafile(tr);	
				else
					add_transistor_to_datafile(Q);	
					// delete current line (Q) and replace with (tr) <- WON'T WORK
					/*tfile << tr.name << " " << tr.z << " " << tr.q_icmax << " " << tr.q_cbmax << " " 
					<< tr.q_vcemax << " " << tr.q_vbemax << " " << tr.q_beta << " " << tr.q_pdmax << " " 
					<< tr.q_tf << " " << tr.q_vbe << " " << tr.q_vcesat;
					
					tfile.close();
					return;
				}*/		
			}
			tfile.close();
			tfile.clear();
		}
	}
	else {
		tfile.close();
		tfile.clear();
	}
	
	if (in_data_folder)
		tfile.open(ifname2, ios::out | ios::trunc);
	else
		tfile.open(transistor_datafile, ios::out | ios::trunc);
		
	if (tfile.fail()) {
		beep();
		cerr << "\n\nError:" << transistor_datafile << " could not be found or created!" << endl;		
		return;
	}
	cout << "\nRewriting Transistor Data File. Please wait..." << endl;
	
	for (unsigned int i = 0; (int)i < Qdata_size; ++i) {
		if (i > 0)
			tfile << "\n";
		tfile << Qdata[i].name << " " << Qdata[i].z << " " << Qdata[i].q_icmax << " " << Qdata[i].q_cbmax << " " 
	  	      << Qdata[i].q_vcemax << " " << Qdata[i].q_vbemax << " " << Qdata[i].q_beta << " " << Qdata[i].q_pdmax << " " 
	          << Qdata[i].q_tf << " " << Qdata[i].q_vbe << " " << Qdata[i].q_vcesat;
	}
	if (!storeDATAA) {
		delete [] Qdata;
		Qdata_size = 0;
		Qdata_capacity = 0;
	}
		
	tfile.close();
}





void draw_tr_mod_window(const transistor& temp) {

	init_menu(12, 10, 0);
	
	if (comp_char(temp.z, 'n'))
		draw_text(" NPN", -2, 12);
		
	else if (comp_char(temp.z, 'p'))
		draw_text(" PNP", -2, 12);
	
	draw_tr_mod_selection();
		
	draw_text(shift_chars(temp.name), 2, 12);
		
	draw_double(temp.q_icmax, 'A', -3);
	draw_double(temp.q_cbmax, 'V', -4);
	draw_double(temp.q_vcemax, 'V', -5);
	draw_double(temp.q_vbemax, 'V', -6);
	draw_double(temp.q_beta, ' ', -7);
	draw_double(temp.q_pdmax, 'W', -8);
	draw_double(temp.q_tf, 'H', -9);
	draw_double(temp.q_vbe, 'V', -10);
	draw_double(temp.q_vcesat, 'V', -11);
}





void draw_tr_mod_selection() {

	draw_text(" DONE", reserved_pos_value, 12);
	draw_text(" CANCEL", -1, 12);
	
	draw_text(" Max C Current", 3, 12);
	draw_text(" Max C to B Volt.", 4, 12);
	draw_text(" Max C to E Volt.", 5, 12);
	draw_text(" Max B to E Volt.", 6, 12);
	draw_text(" Beta", 7, 12);
	draw_text(" Max Power Diss.", 8, 12);
	draw_text(" Typical Frequency", 9, 12);
	draw_text(" Ideal B to E Volt.", 10, 12);
	draw_text(" Ideal C to E Vsat", 11, 12);
}





int Qsearch(transistor& Model, string Q_select, int use_status_bar) {
	
	ifstream infile;					
	transistor Q;
	int match, total, count(0);
	
	match = 0;	
	
	if (use_status_bar) {
		show_clock();
	
		cout << "\nReading Component Data.";
		
		if (use_status_bar == 1)
			draw_text_on_window(" Reading Data");
	}
	if (storeDATAA) {
		if (Qdata_size == 0) {
			no_qdatafile();
			return 0;
		}
		if (useQStringTable) {
			total = Q_names.search(Q_select);
			if (total >= 0 && total < Qdata_size && (comp_char(Qdata[total].z, 'n') || comp_char(Qdata[total].z, 'p'))) {
				Model = Qdata[total];
				match = 1; 
				count = total+1;
				last_qname = Qdata[total].name;
			}
		}
		else {		
			for (total = 0; total < Qdata_size; ++total) {
					
				if (use_status_bar && !((total+1)%search_value))  
					cout << ".";
			
				if ((Qdata[total].name == Q_select) && (comp_char(Qdata[total].z, 'n') || comp_char(Qdata[total].z, 'p'))) {
					Model = Qdata[total]; 
					match = 1;
					count = total+1;
					last_qname = Qdata[total].name;
				}
			}
		}
	}
	else {					
		if (!infile_file(infile, transistor_datafile)) {
			no_qdatafile();
			return 0;
		}
		total = 0;
	
		while (infile.good() && infile >> Q.name >> Q.z >> Q.q_icmax >> Q.q_cbmax >> Q.q_vcemax >> 
			   Q.q_vbemax >> Q.q_beta >> Q.q_pdmax >> Q.q_tf >> Q.q_vbe >> Q.q_vcesat) {
		  
			++total;
		
			if (use_status_bar && !(total%search_value))  
				cout << ".";
		
			if ((Q.name == Q_select) && (comp_char(Q.z, 'n') || comp_char(Q.z, 'p'))) {
				Model = Q; 
				match = 1;
				count = total;
				last_qname = Q.name;
			}					
		}
		infile.close();
	}
	
	if (use_status_bar) {
		if (!useQStringTable)
			cout << "\n" << total << " component entries were searched." << endl;
	
		if (total > max_trs)
			cout << "\nThe file " << transistor_datafile << " contains too many transistor entries (max " << max_trs << ")." << endl;
	
		if (match)
			cout << "\nThe transistor " << Q_select << " was found." << endl;
		else
		cout << "\n!The transistor " << Q_select << " was NOT found!" << endl;
		
		reset_cursor();
	}
	if (match)					
		return count;
	else
		return -count;
}





double get_q_type(int id) {
	
	transistor tr;
	
	tr = Qnum_find(tr, (id - Q_base_id));
				
	switch (tr.z) {
		case 'n':
		case 'N':
			return 1;
						
		case 'p':
		case 'P':
			return 2;

		default:
			return 0;
	}
}				





transistor Qnum_find(transistor &tr, int num) {

	ifstream infile;
	int count(0);
		
	tr.name = unknown1;
	tr.z = 'x';
	tr.name = "";
	tr.z = 0;
	tr.q_icmax = tr.q_cbmax = tr.q_vcemax = tr.q_vbemax = tr.q_beta = tr.q_pdmax = tr.q_tf = tr.q_vbe = tr.q_vcesat = 0;
	
	if (storeDATAA) {
		if (Qdata_size == 0) {
			no_qdatafile();
			return tr;
		}
		if (num < 1 || num > Qdata_size)
			return tr;
					
		tr = Qdata[num-1];
		return tr;
	}
	else { 
		if (!infile_file(infile, transistor_datafile)) {
			no_qdatafile();
			return tr;
		}	
		while (count < num && infile.good() && infile >> tr.name >> tr.z >> tr.q_icmax >> tr.q_cbmax >> tr.q_vcemax >> tr.q_vbemax
		 	   >> tr.q_beta >> tr.q_pdmax >> tr.q_tf >> tr.q_vbe >> tr.q_vcesat) 
			++count;
		
		if (count != num) {
			tr.name = unknown1;
			tr.z = 'x';
			tr.name = "";
			tr.z = 0;
			tr.q_icmax = tr.q_cbmax = tr.q_vcemax = tr.q_vbemax = tr.q_beta = tr.q_pdmax = tr.q_tf = tr.q_vbe = tr.q_vcesat = 0;
		}
	}
	return tr;
}





transistor *read_q_data(int screen, int &last_screen, int &last_q) {

	ifstream infile;
	transistor Q;
	transistor *data = NULL, *tail = NULL;
	int total(0), screenx(1);
	
	cout << "\nReading Transistor Data.";
	
	draw_text_on_window(" Reading Data");
	
	show_clock();
	
	if (screen == 1) {
		Q.name = "NEW";
		Q.z = 'x';
		Q.q_icmax = Q.q_cbmax = Q.q_vcemax = Q.q_vbemax = Q.q_beta = Q.q_pdmax = Q.q_tf = Q.q_vbe = Q.q_vcesat = 0;
		data = add_transistor(Q, data, tail);
		++total;
		++last_q;
		Q.name = "FIND";
		data = add_transistor(Q, data, tail);
		++total;
		++last_q;
	}
	if (storeDATAA) {
		if (Qdata_size == 0) {
			no_qdatafile();
			return data;
		}		
		for (; last_q < Qdata_size + 2*(screen == 1) && last_screen <= screen; ++last_q) {
		
			if (screen == last_screen) { 
				if (screen == 1)
					data = add_transistor(Qdata[last_q-2], data, tail);
				else
					data = add_transistor(Qdata[last_q], data, tail);
			}	
			if ((last_q+1)%(MAX_BOXES - 2) == 0) 
				++last_screen;
			
			if (!((last_q+1)%search_value))  
				cout << ".";					
		}
	}
	else {
		if (!infile_file(infile, transistor_datafile)) {
			no_qdatafile();
			return data;
	}
		while (screenx <= screen && infile.good() && infile >> Q.name >> Q.z >> Q.q_icmax >> Q.q_cbmax >> Q.q_vcemax >> 
		  	Q.q_vbemax >> Q.q_beta >> Q.q_pdmax >> Q.q_tf >> Q.q_vbe >> Q.q_vcesat) {
			
			if (screen == screenx) 
				data = add_transistor(Q, data, tail); 
		
			++total;
				
			if (total%(MAX_BOXES - 2) == 0 && total != 0) 
				++screenx;
			
			if (!(total%search_value))  
				cout << ".";
		}
		infile.close();	
	}
	cout << endl;
	
	reset_cursor();
	
	return data;
} 





int count_qdata(transistor *data) {

	int count(0);

	while (data != 0) {
		++count;
		data = data->next;
	}
	return count;
}





transistor *add_transistor(const transistor& tr, transistor *data, transistor *&tail) {
			
	if (transistor *new_transistor = new transistor) {
						
		copy_transistor(new_transistor, tr);
		
		if (data == 0) {
			tail = new_transistor;
			return new_transistor;
		}		
		tail->next = new_transistor;
		tail = new_transistor;
		return data;
	}
	else {
		cerr << "\nError: Transistor data could not be read because Circuit Solver is out of memory." << endl
			 << "To continue, allocate more memory to Circuit Solver.";
				 
		out_of_memory();
	}
	return data;
}




void add_transistor_to_datafile(const transistor& tr) {

	int i, ret_val;
		
	if (Qdata_size < Qdata_capacity) {
		Qdata[Qdata_size] = tr;
		Qdata[Qdata_size].next = &Qdata[Qdata_size-1];
	}
	else { // double aray size
		transistor *old_data = Qdata;
		
		if (Qdata_capacity == 0)
			Qdata_capacity = 1;
		
		Qdata_capacity *= 2;
		Qdata = memAlloc(Qdata, Qdata_capacity);
			
		for (i = 0; i < Qdata_size; ++i) { 
			Qdata[i] = old_data[i];
			Qdata[i].next = &Qdata[i-1];
		}			
		for (; i < Qdata_capacity; ++i) {
			Qdata[i].name = unknown1;
			Qdata[i].next = &Qdata[i-1];
		}	
		Qdata[Qdata_size] = tr;
		
		if (Qdata_size == 0)
			Qdata[Qdata_size].next = 0;
		else
			Qdata[Qdata_size].next = &Qdata[Qdata_size-1];
			
		delete [] old_data;
	}
	ret_val = Q_names.insert(tr.name, Qdata_size);
	
	if (ret_val == 0)
		cerr << "Warning: Duplicate name found in Transistor datafile: " << tr.name << endl;
	
	++Qdata_size;
}





void draw_transistors(transistor *data, int count) {
	
	if (comp_char(data->z, 'n'))
		draw_NPN(count);
		
	else if (comp_char(data->z, 'p'))
		draw_PNP(count);
	
	if (data->name == "NEW" || data->name == "FIND")
		draw_text(shift_chars(data->name), -count, 12);
		
	else	
		draw_text(shift_chars(data->name), count, 12);
}





void Q_values() {

	function_id = 5;

	enter_qvalues();					 	
	NewQ();
}






void enter_qvalues() {
	
	transistor Q;
	int q_count;
	
	do {
		cout << "\nWhat is the name or part # of this transistor(lowercase letters)? " << endl;
		Add.name = xin(MAX_INPUT_CHARACTERS);
		Add.name = remove_spaces(Add.name, Add.name.length());
		q_count = Qsearch(Q, Add.name, 0);
		
		if (q_count > 0) {
			beep();
			cout << "\nThat name is already taken. Please use a different one." << endl;
		}	
	} while (q_count > 0);
	
	do {
		cout << "\nIs the transistor to be added (n)pn or(p)np? ";
		Add.z = inc();
		
		if(!comp_char(Add.z, 'n') && !comp_char(Add.z, 'p'))
			beep();
			
	} while(!comp_char(Add.z, 'n') && !comp_char(Add.z, 'p'));
	
	if (Add.z == 'N')
		Add.z = 'n';
		
	else if (Add.z == 'P')
		Add.z = 'p';
	
	cout << "Enter the maximum collector current(A): ";
	Add.q_icmax = ind();
	cout << "Enter the maximum collector to base voltage(V): ";
	Add.q_cbmax = ind();
	cout << "Enter the maximum collector to emitter voltage(V): ";
	Add.q_vcemax = ind();
	cout << "Enter the maximum base to emitter voltage(V): ";
	Add.q_vbemax = ind();
	cout << "Enter beta: ";
	Add.q_beta = ind();
	cout << "Enter the maximum power dissipation(W): ";
	Add.q_pdmax = ind();
	cout << "Enter the typical frequency(MHz): ";
	Add.q_tf = ind();
	cout << "Enter the ideal base to emitter voltage(V): ";
	Add.q_vbe = ind();
	cout << "Enter the ideal collector to emitter voltage during saturation(V): ";
	Add.q_vcesat = ind();
	
	last_qname = Add.name;
}




void NewQ() {
	
	ofstream outfile;
	
	show_clock();
	
	if (storeDATAA)
		add_transistor_to_datafile(Add);
	
	if (!outfile_file(outfile, transistor_datafile, 1)) {
		beep();
		cerr << "\n\nError:" << transistor_datafile << " could not be found or created!" << endl;		
		reset_cursor();
		return;
	}		
	outfile << "\n" << Add.name << " " << Add.z << " " << Add.q_icmax << " " << Add.q_cbmax << " " 
			<< Add.q_vcemax << " " << Add.q_vbemax << " " << Add.q_beta << " " << Add.q_pdmax << " " 
			<< Add.q_tf << " " << Add.q_vbe << " " << Add.q_vcesat;

	if (!outfile.good()) {
		beep();
		cerr << "Error: Could not write new transistor to file." << endl;
	}
	else {
		cout << "\nThe new transistor values have been entered into the component datafile." << endl;
	}	
	outfile.close();
	reset_cursor();
	
	return;
}





void copy_transistor(transistor &New, transistor *old) {
		
	New.name = old->name;								
	New.z = old->z;
	New.q_icmax = old->q_icmax;
	New.q_cbmax = old->q_cbmax;
	New.q_vcemax = old->q_vcemax;
	New.q_vbemax = old->q_vbemax;
	New.q_beta = old->q_beta;
	New.q_pdmax = old->q_pdmax;
	New.q_tf = old->q_tf;
	New.q_vbe = old->q_vbe;
	New.q_vcesat = old->q_vcesat;
}





void copy_transistor(transistor *tr, const transistor& old) {
	
	tr->name = old.name;								
	tr->z = old.z;
	tr->q_icmax = old.q_icmax;
	tr->q_cbmax = old.q_cbmax;
	tr->q_vcemax = old.q_vcemax;
	tr->q_vbemax = old.q_vbemax;
	tr->q_beta = old.q_beta;
	tr->q_pdmax = old.q_pdmax;
	tr->q_tf = old.q_tf;
	tr->q_vbe = old.q_vbe;
	tr->q_vcesat = old.q_vcesat;
	tr->next = 0;
}




void no_qdatafile() {
	
	if (num_iterations != 0 || fabs((float)timer - (float)get_time()) < pause_time)
		return;
	
	timer = get_time();		
	beep();	
	cerr << "\n\nError: " << transistor_datafile << " could not be found! Please make sure it is in the folder/directory 'Circuit Solver.Data' or in the same "
		 << "folder/directory as Circuit Solver, or select 'NEW' to create a new datafile." << endl;						 		 
	reset_cursor();
}



