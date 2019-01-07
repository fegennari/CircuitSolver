#include "RLC_Tree.h"
#include "Optimizer.h"



// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.RLC_Tree.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver RLC circuit analysis
// EE219A final project
// Began 10/25/02
// by Frank Gennari


// really should be optimizer input params
int const OPTIMIZE_TLINE  = 1;
int const OPTIMIZE_MOS    = 1;
int const OPTIMIZE_INV    = 1;



// ***** tech_data *****

int tech_data::is_valid() const {

	if (tline_seg <= 0   || mos_rdsn <= 0.0 || mos_rdsp <= 0.0 || mos_cg   <= 0.0 || mos_cd    <= 0.0 || mos_cs <= 0.0 ||
		tline_r0  <= 0.0 || tline_l0 <  0.0 || tline_c0 <= 0.0 ||
		mos_min   <= 0.0 || mos_max  <= 0.0 || wire_min <= 0.0 || wire_max <= 0.0 || mw_area_r <  0.0 ||
		mos_min > mos_max || wire_min > wire_max) {
		return 0;
	}
	if (wire_min < 1.0) {
		cout << "Warning: Min wire size should not be less than 1.0: " << wire_min << "." << endl;
	}
	else if (wire_min > 1.0) {
		cout << "It is recommended that you use a min wire size of 1.0: " << wire_min << "." << endl;
	}
	if (mos_min < 1.0) {
		cout << "Warning: Min MOS size should not be less than 1.0: " << mos_min << "." << endl;
	}
	else if (mos_min > 1.0) {
		cout << "It is recommended that you use a min MOS size of 1.0: " << mos_min << "." << endl;
	}
	if (mos_rdsn > mos_rdsp) {
		cout << "Warning: NMOS resistance of " << mos_rdsn << " is higher than PMOS resistance of " << mos_rdsp << " which may cause problems when optimizing." << endl;
	}
	return 1;
}



// ***** RLC_node *****

int RLC_node::calc_vars() {

	if (lc == 0.0) { // no inductance
		omega = 0.0; // should be infinity
		zeta  = 0.0; // should be infinity
		tpdi0 = tpdi = 0.695*rc;
		tri0  = tri  = 2.195*rc;
		return 0;
	}
	omega = 1.0/sqrt(lc);
	zeta  = 0.5*rc*omega;
	tpdi0 = 1.047*exp(-zeta/0.85) + 1.39*zeta; // 50% delay
	tri0  = 6.017*exp(-pow(zeta, 1.35)/0.4) - 5.0*exp(-pow(zeta, 1.25)/0.64) + 4.39*zeta; // rise time
	tpdi  = tpdi0/omega;
	tri   = tri0/omega;

	return 1;
}



int RLC_node::calc_poles() { // not yet used

	double term1;

	if (lc == 0.0) { // no inductance
		pole1 = -1.0/rc;
		pole2 = 0.0;
		return 0;
	}
	// s = (-b +/- sqrt(b^2 - 4ac))/2a
	term1 = sqrt(rc*rc - 4.0*lc);
	pole1 = (-rc + term1)/(2.0*lc);
	pole2 = (-rc - term1)/(2.0*lc);

	return 1;
}



double RLC_node::calc_gs(double s) { // not yet used

	gs = 1.0/(s*s*lc + s*rc + 1.0);

	return gs;
}



// ***** RLC_tree *****


RLC_tree::RLC_tree() { // constructor

	root          = -1;
	has_inductor  = 0;
	changed       = 0;
	nleaves       = 0;
	ns            = 0;
	nr            = 0;
	nl            = 0;
	nc            = 0;
	mos_area      = 0.0;
	wire_area     = 0.0;
	crit_leaf     = -1;
	graph_is_tree = -1;
}



RLC_tree::~RLC_tree() { // destructor

	vector_delete(nodes);
	vector_delete(branches);
	vector_delete(leaves);
	vector_delete(params);
	vector_delete(opt_params);
	vector_delete(ops);
	vector_delete(opcs);
	vector_delete(parent_params);
	vector_delete(bfs_arr);
}



