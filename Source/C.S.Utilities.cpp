#include "CircuitSolver.h"
#include "Equation.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Utilities.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Utilities
// By Frank Gennari
int const dont_show_timer = 0;
int const use_short_time  = 1;


int useDIALOG(1), saveBOXES(1), storeDATAA(1), storeDATAB(1), counter, cs_time(0), cs_elapsed_time(0), just_reset(0);
char input[MAX_ENTERED_CHARACTERS + 1] = {0};
transistor *Qdata;
IC *ICdata;


extern int clock_speed, time_ms, use_system_tick_for_delay;
extern double last_in, last_out;


string shift_chars(string text);
void replace_spaces(string &str, char replace_char);
void draw_sized_string(const char *display, int tsize);

void add_transistor_to_datafile(const transistor& tr);
void add_ic_to_datafile(string name, string description, int pins);

void quit_app();
void clear_buffer();
int decision_dialog(int id);
int got_click();
void set_text_size(int size);

void language_handler(int count, string text); 







int valid_number(double number) {
	
	if (number > 1e249)
		return 0;
	
	if (number > 0 || number < 0 || number == 0)
		return 1;
		
	return 0;
}




void out(double value) {

	double scaled;
	char units;
	
	units = get_units_and_scale(value, scaled);
	
	cout << scaled << " " << units;
	
	last_out = value;
}




void write_units(ofstream &outfile, double value) {
	
	double scaled;
	char units;
	
	units = get_units_and_scale(value, scaled);

	if (!outfile.good())
		return;
	
	outfile << scaled << units << endl;
}




char get_units_and_scale(double value, double &scaled) {

	int vsign(1);

	if (value < 0) {
		value = -value;
		vsign = -1;
	}
	if (value < .9999e-18) {
		scaled = value;
		//scaled = 0; // close enough to zero?
		return ' ';
	}
	else if (value < .9999e-15) {
		scaled = (vsign*value/1e-18);
		return 'a';
	}
	else if (value < .9999e-12) {
		scaled = (vsign*value/1e-15);
		return 'f';
	}	
	else if (value < .9999e-9) {
		scaled = (vsign*value/1e-12);
		return 'p';
	}	
	else if (value < .9999e-6) {
		scaled = (vsign*value/1e-9);
		return 'n';
	}			
	else if (value < .9999e-3) {
		scaled = (vsign*value/1e-6);
		return MU_CHAR;
	}	
	else if (value < .9999) {
		scaled = (vsign*value/1e-3);
		return 'm';
	}	
	else if (value < .9999e3) {
		scaled = vsign*value;
		return ' ';
	}	
	else if (value < .9999e6) {
		scaled = (vsign*value/1e3);
		return 'K';
	}	
	else if (value < .9999e9) {
		scaled = (vsign*value/1e6);
		return 'M';
	}	
	else if (value < .9999e12) {
		scaled = (vsign*value/1e9);
		return 'G';
	}	
	else if (value < .9999e15) {
		scaled = (vsign*value/1e12);
		return 'T';
	}	
	else if (value < .9999e18) {
		scaled = (vsign*value/1e15);
		return 'P';
	}	
	else {
		scaled = (vsign*value/1e18);
		return 'E';
	}
	scaled = 0;
	return ' ';
}





int in() { // Input int

	int num_i;
	
	do {
		strcpy(input, xin(MAX_INPUT_CHARACTERS));
			
		num_i = atoi(input);
	
		if (input[0] != '0' && num_i == 0) {
			beep();
			cerr << "Invalid Number! Reenter:  ";
		}			
	} while (input[0] != '0' && num_i == 0);
	
	last_in = (double)num_i;
	
	return num_i;
}





double ind() { // Input double	
	
	double x(0);
	static cs_equation d_eq(MAX_INPUT_CHARACTERS);
	
	d_eq.input("Invalid Number! Reenter:  ");
	
	if (comp_char(d_eq.is_one_char(), 'p'))
		return last_in;
			
	if (d_eq.var_with_x()) {
		cout << "\nEnter x:  ";
		x = ind();
	}
	last_in = d_eq.eval(x);
	
	return last_in;
}	





