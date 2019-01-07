#include "Optimizer.h"
#include "RLC_Tree.h"
#include "Matrix.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.Optimizer.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Optimizer - convex optimization cutting plane method for RLC tree circuits
// EE219A final project
// Began 11/5/02



optimizer::optimizer(oracle *Oracle0) {

	assert(Oracle0);

	Oracle   = Oracle0;
	cost     = 0.0;
	old_cost = 0.0;
	iter     = 0;
	vdiff    = 0.0;
	xdiff    = 0.0;
	H        = NULL;
	Hinv     = NULL;
	ai       = NULL;
	g        = NULL;
	z        = NULL;
	x1       = NULL;
	x2       = NULL;
}



optimizer::~optimizer() {

	unsigned i;

	// free analytic center memory
	if (H)    delete_matrix(H, vars.size());
	if (Hinv) delete_matrix(Hinv, vars.size());
	if (ai)   delete_matrix(ai, vars.size());
	if (g)    delete [] g;
	if (z)    delete [] z;
	if (x1)   delete [] x1;
	if (x2)   delete [] x2;

	// might not need to delete these vectors here - their destructors should take care of this
	for (i = 0; i < polytope.size(); ++i) {
		vector_delete(polytope[i].gradient);
		vector_delete(polytope[i].point);
	}
	vector_delete(polytope);

	vector_delete(vars);
	vector_delete(zvars);
	vector_delete(pmin);
	vector_delete(pmax);
	vector_delete(gradient);
	vector_delete(zmin);
	vector_delete(zmax);
}



int optimizer::init(int init_min_vals) {

	unsigned i, size;
	double temp; // don't want initial cost

	if (init_min_vals) {
		Oracle->set_min_vals();
	}
	Oracle->get_params(vars);

	size = vars.size();

	if (size == 0) { // no variables
		return 0;
	}
	zvars.resize(size);
	zmin.resize(size);
	zmax.resize(size);

	Oracle->get_min_max_params(pmin, pmax);
	assert(pmin.size() == size && pmax.size() == size);

	for (i = 0; i < size; ++i) { // calculate initial range of values
		zmin[i] = log(pmin[i]);
		zmax[i] = log(pmax[i]);
	}
	Oracle->eval(vars, temp);

	if (init_min_vals) {
		if (!Oracle->init()) { // failed because of area
			return 1;
		}
	}
	return 2;
}



void optimizer::set_params_to_center() {

	unsigned i, size;

	size = vars.size();
	assert(zvars.size() == size && zmin.size() == size && zmax.size() == size);

	for (i = 0; i < size; ++i) {
		zvars[i] = 0.5*(zmin[i] + zmax[i]);
		vars[i]  = exp(zvars[i]);
	}
}



int optimizer::is_opt() { // only optimal if cutting plane (gradient) is zero vector

	unsigned i;

	for (i = 0; i < gradient.size(); ++i) {
		if (fabs(gradient[i]) > GRADIENT_TOLER) {
			return 0;
		}
	}
	return 1;
}



int optimizer::get_gradient() {

	int feasible;
	unsigned i, size;
	double val, val2, b = 0.0;
	vector<double> gradient2;

	static int last_feasible = 1;
	static double last_val = 0.0;

	feasible = Oracle->get_gradient(gradient, gradient2, val, val2);
	size     = gradient.size();

	if (iter > 1 && !feasible && !last_feasible) {
		assert(val > 0.0);
		xdiff = (last_val - val)/val;

		if (xdiff <= 0.0) {
			cout << "missed constraint = " << val << ", last = " << last_val << " at iteration " << iter << "." << endl;
		}
	}
	last_feasible = feasible;
	last_val      = val;

	// xi=exp(zi) => zi=ln(xi) => (d/dx)zi = (d/dx)ln(xi) = ((d/dx)xi)*((d/dx)ln) = ((d/dx)xi)/xi
	for (i = 0; i < size; ++i) {
		gradient[i] *= vars[i];
	}
	if (USE_ANALYTIC_CENTER) {
		for (i = 0; i < size; ++i) {
			b += gradient[i]*zvars[i];
		}
	}
	polytope.push_back(cutplane(gradient, zvars, b, (char)(feasible - 1)));

	if (gradient2.size() != 0) { // second feasibility cut
		assert(!feasible && gradient2.size() == size);
		
		// xi=exp(zi) => zi=ln(xi) => (d/dx)zi = (d/dx)ln(xi) = ((d/dx)xi)*((d/dx)ln) = ((d/dx)xi)/xi
		for (i = 0; i < size; ++i) {
			gradient2[i] *= vars[i];
		}
		if (USE_ANALYTIC_CENTER) {
			for (i = 0; i < size; ++i) {
				b += gradient2[i]*zvars[i];
			}
		}
		polytope.push_back(cutplane(gradient2, zvars, b, (char)(feasible - 1)));
		vector_delete(gradient2);
	}
	return feasible;
}



