#ifndef _RLC_TREE_H_
#define _RLC_TREE_H_


#include "STL_Support.h"

// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, RLC_Tree.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver RLC circuit analysis - RLC tree classes
// EE219A final project
// Began 10/25/02

// RLC flags
#define USED_FLAG      0x01
#define SEEN_FLAG      0x02
#define LEAF_NODE_FLAG 0x04
#define NODE_CAP_FLAG  0x08

#define TYPE_NMOS  2
#define TYPE_PMOS  4
#define TYPE_TLINE 8
#define TYPE_INV   16

#define TYPE_DELAY          0
#define TYPE_AREA           1
#define TYPE_AREA_AND_DELAY 2

#define TYPE_RC_DELAY 0
#define TYPE_TPDI     1
#define TYPE_TRI      2

#define NO_AREA            0
#define MOS_AREA           1
#define WIRE_AREA          2
#define MOS_PLUS_WIRE_AREA 3


#define TYPE_IS_TLINE(i) (params[i].type == TYPE_TLINE)
#define TYPE_IS_NMOS(i)  (params[i].type == TYPE_NMOS)
#define TYPE_IS_PMOS(i)  (params[i].type == TYPE_PMOS)



struct rlc_op {

	char op;
	unsigned n_src, n_dst, branch;

	rlc_op(char op0, unsigned n_src0, unsigned n_dst0, unsigned branch0) : op(op0),n_src(n_src0),n_dst(n_dst0),branch(branch0) {};
};


struct rlc_op_cap {

	unsigned n_src, n_dst;

	rlc_op_cap(unsigned n_src0, unsigned n_dst0) : n_src(n_src0),n_dst(n_dst0) {};
};



struct input_param {

	char type;
	unsigned branch, ns1, nd2, ng;
	double *comp_val, val, old_val, length;
};


struct tech_data {

	int tline_seg, is_distributed;
	double mos_rdsn, mos_rdsp, mos_cg, mos_cd, mos_cs;
	double tline_r0, tline_l0, tline_c0;
	double mos_min, mos_max, wire_min, wire_max, mw_area_r; // constraints

	tech_data() { // initialization defaults
		is_distributed = 1;
		tline_seg      = 1;
		mos_rdsn       = 1.0;
		mos_rdsp       = 1.0;
		mos_cg         = 1.0;
		mos_cd         = 1.0;
		mos_cs         = 1.0;
		tline_r0       = 1.0;
		tline_l0       = 1.0;
		tline_c0       = 1.0;
		mos_min        = 1.0;
		mos_max        = 10.0;
		wire_min       = 1.0;
		wire_max       = 10.0;
		mw_area_r      = 1.0; // ratio of MOS area to wire area
	}
	int is_valid() const;
};


struct RLC_node { // Maybe this should be a class???

	char flags;
	int parent, fanin;
	unsigned fo_cnt, depth;
	double V, c_gnd, rc, lc, lpath, rpath, cpath; // basic variables
	double gs, pole1, pole2, zeta, omega, tpdi, tri, tpdi0, tri0; // advanced variables
	vector<unsigned> branches;

	RLC_node() {
		parent = -1;
		fanin  = -1;
		depth  = 0;
		flags  = 0;
		fo_cnt = 0;
		V      = 0.0;
		c_gnd  = 0.0;
		rc     = 0.0;
		lc     = 0.0;
		rpath  = 0.0;
		lpath  = 0.0;
		cpath  = 0.0;
	}
	int calc_vars();
	int calc_poles();
	double calc_gs(double s);
};


struct RLC_branch {

	char flags, type; // type: 1 = fixed, 2 = NMOS, 4 = PMOS, 8 = wire
	int n1, n2, param;
	double gr, gl;

	RLC_branch(int n1=-1, int n2=-1) {
		type  = 0;
		flags = 0;
		param = -1;
		gr    = 0.0;
		gl    = 0.0;
	}
};



class RLC_tree {

public:
	// allocation/deletion
	RLC_tree();
	~RLC_tree();
	void alloc_nodes(int num_nodes) {nodes.resize(num_nodes);};
	void build_rc_lc_terms(int build_process_list);
	int set_tech_data(tech_data &data);

	// query
	int is_tree();
	int check_tree(int node, int parent);
	int valid_nodes(int n1, int n2) const;
	int find_branch(int n1, int n2) const;
	unsigned get_num_params() const {return params.size();};
	tech_data get_tech_data() const {return tdata;};

	// print
	void print(int show_branches) const;
	void print_node_info(int node) const;
	void print_params() const;
	void print_leaf_delays() const;

	int is_sizing_monotonic() const;
	int is_sizing_monotonic(vector<double> &x);
	double get_max_delay(int &critical_leaf, int type);
	double get_area(int type) const;

	// add
	int add_vsource(int nplus, int nminus, double value);
	int add_capacitor(int n1, int n2, double value, int is_fixed);
	int add_resistor(int n1, int n2, int type, int &index, double value);
	int add_inductor(int n1, int n2, double value);
	int add_mosfet(int ng, int ns, int nd, int nb, int type, double *comp_val);
	int add_tline(int n1, int n2, double length, double *comp_val);
	int add_inverter(int n1, int n2, double *comp_val);

	void set_min_widths();
	void get_min_max_params(vector<double> &pmin, vector<double> &pmax);
	int get_params(vector<double> &x) const;
	int update_params(vector<double> &x, int test_level);
	void get_gradient(vector<double> &gradient, int type, int dtype);

	int optimize_independent_params(int opt_type);
	void write_params_to_circuit();

private:
	void build_rc_lc_terms_fast();
	void prop_branch_dep(unsigned nn, unsigned nn_parent, int last_tline, int last_mos);

	// *** add support for more than one root ***
	int root, nleaves, ns, nr, nl, nc, graph_is_tree, changed, has_inductor, crit_leaf;
	double mos_area, wire_area;
	tech_data tdata;
	vector<int> parent_params;
	vector<unsigned> leaves, bfs_arr;
	vector<RLC_node> nodes;
	vector<RLC_branch> branches;
	vector<input_param> params, opt_params;
	vector<rlc_op> ops;
	vector<rlc_op_cap> opcs;
};



#endif


