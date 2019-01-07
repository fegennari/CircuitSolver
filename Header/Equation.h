#ifndef _CIRCUIT_SOLVER_EQUATION_H_
#define _CIRCUIT_SOLVER_EQUATION_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Equation.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Equation Header
// By Frank Gennari
#include "StringTable.h"


typedef double (*double_func)(double);


inline int is_token_char(char c) {

	return (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}


inline int is_number(char c) {

	return (c >= '0' && c <= '9');
}


class cs_equation {
	
	public:
		cs_equation(int m_size);
		~cs_equation();
		
		void   input(string error_message);
		int    create_from_expr(const char *expr);
		int    create_from_file(char *filename);
		int    add_const(char **names, double *values, unsigned int num_const, int overwrite);
		int    add_vars(char **names, double *init_vals, unsigned int num_vars, int overwrite);
		void   set_vars(double *var_values);
		int    set_var(char *name, double val); 
		double get_var(char *name);
		inline double get_value(char *name, double input) {return val_from_index(vars.search((string)name), input);};
		double val_from_index(int index, double input);
		inline int    get_index(char *name) {return vars.search((string)name);};
		int    is_var_used(char *name);
		int    is_known_token(char *name);
		int    add_function(char *name, double_func function, int overwrite);
		int    add_equation(char *name, cs_equation *equ, int overwrite);
		int    name_exists(char *name);
		void   set_prev_eval(double val) {prev_eval = val;};
		inline void   set_case_sens(int is_case_sens) {case_sensitive = (is_case_sens != 0);};
		inline void   set_seen(char seen) {eq_was_seen = seen;};
		inline char   was_seen() {return eq_was_seen;};
		inline void   set_owner(cs_equation *owner_eq, int owner_id) {eq_owner_id = owner_id; owner = owner_eq;};
		inline int    owner_id() {return eq_owner_id;};
		inline void   set_used(int used) {eq_is_used = used;};
		inline int    is_used() {return eq_is_used;};
		int    status();
		double eval(double xval);
		int    var_with_x();
		char   is_one_char();
		inline void   copy_equation(char *eq_copy) {strcpy(eq_copy, equation);};
		inline void   copy_recoded_equation(char *eq_copy) {strcpy(eq_copy, recoded_equation);};
		void   copy_number(double *num_copy);
		void   clear();
		void   reset_state();
		void   print();
		void   help();
			
	private:
		int    init_eq();
		int    recode();
		void   recode_number(unsigned int &k);
		int    match_token(unsigned int &k);
		inline int    is_end_of_token(unsigned int index) {return (index >= max_size || index >= strlen(equation) || !is_token_char(equation[index]));};
		void   add_number(double nval, unsigned int k);
		void   increment_num_size();
		int    create_new_equation();
		void   make_all_lower();
		void   load_keywords();
		int    simplify();
		void   copy_back_eq(char *equation1, char *equation2);
		void   copy_back_num(double *numbers1, double *numbers2);
		int    search_for_double_paren();
		inline int    is_operation(char ch) {return (ch == '+' || ch == '-' || ch == '%' || ch == '*' || ch == '/' || ch == '^');};
		int    get_operation_order(char operation);
		double calc_one_y(int op_order);
		double eval_next_expression(int is_operation, int op_order);
		double eval_token(int token, int op_order);
		int    read_from_file();
		double minmax(int id);
		double sum();
		int    get_var_index(char *name);
		void   delete_vars();
		
		StringTable   vars;
		cs_equation **incl_eqs, *owner;
		double_func  *functions;
		char         *equation, *recoded_equation, *char_array;
		char		  undefined_char, eq_was_seen;
		double       *numbers, *temp_num, *values, *consts;
		double        x, b_const, prev_eval, last_eval;
		unsigned int  i, j, max_size, eq_size, rec_size, num_size, num_cap, nvars, nconst;
		unsigned int  num_eqs, num_funcs, func_capacity, eq_capacity;
		int           varies_with_x, n, error, name_error, case_sensitive, uses_b_const;
		int           is_assignment, eq_owner_id, eq_is_used; // 0 = global, 1 = this equation
};



double sec(double x);
double csc(double x);
double cot(double x);
double unit_step(double x, double t);
double delta(double x, double t);
double truncate(double x);
double factorial(int n);
double factorial2(double x);
double roundx2(double x);
int compare_token(const char *exp, int index, const char *token);
int compare_token2(const char *exp, int index, const char *token);
int compare_token_ncs(const char *exp, int index, const char *token);
int is_token_char(char c);
int is_number(char c);


#define pi_ 	3.14159265359
#define exp_ 	2.71828183
#define c_		2.99792458e8
#define q_		1.602e-19
#define ep0		8.854e-12
#define N_		6.024e23

double const mu0 = 4*pi*1e-7;

char const CHAR_OFFSET = 'a' - 'A';

int const MAX_EQUATION_CHARACTERS = MAX_ENTERED_CHARACTERS;
int const MAX_FIXED_TOKENS        = 36;

unsigned int const MAX_CONST    = 30;
unsigned int const MAX_VAR      = 30;
unsigned int const MAX_FUNC     = 30;
unsigned int const MAX_EQ       = 30;
int const          CONST_OFFSET = 128;
int const          VAR_OFFSET   = CONST_OFFSET + MAX_CONST + 1;
int const          FUNC_OFFSET  = VAR_OFFSET   + MAX_VAR   + 1;
int const          EQ_OFFSET    = FUNC_OFFSET  + MAX_FUNC  + 1;

#define num_ops        23
#define max_num_length 32

enum {errorx = 0, sinx,	cosx, tanx, asinx, acosx, atanx, sinhx, coshx, tanhx, 
	  expx, lnx, logx, sqrtx, ceilx, floorx, roundX, truncx, secx, cscx,
	  cotx, absx, factx, ux, deltax, bconst, lasty, sumx, minx, 
	  maxx, n_param, j_param, variablex, numbersx, skipx, ask_parentx, 
	  ask_parent_constx};
	  
enum {EQ_UNKNOWN = 0, EQ_CONST, EQ_VAR, EQ_FUNC, EQ_EQU};

	  
enum {EQ_OK = 0, EQ_ERROR, EQ_UNDEF, EQ_UNINIT, EQ_BAD_SIZE, EQ_NAME_ERROR};


#define NUM_EQ_KEYWORDS    38
#define MAX_KEYWORD_LENGTH 5

const char KEYWORDS[NUM_EQ_KEYWORDS][MAX_KEYWORD_LENGTH+1] = {"sin", "cos", "tan", "asin",
	   "acos", "atan", "sinh", "cosh", "tanh", "exp", "ln", "log", "sqrt", "ceil",
	   "floor", "round", "trunc", "sec", "csc", "cot", "abs", "fact", "u", "delta",
	   "b", "lasty", "sum", "min", "max", "n", "j", "x", "y", "a", "li", "lo", 
	   "pi", "file"};
	   		


/**********************
RECODING:
sinx	sin
cosx	cos
tanx	tan
asinx	asin
acosx	acos
atanx	atan
sinhx	sinh
coshx	cosh
tanhx	tanh
secx	sec
cscx	csc
cotx	cot
expx	exp (e^)
lnx		log (ln)
logx	log10
sqrtx	sqrt
ceilx	ceil
floorx	floor
roundX	round
truncx	trunc		truncate
absx	abs ||		absolute value
factx	fact		factorial
ux		u()			unit step
deltax	delta()		delta function
sumx	sum(,,)		summation
minx	min(-,-) 	minimum
maxx	max(-,-)	maximum
numbersx	constant
variablex	x-variable
aconst		a 	constant, initial set
bconst		b 	constant, always set
last_in		li	last entered number
last_out	lo	last printed number
lasty		y	last y calculated
			n	summation variable
,			,	sum(from_n, to_n, expression) or min(-,-_ or max(-,-)
+, -, %, *, /, ^, (, )
exp, pi
skip flag
***********************/


#endif


