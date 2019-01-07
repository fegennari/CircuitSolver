#ifndef _GRAPH_H_
#define _GRAPH_H_



#include <vector.h>
#include <list.h>

#include "CircuitSolver.h"

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Graph.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Gate Delay/Graph header
// By Frank Gennari
struct Vertex // adjacency list and value(delay)
{
	list<int> adj;
	int value;
};



class Graph
{
	private:
		int size;
		vector<Vertex> V; // all graph vertices

	public:
		Graph(int gsize): size(gsize), V(gsize+1) {}; // size+1 is used because there
		~Graph() {};                                  // is no vertex 0 and last vertex = size
		void insert (int a, int b, float x);
		float distance (int a, int b); // calculates distance only
		bool find_edge(int a, int b);
};



#endif