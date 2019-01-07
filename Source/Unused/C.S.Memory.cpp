#include "CircuitSolver.h"
#include "Memory.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Memory.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver block allocation memory manager (for components in Link Circuits)
// by Frank Gennari

/*class MemoryHandler {

	public:
		bool create(int block_size);
		T *next_avail();
		void free_all();
		void free_rest();
		
	private:
		int size, next;
		component *memory_pool;
		
		bool allocate_new();
};*/


bool MemoryHandler::create(int block_size) {

	size = max(1, abs(block_size));	
	
	return allocate_new();
}




component *MemoryHandler::next_avail() {

	if (next >= size)		
		allocate_new();
	
	return &memory_pool[next++];
	//return (memory_pool + next++); 
}




void MemoryHandler::free_all() {

	delete [] memory_pool;
	
	next = size;
}




void MemoryHandler::free_rest() {

	for (unsigned int i = next; i < size; ++i)
		delete &memory_pool[i];
		
	//delete [] (memory_pool + next);
		
	next = size;
}




bool MemoryHandler::allocate_new() {

	memory_pool = new component[size];
	
	if (!memory_pool) {
		out_of_memory();
		return false;
	}
	next = 0;
	
	return true;
}







