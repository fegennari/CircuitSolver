#include "CircuitSolver.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.OpAmp.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Op Amps
// by Frank Gennari
struct OpAmp {

	double Ao, Ri, Ro, Ib, Ios, Vos, Vp, Vn;
};



void get_ideal_opamp(OpAmp &ideal_opamp);
void get_default_opamp(OpAmp &default_opamp);
void get_opamp_model(OpAmp &opamp);





void get_ideal_opamp(OpAmp &ideal_opamp) {

	ideal_opamp.Ao = bound;
	ideal_opamp.Ri = bound;
	ideal_opamp.Ro = 0;
	ideal_opamp.Ib = 0;
	ideal_opamp.Ios = 0;
	ideal_opamp.Vos = 0;
	ideal_opamp.Vp = bound;
	ideal_opamp.Vn = -bound;
}




void get_default_opamp(OpAmp &default_opamp) {

	default_opamp.Ao = A;
	default_opamp.Ri = 10e6;
	default_opamp.Ro = 100;
	default_opamp.Ib = 1e-9;
	default_opamp.Ios = 100e-12;
	default_opamp.Vos = 1e-3;
	default_opamp.Vp = 15;
	default_opamp.Vn = -15;
}




void get_opamp_model(OpAmp &opamp) {

	opamp.Ao = A;
}




