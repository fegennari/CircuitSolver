#include "CircuitSolver.h"
#include "Equation.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Equation.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// C.S.Equation (for graphing and other uses);
// By Frank Gennari
double_func ops[num_ops] = {sin, sin, cos, tan, asin, acos, atan, sinh, cosh, 
						    tanh, exp, log, log10, sqrt, ceil, floor, roundx2, 
						    truncate, sec, csc, cot, fabs, factorial2};


double last_in(0), last_out(0), IMPULSE_WIDTH(0.001);
int new_eq(1), keywords_loaded(0);
StringTable keywords(0);


// complex number stuff, not yet supported
double magnitude(complex z);
double phase(complex z);




cs_equation::cs_equation(int m_size) {

	if (m_size < 1)
		m_size = 1;

	max_size    = m_size;
	eq_owner_id = 0;
	num_size    = 0;

	equation         = memAlloc(equation, max_size+1);
	recoded_equation = memAlloc(recoded_equation, max_size+1);

	char_array = NULL;
	numbers    = NULL;
	consts     = NULL;
	values     = NULL;
	functions  = NULL;
	incl_eqs   = NULL;
	owner      = NULL;
		
	clear();
	load_keywords();
}


cs_equation::~cs_equation() {

	if (eq_size > 0) {
		if(equation != NULL)
			delete [] equation;
		if(recoded_equation != NULL)
			delete [] recoded_equation;
		if(char_array != NULL)
			delete [] char_array;
		if(numbers != NULL)
			delete [] numbers;
	}
	vars.destroy();
	delete_vars();
}


void cs_equation::input(string error_message) {

	int status(0), var_x = varies_with_x;

	do {
		strcpy(equation, xin(max_size));
		if(comp_char(equation[0], 'p') && equation[1] == 0) {
			varies_with_x = var_x; // might need to reset more
			return;
		}
		if(comp_char(equation[0], 'h') && equation[1] == 0) {
			help();
			cout << "Reenter: ";
		}
		else {
			reset_state();
			status = (init_eq() && recode() && simplify());
			if (status == 0) {
				if (is_assignment) {
					is_assignment = 0;
					cout << "Enter equation: ";
				}
				else
					cerr << error_message;
			}
		}
	} while (!status);	
}



int cs_equation::create_from_expr(const char *expr) {

	if (strlen(expr) >= max_size)
		return 0;
			
	reset_state();	
	strcpy(equation, expr);
	
	return (init_eq() && recode() && simplify());
}



int cs_equation::create_from_file(char *filename) {

	if (strlen(filename) > max_size-6)
		return 0;

	sprintf(equation, "file=%s", filename);
	
	return (init_eq() && recode() && simplify());
}



int cs_equation::init_eq() {

	if (equation[0] == 'f' && equation[1] == 'i' && equation[2] == 'l' && equation[3] == 'e' && equation[4] == '=') {
		if(!read_from_file())
			return 0;
	}
	if (comp_char(equation[0], 'p') && equation[1] == 0)
		return 0;
	if (comp_char(equation[0], 'h') && equation[1] == 0) {
		help();
		return 0;
	}
	eq_size = strlen(equation);
	
	if (eq_size == 0 || eq_size >= max_size)
		return 0;
		
	if (case_sensitive == 0)
		make_all_lower();
	
	return 1;	
}



void cs_equation::load_keywords() {
	
	if (keywords_loaded)
		return;
		
	keywords_loaded = 1;
	
	for (unsigned int k = 0; k < NUM_EQ_KEYWORDS; ++k) {
		keywords.insert(KEYWORDS[k], k);
	}
}



int cs_equation::add_const(char **names, double *values, unsigned int num_const, int overwrite) {

	unsigned int  ccount;
	double       *new_consts = NULL;

	if (num_const < 1 || num_const > MAX_CONST)
		return 0;

	for (ccount = 0; ccount < nvars; ++ccount) {
		if (name_exists(names[ccount])) { // duplicate name
			if (overwrite) {
				vars.remove((string)names[ccount]);
			}
			else {
				name_error = 1;
				return 0;
			}
		}
	}
	new_consts = memAlloc(consts, nconst + num_const);
	
	for(ccount = 0; ccount < nconst; ++ccount) {
		new_consts[ccount] = consts[ccount]; // copy old into new
	}
	delete [] consts;
	consts = new_consts;
	
	for (; ccount < nconst + num_const; ++ccount) {
		vars.insert(names[ccount - nconst], (ccount + CONST_OFFSET));
		consts[ccount] = values[ccount - nconst];
	}
	nconst += num_const;
		
	return 1;
}



int cs_equation::add_vars(char **names, double *init_vals, unsigned int num_vars, int overwrite) {

	unsigned int vcount;

	if (num_vars < 1 || num_vars > MAX_VAR || nvars != 0) // possibly fix to add to vars
		return 0;

	for (vcount = 0; vcount < nvars; ++vcount) {
		if (name_exists(names[vcount])) { // duplicate name
			if (overwrite) {
				vars.remove((string)names[vcount]);
			}
			else {
				name_error = 1;
				return 0;
			}
		}
	}
	nvars  = num_vars;
	values = memAlloc(values, num_vars);

	for (vcount = 0; vcount < nvars; ++vcount) {
		vars.insert(names[vcount], (vcount + VAR_OFFSET));
		if (init_vals == NULL)
			values[vcount] = 0;
		else
			values[vcount] = init_vals[vcount];
	}	
	return 1;
}



void cs_equation::set_vars(double *var_values) {

	for (unsigned int vcount = 0; vcount < nvars; ++vcount) {
		values[vcount] = var_values[vcount];
	}
}



