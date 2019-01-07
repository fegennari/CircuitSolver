#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Diodes.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Diodes
// by Frank Gennari
#define k 1.38e-23
#define q 1.602e-19

double const default_T = 293.0;
double const default_Is = 1.0e-14;
double const default_n = 1.0;


struct Diode {
	double Is, T, n;
};


void get_default_diode(Diode &default_diode);
double cacl_i(Diode &diode, double v);
double calc_v(Diode &diode, double i);





void get_default_diode(Diode &default_diode) {

	default_diode.Is = default_Is;
	default_diode.T = default_T;
	default_diode.n = default_n;
}




double cacl_i(Diode &diode, double v) {

	return diode.Is*(expf(v*q/(diode.n*k*diode.T)) - 1);
}




double calc_v(Diode &diode, double i) {

	return (diode.n*k*diode.T/q)*(log(i/diode.Is) + 1);
}









