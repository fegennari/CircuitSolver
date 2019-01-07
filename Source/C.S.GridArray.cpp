#include "CircuitSolver.h"
#include "GridArray.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.GridArray.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Grid Array Class Member Functions
// by Frank Gennari
extern int useHASHTABLE, max_speed;


/*class GridArray {

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
		
	private:
		llist *mem_alloc();
		void mem_delete();
		
		int format, default_val, endf, block_size, num_blocks;
		unsigned int asize, table_size, currindex, prime_index;
		int *nodenumber; // for array
		llist **nnlist, *currelm; // for hash table
		mem_block *mem_head; // memory
};*/


// GridArray class member functions
bool GridArray::create(int array_size, int data_format, int default_value, unsigned int mem_block_size) {
	
	unsigned int i;

	if (default_value >= 0) {
		internal_error();
		cerr << "Error: Default value must be less than zero: " << default_value << endl;
		return 0;
	}
	format      = data_format;
	default_val = default_value;
	block_size  = mem_block_size;
	num_blocks  = 0;
	endf        = default_val;
	mem_head    = NULL;
	
	array_size = max(array_size, 1);
	
	if (format == CS_DECIDES) {
		if (useHASHTABLE == 1)
			format = HASHTABLE;
		else if (useHASHTABLE == 0)
			format = ARRAY;
		else if (!max_speed && array_size/max_boxes > 1)
			format = HASHTABLE;
		else
			format = ARRAY;
	}
	if (format == ARRAY) {
		asize      = array_size;
		nodenumber = new int[asize];
		if(!nodenumber)
			return 0;
		
		for (i = 0; i < asize; ++i)
			nodenumber[i] = default_val;
	}	
	else {
		table_size  = currindex = 0; 
		currelm     = 0;
		prime_index = 0;
		asize       = init_htable_size;
		nnlist      = new llist*[asize];

		if(!nnlist)
			return 0;	
			
		for (i = 0; i < asize; ++i) 
			nnlist[i] = 0;
	}
	return 1;
}




bool GridArray::set_ht_size(unsigned int size) { // Warning: Must already be created but empty

	unsigned int i;

	if (format == ARRAY || size <= 0)
		return 1;
		
	mem_delete();
	
	for (prime_index = 0; prime_index < NUM_PRIMES; ++prime_index) {
		if (PRIMES[prime_index] > size)
			break;
	}
	if (prime_index > 0) {
		--prime_index;
	}
	asize  = PRIMES[prime_index];
	nnlist = new llist*[asize];

	if(!nnlist)
		return 0;	
			
	for (i = 0; i < asize; ++i) 
		nnlist[i] = 0;
	
	table_size = 0;
	
	return 1;
}




void GridArray::setall(int value) {

	unsigned int i;

	if (format == ARRAY) {
		for (i = 0; i < asize; ++i)
			nodenumber[i] = value;
	}		
	else {
		llist *head;
	
		for (i = 0; i < asize; ++i) {
			head = nnlist[i];
			while (head != 0) {
				head->value = value;
				head        = head->next;
			}
		}
	}
		
}




void GridArray::setmax(int value) { // kind of algorithm specific

	unsigned int i;
	
	if (format == ARRAY) {
		for (i = 0; i < asize; ++i)
			if (nodenumber[i] > value)
			nodenumber[i] = default_val;
	}		
	else {
		llist *head;
		
		for (i = 0; i < asize; ++i) {
			head = nnlist[i];
			while (head != 0) {
				if (head->value > value)
					head->value = default_val;
				head = head->next;
			}
		}
	}
}




void GridArray::remap(int *arr, int x) { // sort of algorithm specific

	if (x < 0)
		return;

	if (format == ARRAY) {
		if (nodenumber[x] >= 0)
			nodenumber[x] = arr[nodenumber[x]];
	}
	else { // hashtable	
		llist *head;
		int index;
		
		if (asize == 0)
			return;
		
		index = x%asize;
		head  = nnlist[index]; // search for item in hash table
		
		while (head != 0) {
			if (head->index == x && head->value >= 0) {
				head->value = arr[head->value];
				return;
			}
			head = head->next;
		}
	}
}