int cs_equation::set_var(char *name, double val) {

	int index = get_var_index(name);
	
	if (index < 0) {
		return 0;
	}
	values[index] = val;
	
	return 1;
}



double cs_equation::get_var(char *name) {

	int index = get_var_index(name);
	
	if (index < 0) {
		return 0;
	}
	return values[index];
}



int cs_equation::get_var_index(char *name) {

	int index = vars.search((string)name);
	
	if (index == default_st_val) {
		name_error = 1;
		return -1;
	}
	index -= VAR_OFFSET;
	
	if (index < 0 || index > (int)nvars) {
		name_error = 1;
		return -1;
	}
	return index;
}



double cs_equation::val_from_index(int index, double input) {

	if (index == default_st_val || index < CONST_OFFSET) {
		name_error = 1;
		return 0;
	}
	index -= CONST_OFFSET;
	if (index < (int)nconst) // constant
		return consts[index];
	
	index -= (VAR_OFFSET - CONST_OFFSET);
	if (index < 0) {
		name_error = 1;
		return 0;
	}
	if (index < (int)nvars) // variable
		return values[index];
	
	index -= (FUNC_OFFSET - VAR_OFFSET);
	if (index < 0) {
		name_error = 1;
		return 0;
	}	
	if (index < (int)num_funcs) // function
		return functions[index](input);
	
	index -= (EQ_OFFSET - FUNC_OFFSET);	
	if (index < 0) {
		name_error = 1;
		return 0;
	}
	if (index < (int)num_eqs) // equation
		return incl_eqs[index]->eval(input);
	
	name_error = 1;	
	return 0;
}



int cs_equation::is_var_used(char *name) {

	unsigned i, eq_counter;
	int index = get_var_index(name);
	
	if (index < 0) {
		return 0;
	}
	for (i = 0; i < max_size; ++i) {
		if (recoded_equation[i] == 0) {
			break;
		}
		if (((int)((unsigned char)recoded_equation[i]) - VAR_OFFSET) == index) {
			return 1;
		}
	}
	for(eq_counter = 0; eq_counter < num_eqs; ++eq_counter) {
		if (incl_eqs[eq_counter]->is_used() && incl_eqs[eq_counter]->is_var_used(name)) // recursively check included equations
			return 1;
	}
	return 0;
}



int cs_equation::is_known_token(char *name) {

	int index = vars.search((string)name);
	
	if (index == default_st_val || index < CONST_OFFSET) {
		name_error = 1;
		return 0;
	}
	index -= CONST_OFFSET;
	if (index < (int)nconst) // constant
		return EQ_CONST;
	
	index -= (VAR_OFFSET - CONST_OFFSET);
	if (index < 0) {
		return EQ_UNKNOWN;
	}
	if (index < (int)nvars) // variable
		return EQ_VAR;
	
	index -= (FUNC_OFFSET - VAR_OFFSET);
	if (index < 0) {
		return EQ_UNKNOWN;
	}	
	if (index < (int)num_funcs) // function
		return EQ_FUNC;
	
	index -= (EQ_OFFSET - FUNC_OFFSET);	
	if (index < 0) {
		return EQ_UNKNOWN;
	}
	if (index < (int)num_eqs) // equation
		return EQ_EQU;
	
	return EQ_UNKNOWN;
}



int cs_equation::add_function(char *name, double_func function, int overwrite) {

	if (num_funcs >= MAX_FUNC) {
		name_error = 1;
		return 0;
	}
	if (name_exists(name)) {
		if (overwrite) {
			vars.remove((string)name);
		}
		else {
			name_error = 1;
			return 0;
		}
	}
	vars.insert((string)name, (FUNC_OFFSET + num_funcs));
	
	if (num_funcs >= func_capacity) {
		functions = double_array_size(functions, func_capacity);
	}
	functions[num_funcs++] = function;
	
	return 1;
}



int cs_equation::add_equation(char *name, cs_equation *equ, int overwrite) {

	if (num_eqs >= MAX_EQ) {
		name_error = 1;
		return 0;
	}
	if (equ == this) {
		cerr << "Error: Recursive equation." << endl;
		error = 1;
		return 0;
	}
	if (name_exists(name)) {
		if (overwrite) {
			vars.remove((string)name);
		}
		else {
			name_error = 1;
			return 0;
		}
	}
	vars.insert((string)name, (EQ_OFFSET + num_eqs));
	
	if (num_eqs >= eq_capacity) {
		incl_eqs = double_array_size(incl_eqs, eq_capacity);
	}
	incl_eqs[num_eqs++] = equ;
	
	return 1;
}



int cs_equation::name_exists(char *name) {

	if (keywords.search((string)name) != default_st_val)
		return 1;
	
	if (vars.search((string)name) != default_st_val)
		return 2;
		
	return 0;
}



int cs_equation::status() {
	
	if (error)
		return EQ_ERROR;
	if (undefined_char)
		return EQ_UNDEF;
	if (max_size < 1)
		return EQ_BAD_SIZE;
	if (eq_size == 0)
		return EQ_UNINIT;
	if (name_error) {
		name_error = 0;
		return EQ_NAME_ERROR;	
	}
	return EQ_OK;
}



double cs_equation::eval(double xval) {

	i = j = n = 0;
	x = xval;
	
	if (eq_size == 0) { // uninitialized
		cerr << "Error: Uninitialized eval." << endl;
		error = 1;
		return 0;
	}
	if (uses_b_const) {
		cout << "\nEnter the constant b:  ";
		b_const = ind();
	}
	eq_was_seen = 1;
	last_eval = calc_one_y(-1);
	eq_was_seen = 0;
	
	return last_eval;
}



