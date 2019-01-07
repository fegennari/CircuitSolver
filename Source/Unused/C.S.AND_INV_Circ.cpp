#include "CircuitSolver.h"
// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.AND_INV_Circ.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.

// Circuit Solver AND/INVERTER Circuit Definitions (EE219B)
// by Frank Gennari
#define NONEXISTANT -1
#define ConstID0     0
#define ConstID1     1
#define baseID       2


struct AND_V {

	int id;
	AND_V *left, *right, *next;
};


struct avEdge {

	bool inv_bit;
	int table_entry, id;
	AND_V *from, *to;
};

int const table_size = 101;


avEdge Const0, Const1;
avEdge hash_table[table_size] = {NULL};


AND_V  *initGraph();
avEdge *opInit(AND_V *initVertex);
avEdge *createAND_V(avEdge *p1, avEdge *p2);
avEdge *opNOT(avEdge *p);
avEdge *opAND(avEdge *p1, avEdge *p2);
avEdge *opOR(avEdge *p1, avEdge *p2);
int    opRank(avEdge *edge);
avEdge *hash_lookup(avEdge *p1, avEdge *p2);
int    hash_function(avEdge *p1, avEdge *p2);
void   opDestroy(avEdge *edge);
void   vDestroy(AND_V *vertex);

// BDD stuff
bdd_ptr bdd_and(bdd_ptr p1, bdd_ptr p2);
void bdd_free(bdd_ptr p);



AND_V  *initGraph() {

	AND_V *start_vertex = new AND_V;
	AND_V *zero_vertex  = new AND_V;
	AND_V *one_vertex   = new AND_V;
	
	zero_vertex->id    = ConstID0;
	zero_vertex->left  = zero_vertex;
	zero_vertex->right = zero_vertex;
	zero_vertex->next  = NULL;
	
	one_vertex->id    = ConstID1;
	one_vertex->left  = one_vertex;
	one_vertex->right = one_vertex;
	one_vertex->next  = NULL;
	
	start_vertex->id    = baseID;
	start_vertex->left  = zero_vertex;
	start_vertex->right = one_vertex;
	start_vertex->next  = NULL;
	
	Const0.inv_bit = true;
	Const0.table_entry = ConstID0;
	Const0.id = ConstID0;
	Const0.from = NULL;
	Const0.to = zero_vertex;
	
	Const0.inv_bit = false;
	Const0.table_entry = ConstID1;
	Const0.id = ConstID1;
	Const0.from = NULL;
	Const0.to = one_vertex;
	
	return start_vertex;
}


avEdge *opInit(AND_V *initVertex) {

	avEdge *edge = new avEdge;
	edge->inv_bit = false;
	edge->id = NONEXISTANT;
	edge->table_entry = NONEXISTANT;
	edge->from = edge->to = initVertex;
	
	return edge;
}


avEdge *createAND_V(avEdge *p1, avEdge *p2) {

	AND_V *new_vertex = new AND_V;
	
	new_vertex->id = 0; // ???
	new_vertex->next = NULL;
	new_vertex->left = p1->to;
	new_vertex->right = p2->to;
	
	p1->from = p2->from = new_vertex;
	
	return opInit(new_vertex);
}


avEdge *opNOT(avEdge *p) {

	p->inv_bit = !p->inv_bit;
	
	return p;
}


avEdge *opAND(avEdge *p1, avEdge *p2) {

	if (p1 == &Const1)
		return p2;
	if (p2 == &Const1)
		return p1;
	if (p1 == p2)
		return p1;
	if (p1 == opNOT(p2))
		return &Const0;
	if (p1 == &Const0 || p2 == &Const0)
		return &Const0;
		
	if (opRank(p1) > opRank(p2))
		swap(p1, p2);
			
	avEdge *p = hash_lookup(p1, p2);
	
	if (p != NULL)
		return p;
		
	return createAND_V(p1, p2);
}



avEdge *opOR(avEdge *p1, avEdge *p2) {

	return opNOT(opAND(opNOT(p1), opNOT(p2)));
}


int    opRank(avEdge *edge) {

	return edge->id; // ???
}


avEdge *hash_lookup(avEdge *p1, avEdge *p2) {

	int index = hash_function(p1, p2);
	
	avEdge p = &hash_table[index];
	
	while (p != NULL) {
		if (p->left == p1 && p->right == p2)
			return p;
			
		p = p->next;
	}
	return NULL;
}


int    hash_function(avEdge *p1, avEdge *p2) {

	return ((p1 << 5) ^ (p2 << 3))%table_size;
}


void   opDestroy(avEdge *edge) {

	if (edge == NULL)
		return;

	delete edge;
}


void   vDestroy(AND_V *vertex) {

	if (vertex == NULL)
		return;

	vDestroy(vertex->left);
	vDestroy(vertex->right);
	
	delete vertex;
}



// ****** BDD Functions ******

bdd_ptr bdd_and(bdd_ptr p1, bdd_ptr p2) {

	bdd_ptr p;
	
	// Stuff
	
	++p.reference_counter;
	
	return p;
}


void bdd_free(bdd_ptr p) {

	--p.reference_counter;
}










