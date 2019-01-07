#ifndef _IO_VECTOR_H_
#define _IO_VECTOR_H_


// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, IOVector.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver IO Vectors for Logic Simulation - header
// By Frank Gennari
//#define USE_UNBOUNDED_BINX

#define MAX_IN_TEST_ALL         16
#define MAX_IN_PER_INT          30
#define MAX_WARNING_INPUTS      1024
#define MAX_LABELLED_IO         1024
#define MAX_FLIPFLOPS           1024
#define NUM_PERMUTATIONS_PER_IV 1


int const MAX_WARNING_IVS = (1 << MAX_IN_TEST_ALL);
int const BITS_PER_INT    = sizeof(int)*8;


typedef char (*logic_func)(char, char);


class BINX {

	public:
		BINX();
		BINX(unsigned numbits);
		BINX(BINX &B);
		~BINX();
		void operator=(BINX &B);
		bool operator==(BINX &B);
		int init(unsigned numbits);
		inline unsigned get_num_bits() {return nbits;};
		char  get_bit(unsigned bit);
		int  set_bit(unsigned bit, char val);
		char  toggle_bit(unsigned bit);
		int  set_from_chars(const char *bitstring, unsigned bit_offset);
		unsigned conv_to_dec();
		unsigned conv_to_dec(unsigned *&dec_arr, unsigned ndigits);
		int  set_to_dec(unsigned dec);
		int  set_to_dec(unsigned *dec_arr, unsigned ndigits);
		void shift_right();
		void shift_left();
		int  increment();
		int  decrement();
		int  add(BINX &num);
		int  subtract(BINX &num);
		int  add(unsigned dec);
		int  subtract(unsigned dec);
		int  any_equal_to(char val);
		int  all_equal_to(char val);
		int  n_known(int n);
		void fix();
		void setall(char val);
		int  apply_logical_operation(logic_func op, BINX &A, BINX &B);
		int  read(istream &in);
		int  read(ifstream &in);
		int  write(ostream &out);
		int  write(ofstream &out);
	private:
		unsigned nbits;
		char* bit_vector;
};


inline char BINX::get_bit(unsigned bit) {

	if (bit >= nbits) {
		cerr << "\nError: Invalid bit index in get_bit of size " << nbits << ": " << (int)bit << endl;
		assert(0);
		return bit_vector[0];
	}
	return bit_vector[bit];
}




inline int BINX::set_bit(unsigned bit, char val) {

	if (bit >= nbits) {
		cerr << "\nError: Invalid bit index in set_bit of size " << nbits << ": " << (int)bit << endl;
		assert(0);
		bit_vector[0] = val;
		return 0;	
	}
	bit_vector[bit] = val;
	
	return 1;
}


/*struct BINX { 
	char bit_vector[max_enteredIO];
};*/


#endif