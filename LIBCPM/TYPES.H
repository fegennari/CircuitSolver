// This file contains some types declarations for ACCPM coordinator


// Written by:    Olivier Péton
//                Logilab, HEC Geneva, Section of Management Studies, 
//                40, Bd du pont d'Arve, 1211 Geneva, Switzerland.
// Last modified: october 12, 2000


// Archive 
// recursive list
// stores the query point and the cut(s) generated at each iteration
typedef struct Archive
{
  double	*point;		// current iterate coordinates
  double	*point2;	// alternative proposition for current point
  double	upper;	    // upper bound for objective
  double	lower;	    // lower bound for objective
  int		nCuts;	    // number of columns (cuts)
  int		nElts;      // number of elements              
  int		*type;		// type of generated cuts (array of dimension ncut)
  double	*gradient;  // vector of generated cuts (array)
  int		*begCol;       
  double	*fValue;	// function value for each cut (array of dimension ncut)
  double	*cTerms;	// rhs term for each cut (array of dimension ncut)
  int		*pIndex;	// subproblem index (array of dimension ncut)
  double	bsup;	    // bsup = sum of fValue[i] for optimality cuts i
  struct	Archive *next;
}Archive;


// Newcut
// description of the cut(s) added at the current iteration

typedef struct Newcut
{
	int     nCuts;
	int     nElts;
	double  bsup;
	double *gradient;
	double *cTerms;
	double *fValue;
	int    *cutType;
	int    *pIndex;
	int    *rowIndices;
	int    *begCol;
}Newcut;



// User Settings : execution parameters of ACCPM 

typedef struct UserSettings
{
//****************************************
//*** GENERAL PARAMETERS               ***
//****************************************
  int cut_structure;	  // 0 = dense, 1 = sparse, 2 = network
  int exp_iter;           // expected number of iterations
  int max_iter;           // max number of iterations
  int do_testcolumn;      // flag indicating if identical column must be searched
  int do_push_bounds;     // flag indicating if the box constraints can be pushed
  int do_detect_infinity; // flag indicating if reaching one box side means that pb is unbounded
  int do_weight;          // flag indicating if weighted analytic centers are computed
  int do_eliminate_cuts;  // flag indicating if useless cuts can be suppressed (take care !!!)
  int do_use_x2;          // flag indicating if an alternative point than a.c. is computed
//****************************************
//*** QUERY POINT GENERATOR PARAMETERS ***
//****************************************
  double centering;        // centering parameter (quality of the a.c. approximation)
  int do_Mitchell;         // flag indicating if Mitchell_add_cuts is used
  int factor_L;            // flag indicating if sparse Cholesky factorization is used
  int explicit_B;          // flag indicating storage of a partial result of the inv. representation
  int dense_GUB;           // take advantage of GUB rows in the constraint matrix = 0, else = 1
  int nonz_in_cuts;        // number of nonzero elements in cuts (= dimension for dense cuts) 
  int densityB;            // number of nonzeros per column in matrix B
  int densityL;            // number of nonzeros per column in a sparse Cholesky factorisation
  int space;               // maximum allocated memory
//***************************************
//*** PROCESS and DISPLAY PARAMETERS  ***
//***************************************
  int verbose;             // flag indicating if extensive output comments must be given
  int step_by_step;        // flag indicating of ACCPM stops after each iteration
  int display_result;      // flag indicating if the final result is displayed
  int save_result;         // flag indicating if the result is saved in the resultfile
  int save_history;        // flag indicating if a history log of the process must be created
//---------------------------------------
  int save_iterates;       // the following variable enumerate the items
  int save_lower_bound;    // that can be stored in the history file. 
  int save_upper_bound;    // set the variables to 1 to save the corresponding item
  int save_cut_type;       // 0 otherwise.
  int save_function_value;
  int save_cuts;
  int save_cterms;
  int save_bsup;
  int save_dual;
}
UserSettings;