int optimizer::run(int init_min_vals) {

	int status, opt, feasible;
	unsigned i, max_iter, size;
	double cdiff, gmag;

	status = init(init_min_vals);

	if (status != 2) {
		return status;
	}
	
	// add cutting planes to enforce monotonicity?
	set_params_to_center(); // best strategy - start with center of polytope (initially an n-D box)

	if (OPTIMIZER_METHOD == OPT_SIMPLE) {
		return simple_optimize(0);
	}
	if (OPTIMIZER_METHOD == OPT_RANDOM) {
		return random_optimize();
	}
	size     = vars.size();
	max_iter = MAX_OPT_ITER*size*size;

	// main loop
	for (iter = 1; iter <= max_iter; ++iter) {
		old_cost = cost;
		status   = Oracle->eval(vars, cost);
		feasible = get_gradient();
		opt      = is_opt();
		assert(cost >= 0.0);
		cdiff = fabs(cost - old_cost)/cost;

		if (DEBUG_MODE) {
			cout << iter << ", s = " << status << ", f = " << feasible << ", cost = " << cost << ", dcost = " << cdiff << ", dval = " << vdiff << endl;
			
			if (DEBUG_MODE > 1) {
				cout << "vars = ";
				for (i = 0; i < size; ++i) {
					cout << vars[i] << " ";
				}
				cout << endl;
				cout << "g = ";
				for (i = 0; i < size; ++i) {
					cout << gradient[i] << " ";
				}
				cout << endl;
				cout << "crit_leaf = " << Oracle->get_crit_leaf() << endl << endl;
			}
		}
		if (status && feasible) { // constraints met
			if (opt || cost == 0.0 || (cdiff < OPT_TOLERANCE_O && vdiff < OPT_TOLERANCE_V)) { // if min cost then stop
				check_edge_values();
				cout << endl << "Optimizer converged after " << iter << " iterations with final cost " << cost << "." << endl;
				
				gmag = 0.0;
				for (i = 0; i < size; ++i) {
					gmag += fabs(gradient[i]);
				}
				cout << "mag(gradient) = " << gmag << "." << endl;

				print_data(0);

				if (OPTIMIZER_METHOD == OPT_ACCPM_THEN_SIMPLE) {
					cout << "Main optimization time:";
					show_elapsed_time();
					cout << endl;
					return simple_optimize(REFINE_SIMPLE_OPT_STEPS);
				}
				if (OPTIMIZER_METHOD == OPT_ACCPM_THEN_RANDOM) {
					cout << "Main optimization time:";
					show_elapsed_time();
					cout << endl;
					return random_optimize();
				}
				return 1;
			}
		}
		else if (iter > 2 && fabs(xdiff) < OPT_TOLERANCE_O && vdiff < OPT_TOLERANCE_V) {
			cout << endl << "Optimizer still fails constraints after " << iter << " iterations with cost " << cost << "." << endl;
			print_data(0);
			return 0; 
		}
		if (!calc_point()) {
			cout << "Cannot find center of polytope at iteration " << iter << "." << endl;
			return 0; // return 0 or 1?
		}
	}
	cerr << "Convergence Error: Optimizer exceeded max iterations of " << max_iter << "." << endl;

	return 0;
}



