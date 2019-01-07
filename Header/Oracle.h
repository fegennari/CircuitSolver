#ifndef _ORACLE_H_
#define _ORACLE_H_


#include "STL_Support.h"
#include "RLC_Tree.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, Oracle.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Oracle header - optimization oracle for RLC tree circuits
// EE219A final project
// Began 11/14/02

/*
Variables:   transistor and interconnect widths x[i] > 0
Constraints: min/max transistor and interconnect widths, max area, max delay
Objective:   minimize a weighted combination of delay and area (and possibly power)
*/


#define DEBUG_MODE 0

#define DEF_ALPHA 0.0



typedef RLC_tree opt_problem;


class oracle {

public:
	oracle(opt_problem *problem);
	~oracle() {};
	void set_delay_model(int dm) {delay_model = dm;};
	void set_objective(double a) {alpha = a;};
	int set_constraint(int type, double value);

	int init();
	int eval(vector<double> &vars, double &cost);
	int get_gradient(vector<double> &cplane, double &val);

	void get_min_max_params(vector<double> &pmin, vector<double> &pmax) {op->get_min_max_params(pmin, pmax);};
	int get_params(vector<double> &x) const {return op->get_params(x);};
	int update_params(vector<double> &x, int test_level) {return op->update_params(x, test_level);};
	void set_min_vals() {op->set_min_widths();};
	int get_crit_leaf() const {return crit_leaf;};
	int is_sizing_monotonic() const {return op->is_sizing_monotonic();};
	void print_data(int amount) const;

private:

	int delay_model, crit_leaf;
	double area, delay, delay0, area0, alpha, max_delay, max_area;
	opt_problem *op;
};



#endif