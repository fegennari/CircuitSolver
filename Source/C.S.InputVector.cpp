#include "CircuitSolver.h"
#include "Logic.h"
#include "GridArray.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.InputVector.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Input Vector Read/Generate for Logic Engine
// By Frank Gennari
int const allow_dontcare = 1;


extern long global_random_seed;


int  read_or_generate_ivs(BINX *&input_vectors, int &num_ivs, int &num_inputs, int &iv_mode, unsigned &num_permutations);
void binx_alloc(BINX *&binx_arr, int size, int nbits);
int  read_input_vector_file(BINX *&input_vectors, int num_inputs);
void get_input_vectors(BINX *input_vectors, int num_ivs, int num_inputs);
void process_iv(BINX *input_vectors, BINX &bits, int num_ivs, int num_inputs, unsigned &curr_iv);
void random_vectors(long rseed1, long rseed2, int num_vectors, int num_bits, BINX *input_vectors, int gen_seed_vector);
void random_permutation(BINX &iv, int &last_permute, unsigned num_permutations);
int  read_bits_from_file(ifstream &infile, BINX &BINARY, int nbits, int num_outputs);
void chars_to_binary(BINX &BINARY, const char *bitstring, int nbits);




int read_or_generate_ivs(BINX *&input_vectors, int &num_ivs, int &num_inputs, int &iv_mode, unsigned &num_permutations) {

	int  use_ivs(0), file_ivs(0), nin(num_inputs);
	long random_seed2;

	num_ivs = iv_mode = 0;
	stop_timer();

	cout << "\nDo you want to use a particular set of input vectors for this circuit? ";
	if (decision()) {
		use_ivs = 1;
		cout << "Are the input vectors in an input file?  ";
		if (decision()) {
			while (num_ivs == 0) {
				num_ivs = read_input_vector_file(input_vectors, num_inputs);
				if (num_ivs == 0) {
					delete [] input_vectors;
					cout << "Try again?  ";
					if (!decision()) {
						return 0;
					}
				}
				file_ivs = 1;
			}
		}
		else {
			cout << "How many input vectors would you like to use? ";
			num_ivs = in();
			if (num_ivs < 1) {
				cout << "You must provide at least one input vector. No simulation will be performed." << endl;
				return 0;
			}
			if (num_ivs > MAX_WARNING_IVS) {
				cout << "Warning: " << num_ivs << " is a very large number of input vectors, so the simulation may take awhile." << endl;
			}
		}
	}
	else {
		use_ivs = 0;		
	}
	start_timer();
	nin = min(num_inputs, MAX_IN_TEST_ALL);

	if (use_ivs == 1 && !file_ivs && num_ivs >= (1 << nin)) {
		cout << "You have requested >= the maximum number of unique input vectors of " << (1 << nin) << ". All input vectors up to that number will be simulated." << endl;
		num_ivs = (1 << nin);
		use_ivs = 0;
	}
	if (!use_ivs) {
		if (num_inputs > MAX_IN_TEST_ALL) {
			cout << "Warning: Only the first " << (1 << nin) << " input vectors will be generated." << endl;
		}
		num_ivs = (1 << nin);
	}
	if (!file_ivs) {
		if (use_ivs) {
			binx_alloc(input_vectors, (num_ivs+1), num_inputs);

			stop_timer();
			cout << "Do you want to use randomly generated input vectors?  ";
			if (decision()) {
				cout << "Use gray code random vectors (faster but less random)?  ";
				if (decision()) {
					cout << "How many bits do you want to be changed between input vectors (1 to " << num_inputs/2 << ")?  ";
					num_permutations = inpg0();
					num_permutations = min(num_permutations, (unsigned)num_inputs/2);
					iv_mode = 2;
				}
				start_timer();
				random_seed2 = get_time();
				random_vectors(global_random_seed, random_seed2, num_ivs, num_inputs, input_vectors, (iv_mode == 2));	
			}
			else {
				get_input_vectors(input_vectors, num_ivs, num_inputs);
			}
			start_timer();
		}
		else {
			binx_alloc(input_vectors, 1, num_inputs);
			iv_mode = 1;
		}
	}
	if (num_ivs == 0 || iv_mode == 1) {
		input_vectors[0].set_to_dec(0);
	}
	if (num_inputs > MAX_WARNING_INPUTS) {
		cerr << "\nWarning: " << num_inputs << " is a very large number of inputs, so the simulation may take awhile.";
	}
	return 1;
}




void binx_alloc(BINX *&binx_arr, int size, int nbits) {

	binx_arr = new BINX[size];

	if (binx_arr == NULL) {
		out_of_memory();
		return;
	}
	for (int i = 0; i < size; ++i) {
		binx_arr[i].init(nbits);
	}
}