int optimizer::simple_optimize(unsigned init_step) { // kind of like iterative n-D bisection

	int opt, converged, status, status0 = 1;
	unsigned stepnum, i, state, size, max_iter;
	double init_cost, cdiff, step = 0.0, temp, over, last_over = 0.0;

	size = vars.size();

	if (size == 0) {
		return 1;
	}
	if (cost == 0.0) {
		status0   = eval_and_check_point(cost, opt, over);
		last_over = over;
	}
	max_iter  = MAX_OPT_ITER*size*size;
	init_cost = cost;
	old_cost  = cost;

	// main loop
	for (stepnum = 1; stepnum < SIMPLE_OPT_STEPS; ++stepnum) {
		for (iter = 1; iter <= max_iter; ++iter) {
			step      = pow(SPLIT_FACTOR, -((double)stepnum));
			converged = 1;
			state     = 0;

			for (i = 0; i < size; ++i) {
				for (state = 0; state <= 1; ++state) {
					temp    = vars[i];
					vars[i] = min(pmax[i], max(pmin[i], (vars[i] + (pmax[i] - pmin[i])*step*(2.0*(0.5-(double)state))))); // choose point
					status  = eval_and_check_point(cost, opt, over);

					if (status == 1 && (status0 == 0 || cost < old_cost)) { // constraints met and lower cost
						cdiff = (old_cost - cost)/cost;

						if (cost == 0.0) { // if min cost then stop
							cout << endl << "Optimizer converged after " << iter << " iterations with final cost " << cost << "." << endl;
							print_data(0);
							return 1;
						}
						old_cost  = cost;
						converged = 0;
						status0   = 1;
						zvars[i]  = log(vars[i]); // is this necessary?
						++state;
					}
					else if (status == 0 && status0 == 0 && over < last_over) { // closer to meeting constraints
						last_over = over;
						converged = 0;
						zvars[i]  = log(vars[i]); // is this necessary?
						++state;
					}
					else {
						vars[i] = temp;
					}
				}
			}
			if (converged) {
				break;
			}
		}
		if (iter > max_iter) {
			cout << "Nonconvergence at stimple optimization step " << stepnum << "." << endl;
		}
	}
	if (status == 0) {
		cout << "Failed to meet constraints by " << over << "." << endl;
		return 0;
	}
	cost = old_cost;
	check_edge_values();

	if (init_cost == 0.0) {
		cout << "best cost = " << cost << "." << endl;
	}
	else {
		cout << "best cost = " << cost << ", previous best = " << init_cost << " = " << cost/init_cost << "*init_cost, improvement = " << (init_cost - cost)/cost << "." << endl;
	}
	print_data(0);

	return 1;
}



int optimizer::random_optimize() { // kind of like simulated annealing but without hill climbing

	int status, status0, opt;
	unsigned size, var, p1maxiter, p2maxiter;
	double val, init_cost, old_val, step, rval, over, last_over;

	size = vars.size();

	if (size == 0) {
		return 1;
	}
	if (cost == 0.0) {
		status0   = eval_and_check_point(cost, opt, over);
		last_over = over;
	}
	init_cost = cost;
	old_cost  = cost;
	p1maxiter = RAND_PHASE1_ITERS*size;
	p2maxiter = RAND_PHASE2_ITERS*size;

	cout << "Random optimize phase 1." << endl;

	// main loop
	for (iter = 1; iter <= p2maxiter; ++iter) {
		var     = rand()%size; // randomly choose variable
		old_val = vars[var];
		rval    = ((double)(rand()%1000))/1000.0; // random new value

		assert(rval >= 0.0 && rval <= 1.0);

		if (iter == p1maxiter) {
			cout << "Random optimize phase 2." << endl;
		}
		if (iter < p1maxiter) { // phase 1: random starting point
			vars[var] = pmin[var] + (pmax[var] - pmin[var])*rval;
		}
		else { // phase 2: move in smaller amounts
			step      = (pmax[var] - pmin[var])/((iter - p1maxiter)*(iter - p1maxiter));
			val       = step*(rval - 0.5);
			vars[var] = max(pmin[var], min(pmax[var], (vars[var] + val)));
			
			if (vars[var] == old_val) { // same value (already max or min)
				continue;
			}
		}
		status = eval_and_check_point(cost, opt, over);

		if (status == 1 && (status0 == 0 || cost < old_cost)) { // found better solution
			if ((old_cost - cost)/cost > RAND_DCOST_TOLER) {
				cout << "vars[" << var << "] = " << old_val << " -> " << vars[var] << ", cost = " << old_cost << " -> " << cost << "." << endl;
			}
			old_cost   = cost;
			status0    = 1;
			zvars[var] = log(vars[var]); // just in case
		}
		else if (status == 0 && status0 == 0 && over < last_over) { // closer to meeting constraints
			last_over  = over;
			zvars[var] = log(vars[var]); // just in case
		}
		else { // worse/invalid solution, reset variables
			vars[var] = old_val;
		}
	}
	if (status == 0) {
		cout << endl << "Optimizer still fails constraints by " << over << " after " << iter << " iterations with cost " << cost << "." << endl;
		print_data(0);
		return 0; 
	}
	cost = old_cost;
	check_edge_values();

	if (init_cost == 0.0) {
		cout << "best cost = " << cost << "." << endl;
	}
	else {
		cout << "best cost = " << cost << ", previous best = " << init_cost << " = " << cost/init_cost << "*init_cost, improvement = " << (init_cost - cost)/cost << "." << endl;
	}
	print_data(0);

	return 1;
}