char inc() { // Input character
	
	strcpy(input, xin(MAX_INPUT_CHARACTERS));
		
	while (input[1] != 0) {
	
		beep();
		cerr << "Invalid selection! Reenter:  ";
		
		strcpy(input, xin(MAX_INPUT_CHARACTERS));			
	}			
	return input[0];
}




unsigned int inp() { // Input positive integer

	int num_i;
	
	do {
		strcpy(input, xin(MAX_INPUT_CHARACTERS));
			
		num_i = atoi(input);
	
		if (input[0] != '0' && num_i == 0) {
			beep();
			cerr << "Invalid Number! Reenter:  ";
		}
		else if (num_i < 0) {
			beep();
			cerr << "Enter a positive integer only:  ";
		}			
	} while ((input[0] != '0' && num_i == 0) || num_i < 0);
	
	last_in = (double)num_i;
	
	return num_i;
}




unsigned int inpg0() {

	int num_i;
	
	do {
		strcpy(input, xin(MAX_INPUT_CHARACTERS));
			
		num_i = atoi(input);
	
		if (input[0] != '0' && num_i == 0) {
			beep();
			cerr << "Invalid Number! Reenter:  ";
		}
		else if (num_i <= 0) {
			beep();
			cerr << "Enter a nonzero positive integer only:  ";
		}			
	} while (num_i <= 0);
	
	last_in = (double)num_i;
	
	return num_i;
}




char *double_to_char(double number) {
	
	int exponent;
	
	for (unsigned int i = 0; i < (unsigned)MAX_CHARACTERS; ++i)
		input[i] = 0;
		
	exponent = get_num_exp(number);
	
	if (exponent == 0)
		sprintf(input, "%f", number);
	else 
		sprintf(input, "%fe%i", number, exponent);
		
	return input;
}




void string_to_char(char *output, string input, int num_chars) {
	
	unsigned int i, max_i = min((unsigned)min(num_chars, MAX_CHARACTERS-1), (unsigned)input.length());
			
	for (i = 0; i < max_i && input[i] != '\0'; ++i) 
		output[i] = input[i]; 
		
	output[i] = 0;
}




int comp_char(char A, char B) {	
	
	if (A == B || A == (char)(B - CHAR_OFFSET)) // 'a' - 'A' should = 32
		return 1;
		
	return 0;
}




int compare_array(const void *A, const void *B) {

	const long *a = (const long *)A, *b = (const long *)B;

	if (*a < *b)
		return -1;
	else if (*a > *b)
		return 1;
	else
		return 0;
}




int get_num_exp(double &number) {
	
	int exponent(0);
	
	if (fabs(number) >= 10000 && number != 0) 
		while (fabs(number) >= 10 && exponent < 400) {
			number /= 10;
			++exponent;
		}
	else if (fabs(number) < .01 && number != 0 && exponent > -400)
		while (fabs(number) < 1) {
			number *= 10;
			--exponent;
		}
	return exponent;
}




// checks in application's folder, data folder, and circuit folder
int infile_file(ifstream &infile, const char *ifname) {  
	
	char ifname2[MAX_CHARACTERS + 1] = {0}, ifname3[MAX_CHARACTERS + 1] = {0};
	
	strcpy(ifname2, data_folder);
	strcat(ifname2, ifname);
	strcpy(ifname3, circuit_folder);
	strcat(ifname3, ifname);
	
	infile.open(ifname, ios::in);
	
	if (infile.fail()) {
		infile.clear();
		infile.open(ifname2, ios::in);
		if (infile.fail()) {
			infile.clear();
			infile.open(ifname3, ios::in);
		}
		else
			return 2;
	}
	else
		return 1;
		
	if (infile.fail())
		return 0;
		
	return 3;
}