// Elmore delay
void RLC_tree::build_rc_lc_terms(int build_process_list) { // assuming graph is a tree

	int net_added, not_prop;
	unsigned i, size, qcurr, qend, b, net1, net2, fanouts;
	unsigned *net_queue = NULL;
	double gr, gl;

	if (graph_is_tree == -1) {
		is_tree(); // if tree status is not known then determine if graph is tree
	}
	if (!graph_is_tree) {
		cout << "Cannot calculate RC and LC terms." << endl;
		return;
	}
	size = nodes.size();

	if (size == 0) {
		changed = 0;
		return;
	}
	if (!changed) {
		build_rc_lc_terms_fast();
		return;
	}
	if (build_process_list) {
		changed = 0;
		vector_delete(ops);
		vector_delete(opcs);
	}
	for (i = 0; i < branches.size(); ++i) { // reset branch flags
		branches[i].flags &= (~SEEN_FLAG);
	}

	// propoagate C from leaves to root
	net_queue = new unsigned[size];
	assert(net_queue);
	qcurr = 0;
	qend  = 0;

	for (i = 1; i < size; ++i) { // add leaf nodes to queue
		fanouts = nodes[i].branches.size();

		if (i != root) {
			if (fanouts > 0) { // otherwise floating node
				--fanouts; // one branch is actually a fanin
			}
		}
		if (fanouts == 0) {
			nodes[i].flags |= (LEAF_NODE_FLAG | USED_FLAG);
			leaves.push_back(i);
			net_queue[qend] = i;
			++qend;
		}
		else {
			nodes[i].flags &= (~USED_FLAG); // disable used flag
		}
		nodes[i].fo_cnt = fanouts;
		nodes[i].cpath  = nodes[i].c_gnd;
		nodes[i].rc     = 0.0;
		nodes[i].lc     = 0.0;
	}
	while (qcurr < qend) { // backwards BFS
		net1 = net_queue[qcurr];

		for (i = 0; i < nodes[net1].branches.size(); ++i) {
			b = nodes[net1].branches[i];

			if (!(branches[b].flags & SEEN_FLAG)) { // new branch
				branches[b].flags |= SEEN_FLAG;

				if (branches[b].n1 == net1) { // branch from net2 (parent) to net1 (child)
					net2 = branches[b].n2;
				}
				else if (branches[b].n2 == net1) {
					net2 = branches[b].n1;
				}
				else {
					assert(0);
				}
				if (nodes[net1].parent != net2) {
					assert(nodes[net1].parent < 0);
					nodes[net1].parent = net2;
					nodes[net1].fanin  = b;
				}
				if (net2 == 0 || !(nodes[net2].flags & USED_FLAG)) {
					assert(nodes[net2].fo_cnt > 0);
					--nodes[net2].fo_cnt;

					if (nodes[net2].fo_cnt == 0) { // add node to queue if this is its last fanout
						nodes[net2].flags |= USED_FLAG;
						assert(qend < size);
						net_queue[qend] = net2;
						++qend;
					}
				}
				if (branches[b].type != TYPE_PROPAGATE) {
					nodes[net2].cpath += nodes[net1].cpath;

					if (build_process_list) {
						opcs.push_back(rlc_op_cap(net1, net2));
					}
				}
			}
		}
		++qcurr;
	}

	// reset branch seen flags
	for (i = 0; i < branches.size(); ++i) {
		if (!(branches[i].flags & SEEN_FLAG)) {
			cout << "Warning: Branch " << i << ": " << branches[i].n1 << "," << branches[i].n2 << " is floating." << endl;
		}
		else {
			branches[i].flags &= (~SEEN_FLAG);
		}
	}
	for (i = 1; i < size; ++i) { // reset node flags
		nodes[i].flags &= (~USED_FLAG);
	}

	// propagate R and L from root to leaves - BFS
	qcurr              = 0;
	qend               = 1;
	net_queue[0]       = root; // start BFS at root
	nodes[root].depth  = 0;
	nodes[root].flags |= USED_FLAG;

	while (qcurr < qend) { // forwards BFS
		net1 = net_queue[qcurr];
		bfs_arr.push_back((unsigned)net1);

		for (i = 0; i < nodes[net1].branches.size(); ++i) {
			b = nodes[net1].branches[i];

			if (!(branches[b].flags & SEEN_FLAG)) {
				branches[b].flags |= SEEN_FLAG;
				not_prop = (branches[b].type != TYPE_PROPAGATE);

				if (branches[b].n1 == net1) {
					net2 = branches[b].n2;
				}
				else if (branches[b].n2 == net1) {
					net2 = branches[b].n1;
				}
				else {
					assert(0);
				}
				net_added = 0;

				if (has_inductor) {
					gr = branches[b].gr;
					gl = branches[b].gl;

					if (!(nodes[net2].flags & USED_FLAG)) {
						net_added      = 1;
						nodes[net2].rc = nodes[net1].rc;
						nodes[net2].lc = nodes[net1].lc;

						if (not_prop) {
							if (gr != 0.0) {
								nodes[net2].rc += nodes[net2].cpath/gr;
							}
							if (gl != 0.0) {
								nodes[net2].lc += nodes[net2].cpath/gl;
							}
						}
					}
					if (not_prop) {
						if (gr != 0.0) {
							nodes[net2].rpath = nodes[net1].rpath + 1.0/gr;
						}
						if (gl != 0.0) {
							nodes[net2].lpath = nodes[net1].lpath + 1.0/gl;
						}
					}
				}
				else { // shortcut if L = 0
					gr = branches[b].gr;

					if (!(nodes[net2].flags & USED_FLAG)) {
						net_added      = 1;
						nodes[net2].rc = nodes[net1].rc;

						if (not_prop && gr != 0.0) {
							nodes[net2].rc += nodes[net2].cpath/gr;
						}
					}
					if (not_prop && gr != 0.0) {
						nodes[net2].rpath = nodes[net1].rpath + 1.0/gr;
					}
				}
				if (net_added) {
					nodes[net2].flags |= USED_FLAG;
					nodes[net2].depth  = nodes[net1].depth + 1;
					assert(qend < size);
					net_queue[qend] = net2;
					++qend;
				}
				if (build_process_list && net_added || not_prop) {
					ops.push_back(rlc_op((net_added + 2*not_prop), net1, net2, b));
				}
			}
		}
		++qcurr;
	}
	delete [] net_queue;

	for (i = 0; i < branches.size(); ++i) { // reset branch flags
		branches[i].flags &= (~SEEN_FLAG);
	}
	for (i = 0; i < nodes.size(); ++i) {
		nodes[i].flags &= (~USED_FLAG); // reset node used flags
	}
}



void RLC_tree::build_rc_lc_terms_fast() {

	unsigned i, b, src, dst;
	double gr, gl;

	// initialize
	for (i = 1; i < nodes.size(); ++i) {
		nodes[i].cpath = nodes[i].c_gnd;
		nodes[i].rc    = 0.0;
		nodes[i].lc    = 0.0;
	}

	// calculate path capacitances
	for (i = 0; i < opcs.size(); ++i) {
		nodes[opcs[i].n_dst].cpath += nodes[opcs[i].n_src].cpath;
	}

	// calculate path admittances and RC delay
	if (has_inductor) {
		for (i = 0; i < ops.size(); ++i) {
			src = ops[i].n_src;
			dst = ops[i].n_dst;

			if (ops[i].op & 0x02) {
				b  = ops[i].branch;
				gr = branches[b].gr;
				gl = branches[b].gl;

				if (ops[i].op & 0x01) {
					nodes[dst].rc = nodes[src].rc;
					nodes[dst].lc = nodes[src].lc;

					if (gr != 0.0) {
						nodes[dst].rc += nodes[dst].cpath/gr;
					}
					if (gl != 0.0) {
						nodes[dst].lc += nodes[dst].cpath/gl;
					}
				}
				if (gr != 0.0) {
					nodes[dst].rpath = nodes[src].rpath + 1.0/gr;
				}
				if (gl != 0.0) {
					nodes[dst].lpath = nodes[src].lpath + 1.0/gl;
				}
			}
			else {
				nodes[dst].rc = nodes[src].rc;
				nodes[dst].lc = nodes[src].lc;
			}
		}
	}
	else { // shortcut if L = 0
		for (i = 0; i < ops.size(); ++i) {
			src = ops[i].n_src;
			dst = ops[i].n_dst;

			if (ops[i].op & 0x02) {
				gr  = branches[ops[i].branch].gr;

				if (ops[i].op & 0x01) {
					nodes[dst].rc = nodes[src].rc;

					if (gr != 0.0) {
						nodes[dst].rc += nodes[dst].cpath/gr;
					}
				}
				if (gr != 0.0) {
					nodes[dst].rpath = nodes[src].rpath + 1.0/gr;
				}
			}
			else {
				nodes[dst].rc = nodes[src].rc;
			}
		}
	}
}