int cs_equation::var_with_x() {
	
	if (varies_with_x)
		return 1;
		
	for(unsigned eq_counter = 0; eq_counter < num_eqs; ++eq_counter) {
		if (incl_eqs[eq_counter]->is_used() && incl_eqs[eq_counter]->var_with_x()) // recursively check included equations
			return 1;
	}	
	return 0;
}



char cs_equation::is_one_char() {

	if (equation[0] == 0 || equation[1] != 0)
		return 0;
	
	return equation[0];	
}



void cs_equation::copy_number(double *num_copy) {

	for (unsigned int k = 0; k < num_size; ++k)
		num_copy[k] = numbers[k];
}



void cs_equation::clear() {
	
	vars.destroy();
	num_cap = nvars = nconst = num_eqs = num_funcs = 0;
	func_capacity = eq_capacity = 0;
	reset_state();
	
	if (eq_size == 0)
		return;
		
	delete_vars();
		
	if(equation != NULL)
		equation[0] = 0;
	if(recoded_equation != NULL)
		recoded_equation[0] = 0;
	if(numbers != NULL) {
		delete [] numbers;
		numbers = NULL;
	}
}


void cs_equation::reset_state() {
	
	if (num_size > 0)
		memset(numbers, 0, sizeof(double)*num_size);
		
	eq_size = rec_size = num_size = 0;
	x = b_const = prev_eval = last_eval = 0;
	uses_b_const = varies_with_x = n = error = name_error = i = j = 0;
	case_sensitive = is_assignment = eq_is_used = 0;
	undefined_char = eq_was_seen = 0;
}



void  cs_equation::delete_vars() {

	if (consts != NULL) {
		delete [] consts;
		consts = NULL;
		nconst = 0;
	}
	if (values != NULL) {
		delete [] values;
		values = NULL;
		nvars = 0;
	}
	if (functions != NULL) {
		delete [] functions;
		functions = NULL;
		num_funcs = 0;
	}
	if (incl_eqs != NULL) {
		for (unsigned int i = 0; i < num_eqs; ++i) {
			if (incl_eqs[i]->owner_id() == 1)
				delete incl_eqs[i]; // delete owned equations	
		}
		delete [] incl_eqs;
		incl_eqs = NULL;
		num_eqs = 0;
	}
}



void cs_equation::print() {

	cout << "\nEquation: ";
	for (i = 0; i < max_size && equation[i] != 0; ++i)
		cout << equation[i];
		
	cout << "\nRecoded: ";
	for (i = 0; i < max_size && recoded_equation[i] != 0; ++i)
		cout << (char)(recoded_equation[i]);
		
	cout << "\nTokens: ";
	for (i = 0; i < max_size && recoded_equation[i] != 0; ++i)
		cout << (int)((unsigned char)recoded_equation[i]) << " ";
			
	cout << "\nNumbers: ";
	for (j = 0; j < num_size; ++j)
		cout << numbers[j] << " ";
		
	cout << endl;
}




