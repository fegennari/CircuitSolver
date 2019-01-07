#include "Oracle.h"
#include "RLC_Tree.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.Oracle.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Optimizer - optimization oracle for RLC tree circuits
// EE219A final project
// Began 11/14/02

/*
Variables:   transistor and interconnect widths x[i] > 0
Constraints: min/max transistor and interconnect widths, max area, max delay
Objective:   minimize a weighted combination of delay and area (and possibly power) D*A^c
*/


int const EVAL_COST_FOR_FAIL = 0;



oracle::oracle(opt_problem *problem) {

	assert(problem);

	op          = problem;
	alpha       = DEF_ALPHA;
	delay       = 0.0;
	area        = 0.0;
	delay0      = 0.0;
	area0       = 0.0;
	max_delay   = 0.0;
	max_area    = 0.0;
	delay_model = 0;
}



int oracle::init() {

	delay0 = delay;
	area0  = area;

	cout << "area = " << area << ", delay = " << delay << ", max_area = " << max_area << ", max_delay = " << max_delay << endl;

	if (max_area == 0.0) {
		return 1;
	}
	if (max_area < area0) {
		cout << "The area constraint of " << max_area << " is less than the minimum width area of " << area0 << ". There is no solution." << endl;
		return 0;
	}
	if (max_area == area0) { // problem solved
		cout << "The only solution to this problem sets all widths to minimum values for an area of " << area0 << "." << endl;
		return 0;
	}
	return 1;
}



int oracle::set_constraint(int type, double value) {

	if (value == 0.0) {
		return 1;
	}
	switch (type) {
	case TYPE_DELAY:
		max_delay = value;
		
		if (max_area != 0.0) {
			cout << "Warning: Delay and area constraints in the same optimization run can cause problems." << endl;
		}
		break;

	case TYPE_AREA:
		if (value < area0) {
			cout << "The area constraint of " << value << " is less than the minimum width area of " << area0 << ". There is no solution." << endl;
			return 0;
		}
		if (value == area0) { // problem solved
			cout << "The only solution to this problem sets all widths to minimum values for an area of " << area0 << "." << endl;
			return 0;
		}
		max_area = value;

		if (max_delay != 0.0) {
			cout << "Warning: Delay and area constraints in the same optimization run can cause problems." << endl;
		}
		break;

	default:
		cerr << "Error: Unrecognized constraint passed to oracle: " << type << "." << endl;
		return 0;
	}
	return 1;
}



int oracle::eval(vector<double> &vars, double &cost) {

	if (!op->update_params(vars, 1)) { // violates constraints
		return 0;
	}
	op->build_rc_lc_terms(1);
	delay = op->get_max_delay(crit_leaf, delay_model);

	if (max_delay > 0.0 || alpha != 0.0) {
		area = op->get_area(MOS_PLUS_WIRE_AREA);
	}
	if (EVAL_COST_FOR_FAIL) {
		if (alpha == 0.0) {
			cost = delay;
		}
		else {
			cost = delay*pow(area, alpha);
		}
	}
	if (DEBUG_MODE) {
		cout << "delay = " << delay << ", area = " << area << endl;
	}
	if (max_delay > 0.0 && delay > max_delay) {
		return 0; // fails delay constraint
	}
	if (max_area > 0.0 && area > max_area) {
		return 0; // fails area constraint
	}
	if (!EVAL_COST_FOR_FAIL) {
		if (alpha == 0.0) {
			cost = delay;
		}
		else {
			cost = delay*pow(area, alpha);
		}
	}
	return 1;
}


// 0 = failed, 1 = good, 2 = optimal
int oracle::get_gradient(vector<double> &gradient, vector<double> &gradient2, double &val, double &val2) {

	unsigned i;
	double amult;

	if (max_delay > 0.0 && delay > max_delay) { // failed delay constraint
		op->get_gradient(gradient, TYPE_DELAY, delay_model);
		val = (delay - max_delay); // feasibility cut
		
		if (max_area > 0.0 && area > max_area) { // failed area constraint also
			op->get_gradient(gradient2, TYPE_AREA, delay_model);
			assert(gradient2.size() == gradient.size());
			val2 = (area - max_area); // feasibility cut #2
		}
		return 0;
	}
	else if (max_area > 0.0 && area > max_area) { // failed area constraint
		op->get_gradient(gradient, TYPE_AREA, delay_model);
		val = (area - max_area); // feasibility cut
		return 0;
	}
	op->get_gradient(gradient, TYPE_DELAY, delay_model);

	if (alpha == 0.0) {
		return 1;
	}
	op->get_gradient(gradient2, TYPE_AREA, delay_model);
	assert(gradient2.size() == gradient.size());

	// chain rule: a=A^alpha => a' = (A^alpha)' = alpha*A^(alpha-1)*A'
	amult = alpha*pow(area, alpha-1.0);

	// multiplication: (d/dx)(ad) = ad'+a'd
	for (i = 0; i < gradient.size(); ++i) {
		gradient[i] = area*gradient[i] + amult*gradient2[i]*delay;
	}
	vector_delete(gradient2);

	return 1;
}



int oracle::meets_constaints(double &over_val) {

	if (max_delay != 0.0 && delay > max_delay) {
		over_val = (delay - max_delay);
		return 0;
	}
	if (max_area != 0.0 && area > max_area) {
		over_val = (area - max_area);
		return 0;
	}
	over_val = 0.0;

	return 1;
}



void oracle::print_data(int amount) const {

	cout << endl << "crit_leaf = " << crit_leaf << ", delay0 = " << delay0 << ", area0 = " << area0 << ", delay = " << delay << ", area = " << area << endl;
	
	if (amount > 0) {
		op->print_params();
	}
}


