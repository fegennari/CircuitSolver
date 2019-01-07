#include "Optimizer.h"
#include "Oracle.h"
#include "Matrix.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.QueryPoint.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Query Point Generator (Analytic Center, etc.) - convex optimization cutting plane method for RLC tree circuits
// EE219A final project
// Began 11/29/02


double line_inersect_plane(vector<double> &gk, vector<double> &g0, vector<double> &pk, vector<double> &p0, int &db0) {

	unsigned i, size;
	double num = 0.0, denom = 0.0;

	size = gk.size();
	assert(g0.size() == size && pk.size() == size && p0.size() == size);

	// gi dot g0
	for (i = 0; i < size; ++i) {
		denom += gk[i]*g0[i];
	}
	if (denom > 0.0) { // vectors in same direction: existing plane is behind the current one
		db0 = 0;
		return 0.0;
	}
	if (denom == 0.0) { // planes are parallel - can this happen?
		db0 = 1;
		return 0.0;
	}

	// gi dot (pi - p0)
	for (i = 0; i < size; ++i) {
		num += gk[i]*(pk[i] - p0[i]);
	}
	db0 = 0;
	assert(num != 0.0); // not sure if this is legal

	return num/denom;
}



int optimizer::calc_point() {

	unsigned i, size;
	double val, sval = 0.0, dval = 0.0;

	size = vars.size();
	assert(size > 0);

	if (USE_1D_BISECTION && size == 1) { // 1D special case: use bisection
		if (iter > 1) { // don't change range if first iteration
			if (gradient[0] < 0) {
				zmin[0] = zvars[0]; // make larger
			}
			else {
				zmax[0] = zvars[0]; // make smaller
			}
		}
		zvars[0] = 0.5*(zmin[0] + zmax[0]);
	}
	else { // get center
		if (USE_ANALYTIC_CENTER) {
			if (!get_analytic_center()) {
				return 0;
			}
		}
		else if (!get_approx_center()) { // fast but not center
			return 0;
		}
	}

	// xi = exp(zi) transform
	for (i = 0; i < size; ++i) {
		val     = exp(zvars[i]);
		sval   += val;
		dval    = fabs(val - vars[i]);
		vars[i] = val;
	}
	vdiff = dval/sval;

	return 1;
}



int optimizer::get_approx_center() {

	int init = 0, db0, ulimit;
	unsigned i, index, size;
	double u, umin;

	size = gradient.size();
	assert(polytope.size() > 0 && size == zvars.size());
	index = polytope.size()-1;

	ulimit = 0;

	// check distance to edges of box (zmin and zmax)
	for (i = 0; i < size; ++i) {
		if (gradient[i] == 0.0) {
			continue;
		}
		u = min((zmax[i] - zvars[i]), (zvars[i] - zmin[i]))/fabs(gradient[i]);

		if (u < 0.0) {
			continue;
		}
		if (u == 0.0) { // can't move any more
			return 1;
		}
		if (!init || u < umin) {
			umin   = u; // record parameter of closest plane
			init   = 1;
			ulimit = -int(i+1);
		}
	}

	// check distance to other cutting planes
	for (i = 0; i < index; ++i) {
		u = -line_inersect_plane(polytope[i].gradient, gradient, polytope[i].point, zvars, db0);
		
		if (u <= 0.0) {
			continue;
		}
		assert(!db0);

		if (!init || u < umin) {
			umin   = u; // record parameter of closest plane
			init   = 1;
			ulimit = i+1;
		}
	}
	umin *= 0.5; // halfway between planes

	// pc = p0 + (u/2)*g0
	for (i = 0; i < size; ++i) {
		zvars[i] -= umin*gradient[i];
	}
	return 1;
}