int cs_equation::recode() {	
	
	int          current_level(1), max_level(1), found_a(0), comma_var(0);
	unsigned int k;
	char         temp;
	double       a_const(0);
			
	if (equation[0] == 0) {
		cerr << "\nExpression Error: You must enter an equation." << endl;
		return 0;
	}
	i = 0;
	char_array = memAlloc(char_array, max_num_length+1);
	increment_num_size();
	
	if (max_size > 2 && equation[1] == '=' && (equation[0] == 'y' || equation[0] == 'Y')) {
		i = 2; // "y="
	}	
	for (j = 0, k = 0; i < max_size && equation[i] != 0 && undefined_char == 0; ++i, ++j) {		
		if (i < 0 || j < 0 || k < 0 || i >= max_size || j >= max_size || k >= max_size) {
			delete [] char_array;
			char_array = NULL;
			internal_error();
			cerr << "\nError: Invalid index: " << i << ", " << j << ", " << k << "." << endl; // Internal error
			return 0;
		}
		switch(equation[i]) {			
			// Trig/Other Functions	
			case 'a':
			case 'A':
				if (compare_token2(equation, i+1, "sin")) {
					recoded_equation[j] = asinx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "cos")) {
					recoded_equation[j] = acosx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "tan")) {
					recoded_equation[j] = atanx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "bs")) {
					recoded_equation[j] = absx;
					i += 2;
				}
				else if (is_end_of_token(i+1)) {
					if (!found_a) {
						cout << "\nEnter the constant a:  ";
						a_const = ind();
						found_a = 1;	
					}
					add_number(a_const, k++);
				}
				else
					undefined_char = equation[i];
				break;
			
			case 'b':
			case 'B':
				if (is_end_of_token(i+1)) {
					recoded_equation[j] = bconst;
					if (!uses_b_const) {
						cout << "\nEnter the constant b:  ";
						uses_b_const = 1;
						b_const = ind();
					}
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'c':
			case 'C':
				if (compare_token2(equation, i+1, "osh")) {
					recoded_equation[j] = coshx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "os")) {
					recoded_equation[j] = cosx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "eil")) {
					recoded_equation[j] = ceilx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "sc")) {
					recoded_equation[j] = cscx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "ot")) {
					recoded_equation[j] = cotx;
					i += 2;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'd':
			case 'D':	
				if (compare_token2(equation, i+1, "elta")) {
					recoded_equation[j] = deltax;
					varies_with_x = 1;
					i += 4;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'e':
			case 'E':
				if (compare_token2(equation, i+1, "xp")) {
					if (i < max_size - 3 && (equation[i+3] == '(' || equation[i+3] == 'x' || equation[i+3] == 'k')) 
						recoded_equation[j] = expx;						
					else {
						add_number(exp_, k++);
					}
					i += 2;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'f':
			case 'F':
				if (compare_token2(equation, i+1, "loor")) {
					recoded_equation[j] = floorx;
					i += 4;
				}
				else if (compare_token2(equation, i+1, "act")) {
					recoded_equation[j] = factx;
					i += 3;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'l':
			case 'L':
				if (compare_token2(equation, i+1, "n")) {
					recoded_equation[j] = lnx;
					i += 1;
				}
				else if (compare_token2(equation, i+1, "og")) {
					recoded_equation[j] = logx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "i")) {
					i += 1;
					add_number(last_in, k++);
				}
				else if (compare_token2(equation, i+1, "o")) {
					i += 1;
					add_number(last_out, k++);
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'm':
			case 'M':
				if (compare_token2(equation, i+1, "in")) {
					recoded_equation[j] = minx;
					recoded_equation[j+1] = '(';
					i += 3;
					++current_level;
					++j;
					++comma_var;
				}
				else if (compare_token2(equation, i+1, "ax")) {
					recoded_equation[j] = maxx;
					recoded_equation[j+1] = '(';
					i += 3;
					++current_level;
					++j;
					++comma_var;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'r':
			case 'R':
				if (compare_token2(equation, i+1, "ound")) {
					recoded_equation[j] = roundX;
					i += 4;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 's':
			case 'S':
				if (compare_token2(equation, i+1, "inh")) {
					recoded_equation[j] = sinhx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "in")) {
					recoded_equation[j] = sinx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "qrt")) {
					recoded_equation[j] = sqrtx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "ec")) {
					recoded_equation[j] = secx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "um(")) {
					recoded_equation[j] = sumx;
					recoded_equation[j+1] = '(';
					i += 3;
					++current_level;
					++j;
					++comma_var;
				}
				else
					undefined_char = equation[i];
				break;
				
			case 't':
			case 'T':
				if (compare_token2(equation, i+1, "anh")) {
					recoded_equation[j] = tanhx;
					i += 3;
				}
				else if (compare_token2(equation, i+1, "an")) {
					recoded_equation[j] = tanx;
					i += 2;
				}
				else if (compare_token2(equation, i+1, "runc")) {
					recoded_equation[j] = truncx;
					i += 4;
				}
				else
					undefined_char = equation[i];
				break;
			
			case 'u':
			case 'U':
				if (is_end_of_token(i+1)) {
					varies_with_x = 1;
					recoded_equation[j] = ux;
				}
				else
					undefined_char = equation[i];
				break;
			
			// Parentheses
			case '(':
				++current_level;
				if (current_level > max_level)
					max_level = current_level;
				if (comma_var > 0)
					++comma_var;
				recoded_equation[j] = equation[i];
				break;
			
			case ')':
				recoded_equation[j] = equation[i];
				if (comma_var > 0)
					--comma_var;
				if (current_level == 1)
					undefined_char = equation[i];
				else
					--current_level;
				break;
			
			// Negate or Subtract
			case '-':
				if (i == 0 || equation[i-1] == '(' || is_operation(equation[i-1])) { 
					if (i >= max_size - 1) // ^Negative 
						undefined_char = equation[i];
					else if (is_number(equation[i+1]) || equation[i+1] == '.' || (compare_token(equation, i+1, "exp") && equation[i+4] != '(') || compare_token(equation, i+1, "pi") || equation[i+1] == 'q' || equation[i+1] == 'n') { // Number
						numbers[k] = -1.0; // ^Numbers and Constants
						--j;
					}
					else {
						increment_num_size();
						numbers[k] = -1.0;
						recoded_equation[j] = numbersx;
						++k;
						recoded_equation[++j] = '*';// insert a '*'
					}
				}		 
				else { // Subtraction
					if (i >= max_size - 1)
						undefined_char = equation[i];
					else
						recoded_equation[j] = equation[i];
				}
				break;
			
			// Numbers			
			case '.':
				if (i >= max_size - 1 || equation[i+1] < '0' || equation[i+1] > '9')
					undefined_char = equation[i];
			
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': // Recoding numbers is kind of long.			
				recode_number(k);				
				break;
			
			// Constants
			case 'p':
			case 'P':
				if (compare_token2(equation, i+1, "i")) {
					i += 1;
					add_number(pi, k++);
				}
				else
					undefined_char = equation[i];
				break;
				
			case 'q':
			case 'Q':
				if (is_end_of_token(i+1)) {
					add_number(q_, k++);
				}
				else
					undefined_char = equation[i];
				break;
				
			// Arithmetic Functions (except -, which is taken care of above)	
			case '+':
			case '*':
			case '/':
			case '^':
			case '%':
				if (i >= max_size - 1 || (i > 0 && is_operation(equation[i-1])))
					undefined_char = equation[i];
				else
					recoded_equation[j] = equation[i];
				break;
					
			case '=': 
				cerr << "\nError: improper use of character '='.";
				undefined_char = equation[i];
				--j;
				break;
				
			// X-Variable
			case 'x':
			case 'X':
				if (is_end_of_token(i+1)) {
					varies_with_x = 1;
					recoded_equation[j] = variablex;
				}
				else
					undefined_char = equation[i];
				break;
				
			// Previous Y
			case 'y':
			case 'Y':
				if (is_end_of_token(i+1)) {
					if (new_eq) {
						undefined_char = equation[i];
						break;
					}
					varies_with_x = 1;
					recoded_equation[j] = lasty;
				}
				else
					undefined_char = equation[i];
				break;
				
			// Summation Characters
			case 'n':
			case 'N':
				if (equation[i+1] == '_' && is_end_of_token(i+2))
					add_number(N_, k++);
				else if (comma_var > 0 && is_end_of_token(i+1))
					recoded_equation[j] = n_param;
				else
					undefined_char = equation[i];
				break;
				
			case ',':
				if (comma_var > 0)
					recoded_equation[j] = ',';
				else
					undefined_char = equation[i];
				break;
			
			// Unimplimented Symbols, may add later
			case 'i': // Complex numbers
			case 'j':
				if (is_end_of_token(i+1)) {
					cerr << "\nContent Error: Circuit Solver does not currently support equations containing complex numbers.";
					recoded_equation[j] = j_param;
					undefined_char = equation[i];
				}
				else
					undefined_char = equation[i];
				break;
			
			// Space is ignored
			case ' ':
				--j;
				break;
				
			// Undefined	
			default:
				undefined_char = equation[i];
			
		}
		if (undefined_char != 0) {
			if (match_token(k)) {
				undefined_char = 0;
			} // assignment
			else if (j == 0 && equation[i+1] == '=' && i+2 < max_size && i+2 < strlen(equation)) {
				if(!create_new_equation()) {
					cerr << "Error creating new equation." << endl;
					--j;
					max_level = -1;
				}
				is_assignment = 1;
				max_level = -1;
				break;
			}
			else {
				cerr << "\nExpression Error: Undefined or incorrectly placed symbol: " << undefined_char << ": ";
				for (unsigned int l = 0; l <= i && l < max_size; ++l)
					cout << equation[l];
				cout << "_" << endl;
				--j;
				max_level = -1;
			}
		}
		else if (j > 0 && j < max_size - 1 && (recoded_equation[j-1] == numbersx || recoded_equation[j-1] == bconst || recoded_equation[j-1] == lasty || recoded_equation[j-1] == n_param || recoded_equation[j-1] == variablex || recoded_equation[j-1] == ')') && (!(is_operation(recoded_equation[j]) || recoded_equation[j] == ',' || recoded_equation[j] == ')' || recoded_equation[j] == '(') || recoded_equation[j] == '(')) {
			temp = recoded_equation[j];
			recoded_equation[j] = '*';
			recoded_equation[j+1] = temp;
			++j;
		}		
	}	
	if (current_level > 1) {
		if (current_level == 2)
			cerr << "\nWarning: A closing parenthesis is missing from the end of the expression." << endl;
		else
			cerr << "\nWarning: " << (current_level - 1) << " closing parentheses are missing from the end of the expression." << endl;
	}
	recoded_equation[j] = 0;
	rec_size = j;	
	numbers[k] = 0;	
	delete [] char_array;
	char_array = NULL;
		
	return (max_level >= 0);
}