int RLC_tree::set_tech_data(tech_data &data) {

	if (!data.is_valid()) {
		return 0;
	}
	tdata = data;

	return 1;
}



int RLC_tree::is_tree() { // could remember result of last call if graph unchanged

	int result;
	unsigned i, count(0);

	if (ns != 1 || root < 0 || root > (int)nodes.size()) {
		graph_is_tree = 0;
		return 0;
	}
	for (i = 0; i < nodes.size(); ++i) {
		if (i != 0 && nodes[i].branches.size() == 0) {
			cout << "Warning: Node " << i << " is floating." << endl;
		}
		nodes[i].flags &= (~SEEN_FLAG);
	}
	result = check_tree(root, -1);

	for (i = 1; i < nodes.size(); ++i) { // skip ground
		if (!(nodes[i].flags & SEEN_FLAG)) {
			++count;
		}
	}
	if (count > 0) {
		cout << "There are " << count << " unreachable nodes in this circuit." << endl;
	}
	graph_is_tree = result;

	for (i = 0; i < nodes.size(); ++i) {
		nodes[i].flags &= (~SEEN_FLAG);
	}
	return result;
}



int RLC_tree::check_tree(int node, int parent) { // recusive DFS

	unsigned i, node2, n1, n2, branch;

	assert(node >= 0 && node < (int)nodes.size());

	if (nodes[node].flags & SEEN_FLAG) { // already seen - cycle or reconvergent fanout
		return 0;
	}
	nodes[node].flags |= SEEN_FLAG; // mark as seen

	for (i = 0; i < nodes[node].branches.size(); ++i) {
		branch = nodes[node].branches[i];
		n1     = branches[branch].n1;
		n2     = branches[branch].n2;

		if (n1 == node) {
			node2 = n2;
		}
		else if (n2 == node) {
			node2 = n1;
		}
		else {
			assert(0);
		}
		if (node2 != 0 && node2 != parent && !check_tree(node2, node)) {
			return 0;
		}
	}
	return 1;
}



int RLC_tree::valid_nodes(int n1, int n2) const {

	return (n1 != n2 && n1 >= 0 && n2 >= 0 && n1 < (int)nodes.size() && n2 < (int)nodes.size());
}



int RLC_tree::find_branch(int n1, int n2) const {

	unsigned i, b;

	assert(valid_nodes(n1, n2));

	if (nodes[n1].branches.size() == 0 || nodes[n2].branches.size() == 0) {
		return -1;
	}
	if (n2 < n1) {
		swap(n1, n2);
	}
	for (i = 0; i < nodes[n1].branches.size(); ++i) {
		b = nodes[n1].branches[i];

		if (branches[b].n2 == n2) {
			return b;
		}
	}
	return -1;
}



void RLC_tree::print(int show_branches) const {

	int i, j, b;

	cout << endl;

	for (i = 0; i < (int)nodes.size(); ++i) {
		cout << "N " << i << ": f = " << (int)nodes[i].flags << ", V = " << nodes[i].V << ", C = " << nodes[i].c_gnd << endl;
		cout << "  rp = " << nodes[i].rpath << ", lp = " << nodes[i].lpath << ", cp = " << nodes[i].cpath << ", rc = " << nodes[i].rc << ", lc = " << nodes[i].lc << endl;
		cout << "  parent = " << nodes[i].parent << ", fanin_b = " << nodes[i].fanin << ", depth = " << nodes[i].depth << endl;

		if (show_branches) {
			for (j = 0; j < (int)nodes[i].branches.size(); ++j) {
				b = nodes[i].branches[j];
				cout << "    B " << branches[b].n1 << ", " << branches[b].n2 << ", f = " << (int)branches[b].flags << ", 1/R = " << branches[b].gr << ", 1/L = " << branches[b].gl << endl;
			}
		}
	}
}



void RLC_tree::print_node_info(int node) const {

	if (node < 0 || node >= (int)nodes.size()) {
		return;
	}
	if (node == 0) {
		cout << "Ground" << endl;
		return;
	}
	cout << "Node " << node << ": f = " << (int)nodes[node].flags << ", V = " << nodes[node].V << ", C = " << nodes[node].c_gnd << endl;
	cout << "  rp = " << nodes[node].rpath << ", lp = " << nodes[node].lpath << ", cp = " << nodes[node].cpath << ", rc = " << nodes[node].rc << ", lc = " << nodes[node].lc << endl;
	cout << "  parent = " << nodes[node].parent << ", fanin_b = " << nodes[node].fanin << ", depth = " << nodes[node].depth << endl;
}



void RLC_tree::print_params() const {

	unsigned i;

	for (i = 0; i < params.size(); ++i) {
		cout << "params[" << i << "]: ns1 = " << params[i].ns1 << ", nd2 = " << params[i].nd2 << ", val = " << params[i].val << endl;
	}
}



void RLC_tree::print_leaf_delays() const {

	unsigned i;

	cout << "leaf delays: ";
	for (i = 0; i < leaves.size(); ++i) {
		cout << nodes[leaves[i]].rc << " ";
	}
	cout << endl;
}



// get width params from circuit structure
int RLC_tree::is_sizing_monotonic() const {

	unsigned i, j, b;
	int fanin;
	double gr;
	char type;

	for (i = 0; i < nodes.size(); ++i) {
		if (i == root || nodes[i].branches.size() < 2) {
			continue; // skip root and nodes without inputs and outputs
		}
		fanin = nodes[i].fanin; // fanin branch

		if (fanin < 0) {
			continue;
		}
		type = branches[fanin].type;

		if (type != 2 && type != 4 && type != 8) {
			continue; // types not supported
		}
		gr = branches[fanin].gr;

		for (j = 0; j < nodes[i].branches.size(); ++j) {
			b = nodes[i].branches[j];

			if (b == nodes[i].fanin) {
				continue; // skip fanin node - want fanouts only
			}
			if (branches[b].type != type) {
				continue; // don't compare branches of different type
			}
			if ((branches[b].gr - gr)/gr > 1.0E-6) {
				return 0; // fanout has lower impedance
			}
		}
	}
	return 1;
}


