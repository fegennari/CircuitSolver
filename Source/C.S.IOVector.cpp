#include "CircuitSolver.h"
#include "Logic.h"


// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.IOVector.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver IO Vector processing for Logic Simulation
// By Frank Gennari


/*class BINX {

	public:
		BINX();
		BINX(unsigned numbits);
		BINX(BINX &B);
		~BINX();
		void operator=(BINX &B);
		bool operator==(BINX &B);
		int init(unsigned numbits);
		unsigned get_num_bits();
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
}*/



BINX::BINX() {

	bit_vector = NULL;
	nbits      = 0;
}




BINX::BINX(unsigned numbits) {

	bit_vector = NULL;
	nbits      = numbits;
	
	if (nbits > 0) {
		bit_vector = memAlloc(bit_vector, nbits);
	}
}



// copy constructor
BINX::BINX(BINX &B) {

	unsigned i;

	bit_vector = NULL;
	nbits      = B.get_num_bits();
	
	if (nbits > 0) {
		bit_vector = memAlloc(bit_vector, nbits);

		for (i = 0; i < nbits; ++i) {
			bit_vector[i] = B.get_bit(i);
		}
	}
}




BINX::~BINX() {

	if (bit_vector != NULL) {
		delete [] bit_vector;
	}
}




void BINX::operator=(BINX &B) {

	unsigned i;

	if (nbits != B.get_num_bits()) {
		internal_error();
		cerr << "Error in BINX::operator=: nbits1 of " << nbits << " does not equal nbits2 of " << (int)B.get_num_bits() << endl;
		return;
	}
	for (i = 0; i < nbits; ++i) {
		bit_vector[i] = B.get_bit(i);
	}
}




bool BINX::operator==(BINX &B) {

	unsigned i;

	if (nbits != B.get_num_bits()) {
		internal_error();
		cerr << "Error in BINX::operator==: nbits1 of " << nbits << " does not equal nbits2 of " << (int)B.get_num_bits() << endl;
		return 0;
	}
	for (i = 0; i < nbits; ++i) {
		if (bit_vector[i] != B.get_bit(i)) {
			return 0;
		}
	}
	return 1;
}




int BINX::init(unsigned numbits) {

	if (bit_vector != NULL || nbits > 0 || numbits == 0) {
		return 0;
	}
	nbits      = numbits;
	bit_vector = memAlloc(bit_vector, nbits);
	
	return 1;
}




char BINX::toggle_bit(unsigned bit) {

	if (bit >= nbits) {
		cerr << "\nError: Invalid bit index in toggle_bit of size " << nbits << ": " << (int)bit << endl;
		assert(0);
		return 2;
	}
	if (bit_vector[bit] == 0) {
		bit_vector[bit] = 1;
		return 1;
	}
	if (bit_vector[bit] == 1) {
		bit_vector[bit] = 0;
		return 0;
	}
	return 2; // unknown
}




int BINX::set_from_chars(const char *bitstring, unsigned bit_offset) {

	unsigned i;
	char bit;

	for (i = 0; i < nbits; ++i) {
		if (i < bit_offset) {
			bit_vector[i] = 0; // add leading zeros
		}
		else {
			bit = bitstring[i - bit_offset];
			
			switch (bit) {
				case 0: // NULL termination
					return 0;
				case '0':
					bit_vector[i] = 0;
					break;
				case '1':
					bit_vector[i] = 1;
					break;
				case '-':
					bit_vector[i] = -2;
					break;
				default:
					bit_vector[i] = 2;
			}
		}
	}
	return 1;
}




unsigned BINX::conv_to_dec() {
	
	unsigned sum(0);
	int i;
	
	for (i = nbits-1; i >= 0; --i) {
		sum += ((unsigned)bit_vector[i]) << i;
	}
	return sum;
}




unsigned BINX::conv_to_dec(unsigned *&dec_arr, unsigned ndigits) {

	unsigned num_ints, sum(0), cntr;
	int i;

	num_ints = (unsigned)ceil((double)nbits/(double)BITS_PER_INT); // about num_bits/32

	if (ndigits < num_ints && dec_arr != NULL) {
		ndigits = num_ints;
		delete [] dec_arr;
		dec_arr = NULL;
	}
	if (dec_arr == NULL) {
		dec_arr = memAlloc(dec_arr, num_ints);
	}
	for (i = nbits-1, cntr = 0; i >= 0; --i, ++cntr) {
		if (cntr > 0 && cntr%BITS_PER_INT == 0) {
			dec_arr[cntr/BITS_PER_INT] = sum; // enter next digit
			sum = 0;
		}
		sum += ((unsigned)bit_vector[i]) << i;
	}
	dec_arr[num_ints-1] = sum; // last digit

	return ndigits;
}




