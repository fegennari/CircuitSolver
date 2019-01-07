#include "CircuitSolver.h"
#include "Equation.h"


// Circuit Solver is copyright © 1999-2001 by Frank Gennari.
// Circuit Solver, C.S.Polynomial.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Polynomial Differentiation/Integration/Equation Solver
// By Frank Gennari 
extern int uses_b_const;
extern double saved_number[], b_const;
extern char saved_equation[];


struct polynomial {
	
	int n;
	double a[MAX_EQUATION_CHARACTERS];
};


void integrate(polynomial poly, polynomial poly_int, double C);
void diff(polynomial poly, polynomial poly_diff);
double eval(polynomial poly, double val);
double num_integrate(polynomial poly, double a, double b);
char *convert_to_chars(polynomial poly);
void print_poly_to_screen(polynomial poly);
void destroy(polynomial poly);




void integrate(polynomial poly, polynomial poly_int, double C) {
		
	poly_int.n = poly.n + 1;
	
	
	
	poly_int.a[0] += C;
}




void diff(polynomial poly, polynomial poly_diff) {
	
	poly_diff.n = poly.n - 1;
}




double eval(polynomial poly, double val) {
	
	double result = 0;
	
	for (int i = 0; i <= poly.n; ++i)
		result = result + poly.a[i]*pow(val, i);
	
	return result;
}




double num_integrate(polynomial poly, double a, double b) {
	
	double val;
	polynomial poly_int;
	
	for (int i = 0; i < MAX_EQUATION_CHARACTERS; ++i)
		poly_int.a[i] = 0;
	
	poly_int.n = poly.n + 1;
		
	if (a == b)
		return 0;
	
	integrate(poly, poly_int, 0);
	
	val = eval(poly, b) - eval(poly, a);
		
	return val;
}




char *convert_to_chars(polynomial poly) {

	char poly_eq[MAX_EQUATION_CHARACTERS] = {0};
	
	poly_eq[0] = 'k';
	poly_eq[1] = '+';
	poly_eq[2] = 'k';
	poly_eq[3] = '*';
	poly_eq[4] = 'x';
	
	for (int i = 5; i <= 6*poly.n; i = i+6) {
		poly_eq[i] = '+';
		poly_eq[i+1] = 'k';
		poly_eq[i+2] = '*';
		poly_eq[i+3] = 'x';
		poly_eq[i+4] = '^';
		poly_eq[i+5] = '0' + (i+7)/6;
	}
	//for (int j = 0; j <= poly.n; ++j)
		//saved_number[j] = poly.a[j];
	
	return poly_eq;
}




void print_poly_to_screen(polynomial poly) {
	
	cout << poly.a[0];
	
	if (poly.a[1] != 0)
		cout << "+" << poly.a[1] << "x";
	
	for (int i = 2; i <= poly.n; ++i)
		if (poly.a[i] != 0) 
			cout << "+" << poly.a[i] << "x^" << ('0' + i);	
}