// converts segment of character array into floating point number, supports scientific notation
void cs_equation::recode_number(unsigned int &k) {
		
	int found_decimal(0);
	char numcount(0);
		
	char_array[0] = equation[i];
	found_decimal = 0;
		
	for (numcount = 1; i + numcount < max_size && numcount < max_num_length && undefined_char == 0 && (is_number(equation[i+numcount]) || equation[i+numcount] == '.' || (equation[i+numcount] == '-' && (i + numcount > 0 && comp_char(equation[i+numcount-1], 'e'))) || comp_char(equation[i+numcount],'e')); ++numcount) {
		if (equation[i+numcount] == '.') {
			if (found_decimal)
				undefined_char = equation[i+numcount];
			else
				found_decimal = 1;
		}		
		else if (comp_char(equation[i+numcount], 'e')) {
			if (i + numcount < max_size - 1 && (equation[i+numcount+1] == '-' || is_number(equation[i+numcount+1]))) 
				char_array[numcount] = equation[i+numcount];
			else if (i + numcount < max_size - 1 && comp_char(equation[i+numcount+1], 'x'))
				found_decimal = 2;
			else 
				undefined_char = equation[i+numcount];
		}
		char_array[numcount] = equation[i+numcount];
	}	
	if (found_decimal == 2) {
		char_array[numcount] = 0;
		--numcount;
	}	
	if (i + numcount + 1 < max_size) 
		char_array[numcount] = 0;
		
	add_number(atof(char_array), k++);
	i += numcount-1;	
}




int cs_equation::match_token(unsigned int &k) {

	int  val;
	char token_buffer[MAX_EQUATION_CHARACTERS] = {0};
	
	if (!is_token_char(equation[i]))
		return 0;
	
	for (val = (int)i; i < max_size && ((int)i - val) < MAX_EQUATION_CHARACTERS && is_token_char(equation[i]); ++i)
		token_buffer[i - val] = equation[i];
		
	token_buffer[i - val] = 0;
	--i;
			
	if (keywords.search((string)token_buffer) != default_st_val) {
		internal_error();
		cerr << "Error: Token in equation is keyword: " << token_buffer << "." << endl;
		return 0;
	}
	val = vars.search((string)token_buffer);
	if (val != default_st_val) {
		recoded_equation[j] = (char)val;
		if (val >= EQ_OFFSET && val < (int)num_eqs) {
			incl_eqs[val - EQ_OFFSET]->set_used(1);	
		}
		return 1;
	}
	if(owner != NULL) {
		val = owner->is_known_token(token_buffer);
		switch(val) {
			case EQ_UNKNOWN:
				return 0;
			case EQ_CONST:
				recoded_equation[j] = ask_parent_constx;
				increment_num_size();
				numbers[k++] = owner->get_index(token_buffer);
				return 1;
			case EQ_VAR:
			case EQ_FUNC:
			case EQ_EQU:
				recoded_equation[j] = ask_parentx;
				increment_num_size();
				numbers[k++] = owner->get_index(token_buffer);
				return 1;
		}
	}
	return 0;
}




void cs_equation::add_number(double nval, unsigned int k) {

	increment_num_size();	
	recoded_equation[j] = numbersx;
		
	if (numbers[k] == 0)
		numbers[k] = nval;
	else
		numbers[k] = numbers[k]*nval;		
}