// checks in application's folder and data folder
int outfile_file(ofstream &outfile, const char *ifname, int app) {  
	
	ifstream infile;
	char ifname2[MAX_CHARACTERS + 1] = {0};
	
	strcpy(ifname2, data_folder);
	strcat(ifname2, ifname);
	
	infile.open(ifname2, ios::in);
	
	if (app) {
		if (!infile.fail()) {
			infile.close();
			outfile.open(ifname2, ios::out | ios::app);
		}
		else {
			infile.clear();
			infile.open(ifname, ios::in);
		
			if (!infile.fail()) {
				infile.close();
				outfile.open(ifname, ios::out | ios::app);
			}
			else {
				outfile.open(ifname2, ios::out | ios::app);
			
				if (outfile.fail() || !filestream_check(outfile)) {
					outfile.clear();
					outfile.open(ifname, ios::out | ios::app);
				}
			}
		}
	}
	else {
		if (!infile.fail()) {
			infile.close();
			outfile.open(ifname2, ios::out | ios::trunc);
		}
		else {
			infile.clear();
			infile.open(ifname, ios::in);
		
			if (!infile.fail()) {
				infile.close();
				outfile.open(ifname, ios::out | ios::trunc);
			}
			else {
				outfile.open(ifname2, ios::out | ios::trunc);
			
				if (outfile.fail() || !filestream_check(outfile)) {
					outfile.clear();
					outfile.open(ifname, ios::out | ios::trunc);
				}
			}
		}
	}
	if (outfile.fail() || !filestream_check(outfile))
		return 0;
		
	return 1;
}






int overwrite_file(const char *name) {  // checks in application's folder and data folder
	
	ifstream infile;
	 
	if (infile_file(infile, name)) {
		infile.close();
		cout << "\n" << name << " already exists. Replace it?  ";
		if (decision())
			return 1;
		else 
			return 0;
	}
	else
		infile.clear();
	return 1;
}




int filestream_check(ifstream &filestream) {

	if (!filestream.good()) {
		filestream.close();
		cerr << "Error in input file stream." << endl;
		return 0;
	}
	return 1;
}




int filestream_check(ofstream &filestream) {

	if (!filestream.good()) {
		filestream.close();
		cerr << "Error in output file stream." << endl;
		return 0;
	}
	return 1;
}






int decision() {

	int cont(3), bad_count(0);
	string YorN;
	
	while (cont == 3) {
		
		YorN = xin(MAX_INPUT_CHARACTERS);
		
		if (YorN == "y" || YorN == "Y" || YorN == "yes" || YorN == "Yes" || YorN == "YES")
			cont = 1;
		
		else if (YorN == "n" || YorN == "N" || YorN == "no" || YorN == "No" || YorN == "NO") 
			cont = 0;
		
		else {
			++bad_count;
			language_handler(bad_count, "Yes or No? ");
		}
	}		
	return cont;
}





int xdecision(int id) {

	int cont(3), bad_count(0);
	string YorN;
		
	if (useDIALOG && (id == 1 || id == 2)) 
		return decision_dialog(id);
	
	cout << "(Yes, No, or Cancel)? ";
	
	while (cont == 3) {
		
		YorN = xin(MAX_INPUT_CHARACTERS);
		
		if (YorN == "y" || YorN == "Y" || YorN == "yes" || YorN == "Yes" || YorN == "YES")
			cont = 1;
		
		else if (YorN == "n" || YorN == "N" || YorN == "no" || YorN == "No" || YorN == "NO") 
			cont = 0;
			
		else if (YorN == "c" || YorN == "C" || YorN == "cancel" || YorN == "Cancel" || YorN == "CANCEL") 
			cont = 2;
		
		else {
			++bad_count;
			language_handler(bad_count, "Yes, No, or Cancel? ");
		}
	}		
	return cont;
}




void language_handler(int count, string text) {

	beep();
	
	if (count > 3)
		cout << "Please, just enter a simple answer: ";
		
	cout << text;
}