int GridArray::replace_if_default(int x, int val) { // sort of algorithm specific

	if (x < 0)
		return 0;

	if (format == ARRAY) {
		if (x >= (int)asize)
			return 0;

		if (nodenumber[x] == default_val) {
			nodenumber[x] = val;
			return 1;
		}
	}
	else { // hashtable	- similar to operator[]
		llist *head, *new_elem;
		int index;
		
		if (asize == 0)
			rehash();
		
		index = x%asize;
		head  = nnlist[index]; // search for item in hash table
		
		while (head != 0) {
			if (head->index == x) {
				if (head->value == default_val) {
					head->value = val;
					return 1;
				}
				return 0;
			}
			head = head->next;
		}
		new_elem = mem_alloc(); // not found: add new element
		if (new_elem == NULL) {
			out_of_memory();
			return 0;
		}
		if (table_size/asize > max_load_factor) { // resize / rehash
			rehash();	
			index = x%asize;	
		}
		++table_size;
		
		new_elem->index = x;
		new_elem->value = val;
		new_elem->next  = nnlist[index];		
		nnlist[index]   = new_elem;
		
		return 1;
	}
	return 0;
}




int GridArray::replace_value(int x, int val) {

	if (x < 0) {
		return default_val;
	}
	int old_val;

	if (format == ARRAY) {
		if (x >= (int)asize) {
			return default_val;
		}
		old_val       = nodenumber[x];
		nodenumber[x] = val;
		return old_val;
	}
	else { // hashtable	- similar to operator[]
		llist *head;
		int index;
		
		if (asize == 0 || table_size == 0) {
			return default_val;
		}
		index = x%asize;
		head  = nnlist[index]; // search for item in hash table
		
		while (head != 0) {
			if (head->index == x) {
				old_val     = head->value;
				head->value = val;
				return old_val;
			}
			head = head->next;
		}	
	}
	return default_val;
}




int& GridArray::getnext(int end_flag, int &index) {
	
	endf = end_flag;
	
	if (format == ARRAY) {
		if (currindex < asize) {
			index = currindex;
			return nodenumber[currindex++];
		}
		index = -1;
		return endf;
	}
	unsigned int i;
	llist *temp;

	if (currelm != 0) {
		temp    = currelm;
		currelm = currelm->next;
		index   = temp->index;
		return temp->value;
	}
	else {
		for (i = currindex + 1; i < asize && nnlist[i] == 0; ++i) {};
		
		if (i >= asize) {
			index = -1;
			return endf;
		}
		currindex = i;
		currelm   = nnlist[i]->next;
		index     = nnlist[i]->index;
		return nnlist[i]->value;
	}
}




void GridArray::resetnext() {
	
	currindex = 0;
	
	if (format == HASHTABLE) {
		unsigned int i;

		for (i = 0; i < asize && nnlist[i] == 0; ++i) {};

		if (i >= asize) {
			return;
		}
		currelm   = nnlist[i];
		currindex = i;
	}
}




int GridArray::remove(int x) {
	
	if (format == ARRAY) {
		nodenumber[x] = default_val; // lazy deletion
		return 1;
	}
	int index;
	llist *temp, *head;
	
	if (table_size == 0)
		return 0;
		
	index = x%asize;
	head  = nnlist[index];
	
	if (head == 0)
		return 0;
		
	if (head->index == x) {
		temp = head->next;
		if (block_size < 2)
			delete head;
		nnlist[index] = temp;
		--table_size;
		return 1;
	}
	temp = head;
	head = head->next;
	
	while (head != 0) {
		if (head->index == x) {
			temp->next = head->next;
			if (block_size < 2)
				delete head;
			--table_size;
			return 1;
		}
		temp = head;
		head = head->next;
	}
	return 0; // not found
}




void GridArray::destroy() {

	if (format == ARRAY) {
		if (nodenumber != NULL) {
			delete [] nodenumber;
			nodenumber = NULL;
		}
	}	
	else {
		mem_delete();
		table_size  = 0;
		asize       = 0;
		prime_index = 0;
	}
}




int& GridArray::operator[](int x) {
			
	if (format == ARRAY) {
		return nodenumber[x];
	}
	// hashtable		
	llist *head, *new_elem;
	int index;

	if (asize == 0)
		rehash();
	
	index = x%asize;
	head  = nnlist[index]; // search for item in hash table
	
	while (head != 0) {
		if (head->index == x)
			return head->value;
			
		head = head->next;
	}
	new_elem = mem_alloc(); // not found: add new element
	if (new_elem == NULL) {
		out_of_memory();
		return default_val;
	}
	if (table_size/asize > max_load_factor) { // resize / rehash
		rehash();
		index = x%asize;
	}
	++table_size;
	
	new_elem->index = x;
	new_elem->value = default_val;
	new_elem->next  = nnlist[index];
	nnlist[index]   = new_elem;
	
	return new_elem->value;
}