void cs_equation::increment_num_size() {

	unsigned i;
	double *temp = NULL;

	if(num_size >= num_cap) {
		if(num_cap == 0)
			num_cap = 1;
		else
			num_cap *= 2;
						
		temp = memAlloc(temp, num_cap);
		for (i = 0; i < num_size; ++i) {
			temp[i] = numbers[i];
		}
		for(; i < num_cap; ++i) {
			temp[i] = 0;
		}		
		if(numbers != NULL)
			delete [] numbers;
		numbers = temp;		
	}
	++num_size;
}




int cs_equation::create_new_equation() {

	char        *name = NULL;
	double       val;
		
	cs_equation *new_eq = new cs_equation(max_size);
	
	if (new_eq == NULL) {
		out_of_memory();
		return 0;
	}
	name = memAlloc(name, i+2);
	for (unsigned index = 0; index < i+1; ++index) {
		name[index] = equation[index];
	}	
	name[i+1] = 0;	
	new_eq->set_owner(this, 1);	
	new_eq->set_case_sens(case_sensitive);	
	new_eq->create_from_expr((char *)(equation + i + 2));
		
	if(!new_eq->var_with_x()) { // equation is not dependent on x - constant
		val = new_eq->eval(0);
		delete new_eq;
		return add_const(&name, &val, 1, 1);
	}	
	if(!add_equation(name, new_eq, 1)) {
		delete new_eq;
		return 0;
	}	
	return 1;
}




void cs_equation::make_all_lower() {

	for (i = 0; i < eq_size; ++i) {
		if (equation[i] >= 'A' && equation[i] <= 'Z')
			equation[i] += ('a' - 'A');
	}
}



// Simplify the equation for faster evaluation
/****************************

1. x remove one token function parentheses	sin(x)	=>	sinx
2.   remove unnecessary parentheses			(2*x)+1	=>	2*x+1			
3. x remove repeated nested parentheses		2*((x+1))	=>	2*(x+1)
4.   simplify operations on constants		1+2*2	=>	5
5.   remove additions of 0					x+0		=>	x
6.   remove multiplications by 1			1*x		=>	x
7.   simplify constant functions			sin(0)	=>	0

skip flag

****************************/

int cs_equation::simplify() {
	
	//int k, l;	
	char_array = memAlloc(char_array, rec_size+1);
	
	for (i = 0, j = 0; j < rec_size && recoded_equation[i] != 0;) {
		if (recoded_equation[i] == '(') {
			if (i < max_size - 2 && recoded_equation[i+2] == ')') { // #1
				char_array[j++] = recoded_equation[i+1];
				i += 3;
			}
			else if (i < max_size - 1 && recoded_equation[i+1] == '(') { // #3
				if (search_for_double_paren()) {
					++i;
				}
				else {
					char_array[j++] = recoded_equation[i++];
				}
			}
			else {
				char_array[j++] = recoded_equation[i++];
			}
		}	
		else if (recoded_equation[i] == skipx) 
			++i;	
		else {
			char_array[j++] = recoded_equation[i++];
		}
	}	
	/*temp_num = memAlloc(num_size);
	for (k = 0, l = 0; l < num_size; ++l, ++k) {
		temp_num[l] = numbers[k];
	}
	copy_back_num(numbers, temp_num);
	delete [] temp_num;*/
	
	char_array[j] = 0;
	copy_back_eq(recoded_equation, char_array);
	delete [] char_array;
	char_array = NULL;
	
	return 1;
}




// Copy equation2 into equation1 and set equation2 to 0 (NULL)
void cs_equation::copy_back_eq(char *equation1, char *equation2) {

	unsigned int k;

	for (k = 0; k < max_size-1 && equation2[k] != 0; ++k) {
		equation1[k] = equation2[k];
		equation2[k] = 0;
	}
	equation1[k] = 0;
}




// Copy numbers2 into numbers1 and set numbers2 to 0
void cs_equation::copy_back_num(double *numbers1, double *numbers2) {

	for (unsigned int k = 0; k < num_size; ++k) {
		numbers1[k] = numbers2[k];
		numbers2[k] = 0;
	}
}




// Searches for "))" before other parentheses
int cs_equation::search_for_double_paren() {

	for (unsigned int k = i+2; k < max_size - 1 && recoded_equation[k] != 0 && recoded_equation[k] != '('; ++k) {
		if (recoded_equation[k] == ')') {
			if (recoded_equation[k+1] == ')') {
				recoded_equation[k+1] = skipx;
				return 1;
			}
			else
				return 0;
		}
	}
	return 0;
}




// Operation order (highest priority to lowest priority): functions, ^, {/, *, %}, {+, -}
inline int cs_equation::get_operation_order(char operation) {

	switch (operation) {
	
		case '+':
		case '-':
			return 1;
			
		case '*':
		case '%':
		case '/':
			return 2;
			
		case '^':
			return 3;
	}
	return 0; // not an operation (function() or other)
}