// given width params (before written to circuit)
int RLC_tree::is_sizing_monotonic(vector<double> &x) {

	unsigned i, size, ppi;

	size = params.size();

	if (x.size() != size) {
		cerr << "Error: Paramater input vector is of incorrect size (" << x.size() << " vs. " << size << ") (ism)." << endl;
		return 0;
	}
	if (parent_params.size() != size) { // luckily only done once
		parent_params.resize(size);

		for (i = 0; i < size; ++i) {
			parent_params[i] = -1;
			branches[params[i].branch].param = i;
		}
		prop_branch_dep((unsigned)root, 0, -1, -1, -1); // determine branch dependencies for width params
	}
	for (i = 0; i < size; ++i) {
		assert(parent_params.size() == size);
		ppi = (unsigned)parent_params[i];

		if (ppi >= 0 && ppi < size && x[i] > x[ppi]) {
			return 0; // size is larger than parent - non-monotonic/decreasing size
		}
	}
	return 1;
}


// used in is_sizing_monotonic()
void RLC_tree::prop_branch_dep(unsigned nn, unsigned nn_parent, int last_tline, int last_mos, int last_inv) { // recursive

	unsigned i, branch, nn2;
	int param;

	// *** doesn't work for parallel branches with params ***
	for (i = 0; i < nodes[nn].branches.size(); ++i) {
		branch = nodes[nn].branches[i];
		param  = branches[branch].param;

		if (param < 0) {
			continue;
		}
		if (branches[branch].n1 == nn) {
			nn2 = branches[branch].n2;
		}
		else if (branches[branch].n2 == nn) {
			nn2 = branches[branch].n1;
		}
		else {
			assert(0);
		}
		if (nn2 == 0) {
			continue;
		}
		assert(nn != nn2); // shorted branch

		if (nn2 == nn_parent) {
			continue;
		}
		if (TYPE_IS_TLINE(param)) { // tline
			parent_params[param] = last_tline;
			prop_branch_dep(nn2, nn, param, last_mos, last_inv);
		}
		else if (TYPE_IS_INV(param)) { // inverter
			parent_params[param] = last_inv;
			prop_branch_dep(nn2, nn, last_tline, last_mos, param);
		}
		else { // mos
			parent_params[param] = last_mos;
			prop_branch_dep(nn2, nn, last_tline, param, last_inv);
		}
	}
}



// type: 0 = Elmore RC, 1 = RLC tdpi (propagation delay), 2 = RLC tri (rise time)
double RLC_tree::get_max_delay(int &critical_leaf, int type) {

	int i;
	unsigned leaf;
	double max_delay(0.0);

	critical_leaf = -1;

	for (i = 0; i < (int)leaves.size(); ++i) {
		leaf = leaves[i];

		switch (type) {
		case TYPE_RC_DELAY:
			if (nodes[leaf].rc > max_delay) {
				max_delay     = nodes[leaf].rc; // rc delay
				critical_leaf = leaf;
			}
			break;

		case TYPE_TPDI:
			nodes[leaf].calc_vars();

			if (nodes[leaf].tpdi > max_delay) {
				max_delay     = nodes[leaf].tpdi; // propagation delay
				critical_leaf = leaf;
			}
			break;

		case TYPE_TRI:
			nodes[leaf].calc_vars();

			if (nodes[leaf].tri > max_delay) {
				max_delay     = nodes[leaf].tri; // rise time
				critical_leaf = leaf;
			}
			break;

		default:
			cerr << "Illegal case type in get_max_delay(): " << type << "." << endl;
			assert(0);
		}
	}
	crit_leaf = critical_leaf;

	return max_delay;
}



// 1 = MOS, 2 = wire, 3 = MOS + wire
double RLC_tree::get_area(int type) const {

	unsigned i;
	double area = 0.0;

	if (type == NO_AREA) {
		return 0.0; // assert?
	}
	if (type <= MOS_PLUS_WIRE_AREA) { // dynamic area
		for (i = 0; i < params.size(); ++i) {
			switch (params[i].type) {
			case TYPE_TLINE:
				if (type & 0x01) {
					area += params[i].length*params[i].val;
				}
				break;
			case TYPE_NMOS:
			case TYPE_PMOS:
				if (type & 0x02) { 
					area += tdata.mw_area_r*params[i].val;
				}
				break;
			case TYPE_INV:
				if (type & 0x02) {
					area += tdata.mw_area_r*params[i].val*(1.0 + tdata.mos_rdsp/tdata.mos_rdsn);
				}
				break;
			default:
				cerr << "Illegal type in RLC tree get_area(): " << params[i].type << "." << endl;
				return 0.0;
			}
		}
		return area;
	}
	type -= 3;

	switch (type) { // initial area
	case MOS_AREA: // MOS
		return tdata.mw_area_r*mos_area;
	case WIRE_AREA: // wire
		return wire_area;
	case MOS_PLUS_WIRE_AREA: // MOS + wire
		return tdata.mw_area_r*mos_area + wire_area;
	default:
		cerr << "Illegal case type in get_area(): " << (type + 3) << "." << endl;
		assert(0);
	}
	return 0.0;
}



int RLC_tree::add_vsource(int nplus, int nminus, double value) {

	unsigned b;

	assert(valid_nodes(nplus, nminus));

	if (value == 0.0) {
		cerr << "Error: Cannot have zero volt source." << endl;
		return 0;
	}
	if (nplus < nminus) {
		swap(nplus, nminus);
		value = -value;
	}
	if (nminus != 0) { // voltage sources must go to ground
		cout << "n- = " << nminus << ", n+ = " << nplus << endl;
		cerr << "Error: Voltage sources between internal nodes are not yet supported." << endl;
		return 0;
	}
	if (nminus == 0) {
		if (nodes[nplus].V == value) { // duplicate source with same value
			return 0;
		}
		if (nodes[nplus].V != 0.0) {
			cerr << "Error: Multiple voltage sources driving the same node." << endl;
			return 0;
		}
		nodes[nplus].V = value;

		if (ns == 0) {
			root = nplus;
		}
	}
	else { // create new source branch
		RLC_branch branch;
		branch.n1 = nminus;
		branch.n2 = nplus;
		b         = branches.size();
		branches.push_back(branch);
		nodes[nminus].branches.push_back(b);
		nodes[nplus].branches.push_back(b);
		changed       = 1;
		graph_is_tree = -1;
	}
	++ns;

	return 1;
}



