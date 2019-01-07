#ifndef _GRID_ARRAY_H_
#define _GRID_ARRAY_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, GridArray.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Grid Array: Hash Table and Resizing Array Header plus lists (for NODENUMBER array)
// By Frank Gennari
#define TEMP_GA_VALUE -5


int const init_htable_size = PRIMES[0];
float const max_load_factor = 2.0;

enum {ARRAY, HASHTABLE, CS_DECIDES};



struct llist {

	int value, index;
	llist *next;
};


struct mem_block {

	int capacity, used;
	llist *data;
	mem_block *next;
};




// Wow, finally real C++ in Circuit Solver!
class GridArray {

	public:
		bool create(int array_size, int data_format, int default_value, unsigned int mem_block_size);
		bool set_ht_size(unsigned int size);
		void setall(int value);
		void setmax(int maxval);
		void remap(int *arr, int x);
		int  replace_if_default(int x, int val);
		int  replace_value(int x, int val);
		int& getnext(int end_flag, int &index);
		void resetnext();
		int  remove(int x);
		void destroy();
		int& operator[](int x);
		int  assign_incremental_indices(int start_val);
		void rehash();
		void print();
		unsigned int size();
		
	private:
		llist *mem_alloc();
		void mem_delete();
		
		int format, default_val, endf, block_size, num_blocks;
		unsigned int asize, table_size, currindex, prime_index;
		int *nodenumber; // for array
		llist **nnlist, *currelm; // for hash table
		mem_block *mem_head; // memory
};



inline void zero(GridArray GA);
inline void one(GridArray GA);


inline void zero(GridArray GA) {

	GA.setall(0);
}




inline void one(GridArray GA) {

	GA.setall(1);
}


#endif
