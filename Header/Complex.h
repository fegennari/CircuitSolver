#ifndef _COMPLEX_H_
#define _COMPLEX_H_

#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Complex.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Complex Numbers Header
// By Frank Gennari
struct polar {
	double mag, phase;
};



complex get_j();
istream& operator>>(ifstream& in, complex& z);
ostream& operator<<(ostream& out, complex& z);
complex operator+(complex num1, complex num2);
complex operator-(complex num1, complex num2);
complex operator*(complex num1, complex num2);
complex operator/(complex num1, complex num2);
complex operator+(complex num2, double num1);
complex operator+(double num1, complex num2);
complex operator-(complex num2, double num1);
complex operator-(double num1, complex num2);
complex operator*(complex num2, double num1);
complex operator*(double num1, complex num2);
complex operator/(complex num2, double num1);
complex operator/(double num1, complex num2);
void out(complex Z);
void out2(complex Z);
void outp(complex Z);
void show_equation(complex z, double f, char units, char function);
int get_complex_form();
complex real_to_complex(double real);
double magnitude(complex z);
double ri_sum(complex z);
double phase(complex z);
double phase2(complex z);
complex phase_shift(complex z, double phase);
double get_angle(double x, double y);
complex expc(double theta);
complex exp(complex z);
polar RtoP(complex z);
complex PtoR(polar p);
complex parallel(complex num1, complex num2);
complex impedance(int ID, double f, double Value);
complex impedance(char comp, double f, double value);
complex admittance(int ID, double f, double Value);
void Z_equiv();
complex *memAlloc_complex_zero(complex *data, int size);


#endif