int RLC_tree::add_capacitor(int n1, int n2, double value, int is_fixed) {

	assert(valid_nodes(n1, n2));

	if (value == 0.0) { // no capacitor - OK
		return 0;
	}
	else if (value < 0.0) {
		cout << "Warning: Negative valued capacitor." << endl;
	}
	if (n2 < n1) {
		swap(n1, n2);
	}
	if (n1 != 0) {
		cerr << "Error: Currently, only grounded capacitors are supported." << endl;
		return 0;
	}
	nodes[n2].c_gnd += value;
	++nc;

	if (is_fixed) {
		nodes[n2].flags |= NODE_CAP_FLAG;
	}
	return 1;
}



int RLC_tree::add_resistor(int n1, int n2, int type, int &index, double value) {

	assert(valid_nodes(n1, n2));

	if (type != TYPE_PROPAGATE && value == 0.0) {
		cout << "Error: Zero value resistor is illegal." << endl;
		return 0;
	}
	else if (value < 0.0) {
		cout << "Warning: Negative valued resistor." << endl;
	}
	if (n2 < n1) {
		swap(n1, n2);
	}
	index = find_branch(n1, n2);

	if (index < 0) { // create new resistor/inductor branch
		RLC_branch branch;
		branch.type = type;
		branch.n1   = n1;
		branch.n2   = n2;
		index       = branches.size();

		if (type != TYPE_PROPAGATE) {
			branch.gr = 1.0/value;
		}
		branches.push_back(branch);
		nodes[n1].branches.push_back(index);
		nodes[n2].branches.push_back(index);
		changed       = 1;
		graph_is_tree = -1;
	}
	else {
		if (type != TYPE_RESISTOR || branches[index].type != TYPE_RESISTOR) {
			if (branches[index].type != type) {
				cout << "Warning: Parallel resistive connections of multiple types are not supported." << endl;
				branches[index].type |= type;
			}
			cout << "Warning: Parallel components cannot be optimized correctly." << endl;
		}
		branches[index].gr += 1.0/value;
	}
	++nr;

	return 1;
}



int RLC_tree::add_inductor(int n1, int n2, double value) {

	int index;
	unsigned b;

	assert(valid_nodes(n1, n2));

	if (value == 0.0) {
		cout << "Error: Zero value inductor is illegal." << endl;
		return 0;
	}
	else if (value < 0.0) {
		cout << "Warning: Negative valued inductor." << endl;
	}
	if (n2 < n1) {
		swap(n1, n2);
	}
	index = find_branch(n1, n2);

	if (index < 0) { // create new resistor/inductor branch
		RLC_branch branch;
		branch.n1  = n1;
		branch.n2  = n2;
		branch.gl += 1.0/value;
		b          = branches.size();
		branches.push_back(branch);
		nodes[n1].branches.push_back(b);
		nodes[n2].branches.push_back(b);
		changed       = 1;
		graph_is_tree = -1;
	}
	else {
		branches[index].gl += 1.0/value;
	}
	++nl;
	has_inductor = 1;

	return 1;
}



int RLC_tree::add_mosfet(int ng, int ns, int nd, int nb, int type, double *comp_val) {

	int index;
	double value;

	assert(comp_val != NULL);
	value = *comp_val;
	assert(value > 0); // size > 0

	if (value < tdata.mos_min || value > tdata.mos_max) {
		cout << "Warning: MOS size of " << value << " is not within the range [" << tdata.mos_min << "," << tdata.mos_max << "]." << endl;
	}
	if (type == 0) { // NMOS
		if (!add_resistor(ns, nd, TYPE_NMOS, index, tdata.mos_rdsn/value)) return 0;
	}
	else { // PMOS
		if (!add_resistor(ns, nd, TYPE_PMOS, index, tdata.mos_rdsp/value)) return 0;
	}
	if (tdata.mos_cg > 0 &&             !add_capacitor(ng, nb, tdata.mos_cg*value, 1)) return 0;
	if (tdata.mos_cd > 0 && nd != nb && !add_capacitor(nd, nb, tdata.mos_cd*value, 0)) return 0;
	if (tdata.mos_cs > 0 && ns != nb && !add_capacitor(ns, nb, tdata.mos_cs*value, 0)) return 0;

	//if (!add_resistor(ng, ns, TYPE_PROPAGATE, index, 0.0)) return 0; // might need this

	if (OPTIMIZE_MOS) {
		input_param param = {char(2*(type+1)), (unsigned)index, (unsigned)ns, (unsigned)nd, (unsigned)ng, comp_val, value, value, 1.0};
		params.push_back(param);
	}
	mos_area += value;
	
	return 1;
}



int RLC_tree::add_tline(int n1, int n2, double length, double *comp_val) {

	int index;
	double value, rval, lval, cval;

	assert(comp_val != NULL);
	value = *comp_val;
	assert(value > 0); // size > 0
	assert(length > 0);

	if (value < tdata.wire_min || value > tdata.wire_max) {
		cout << "Warning: Transmission line (wire) size of " << value << " is not within the range [" << tdata.wire_min << "," << tdata.wire_max << "]." << endl;
	}
	rval = length*tdata.tline_r0/value;
	lval = length*tdata.tline_l0/value;
	cval = length*tdata.tline_c0*value;

	if (tdata.tline_seg > 1) { // multi-segment model
		unsigned i, n11, n22;
		double rseg, cseg, lseg;
		RLC_node node;

		n11  = n1;
		n22  = nodes.size() - 1;
		rseg = rval/tdata.tline_seg; // resistance per segment
		lseg = lval/tdata.tline_seg;
		cseg = cval/tdata.tline_seg;

		for (i = 0; i < (unsigned)tdata.tline_seg; ++i) {
			if (i < (unsigned)tdata.tline_seg-1) {
				nodes.push_back(node); // add node except for last segment
				++n22; // current node index
			}
			else {
				n22 = n2;
			}
			if (rval > 0.0 && !add_resistor(n11, n22, TYPE_TLINE, index, rseg)) return 0;
			if (lval > 0.0 && !add_inductor(n11, n22, lseg))                    return 0;
			if (i > 0 && cval > 0.0 && !add_capacitor(n11, 0, cseg, 0))         return 0;
			n11 = n22;
		}
		cval = cseg;

		if (OPTIMIZE_TLINE) {
			cout << "Cannot optimize multi-segment transmission line." << endl;
		}
	}
	else {
		if (rval > 0.0 && !add_resistor(n1, n2, TYPE_TLINE, index, rval)) return 0;
		if (lval > 0.0 && !add_inductor(n1, n2, lval))                    return 0;

		if (OPTIMIZE_TLINE) {
			input_param param = {8, (unsigned)index, (unsigned)n1, (unsigned)n2, (unsigned)0, comp_val, value, value, length};
			params.push_back(param);
		}
	}
	if (tdata.is_distributed) {
		if (cval > 0.0 && !add_capacitor(n1, 0, 0.5*cval, 0)) return 0; // end capacitors
		if (cval > 0.0 && !add_capacitor(n2, 0, 0.5*cval, 0)) return 0;
	}
	else { // lumped
		if (cval > 0.0 && !add_capacitor(n2, 0, cval, 0)) return 0; // end capacitor
	}
	wire_area += value*length;

	return 1;
}