int BINX::set_to_dec(unsigned dec) {

	int i;

	for (i = nbits-1; i >= 0; --i) {
		bit_vector[i] = (char)(dec%2);
		dec >>= 1;

		if (dec == 0) { // all remaining bits are zero
			--i;
			break;
		}
	}
	for (; i >= 0; --i) {
		bit_vector[i] = 0;
	}
	return (dec == 0); // if dec != 0 then overflow
}




int BINX::set_to_dec(unsigned *dec_arr, unsigned ndigits) {

	int i;
	unsigned num_ints, cntr, dec(0);

	if (dec_arr == NULL || ndigits == 0) {
		return 0;
	}
	num_ints = (unsigned)ceil((double)nbits/(double)BITS_PER_INT); // about num_bits/32
	num_ints = min(num_ints, ndigits);

	for (i = nbits-1, cntr = 0; i >= 0; --i, ++cntr) {
		if (cntr%BITS_PER_INT == 0) {
			dec = dec_arr[cntr/BITS_PER_INT]; // retrieve next digit
		}
		if (dec == 0) {
			bit_vector[i] = 0;
		}
		else {
			bit_vector[i] = (char)(dec%2);
			dec >>= 1;
		}
	}
	return 1;
}




void BINX::shift_right() {
	
	if (nbits == 0)
		return;
	
	unsigned i;
	char temp = bit_vector[nbits-1];
	
	for (i = 1; i < nbits; ++i) {
		bit_vector[i] = bit_vector[i-1];
	}
	bit_vector[0] = temp;
}



void BINX::shift_left() {
	
	if (nbits == 0)
		return;
	
	unsigned i;
	char temp = bit_vector[0];
	
	for (i = 0; i < nbits-1; ++i) {
		bit_vector[i] = bit_vector[i+1];
	}
	bit_vector[nbits-1] = temp;
}



int BINX::increment() {

	for (int i = nbits - 1; i >= 0; --i) {
		if (bit_vector[i] == 0) {
			bit_vector[i] = 1;
			return 1; // end carry
		}
		if (bit_vector[i] == 1) {
			bit_vector[i] = 0; // then carry the 1
		}
		else {
			cerr << "Error in increment: bit is neither 0 nor 1: " << (int)bit_vector[i] << "." << endl;
			return 0;
		}
	}
	return 0; // overflow
}




int BINX::decrement() {

	for (int i = nbits - 1; i >= 0; --i) {
		if (bit_vector[i] == 1) {
			bit_vector[i] = 0;
			return 1; // end carry
		}
		if (bit_vector[i] == 0) {
			bit_vector[i] = 1; // then carry the -1
		}
		else {
			cerr << "Error in decrement: bit " << i << " is neither 0 nor 1: " << (int)bit_vector[i] << "." << endl;
			return 0;
		}
	}
	return 0; // underflow
}




int BINX::add(BINX &num) {

	int i;
	char ai, bi, carry(0);

	for (i = nbits - 1; i >= 0; --i) {
		ai = bit_vector[i];
		bi = num.get_bit(i);
		
		if (ai != 0 && ai != 1) {
			cerr << "Error in add: bit " << i << " of first argument is neither 0 nor 1: " << (int)ai << "." << endl;
			return 0;
		}
		if (bi != 0 && bi != 1) {
			cerr << "Error in add: bit " << i << " of second argument is neither 0 nor 1: " << (int)bi << "." << endl;
			return 0;
		}
		bit_vector[i] = ((ai ^ bi) ^ carry);
		carry         = ((ai + bi + carry) > 1);
	}
	return (carry == 0); // overflow if carry == 1
}