// Recursive function to calculate a y value given an equation and an x value
double cs_equation::calc_one_y(int op_order) {
	
	unsigned int old_i(i), index;
	double val(0);
		
	if (i < 0 || j < 0 || error) {
		return 0;
	}
	if (recoded_equation[0] == numbersx && recoded_equation[1] == 0)
		return numbers[0];
	
	for (; i < max_size && recoded_equation[i] != 0 && !error; ++i, old_i = i) {				
		if (recoded_equation[i] > 0 && recoded_equation[i] < num_ops) 
			val = ops[recoded_equation[old_i]](eval_next_expression(0, op_order));
		else				
			switch (recoded_equation[i]) {				
				case numbersx:
					val = numbers[j];
					++j;
					break;				
				case variablex:
					val = x;
					break;					
				case '(':
					++i;
					val = calc_one_y(-1);
					break;				
				case ')':
				case ',':
					return val;
					break;				
				case '+':
					if (op_order >= 1)
						return val;
					val = val + eval_next_expression(1, 1);
					break;				
				case '-':
					if (op_order >= 1)
						return val;
					val = val - eval_next_expression(1, 1);
					break;					
				case '%':
					if (op_order >= 2)
						return val;
					val = (int)val%(int)eval_next_expression(1, 2);
					break;				
				case '*':
					if (op_order >= 2)
						return val;
					val = val*eval_next_expression(1, 2);
					break;				
				case '/':
					if (op_order >= 2)
						return val;
					val = val/eval_next_expression(1, 2);
					break;				
				case '^':
					if (op_order >= 3)
						return val;
					val = pow(val, eval_next_expression(1, 3));
					break;					
				case bconst:
					val = b_const;
					break;					
				case n_param:
					val = n;
					break;						
				case lasty:
					val = prev_eval;
					break;				
				case ux:
					val = unit_step(x, eval_next_expression(0, op_order));
					break;					
				case deltax:
					val = delta(x, eval_next_expression(0, op_order));
					break;				
				case sumx:
					val = sum();
					break;					
				case minx:
				case maxx:
					val = minmax(recoded_equation[i]);
					break;					
				case ask_parent_constx:
					index = (int)numbers[j++];
					val = owner->val_from_index(index, 0);
					break;					
				case ask_parentx:
					index = (int)numbers[j++];
					val = owner->val_from_index(index, eval_next_expression(0, op_order));
					break;			
				default:
					if (i > 0 && recoded_equation[i-1] == numbersx) { // negative equation
						val *= eval_token((unsigned char)recoded_equation[i], op_order);
					}
					else {
						val = eval_token((unsigned char)recoded_equation[i], op_order);
					}
					if (error) {
						cerr << "Offending token (1): " << recoded_equation[i] << " = [" << (int)((unsigned char)recoded_equation[i]) << "]." << endl; // If parser did not catch an error
						return 0;
					}
			}
	}
	if (error)
		return 0;
			
	return val;
}




// An extension of calc_one_y, also recursive and very confusing
double cs_equation::eval_next_expression(int is_operation, int op_order) {
	
	int    op_order2(get_operation_order(recoded_equation[i+2]));
	double val(0);
		
	if (i == max_size-1) {
		cerr << "Error: Exceeded max i." << endl;
		error = 1;
	}
	else if (recoded_equation[i+1] == 0) {
		cerr << "Error: Zero token." << endl;
		error = 1;
	}
	else if (recoded_equation[i+1] == variablex) {
		++i;
		if ((is_operation || op_order != -1) && op_order2 >= op_order && op_order2 > 0) {
			val = calc_one_y(op_order);
			--i;		
		}
		else
			return x;
	}
	else if (recoded_equation[i+1] == numbersx || recoded_equation[i+1] == bconst) {
		++i;
		if (op_order2 >= op_order && op_order2 > 0 && (is_operation || (op_order != -1 && recoded_equation[i-1] == ')')) && (i == max_size-2 || recoded_equation[i+1] != ')')) {
			val = calc_one_y(op_order);
			--i;		
		}
		else if (recoded_equation[i] == bconst) {
			val = b_const;
		}
		else {
			val = numbers[j];
			++j;
			if (j >= max_size) {
				cerr << "Error: Exceeded max j." << endl;
				error = 1;
			}
		}
	} 
	else if (recoded_equation[i+1] == '(') {
		i += 2;
		val = calc_one_y(-1);
		
		if (is_operation && get_operation_order(recoded_equation[i+1]) > op_order) { // Operations between expressions in parentheses
			switch (recoded_equation[i+1]) {
				case '%':
					++i;
					val = (int)val%(int)eval_next_expression(1, op_order);
					break;			
				case '*':
					++i;
					val = val*eval_next_expression(1, op_order);
					break;			
				case '/':
					++i;
					val = val/eval_next_expression(1, op_order);
					break;			
				case '^':
					++i;
					val = pow(val, eval_next_expression(1, op_order));
					break;
			}
		}
	}
	else if (is_operation) {
		++i;
		val = calc_one_y(get_operation_order(recoded_equation[i-1]));
		--i;
	}
	else {
		++i;
		if ((is_operation || op_order != -1) && op_order2 >= op_order && op_order2 > 0) {
			val = calc_one_y(op_order);
			--i;		
		}
		else {
			val = eval_token((unsigned char)recoded_equation[i], op_order);
		}
		//error = 1;
		//cout << "Offending token (2): " << recoded_equation[i+1] << " = [" << (int)((unsigned char)recoded_equation[i]) << "]." << endl;; // Error that parser did not catch
	}	
	return val; 
}




double cs_equation::eval_token(int token, int op_order) {

	double val;
	
	token -= CONST_OFFSET;
	if (token < 0) {
		cerr << "Error: Invalid token(1): " << token << "." << endl;
		error = 1;
		return 0;
	}
	if (token < (int)nconst) // constant
		return consts[token];
	
	token -= (VAR_OFFSET - CONST_OFFSET);
	if (token < 0) {
		cerr << "Error: Invalid token(2): " << token << "." << endl;
		error = 1;
		return 0;
	}
	if (token < (int)nvars) // variable
		return values[token];
	
	token -= (FUNC_OFFSET - VAR_OFFSET);
	if (token < 0) {
		cerr << "Error: Invalid token(3): " << token << "." << endl;
		error = 1;
		return 0;
	}	
	if (token < (int)num_funcs) // function
		return functions[token](eval_next_expression(0, op_order));
	
	token -= (EQ_OFFSET - FUNC_OFFSET);	
	if (token < 0) {
		cerr << "Error: Invalid token(4): " << token << "." << endl;
		error = 1;
		return 0;
	}	
	if (token < (int)num_eqs) { // equation		
		if (incl_eqs[token]->was_seen()) { // check for recursive equation
			cerr << "Error: Recursive equation dependency. Aboting call an returning 0." << endl;
			error = 1;
			return 0;
		}
		incl_eqs[token]->set_seen(1);
		val = incl_eqs[token]->eval(eval_next_expression(0, op_order));
		incl_eqs[token]->set_seen(0);
		return val;
	}
	cerr << "Error: Invalid token(5): " << token << "." << endl;
	error = 1;

	return 0;
}