int RLC_tree::add_inverter(int n1, int n2, double *comp_val) {

	int index;
	unsigned nnode;
	double value, value2, value3, pn_ratio;
	RLC_node node;

	assert(comp_val != NULL);
	value = *comp_val;
	assert(value > 0); // size > 0

	pn_ratio = tdata.mos_rdsp/tdata.mos_rdsn;
	value2   = value*pn_ratio; // PMOS width
	value3   = value*(1.0 + pn_ratio);

	if (value < tdata.mos_min || value > tdata.mos_max || value2 < tdata.mos_min || value2 > tdata.mos_max) {
		cout << "Warning: Inverter size of " << value << " is does not contain transistors within the range [" << tdata.mos_min << "," << tdata.mos_max << "]." << endl;
	}
	nnode = nodes.size();
	nodes.push_back(node);

	if (!add_resistor(n1, nnode, TYPE_PROPAGATE, index, 0.0))              return 0; // propagation resistor
	if (!add_resistor(nnode, n2, TYPE_INV, index, tdata.mos_rdsn/value))   return 0;

	if (tdata.mos_cg > 0 && !add_capacitor(n1, 0, tdata.mos_cg*value3, 1)) return 0;
	if (tdata.mos_cd > 0 && !add_capacitor(n2, 0, tdata.mos_cd*value3, 0)) return 0;

	if (OPTIMIZE_INV) {
		input_param param = {TYPE_INV, (unsigned)index, (unsigned)n1, (unsigned)n2, (unsigned)nnode, comp_val, value, value, 1.0};
		params.push_back(param);
	}
	mos_area += value3;
	
	return 1;
}



void RLC_tree::get_min_max_params(vector<double> &pmin, vector<double> &pmax) {

	unsigned i, size;

	size = params.size();
	pmin.resize(size);
	pmax.resize(size);

	for (i = 0; i < size; ++i) {
		if (TYPE_IS_TLINE(i)) { // tline
			pmin[i] = tdata.wire_min;
			pmax[i] = tdata.wire_max;
		}
		else if (TYPE_IS_INV(i)) { // inverter
			pmin[i] = tdata.mos_min;
			pmax[i] = (tdata.mos_rdsn/tdata.mos_rdsp)*tdata.mos_max;
		}
		else { // MOS
			pmin[i] = tdata.mos_min;
			pmax[i] = tdata.mos_max;
		}
	}
}



void RLC_tree::set_min_widths() {

	unsigned i;
	vector<double> x(params.size());

	for (i = 0; i < params.size(); ++i) {
		if (TYPE_IS_TLINE(i)) { // tline
			x[i] = tdata.wire_min;
		}
		else if (TYPE_IS_INV(i)) { // inverter
			x[i] = tdata.mos_min;
		}
		else { // MOS
			x[i] = tdata.mos_min;
		}
	}
	update_params(x, 0);
}



int RLC_tree::get_params(vector<double> &x) const {

	unsigned i, size;

	size = params.size();

	if (size == 0) {
		return 0;
	}
	x.resize(size);

	for (i = 0; i < size; ++i) {
		x[i] = params[i].val;
	}
	return 1;
}


// test_level: 0 = no feasibility test, 1 = check valid widths, 2 = check valid widths + check monotonic
int RLC_tree::update_params(vector<double> &x, int test_level) {

	int changed(0);
	unsigned i, size, b, ns1, nd2, ng;
	double val, diff, length, cap, inv_scale;

	size = params.size();

	if (x.size() != size) {
		cerr << "Error: Paramater input vector is of incorrect size (" << x.size() << " vs. " << size << ") (up)." << endl;
		return 0;
	}
	if (test_level > 0) { // check for valid widths (constraints)
		for (i = 0; i < size; ++i) {
			if (TYPE_IS_TLINE(i)) { // tline
				if (x[i] < tdata.wire_min || x[i] > tdata.wire_max) {
					cerr << "Illegal tline width: " << x[i] << endl;
					return 0;
				}
			}
			else if (TYPE_IS_INV(i)) { // inverter
				if (x[i] < tdata.mos_min || x[i] > (tdata.mos_rdsn/tdata.mos_rdsp)*tdata.mos_max) {
					cerr << "Illegal Inverter width: " << x[i] << endl;
					return 0;
				}
			}
			else { // MOS
				if (x[i] < tdata.mos_min || x[i] > tdata.mos_max) {
					cerr << "Illegal MOS width: " << x[i] << endl;
					return 0;
				}
			}
		}
	}
	if (test_level > 1) { // check for monotonic widths
		if (!is_sizing_monotonic(x)) {
			return 0;
		}
	}
	inv_scale = 1.0 + tdata.mos_rdsp/tdata.mos_rdsn;

	for (i = 0; i < size; ++i) {
		val = params[i].val;

		if (val != x[i]) {
			changed = 1;
			b       = params[i].branch;
			ns1     = params[i].ns1;
			nd2     = params[i].nd2;
			diff    = x[i] - val; // calculate the width difference

			// add 1/R, 1/L, C diff so that parallel combination is equal to correct value
			if (TYPE_IS_TLINE(i)) { // tline
				length = params[i].length;
				assert(length > 0.0);

				if (tdata.tline_r0 > 0.0) {
					branches[b].gr += diff/(length*tdata.tline_r0);
				}
				if (tdata.tline_l0 > 0.0) {
					branches[b].gl += diff/(length*tdata.tline_l0);
				}
				if (tdata.is_distributed) {
					cap = 0.5*length*tdata.tline_c0*diff;
					nodes[ns1].c_gnd += cap;
					nodes[nd2].c_gnd += cap;
				}
				else { // lumped
					nodes[nd2].c_gnd += length*tdata.tline_c0*diff;
				}
			}
			else if (TYPE_IS_INV(i)) { // inverter
				nodes[ns1].c_gnd += tdata.mos_cg*diff*inv_scale;
				nodes[nd2].c_gnd += tdata.mos_cd*diff*inv_scale;
				branches[b].gr   += diff/tdata.mos_rdsn;
			}
			else { // transistor
				ng = params[i].ng;
				nodes[ng].c_gnd  += tdata.mos_cg*diff;
				nodes[ns1].c_gnd += tdata.mos_cs*diff;
				nodes[nd2].c_gnd += tdata.mos_cd*diff;

				if (TYPE_IS_NMOS(i)) { // NMOS
					branches[b].gr += diff/tdata.mos_rdsn;
				}
				else { // PMOS
					branches[b].gr += diff/tdata.mos_rdsp;
				}
			}
			params[i].old_val = val; // store last value for future comparison
			params[i].val     = x[i];
		}
	}
	return changed + 1;
}