int GridArray::assign_incremental_indices(int start_val) {

	int counter(start_val);
	unsigned int i;

	if (format == ARRAY) {
		for (i = 0; i < asize; ++i) {
			if (nodenumber[i] == TEMP_GA_VALUE)
				nodenumber[i] = counter++;
		}
		return --counter;
	}
	else {
		llist *head;

		for (i = 0; i < asize; ++i) {
			head = nnlist[i];
			while (head != 0) {
				if (head->value == TEMP_GA_VALUE) {
					head->value = counter++;
				}
				head = head->next;
			}
		}
		return --counter;
	}
}




void GridArray::rehash() {
	
	if (format == ARRAY)
		return;

	unsigned int i, old_asize(asize);
	int index;
	llist *head, *temp, **nnlist2 = NULL;
	
	if (prime_index < NUM_PRIMES)
		++prime_index;
		
	asize = PRIMES[prime_index];
	
	//cout << "Rehash: New size is " << asize << endl;
	
	nnlist2 = memAlloc(nnlist2, asize); // create new, larger hashtable
	if(!nnlist2)
		return;

	for (i = 0; i < asize; ++i)
		nnlist2[i] = NULL;
	
	for (i = 0; i < old_asize; ++i) { 
		head = nnlist[i];
		while (head != 0) {
			index          = (head->index)%asize;
			temp           = head->next;
			head->next     = nnlist2[index];
			nnlist2[index] = head;
			head           = temp;
		}
	}
	delete [] nnlist; // delete old hashtable
	nnlist = nnlist2;	
}




llist *GridArray::mem_alloc() {

	if (block_size < 2)
		return new llist;
		
	if (mem_head == NULL || num_blocks == 0 || mem_head->used >= mem_head->capacity) {	
		mem_block *new_block = new mem_block;
		if (new_block == NULL)
			return NULL;

		new_block->data = new llist[block_size];
		if (new_block->data == NULL) {
			delete new_block;
			return NULL;
		}
		new_block->capacity = block_size;
		new_block->used     = 0;
		new_block->next     = mem_head;
		mem_head            = new_block;
		++num_blocks;
	}
	return &(mem_head->data[mem_head->used++]);
}





void GridArray::mem_delete() {
		
	if (nnlist != NULL) {
		if (block_size < 2) {
			for (unsigned int i = 0; i < asize; ++i) 
				destroyL(nnlist[i]);
		}
		else {
			mem_block *temp;
		
			while (mem_head != NULL) {
				delete []  mem_head->data;
				temp     = mem_head->next;
				delete     mem_head;
				mem_head = temp;
			}
			num_blocks = 0;
		}
		delete [] nnlist;
		nnlist = NULL;
	}
}




void GridArray::print() {
	
	llist *head;
	ofstream outfile;
	string name;
	
	if (format == ARRAY) {
		name = "Array.txt";
	}
	else {
		name = "Hashtable.txt";
	}
	outfile.open(name.c_str(), ios::out | ios::trunc);

	if (outfile.fail() || !outfile.good()) {
		cerr << "Error creating " << name << "." << endl;
		return;
	}
	else {
		cout << "Writing Grid Array to " << name << "." << endl;
	}
	for (unsigned int i = 0; i < asize; ++i) {
		outfile << i;
		if (i < 10)
			outfile << " "; 
		outfile << " | ";

		if (format == ARRAY) {
			outfile << nodenumber[i];
		}
		else {
			head = nnlist[i];
			while (head != 0) {
				outfile << head->index << "," << head->value << "\t";
				head = head->next;
			}
		}
		outfile << endl;
	}
	outfile.close();
}




unsigned int GridArray::size() {

	if (format == ARRAY) { // slow for arrays
		int arr_size(0);
		unsigned i;

		for (i = 0; i < asize; ++i) {
			if (nodenumber[i] != default_val) {
				++arr_size;
			}
		}
		return arr_size;
	}
	return table_size;
}


