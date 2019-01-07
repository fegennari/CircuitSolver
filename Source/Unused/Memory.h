#ifndef _MEMORY_H_
#define _MEMORY_H_


#include "Logic.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Memory.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Header for block memory allocation (components in Link Circuit)
// By Frank Gennari
class MemoryHandler {

	public:
		bool create(int block_size);
		component *next_avail();
		void free_all();
		void free_rest();
		
	private:
		int size, next;
		component *memory_pool;
		
		bool allocate_new();
};


#endif