void RLC_tree::get_gradient(vector<double> &gradient, int type, int dtype) {

	int swap_n = 0, calc_l = 0;
	unsigned i, n1, n2;
	double dRC, dLC, pval, R, Rp, L, Lp, C1, C2, Cp;
	double RC, LC, omega, zeta, d_omega, d_zeta, expz, dzetap, inv_scale;

	gradient.resize(params.size());
	inv_scale = 1.0 + tdata.mos_rdsp/tdata.mos_rdsn;

	if (type == TYPE_AREA) {
		for (i = 0; i < params.size(); ++i) {
			if (TYPE_IS_TLINE(i)) { // tline
				gradient[i] = params[i].length;
			}
			else if (TYPE_IS_INV(i)) { // inverter
				gradient[i] = tdata.mw_area_r*inv_scale;
			}
			else { // mos
				gradient[i] = tdata.mw_area_r;
			}
		}
		return;
	}
	if (type != TYPE_DELAY) {
		cerr << "Illegal case type in get_gradient(): " << type << "." << endl;
		assert(0);
	}
	vector<bool>   b_path(branches.size(), false);
	vector<bool>   processed(nodes.size(), false);
	vector<double> r_tree(nodes.size(), 0.0), l_tree;

	if (has_inductor && dtype != 0) {
		l_tree.resize(nodes.size());
		calc_l = 1;
	}
	if (crit_leaf >= 0) { // calculate path to crit_leaf
		assert(crit_leaf < (int)nodes.size());
		i = crit_leaf;
		processed[root] = true;

		while (i >= 0 && i != root) { // set tree resistances along path from crit_leaf->root
			r_tree[i]    = nodes[i].rpath; // tree->path resistance
			processed[i] = true;

			if (calc_l) {
				l_tree[i] = nodes[i].lpath;
			}
			if (nodes[i].fanin >= 0) {
				b_path[nodes[i].fanin] = true;
			}
			i = nodes[i].parent;
		}
		assert(bfs_arr.size() > 0 && bfs_arr[0] == root);

		for (i = 1; i < bfs_arr.size(); ++i) { // skip root
			n1 = bfs_arr[i];

			if (processed[n1] == false && branches[nodes[n1].fanin].type != TYPE_PROPAGATE) {
				assert(nodes[n1].parent >= 0);
				r_tree[n1] = r_tree[nodes[n1].parent]; // get resistance from parent node
			
				if (calc_l) {
					assert(nodes[n1].parent >= 0);
					l_tree[n1] = l_tree[nodes[n1].parent]; // get inductance from parent node
				}
			}
		}
	}
	else {
		assert(0); // might change this later
	}
	for (i = 0; i < params.size(); ++i) {
		dRC  = 0.0;
		dLC  = 0.0;
		n1   = params[i].ns1;
		n2   = params[i].nd2;
		pval = params[i].val;

		if (nodes[n1].depth > nodes[n2].depth) { // branch flows from n2->n1
			swap_n = 1;
			swap(n1, n2); // make branch flow from n1->n2
		}
		if (TYPE_IS_TLINE(i)) { // tline
			R  = tdata.tline_r0*params[i].length;
			C1 = tdata.tline_c0*params[i].length;
			C2 = C1;

			if (calc_l) {
				L = tdata.tline_l0*params[i].length;
			}
			if (tdata.is_distributed) {
				C1 *= 0.5; // only half the cap on output node
			}
		}
		else if (TYPE_IS_INV(i)) { // inverter
			R  = tdata.mos_rdsn;
			C1 = tdata.mos_cd*inv_scale;
			C2 = C1;
			L  = 0.0;

			// add effects of cg
			Rp = r_tree[params[i].ng]; // rpath at node where root->crit_leaf path meets root->gate of param[i] path

			// add gate cap loading effect: (d/dw)(RpCiw) = RpCi
			dRC += Rp*tdata.mos_cg*inv_scale;
		}
		else { // mos
			if (TYPE_IS_NMOS(i)) {
				R = tdata.mos_rdsn;
			}
			else {
				R = tdata.mos_rdsp;
			}
			if (swap_n) {
				C1 = tdata.mos_cs; // outgoing cap
				C2 = C1 + tdata.mos_cd; // total s-d cap
			}
			else {
				C1 = tdata.mos_cd; // outgoing cap
				C2 = C1 + tdata.mos_cs; // total s-d cap
			}
			L = 0.0;

			// add effects of cg
			Rp = r_tree[params[i].ng]; // rpath at node where root->crit_leaf path meets root->gate of param[i] path

			// add gate cap loading effect: (d/dw)(RpCiw) = RpCi
			dRC += Rp*tdata.mos_cg;
		}

		// add upstream R terms
		Rp = r_tree[n1]; // rpath at node where root->crit_leaf path meets root->n1 path

		// add upstream R terms for common path with critical branch: (d/dw)(RpCiw) = RpCi
		dRC += Rp*C2;

		if (calc_l) {
			// add upstream L terms
			Lp = l_tree[n1]; // lpath at node where root->crit_leaf path meets root->n1 path

			// add upstream L terms for common path with critical branch: (d/dw)(LpCiw) = LpCi
			dLC += Lp*C2;
		}
		if (b_path[params[i].branch] == true) {
			// subtract self cap from path cap
			Cp = nodes[n2].cpath - C1*pval;

			// subtract downstream C terms: (d/dwi)(RiCp/wi) = -RiCp/wi^2
			dRC -= R*Cp/(pval*pval);

			if (calc_l && L != 0.0) {
				// subtract downstream C terms: (d/dwi)(LiCp/wi) = -LiCp/wi^2
				dLC -= L*Cp/(pval*pval);
			}
		}
		else {
			Cp = 0.0;
		}
		if (calc_l) {
			nodes[n2].calc_vars();
			RC      = nodes[n2].rc;
			LC      = nodes[n2].lc;
			omega   = nodes[n2].omega;
			zeta    = nodes[n2].zeta;
			d_omega = -0.5*dLC/(LC*sqrt(LC)); // chain rule
			d_zeta  = 0.5*(RC*d_omega + dRC*omega); // multiplication rule
		}
		switch (dtype) {
		case TYPE_RC_DELAY: // RC delay
			gradient[i] = dRC;
			break;
		case TYPE_TPDI: // propagation delay
			gradient[i] = 0.695*dRC; // function @ LC = 0

			if (!has_inductor) {
				break;
			}
			expz = exp(-zeta/0.85); // gradient of tpdi
			gradient[i] += 1.047*(-expz*d_omega/(omega*omega) - zeta*expz*d_zeta/(0.85*omega));
			break;
		case TYPE_TRI: // rise time
			gradient[i] = 2.195*dRC; // function @ LC = 0

			if (!has_inductor) {
				break;
			}
			dzetap = 1.35*pow(zeta, 0.35)*d_zeta; // gradient of tri
			expz   = exp(-pow(zeta, 1.35)/0.4);
			gradient[i] += 6.017*(-expz*d_omega/(omega*omega) - zeta*expz*dzetap/(0.4*omega));
			
			dzetap = 1.25*pow(zeta, 0.25)*d_zeta;
			expz   = exp(-pow(zeta, 1.25)/0.64);
			gradient[i] += 5.0*(-expz*d_omega/(omega*omega) - zeta*expz*dzetap/(0.64*omega));
			break;
		default:
			cerr << "Illegal case type in RLC_Tree get_gradient(): " << dtype << "." << endl;
			assert(0);
		}
	}
}