int read_input_vector_file(BINX *&input_vectors, int num_inputs) {

	int num_ivs, nbits, num_outputs;
	unsigned curr_iv;
	ifstream infile;
	BINX BINARY(num_inputs);
	char filename[MAX_SAVE_CHARACTERS + 1] = {0};

	cout << "\nOpen input file named:  ";
	cin  >> ws;
	strcpy(filename, xin(MAX_SAVE_CHARACTERS));

	if (!infile_file(infile, filename)) {
		beep();
		cerr << "\nThe input file " << filename << " could not be opened!" << endl;
		return 0;
	}
	if (!filestream_check(infile)) {
		return 0;
	}
	if (!(infile >> num_ivs >> nbits >> num_outputs)) {
		cerr << "Error reading vector count, input bits, and output bits from first line of file." << endl;
		return 0;
	}
	if (num_ivs < 1) {
		cout << "The input file contains no input vectors." << endl;
		return 0;
	}
	if (nbits < 1) {
		cerr << "Error: No input bits." << endl;
		return 0;
	}
	if (nbits != num_inputs) {
		cerr << "Error: Bit count in input file disagrees with the number of inputs: " << nbits << " vs. " << num_inputs << "." << endl;
		return 0;
	}
	if (num_ivs > MAX_WARNING_IVS) {
		cout << "Warning: " << num_ivs << " is a very large number of input vectors, so the simulation may take awhile." << endl;
	}
	start_timer();
	binx_alloc(input_vectors, (num_ivs+1), num_inputs);
	BINARY.set_to_dec(0); // may not be necessary

	for (curr_iv = 0; (int)curr_iv < num_ivs; ++curr_iv) {
		if (!read_bits_from_file(infile, BINARY, num_inputs, num_outputs)) {
			infile.close();
			return curr_iv;
		}
		process_iv(input_vectors, BINARY, num_ivs, num_inputs, curr_iv);
	}
	infile.close();
	stop_timer();

	return num_ivs;
}




void get_input_vectors(BINX *input_vectors, int num_ivs, int num_inputs) {

	unsigned curr_iv;
	BINX BINARY(num_inputs);

	BINARY.set_to_dec(0); // may not be necessary

	cout << "\nEnter the input vectors in the form of " << num_inputs << " ones, zeros, X's for don't cares, or '-'s for both 0 and 1: " << endl;
	
	for (curr_iv = 0; (int)curr_iv < num_ivs; ++curr_iv) {
		inb(BINARY, num_inputs);
		process_iv(input_vectors, BINARY, num_ivs, num_inputs, curr_iv);
	}
}




void process_iv(BINX *input_vectors, BINX &bits, int num_ivs, int num_inputs, unsigned &curr_iv) {

	unsigned i, j, k, num_01(0);
	BINX vectorx(num_inputs);

	input_vectors[curr_iv] = bits;

	for (i = 0; (int)i < num_inputs; ++i) {
		if (bits.get_bit(i) == -2) {
			++num_01;
		}
	}
	if (num_01 > 0) {
		for (i = 0; i < ((unsigned)1 << num_01); ++i) {
			if ((int)curr_iv >= num_ivs) {
				cout << "\nExceeded input vector count." << endl;
				break;
			}
			vectorx.set_to_dec(i);
			input_vectors[curr_iv] = bits;
																		
			for (k = 0, j = 0; (int)j < num_inputs; ++j) {
				if (bits.get_bit(j) == -2) {
					input_vectors[curr_iv].set_bit(j, vectorx.get_bit(k));
					++k;
				}
			}
			++curr_iv;
		}
		--curr_iv;
	}
}




