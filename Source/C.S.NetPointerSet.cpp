#include "CircuitSolver.h"
#include "NetPointerSet.h"



// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.NetPointerSet.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Net Pointer Set class member functions
// by Frank Gennari
NetPointerSet::NetPointerSet(unsigned size) {

	data       = NULL;
	curr       = NULL;
	last       = NULL;
	mem_head   = NULL;
	xsize      = size;
	net_id     = 0;
	num_blocks = 0;

	if (size == 0)
		return;

	data = memAlloc_init(data,  size, (ll_unsigned *)NULL);
}



NetPointerSet::~NetPointerSet() {

	if (data != NULL) {
		if (LLU_MEM_BLOCK_SIZE < 2) {
			for (unsigned int i = 0; i < xsize; ++i) 
				destroyL(data[i]);
		}
		else {
			llu_mem_block *temp;
		
			while (mem_head != NULL) {
				delete []  mem_head->data;
				temp     = mem_head->next;
				delete     mem_head;
				mem_head = temp;
			}
			num_blocks = 0;
		}
		delete [] data;
		data = NULL;
	}
}




int NetPointerSet::set_size(unsigned size) {

	if (size == xsize)
		return 1;
		
	if (num_blocks != 0) {
		cerr << "Error: Cannot change NetPointerSet size after items have been added." << endl;
		return 0;
	}
	if (data != NULL) {
		delete [] data;
		data = NULL;
	}
	curr     = NULL;
	last     = NULL;
	mem_head = NULL;
	xsize    = size;

	if (size == 0)
		return 1;

	data = memAlloc_init(data,  size, (ll_unsigned *)NULL);
	
	return (data != NULL);
}



int NetPointerSet::insert(unsigned ins_net_id, unsigned val) {

	if (ins_net_id >= xsize)
		return 0;

	ll_unsigned *new_ll = mem_alloc(); // custom memory allocation

	new_ll->val  = val;
	new_ll->next = data[ins_net_id];
	data[ins_net_id] = new_ll;

	return 1;
}



int NetPointerSet::set_iterator_net(unsigned it_net_id) {

	if (it_net_id >= xsize)
		return 0;

	net_id = it_net_id;
	curr   = data[net_id];
	last   = NULL;

	return (curr != NULL);
}



int NetPointerSet::advance_iterator() {

	last = curr;

	if (curr == NULL) {
		return 0;
	}
	if (curr->next == NULL) {
		curr = NULL;
		return 0;
	}
	curr = curr->next;

	return 1;
}



int NetPointerSet::remove_curr() {

	if (curr == NULL) {
		return 0;
	}
	if (last == NULL) { // first element
		data[net_id] = curr->next;
	}
	else {
		last->next = curr->next;
	}
	if (LLU_MEM_BLOCK_SIZE < 2) {
		delete curr;
		curr = last;
	}
	// curr not actually deleted here - deleted along with entire memory block in destructor
	// leave curr pointing to deleted element since it is still physically there

	return 1;
}



// slower than other operations because individual net sizes are not recorded
unsigned NetPointerSet::get_size(unsigned net_id_val) {

	if (net_id_val >= xsize)
		return 0;

	unsigned size(0);
	ll_unsigned *head = data[net_id_val];

	while (head != NULL) {
		++size;
		head = head->next;
	}
	return size;
}



int NetPointerSet::net_empty(unsigned net_id_val) {

	if (net_id_val >= xsize)
		return 1;

	return (data[net_id_val] == NULL);
}



int NetPointerSet::net_multi(unsigned net_id_val) {

	if (net_id_val >= xsize)
		return 0;

	return (data[net_id_val] != NULL && data[net_id_val]->next != NULL);
}



int NetPointerSet::net_single(unsigned net_id_val) {

	if (net_id_val >= xsize)
		return 0;

	return (data[net_id_val] != NULL && data[net_id_val]->next == NULL);
}



net_it_state NetPointerSet::it_get_state() {

	net_it_state state;

	state.curr   = curr;
	state.last   = last;
	state.net_id = net_id;

	return state;
}



int NetPointerSet::net_set_state(net_it_state state) {

	if (state.net_id >= xsize)
		return 0;

	net_id = state.net_id;
	curr   = state.curr;
	last   = state.last;

	return 1;
}



ll_unsigned *NetPointerSet::mem_alloc() {

	if (LLU_MEM_BLOCK_SIZE < 2)
		return new ll_unsigned;
		
	if (mem_head == NULL || num_blocks == 0 || mem_head->used >= mem_head->capacity) {	
		llu_mem_block *new_block = new llu_mem_block;
		if (new_block == NULL)
			return NULL;

		new_block->data = new ll_unsigned[LLU_MEM_BLOCK_SIZE];
		if (new_block->data == NULL) {
			delete new_block;
			return NULL;
		}
		new_block->capacity = LLU_MEM_BLOCK_SIZE;
		new_block->used     = 0;
		new_block->next     = mem_head;
		mem_head            = new_block;
		++num_blocks;
	}
	return &(mem_head->data[mem_head->used++]);
}