int optimizer::check_edge_values() { // check if wi = wmin or wi = wmax is better than close value

	int changed = 0, opt, do_check, status;
	unsigned i;
	double val, new_cost, over;

	cout << "Checking edge values." << endl;

	for (i = 0; i < vars.size(); ++i) {
		val = vars[i];

		if (val == pmin[i] || val == pmax[i]) { // already at edge
			continue;
		}
		if ((val - pmin[i])/val < EDGE_THRESHOLD) { // close to lower edge
			vars[i]  = pmin[i];
			do_check = 1;
		}
		else if ((pmax[i] - val)/val < EDGE_THRESHOLD) { // close to upper edge
			vars[i]  = pmax[i];
			do_check = 1;
		}
		else {
			do_check = 0;
		}
		if (do_check) {
			status = eval_and_check_point(new_cost, opt, over);

			if (status == 1 && new_cost < cost) {
				cost     = new_cost; // new cost is better
				zvars[i] = log(vars[i]);
				changed  = 1;
			}
			else {
				vars[i] = val; // reset old values
			}
		}
	}
	return changed;
}



int optimizer::check_optimal_results(double perturb_delta) {

	unsigned i;
	double old_val;

	cout << "Checking optimality of results." << endl;

	if (perturb_delta == 0.0) {
		perturb_delta = 10.0*OPT_TOLERANCE_V;
	}

	// sizing must be monotonic
	if (!Oracle->is_sizing_monotonic(vars)) {
		cout << "Width parameter sizing is not monotonic." << endl;
		return 0;
	}

	// perturb each parameter by +/- delta
	for (i = 0; i < vars.size(); ++i) {
		old_val  = vars[i];
		vars[i] += perturb_delta*vars[i];

		if (!check_one_optimal_result(old_val, i)) {
			return 0;
		}
		vars[i] -= 2.0*perturb_delta*vars[i];

		if (!check_one_optimal_result(old_val, i)) {
			return 0;
		}
		vars[i] = old_val;
	}

	// reset the old values
	Oracle->eval(vars, cost);
	get_gradient();

	return 1;
}



int optimizer::check_one_optimal_result(double old_val, int i) {

	int opt, status;
	unsigned j;
	double new_cost, dcost, over;
	
	for (j = 0; j < vars.size(); ++j) { // check against min/max values
		if (vars[j] < pmin[j] || vars[j] > pmax[j]) {
			return 1;
		}
	}
	status = eval_and_check_point(new_cost, opt, over);

	if (status != 2) {
		return 1;
	}
	dcost = (cost - new_cost)/new_cost;

	if (dcost > 10.0*OPT_TOLERANCE_O) {
		cout << "Changing parameter " << i << " from " << old_val << " (cost " << cost << ") to " << vars[i] << " (cost " << new_cost << ") for dx = " << (vars[i] - old_val) << " (dc = " << (cost - new_cost) << ") yields a more optimal solution." << endl;
		cout << "dcost = " << (cost - new_cost)/cost << "." << endl;

		if ((int)gradient.size() > i) {
			cout << "gradient[" << i << "] = " << gradient[i] << endl;

			if ((vars[i] - old_val)*gradient[i] > 0.0) {
				cout << "Error in gradient!" << endl;
			}
			else {
				double *x = new double[vars.size()];

				for (j = 0; j < vars.size(); ++j) {
					x[j] = log(vars[j]);
				}
				if (!check_dot_products(x)) {
					cout << "Error in plane normal!" << endl;
				}
				delete [] x;
			}
		}
		vars[i] = old_val;
		Oracle->eval(vars, cost); // reset the old values

		if ((int)gradient.size() > i) {
			get_gradient();
		}
		return 0;
	}
	return 1;
}



int optimizer::eval_and_check_point(double &cost_val, int &opt, double &over_val) {

	int status;

	status = Oracle->eval(vars, cost_val);
	
	if (!Oracle->meets_constaints(over_val) || !status) {
		return 0;
	}
	opt = is_opt();
	assert(cost >= 0.0);

	return 1;
}



void optimizer::print_data(int amount) const {

	unsigned i;

	Oracle->print_data(amount);
	cout << "iterations = " << iter << ", cost = " << cost << endl;
	cout << "vars = " << endl;

	for (i = 0; i < vars.size(); ++i) {
		cout << vars[i] << endl;
	}
	if (amount > 0) {
		cout << endl << endl << "gradient = " << endl;

		for (i = 0; i < gradient.size(); ++i) {
			cout << gradient[i] << endl;
		}
	}
	cout << endl << endl;
}