int BINX::subtract(BINX &num) {

	int i;
	char ai, bi, borrow(0);

	for (i = nbits - 1; i >= 0; --i) {
		ai = bit_vector[i];
		bi = num.get_bit(i);
		
		if (ai != 0 && ai != 1) {
			cerr << "Error in subtract: bit " << i << " of first argument is neither 0 nor 1: " << (int)ai << "." << endl;
			return 0;
		}
		if (bi != 0 && bi != 1) {
			cerr << "Error in subtract: bit " << i << " of second argument is neither 0 nor 1: " << (int)bi << "." << endl;
			return 0;
		}
		bit_vector[i] = ((ai ^ bi) ^ borrow);
		borrow        = ((bi + borrow) > ai);
	}
	return (borrow == 0); // underflow if borrow == 1
}




int BINX::add(unsigned dec) {

	int i;
	char ai, bi, carry(0);

	for (i = nbits - 1; i >= 0; --i) {
		if (dec == 0 && carry == 0) {
			return 1; // end of decimal and carry chain
		}
		ai = bit_vector[i];
		bi = dec%2;
		dec >>= 1;
		
		if (ai != 0 && ai != 1) {
			cerr << "Error in add: bit " << i << " of first argument is neither 0 nor 1: " << (int)ai << "." << endl;
			return 0;
		}
		bit_vector[i] = ((ai ^ bi) ^ carry);
		carry         = ((ai + bi + carry) > 1);
	} // overflow if carry == 1
	return (carry == 0);
}




int BINX::subtract(unsigned dec) {

	int i;
	char ai, bi, borrow(0);

	for (i = nbits - 1; i >= 0; --i) {
		if (dec == 0 && borrow == 0) {
			return 1; // end of decimal and borrow chain
		}
		ai = bit_vector[i];
		bi = dec%2;
		dec >>= 1;
		
		if (ai != 0 && ai != 1) {
			cerr << "Error in subtract: bit " << i << " of first argument is neither 0 nor 1: " << (int)ai << "." << endl;
			return 0;
		}
		bit_vector[i] = ((ai ^ bi) ^ borrow);
		borrow        = ((bi + borrow) > ai);
	}
	return (borrow == 0); // underflow if borrow == 1
}




int BINX::any_equal_to(char val) {

	for (unsigned i = 0; i < nbits; ++i) {
		if (bit_vector[i] == val) {
			return 1;
		}
	}	
	return 0;
}




int BINX::all_equal_to(char val) { 

	for (unsigned i = 0; i < nbits; ++i) {
		if (bit_vector[i] != val) {
			return 0;
		}
	}	
	return 1;
}



int BINX::n_known(int n) {

	if (n <= 0)
		return 1;
		
	if (n > (int)nbits)
		n = nbits;
	
	for (unsigned i = 0; (int)i < n; ++i) {
		if (bit_vector[i] & ~1) {
			return 0;
		}
	}	
	return 1;
}




void BINX::fix() { 

	for (unsigned i = 0; i < nbits; ++i) {
		bit_vector[i] = LOGIC(bit_vector[i]);
	}
}



void BINX::setall(char val) { 

	for (unsigned i = 0; i < nbits; ++i) {
		bit_vector[i] = val;
	}
}




int BINX::apply_logical_operation(logic_func op, BINX &A, BINX &B) {

	if (nbits != A.get_num_bits() || nbits != B.get_num_bits()) {
		return 0;
	}
	for (unsigned i = 0; i < nbits; ++i) {
		bit_vector[i] = op(A.get_bit(i), B.get_bit(i));
	}
	return 1;
}




int BINX::read(istream &in) {

	char bit;
	unsigned i;
	
	for (i = 0; i < nbits; ++i) {
		if (!(in >> bit)) {
			return 0;
		}
		bit_vector[i] = bit;
	}
	return 1;
}




int BINX::read(ifstream &in) {

	char bit;
	unsigned i;
	
	for (i = 0; i < nbits; ++i) {
		if (!(in >> bit)) {
			return 0;
		}
		bit_vector[i] = bit;
	}
	return 1;
}




int BINX::write(ostream &out) {

	for (unsigned i = 0; i < nbits; ++i) {
		if (!(out << (int)bit_vector[i])) {
			return 0;
		}
	}
	return 1;
}




int BINX::write(ofstream &out) {

	for (unsigned i = 0; i < nbits; ++i) {
		if (!(out << (int)bit_vector[i])) {
			return 0;
		}
	}
	return 1;
}





