#ifndef _NET_POINTER_SET_H_
#define _NET_POINTER_SET_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, NetPointerSet.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Net Pointer Set classes for circuit hypergraphs
// By Frank Gennari
#define LLU_MEM_BLOCK_SIZE 1000


struct ll_unsigned {

	unsigned     val;
	ll_unsigned *next;
};


struct net_it_state {

	unsigned     net_id;
	ll_unsigned *curr, *last;
};


struct llu_mem_block {

	unsigned       capacity, used;
	ll_unsigned   *data;
	llu_mem_block *next;
};



class NetPointerSet {

public:
	NetPointerSet(unsigned size);
	~NetPointerSet();
	int set_size(unsigned size);
	int insert(unsigned ins_net_id, unsigned val);
	int set_iterator_net(unsigned it_net_id);
	int advance_iterator();
	int remove_curr();
	inline unsigned get_curr() {if (curr == NULL) {return 0;} return curr->val;};
	unsigned get_size(unsigned net_id_val);
	int net_empty(unsigned net_id_val);
	int net_single(unsigned net_id_val);
	int net_multi(unsigned net_id_val);
	net_it_state it_get_state();
	int net_set_state(net_it_state state);

private:
	ll_unsigned *mem_alloc();

	unsigned        xsize, net_id, num_blocks;
	ll_unsigned   **data, *curr, *last;
	llu_mem_block  *mem_head; // used for custom memory allocation
};


#endif