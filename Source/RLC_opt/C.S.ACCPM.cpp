// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.ACCPM.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Optimizer: ACCPM Optimization Interface - convex optimization cutting plane method for RLC tree circuits
// EE219A final project
// Began 11/29/02
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 
#include <time.h>
#include <iostream.h>

#include "user.h"
#include "utils.h"
#include "coordinator.h"
//#include "libcpm.h"


//#define HAVE_CORRECT_LIBRARIES


#ifdef HAVE_CORRECT_LIBRARIES


extern void CPM_build_cut(struct Newcut*, int,double *, int, double *, double, int, double, int);
extern int CPM_coordinator(struct user_defined, int , char **);


int run_accpm_opt() {

	int error = 0;
	struct user_defined *my_variables = NULL;

	int argc = 0;
	char **argv = NULL;

	error = CPM_coordinator(my_variables, argc, argv);

	if (error) {
		cout << "ACCPM returned error code " << error << "." << endl;
		return 0;
	}
	cout << "ACCPM successful." << endl;

	return 1;
}



void set_fixed_box(int d, char* data, double *l, double *u, int* lw_detect, int* up_detect) {

	// d = problem dimension
	// data = datafile
	// u : vector of fixed upper bounds
	// l : vector of fixed lower bounds

	int i;

	for (i=0; i<d; i++) {
		//	 l[i] = ...;
		//	 u[i] = ...;
	}
}


void modify_box(int d, double *Ml, double *Mu, double *l, double *u, double *x0) {

	// d = problem dimension
	// u : vector of fixed upper bounds
	// l : vector of fixed lower bounds
	// Mu : vector of pushable upper bounds
	// Ml : vector of pushable lower bounds
	// x0 : initial point

	int i;

	for (i=0; i<d; i++) {
		//	 Ml[i] = ...;
		//	 Mu[i] = ...;
		// x0[i] = 0.5* (Ml[i] + Mu[i]);   //center of the initial box
	}	
}


void modify_bounds(int dim,char * datafile,double *Mu, double *Ml, double& binf_opt,double& bsup_opt) {

	//bsup_opt = ...;
	//binf_opt = ...;
}



//-------------------------------------------------------------------------------
void CPM_oracle(struct Newcut *cut, int dim, int sparsity, char *datafile, double* x, double **x2, int use_x2,struct Archive *history, struct user_defined *my_variables) {

	double  bsup       =  0.0;
	int     type       = -1;
	double  f          =  0;
	int     subproblem =  0;
	double* gradient   = new double[dim];
	int i;

	for (i=0;i<dim;i++)
		gradient[i] = 0;

	printf("Useless oracle\n");

	CPM_build_cut(cut,dim,x,sparsity,gradient,f,type,bsup,subproblem);

	delete(gradient);
}

#endif



