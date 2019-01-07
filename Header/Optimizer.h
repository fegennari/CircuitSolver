#ifndef _OPTIMIZER_H_
#define _OPTIMIZER_H_


#include "STL_Support.h"
#include "Oracle.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, Optimizer.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Optimizer header - convex optimization cutting plane method
// EE219A final project
// Began 11/5/02



#define MAX_OPT_ITER    50     // make this bigger?
#define OPT_TOLERANCE_O 1.0E-6 // tolerance for objective
#define OPT_TOLERANCE_V 1.0E-4 // tolerance for values

#define NEWTON_TOLERANCE 1.0E-6
#define MAX_NEWTON_ITERS 100

#define USE_ANALYTIC_CENTER      1 // otherwise line-plane intersection method which is not working in all cases
#define INIT_GUESS_APPROX_CENTER 1 // approx center intersect = O(P*N), Ch. 11 = O(N^3+P*N^2)



struct cutplane {

	char is_constraint;
	double b;
	vector<double> gradient, point;

	cutplane(vector<double> &g, vector<double> &p, double bb, char constraint) : gradient(g), point(p), b(bb), is_constraint(constraint) {};
};



double line_inersect_plane(vector<double> &gk, vector<double> &g0, vector<double> &pk, vector<double> &p0, int &db0);



class optimizer {

public:
	optimizer(oracle *Oracle0);
	~optimizer();
	double get_val_change() const {return vdiff;};
	int run(int init_min_vals);
	int check_optimal_results();
	vector<double> get_vars() const {return vars;};
	unsigned get_num_vars() const {return vars.size();};
	void print_data(int amount) const;

private:
	int init(int init_min_vals);
	int calc_point();
	void set_params_to_center();
	int get_approx_center();
	int get_analytic_center();
	void build_hessian(double **H, double *g, double *x, int skip_last_plane);
	int is_opt();
	int get_gradient();
	int check_one_optimal_result(double old_val, int i);

	unsigned iter;
	double cost, old_cost, vdiff;
	oracle *Oracle;
	vector<double> vars, zvars, pmin, pmax, zmin, zmax, gradient;
	vector<cutplane> polytope;

	// for analytic center
	double **H, **Hinv, **ai, *x1, *x2, *z, *g;
};




#endif