string shift_chars(string text) {
	
	char text2[MAX_CHARACTERS + 2] = {0};
	unsigned int count, max_count = min((unsigned)MAX_CHARACTERS, text.length());
	
	for (count = 0; count < max_count && text[count] != '\0'; ++count) {
		text2[count + 1] = text[count]; 
	}
	if (count == 0)
		return "_Unknown";
	
	text2[0] = '_';
	text2[count + 1] = '\0';
	
	return text2;
}





int shift_and_remove_zeros(char *display, char *display2) {
	
	int count1, count2, found_decimal(0);
	
	display2[0] = 'x';
	
	for (count1 = 0, count2 = 0; count1 < MAX_CHARACTERS - 4 && display[count1] != 0; ++count1) {
		if (display[count1] == '.')
			found_decimal = 1;
		
		if (!found_decimal || atof(display + count1) != 0 || (display[count1] != '0' && display[count1] != '.' && display[count1] != 0)) {
			display2[count2 + 1] = display[count1];
			++count2;
		}
	}	
	return count2;
}




void replace_spaces(string &str, char replace_char) {

	for (unsigned int i = 0; i < str.size(); ++i) {
		if (str[i] == ' ')
			str[i] = replace_char;
	}
}




void draw_string(const char *display) {

	char *display2 = convert_to_constparam255(display);
	draw_string2(display2);
	free(display2);
}




void draw_sized_string(const char *display, int tsize) {
	
	set_text_size(tsize);
	char *display2 = convert_to_constparam255(display);
	draw_string2(display2);
	free(display2);
	set_text_size(12);
}




void draw_string2(const char *display) {

	draw_string2((const unsigned char *)display);
}




char *convert_to_constparam255(const char *display) {

	char *display2 = _strdup(display);

	if (strlen(display) == 0)
		return display2;

	//PARAM255 is only 255 char's long
	if (strlen(display2) < 253) //Its okay
		display2[0] = (char)(strlen(display2) - 1);
		
	else //Truncate to 254 characters 
		display2[0] = (unsigned char)254;

	return display2;
}




void print_ch(unsigned char *ch) { // Need for PC at least

	for (unsigned int i = 0; i < MAX_CHARACTERS-1 && ch[i] != 0; ++i)
		cout << (char)ch[i];
}




void print_err_ch(unsigned char *ch) { // Same as above, but prints to cerr

	for (unsigned int i = 0; i < MAX_CHARACTERS-1 && ch[i] != 0; ++i)
		cerr << (char)ch[i];
}





int sign(int num) {

	if (num >= 0)
		return 1;
		
	if (num < 0)
		return -1;
		
	return 0;
}





void delay(double time) {
	
	if (use_system_tick_for_delay) {
		reset_timer(1);
		//while (elapsed_time() < 1000*time) {};
	}
	else
		for (int x = 0; x < DELAY*time*clock_speed; ++x){};
}




// this is a good random number generator written by Stephen E. Derenzo
void randome(long &s1ptr, long &s2ptr, double &ranptr) {

	if ((s1ptr = 40014*(s1ptr%53668) - 12211*(s1ptr/53668)) < 0) {
		s1ptr += 2147483563;
	}
	if ((s2ptr = 40692*(s2ptr%52774) - 3791*(s2ptr/52774)) < 0) {
		s2ptr += 2147483399;
	}
	if ((ranptr = s1ptr - s2ptr) < 1) {
		ranptr += 2147483562.;
	}
	ranptr = ranptr/2147483563.;
}