int optimizer::get_analytic_center() {

	int noconverge = 0, bad_point;
	unsigned i, j, size, iterations = 0;
	double zeta, xsum, xdiff, dx, val;
	vector<double> zvars_orig = zvars;

	size = vars.size();

	// allocate matrices and vectors if NULL
	if (!H)    H    = create_matrix(H,    size, size);
	if (!Hinv) Hinv = create_matrix(Hinv, size, size);
	if (!ai)   ai   = create_matrix(ai,   size, 2*size);
	if (!g)    g    = new double[size];
	if (!z)    z    = new double[size];
	if (!x1)   x1   = new double[size];
	if (!x2)   x2   = new double[size];

	if (DEBUG_MODE > 1) {
		cout << "Xprev = ";
		for (i = 0; i < size; ++i) {
			cout << zvars[i] << "(" << exp(zvars[i]) << ") ";
		}
		cout << endl;
	}
	if (INIT_GUESS_APPROX_CENTER) { // initial guess is halfway between last point and closest plane
		if (!get_approx_center()) {
			return 0;
		}
	}
	for (i = 0; i < size; ++i) {
		x1[i] = zvars[i];
	}
	if (!INIT_GUESS_APPROX_CENTER) { // another approach to choosing an initial x	
		build_hessian(H, g, x1, 1);
		assert(square_inverse_no_alloc(H, ai, Hinv, size));

		// x2 = Hinv*am
		for (i = 0; i < size; ++i) {
			x2[i] = 0.0;
			for (j = 0; j < size; ++j) {
				x2[i] += gradient[j]*Hinv[i][j];
			}
		}
		// val = amT*x2 = amT*Hinv*am
		val = 0.0;
		for (i = 0; i < size; ++i) {
			val += x2[i]*gradient[i];
		}
		assert(val > 0.0);
		val = sqrt(val);

		for (i = 0; i < size; ++i) {
			x1[i] -= x2[i]/val;
		}
	}
	if (DEBUG_MODE > 1) {
		cout << "Xinit = ";
		for (i = 0; i < size; ++i) {
			cout << x1[i] << "(" << exp(x1[i]) << ") ";
		}
		cout << endl;
	}
	assert(check_dot_products(x1));

	do {
		build_hessian(H, g, x1, 0);
		assert(square_inverse_no_alloc(H, ai, Hinv, size));
		zeta = 1.0; // step size, maybe make smaller

		//z = Hinv*g;
		for (i = 0; i < size; ++i) {
			val = 0.0;

			for (j = 0; j < size; ++j) {
				val -= Hinv[i][j]*g[j];
			}
			z[i] = val;
		}
		do {
			//x2 = x1 - zeta*g; // Newton iteration
			for (i = 0; i < size; ++i) {
				x2[i] = x1[i] + zeta*z[i];
			}
			bad_point = 0;

			if (!check_dot_products(x2)) { // check if inside polytope
				bad_point = 1;
				zeta     /= 2.0;

				if (zeta < 1.0E-12) {
					cerr << "Convergence error in Newton iteration parameter stepping." << endl;
					return 0;
				}
			}
		} while (bad_point);

		xsum  = 0.0;
		xdiff = 0.0;

		for (i = 0; i < size; ++i) {
			xsum  += fabs(x2[i]);
			xdiff += fabs(x2[i] - x1[i]);
		}
		dx = xdiff/xsum;

		if (DEBUG_MODE > 1) {
			cout << "X[" << iterations << "] = ";
			for (i = 0; i < size; ++i) {
				cout << x1[i] << "(" << exp(x1[i]) << ") ";
			}
			cout << ", sum = " << xsum << ", diff = " << xdiff << ", dx = " << dx << endl;
		}
		swap(x1, x2); // update x
		++iterations;

		if (iterations > MAX_NEWTON_ITERS) {
			cerr << "Nonconvergence after " << MAX_NEWTON_ITERS << " Newton iterations." << endl;
			cout << "sum = " << xsum << ", diff = " << xdiff << ", dx = " << dx << "." << endl;
			cout << endl << "X = ";
			for (i = 0; i < size; ++i) {
				cout << x1[i] << "(" << exp(x1[i]) << ")" << endl;
			}
			cout << endl;
			noconverge = 1;
			break;
		}
	} while (dx >= NEWTON_TOLERANCE);

	val = 0.0; // error checking
	for (i = 0; i < size; ++i) { // error check: correct side of last plane
		val += (zvars_orig[i] - x1[i])*gradient[i]; // dot product
	}
	assert(val > 0.0);

	if (DEBUG_MODE > 1) {
		cout << "Newton Iterations = " << iterations << "." << endl; // TESTING
	}
	if (DEBUG_MODE > 2) {
		cout << "Xfinal = ";
		for (i = 0; i < size; ++i) {
			cout << x1[i] << "(" << exp(x1[i]) << ") ";
		}
		cout << endl;
	}
	if (!noconverge) {
		for (i = 0; i < size; ++i) {
			zvars[i] = x1[i];
		}
	}
	return !noconverge;
}



// build Hessian and Gradient
void optimizer::build_hessian(double **H, double *g, double *x, int skip_last_plane) {

	unsigned i, j, p, n;
	double aa, denom, denom_sq, d1, d2, num;
	double *Hi;
	vector<double> gr;

	n = vars.size();

	assert(H != NULL && n > 0);

	// reset to all zeros
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			H[i][j] = 0.0;
		}
		g[i] = 0.0;
	}

	// add fixed min/max width planes
	for (i = 0; i < n; ++i) {
		d1       = 1.0/(x[i] - zmax[i]);
		d2       = 1.0/(-x[i] - zmin[i]);
		H[i][i] += d1*d1 + d2*d2;
		g[i]    -= d1 - d2;
	}

	// add contribution from each plane
	for (p = 0; p < polytope.size()-skip_last_plane; ++p) {
		aa = 0.0;
		gr = polytope[p].gradient; // make a local copy

		for (i = 0; i < n; ++i) {
			aa += gr[i]*x[i];
		}
		denom    = (aa - polytope[p].b);
		denom_sq = denom*denom;

		if (denom == 0.0) {
			cerr << "Error: Zero denominator at a = " << aa << ", b = " << polytope[p].b << ", plane " << p << "." << endl;
			assert(0);
		}
		for (i = 0; i < n; ++i) {
			num = gr[i]/denom_sq;
			Hi  = H[i];

			for (j = 0; j < n; ++j) {
				Hi[j] += num*gr[j];
			}
			g[i] -= gr[i]/denom;
		}
	}
}



int optimizer::check_dot_products(double *x) {

	unsigned i, p, size;
	double dotp;

	assert(x);
	size = vars.size();

	for (i = 0; i < size; ++i) { // check against min/max values
		if (x[i] < zmin[i] || x[i] > zmax[i]) {
			return 0;
		}
	}
	for (p = 0; p < polytope.size(); ++p) { // check against each cutting plane
		dotp = 0.0;

		for (i = 0; i < size; ++i) {
			dotp += (polytope[p].point[i] - x[i])*polytope[p].gradient[i];
		}
		if (dotp <= 0.0) {
			return 0;
		}
	}
	return 1;
}