void random_vectors(long rseed1, long rseed2, int num_vectors, int num_bits, BINX *input_vectors, int gen_seed_vector) {

	int more_than_half(0);
	unsigned long gen_vect, num_vals, *rand_vects = NULL;
	unsigned int curr_iv, i, num_ints, *rand_digits = NULL;
	double random_num;

	num_vals = (1 << min(num_bits, (BITS_PER_INT-1)));
	
	if (num_bits == 0 || num_vectors == 0 || num_vectors >= (int)num_vals)
		return;

	if (gen_seed_vector || num_bits > MAX_IN_PER_INT) {
		num_ints   = (unsigned)ceil((double)num_bits/(double)BITS_PER_INT); // about num_bits/32
		rand_digits = memAlloc(rand_digits, num_ints);

		if (gen_seed_vector) {
			num_vectors = 1;
		}
		for (curr_iv = 0; (int)curr_iv < num_vectors; ++curr_iv) {
			for (i = 0; i < num_ints; ++i) {
				randome(rseed1, rseed2, random_num);
				rand_digits[i] = (unsigned)(2*random_num*num_vals);
			}
			input_vectors[curr_iv].set_to_dec(rand_digits, num_ints);
		}
		return;
	}
	GridArray vectors;

	if (num_vectors > int(num_vals >> 1)) {
		num_vectors = num_vals - num_vectors;
		more_than_half = 1;
	}
	else {
		rand_vects = memAlloc(rand_vects, num_vectors);
	}
	if (!vectors.create(0, HASHTABLE, -1, num_vectors)) {
		out_of_memory();	
	}
	for (curr_iv = 0; (int)curr_iv < num_vectors; ++curr_iv) {
		do {
			randome(rseed1, rseed2, random_num);
			gen_vect = (unsigned long)(random_num*num_vals);
		} while (vectors[gen_vect] != -1);
		
		vectors[gen_vect] = gen_vect;
		
		if (!more_than_half) {
			rand_vects[curr_iv] = gen_vect;
		}
	}	
	if (!more_than_half) {
		qsort((void *)rand_vects, num_vectors, sizeof(long), compare_array);
		
		for (curr_iv = 0; (int)curr_iv < num_vectors; ++curr_iv) {
			input_vectors[curr_iv].set_to_dec(rand_vects[curr_iv]);
		}
		delete [] rand_vects;
	}
	else {
		for (i = 0, curr_iv = 0; (int)curr_iv < num_vals; ++curr_iv) {
			if (vectors[curr_iv] == -1) {
				input_vectors[i++].set_to_dec(curr_iv);
			}
		}
	}	
	vectors.destroy();
}




void random_permutation(BINX &iv, int &last_permute, unsigned num_permutations) {

	unsigned i, nbits, permute_bit;
	long rseed2;
	double random_num;

	nbits  = iv.get_num_bits();
	rseed2 = get_time();

	for (i = 0; i < num_permutations; ++i) {
		do {
			randome(global_random_seed, rseed2, random_num);
			permute_bit = (unsigned)(random_num*nbits);

		} while (permute_bit == last_permute); // make sure we don't switch the same bit back

		iv.toggle_bit(permute_bit);
		last_permute = permute_bit;
	}
}




int read_bits_from_file(ifstream &infile, BINX &BINARY, int nbits, int num_outputs) {

	int is_ok(1);
	unsigned i;
	char c;
	string str;

	if (!infile.good() || !(infile >> str)) {
		cerr << "Error reading bits from file." << endl;
		return 0;
	}
	if (str.length() != nbits) {
		cerr << "Error: Expecting " << nbits << " bits in input line but got " << str.length() << "." << endl;
		return 0;
	}
	for (i = 0; (int)i < nbits; ++i) {
		if (!valid_binary(str[i], allow_dontcare)) {
			is_ok = 0;
			break;
		}
	}
	for (i = 0; (int)i < num_outputs; ++i) {
		infile >> c; // read and ignore output bits
	}
	if (!is_ok) {
		cout << "\nError: Invalid input bit: " << str[i] << "." << endl;
		return 0;
	}
	chars_to_binary(BINARY, str.c_str(), nbits);

	return 1;
}




void inb(BINX &BINARY, int nbits) { 
	
	int is_ok(1);
	unsigned i;
	char *input = NULL;

	input = memAlloc(input, nbits+2);
	
	do {
		strcpy(input, xin(nbits + 2));
		is_ok = 1;

		if (input[nbits] != 0)
			is_ok = 0;
		
		for (i = 0; (int)i < nbits && is_ok; ++i) {
			if (!valid_binary(input[i], allow_dontcare)) {
				is_ok = 0;
				break;
			}
		}
		if (!is_ok) {
			beep();
			cout << "\nInvalid input, please enter only " << nbits << " 1s, 0s, and Xs:  ";
		}
	} while (!is_ok);
	
	chars_to_binary(BINARY, input, nbits);
	delete [] input;
}




void chars_to_binary(BINX &BINARY, const char *bitstring, int nbits) {

	unsigned i;

	for (i = 0; (int)i < nbits; ++i) {
		switch (bitstring[i]) {
			case 0: // NULL termination
				return;
			case '0':
				BINARY.set_bit(i, 0);
				break;
			case '1':
				BINARY.set_bit(i, 1);
				break;
			case '-':
				BINARY.set_bit(i, -2);
				break;
			default:
				BINARY.set_bit(i, 2);
		}
	}
}


inline int valid_binary(char bin, int dontcare_ok) {

	switch (bin) {
	case '0':
	case '1':
		return 1;
	case '2':
	case '-':
	case 'x':
	case 'X':
	case 'd':
	case 'D':
		return dontcare_ok;
	}
	return 0;
}