int cs_equation::read_from_file() {

	ifstream infile;
	char filename[MAX_SAVE_CHARACTERS + 1] = {0};
	
	for (i = 5, j = 0; j < (unsigned)MAX_SAVE_CHARACTERS && equation[i] != 0; ++i, ++j)
		filename[j] = equation[i];
	
	if (!infile_file(infile, filename)) {
		beep();	
		cerr << "\nThe equation file " << filename << " could not be opened!" << endl;
		return 0;
	}
	if (!filestream_check(infile)) {
		return 0;
	}
	infile.getline(equation, max_size);
	infile.close();
	
	return 1;
}



// Show all possible operations
void cs_equation::help() {

	cout << "The following is a list of supported equation functions/symbols:"
		<< "\nsin(), cos(), tan(), asin(), acos(), atan()\nsinh()" 
		<< ", cosh(), tanh(), sec(), csc(), cot()\nexp(), log() - ln" 
		<< ", log10()\nsqrt(), ceil(), floor()\nround(), trunc(), abs()\nfact() - factorial" 
		<< "\nu() - unit step\ndelta() - delta/impulse function\nsum(<from>, <to>, <expression>)"
		<< "\nmin(<expression>, <expression>)\nmax(<expression>, <expression>)" 
		<< "\nn - summation variable\na - constant, entered once\nb - constant, entered each time" 
		<< "\nli - last input/entered number\nlo - last output/printed number" 
		<< "\ny - y value from current graph\nexp - exponential (2.71828)\npi - pi (3.14159)"
		<< "\n+, -, %, *, /, ^, (, )"
		<< "\noperation order = functions, ^, {/, *, %}, {+, -}" << endl;      
}




double cs_equation::sum() {

	int start(0), stop(0), temp_i, temp_j;
	double val(0);
	
	i += 2;
	start = (int)calc_one_y(-1);
	++i;
	stop = (int)calc_one_y(-1);
	++i;
	temp_i = i;
	temp_j = j;
	
	if (start > stop)
		swap(start, stop);
	
	for (n = start; n <= stop; ++n) {
		i = temp_i;
		j = temp_j;
		val += calc_one_y(-1);
	}
	return val;
}




double cs_equation::minmax(int id) {
	
	double val1(0), val2(0);
	
	i += 2;
	val1 = calc_one_y(-1);
	++i;
	val2 = calc_one_y(-1);
	
	if (id == maxx)
		return max(val1, val2);
	
	return min(val1, val2);
}



// ********************************************************
// Equation Functions



inline double sec(double x) {

	return 1.0/cos(x);
}




inline double csc(double x) {

	return 1.0/sin(x);
}




inline double cot(double x) {

	return 1.0/tan(x);
}




inline double unit_step(double x, double t) {
		
	if (x >= t)
		return 1.0;
		
	return 0.0;
}




inline double delta(double x, double t) {

	if (fabs(t - x) < IMPULSE_WIDTH/2.0)
		return 1.0/IMPULSE_WIDTH;
		
	return 0.0;
}




inline double truncate(double x) {

	return (x - (int)x);
}




inline double factorial(int n) {

	double fact(1.0);

	for (int i = 2; i <= n; ++i) {
		fact *= i;
	}
	return fact;	
}




inline double factorial2(double x) {

	double fact(1.0);

	for (int i = 2; i <= x; ++i) {
		fact *= i;
	}
	return fact;	
}




inline double roundx2(double x) {

	int num2((int)x);
	
	if (fabs(x - num2) >= 0.5) {
		if (x >= 0.0)
			return ceil(x);
		else
			return floor(x);
	}	
	if (x >= 0.0)
		return floor(x);
		
	return ceil(x);
}



// normal string comparison
int compare_token(const char *exp, int index, const char *token) {

	if (index < 0 || index >= (int)strlen(exp) || (int)strlen(exp) - index < (int)strlen(token))
		return 0;
	
	for (unsigned int i = 0; i < strlen(token); ++i) {
		if (exp[index+i] != token[i])
			return 0;
	}
	return 1;
}



// character after token must be non alpha and not an underscore
int compare_token2(const char *exp, int index, const char *token) {

	unsigned int i;
	
	if (index < 0 || index >= (int)strlen(exp) || (int)strlen(exp) - index < (int)strlen(token))
		return 0;
	
	for (i = 0; i < strlen(token); ++i) {
		if (exp[index+i] != token[i])
			return 0;
	}
	if ((strlen(exp) - index == strlen(token)) || !is_token_char(exp[index+i]))
		return 1;
		
	return 0;
}



// non case sensitive comparison
int compare_token_ncs(const char *exp, int index, const char *token) {

	if (index < 0 || index >= (int)strlen(exp) || (int)strlen(exp) - index < (int)strlen(token))
		return 0;
	
	for (unsigned int i = 0; i < strlen(token); ++i) {
		if (exp[index+i] != token[i] && exp[index+i] != (token[i] - CHAR_OFFSET))
			return 0;
	}
	return 1;
}