void read_data() {
	
	ifstream infile;
	transistor Q;
	IC ic;
	string name, description;
	int pins;
			
	if (!infile_file(infile, transistor_datafile)) {
		cout << transistor_datafile << " *NOT* Found!" << endl;
		storeDATAA = 0;
	}	
	else {
		cout << transistor_datafile << " Found." << endl;
		if (storeDATAA) {
			cout << "Reading " << transistor_datafile << "..." << endl;
			
			while (infile.good() && infile >> Q.name >> Q.z >> Q.q_icmax >> Q.q_cbmax >> Q.q_vcemax >> 
		  		   Q.q_vbemax >> Q.q_beta >> Q.q_pdmax >> Q.q_tf >> Q.q_vbe >> Q.q_vcesat) 
		  		   
		  		   add_transistor_to_datafile(Q);
		}
	}	
	infile.close();
	infile.clear();
		
	if (!infile_file(infile, ic_datafile)) {
		cout << ic_datafile << " *NOT* Found!" << endl;
		storeDATAB = 0;
	}
	else {
		cout << ic_datafile << " Found." << endl;
		if (storeDATAB) {
			cout << "Reading " << ic_datafile << "..." << endl;
			
			while (infile.good() && infile >> name >> description >> pins)
				add_ic_to_datafile(name, description, pins);
		}
	}			
	infile.close();
}




void reset_timer(int is_ms) {

#ifdef HAS_MS_TIMER
	if (is_ms)
		time_ms = 1;
	else
		time_ms = 0;
#else
	time_ms = 0;
#endif

	cs_time = get_time();
	cs_elapsed_time = 0;
	just_reset = 1;
}


void start_timer() {

	cs_time = get_time();
}


void stop_timer() {

	cs_elapsed_time += get_time() - cs_time;
}


int elapsed_time() {

	return cs_elapsed_time + get_time() - cs_time;
}


void show_elapsed_time() {

	char time[32];
	int etime;
	
	if (dont_show_timer)
		return;
		
	if (just_reset) {
		cout << ".";
		just_reset = 0;
	}
	else
		cout << " ";
	
	etime = elapsed_time();
	format_time(etime, time);	
	print_ch((unsigned char *)time);
}


void format_time(int time, char *time_string) {

	unsigned int hr, minute, sec, subsec(0), ss1, ss2, ss3;
	float fp_time;
	
	if (time_ms) {
		subsec = time%1000;
		fp_time = (float)(time/1000.0);
		time = time/1000;
	}
	else {
		fp_time = (float)time;
	}
	if (use_short_time) {
		sec = (unsigned int)fp_time;
		subsec = (unsigned int)(1000*((fp_time + .0001) - (double)sec));
		ss1 = subsec%10;      // thousandths
		ss2 = (subsec/10)%10; // hundredths
		ss3 = subsec/100;     // tenths
		sprintf(time_string, " %i.%i%i%i ", sec, ss3, ss2, ss1);
		return;
	}
	hr =  time/3600;
	minute = (time/60)%60;
	sec = time%60;
	
	if (minute < 10) {
		if (sec < 10)
			sprintf(time_string, " %i:0%i:0%i.%i ", hr, minute, sec, subsec);
		else
			sprintf(time_string, " %i:0%i:%i.%i ", hr, minute, sec, subsec);
	}
	else {
		if (sec < 10)
			sprintf(time_string, " %i:%i:%i.%i ", hr, minute, sec, subsec);
		else
			sprintf(time_string, " %i:%i:%i.%i ", hr, minute, sec, subsec);
	}
}




void print_date_and_time() {

	time_t t;

	time(&t);
	print_ch((unsigned char *)ctime(&t));
}




void out_of_memory() {
	
	counter = 0;
	
	beep();
	memory_message();
		
	cerr << "\nError: Circuit Solver has run out of memory and will now quit.";
	
	#ifdef xMAC_OS
	cerr << " You may want to allocate more memory to Circuit Solver to evaluate circuits this complex." << endl;
	#endif
	
	#ifdef xWIN32
	cerr << " You may want to modify your virtual memory options to evaluate circuits this complex." << endl;
	#endif
	
	fflush(stderr);
	
	while (!got_click() && counter < 10) {
		delay(.3);
		++counter;
	}
	
	#ifdef xMAC_OS
	quit_app();
	#endif
	
	#ifdef xWIN32
	exit_error();
	#endif
}




void demo_error() {

	beep();
	cout << "\nThis option is not available in the DEMO version of Circuit Solver!" << endl;
}




void internal_error() {

	cout << "An internal error has occurred, which should not have happened. You may have found a bug in the software." << endl;
}





