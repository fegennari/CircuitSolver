#include "CircuitSolver.h"
#include "Logic.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.MOSFET.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver MOSFETs
// By Frank Gennari
#define q	1.602e-19
#define E0	8.854e-14
#define Es	11.7*E0
#define Ei	3.9*E0


int const def_use_delay_not_wl = 0; // allows delay in logic simulation, not used yet

int mos_delay_not_wl(def_use_delay_not_wl);



struct mosfet {

	char*model;
	int level;
	double phi, tox, xj, tpg, vt0, delta, ld, kp, u0, theta, rsh, gamma, nsub, nfs, vmax, eta, kappa, cgd0, cgs0, cgb0, cj, mj, cjsw, mjsw, pb;
};


component *pick_MOSFET(component *head, string name, int pins, string description);
double calc_Ids(const mosfet& MOS, double Vs, double Vd, double Vg, double Vb, double w_l, int type);


void draw_PFET(int box_num);
void draw_NFET(int box_num);




component *pick_MOSFET(component *head, string name, int pins, string description) {

	init_menu(2, 10, 0);
	
	draw_NFET(0);
	draw_PFET(1);
	
	cout << "\nChoose a type of MOSFET." << endl;
	
	return draw_component_at_click((FET_ID + which_box(GetClick())), head, name, pins, description);
}




double calc_Ids(const mosfet& MOS, double Vs, double Vd, double Vg, double Vb, double w_l, int type) {

	double Ids(0), Vt, Ci;
	
	Ci = .01*Ei/MOS.tox; // m->cm
	
	if (type == 0) { // NMOS
		
		Vt = -sqrt(2*q*Es*MOS.nsub*fabs(MOS.phi))/Ci - fabs(MOS.phi);
	
		if (Vg - Vs < Vt) { // Cutoff
			Ids = 0;
		}
		else { // triode
			Ids = MOS.u0*MOS.tox*w_l*((Vg - Vs - MOS.vt0)*(Vd - Vs) - 0.5*(Vd - Vs)*(Vd - Vs)); 
		}
		// saturation
	}
	else if (type == 1) { // PMOS
	
		Vd = Vb = w_l = 0; // Just temporary

		//Ids = ???;
	}
	
	return Ids;	
}