// if component connected to root, make width max for lowest resistance
// if component connected to floating leaf, make width min for lowest capacitance
int RLC_tree::optimize_independent_params(int opt_type) {

	int this_width_set, width_set = 0, mos_mm = 0, wire_mm = 0;
	unsigned i, j, n1, n2, size;
	double temp;
	vector<double> x;
	vector<input_param> params2;

	size = params.size();

	if (size == 0) {
		return 2;
	}
	assert(parent_params.size() == 0);

	if (opt_type == TYPE_AREA) { // minimize area - all min widths
		set_min_widths();
		return 1;
	}
	if (tdata.mos_min == tdata.mos_max) {
		mos_mm = 1; // mos width is fixed
	}
	if (tdata.wire_min == tdata.wire_max) {
		wire_mm = 1; // wire width is fixed
	}
	x.resize(size);

	// find independent params
	for (i = 0; i < size; ++i) {
		this_width_set = 0;
		n1 = params[i].ns1;
		n2 = params[i].nd2;

		if (wire_mm && TYPE_IS_TLINE(i)) {
			x[i] = tdata.wire_min; // size is fixed
			this_width_set = 1;
		}
		else if (mos_mm && !TYPE_IS_TLINE(i)) {
			x[i] = tdata.mos_min; // size is fixed
			this_width_set = 1;
		}
		else if ((((nodes[n1].flags & LEAF_NODE_FLAG) != 0) && ((nodes[n1].flags & NODE_CAP_FLAG) == 0)) || (((nodes[n2].flags & LEAF_NODE_FLAG) != 0) && ((nodes[n2].flags & NODE_CAP_FLAG) == 0))) { // end node with no cap - make small
			if (TYPE_IS_TLINE(i)) { // tline
				x[i] = tdata.wire_min; // set to min width for min capacitance
			}
			else { // mos or inverter
				x[i] = tdata.mos_min;
			}
			this_width_set = 1;
		}
		else if (opt_type == TYPE_DELAY) {
			if (nodes[n1].rpath == 0.0 || nodes[n2].rpath == 0.0 || nodes[n1].V != 0.0 || nodes[n2].V != 0.0) { // min resistance for root
				if (TYPE_IS_TLINE(i)) { // tline
					x[i] = tdata.wire_max; // set to max width for min resistance
					this_width_set = 1;
				}
				else if (TYPE_IS_INV(i)) { // inverter
					x[i] = (tdata.mos_rdsn/tdata.mos_rdsp)*tdata.mos_max;
					this_width_set = 1;
				}
				else if (nodes[params[i].ng].rpath == 0.0) { // mos, no problem with load capacitance
					x[i] = tdata.mos_max;
					this_width_set = 1;
				}
			}
		}
		if (this_width_set) {
			if (opt_params.size() == 0) { // first one
				for (j = 0; j < i; ++j) { // add other params to new param list
					params2.push_back(params[j]);
					x[j] = params[j].val;
				}
			}
			temp = params[i].val;
			params[i].val = x[i];
			opt_params.push_back(params[i]);
			params[i].val = temp;
			width_set = 1;
		}
		else if (width_set) {
			params2.push_back(params[i]);
			x[i] = params[i].val;
		}
	}
	if (!width_set) {
		return 0;
	}
	update_params(x, 0); // update values with optimized independent params
	vector_delete(params);
	params = params2;

	if (params2.size() == 0) { // nothing left to optimize
		return 2;
	}
	return 1;
}



void RLC_tree::write_params_to_circuit() {

	unsigned i;

	for (i = 0; i < params.size(); ++i) { // variable params
		*(params[i].comp_val) = params[i].val;
	}
	for (i = 0; i < opt_params.size(); ++i) { // already optimized params
		*(opt_params[i].comp_val) = opt_params[i].val;
	}
}



