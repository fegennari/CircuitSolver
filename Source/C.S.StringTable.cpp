#include "CircuitSolver.h"
#include "StringTable.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.GridArray.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver StringTable Class Member Functions (string hashtable)
// by Frank Gennari
int use_ic_stringtable(1), use_q_stringtable(1);



unsigned int hash_string(const string &str, int tsize) {

	unsigned int i, hashval(0), length(str.length());
	
	for (i = 0; i < length; ++i) {
		hashval = ((hashval << 8) + str[i])%tsize;
	}		
	return hashval;
}


/*class StringTable {

	public:
		StringTable(unsigned int table_size_index = 0);
		~StringTable();
		int insert(const string &input, int value);
		int update(const string &input, int value);
		int search(const string &input);
		int remove(const string &input);
		int search_and_insert(const string &input, int val);
		unsigned get_num_strings();
		string  find_string(int val);
		string *make_string_array(unsigned &arr_size);
		void destroy();
		
	private:
		void init_table(unsigned int table_size_index);
		void rehash();
		
		unsigned int table_size, num_strings, prime_index;
		int max_val;
		stringlist **slist; 
};*/



// StringTable class member functions
StringTable::StringTable(unsigned int table_size_index) {

	init_table(table_size_index);	
}




void StringTable::init_table(unsigned int table_size_index) {

	num_strings      = 0;
	max_val          = -1;
	table_size_index = min(table_size_index, NUM_PRIMES);
	prime_index      = table_size_index;
	table_size       = PRIMES[table_size_index];
	
	slist = memAlloc(slist, table_size);
	if (slist == NULL)
		return;

	for (unsigned int i = 0; i < table_size; ++i) {
		slist[i] = NULL;
	}
}




StringTable::~StringTable() {
	
	for (unsigned int i = 0; i < table_size; ++i)
		destroyL(slist[i]);
		
	delete [] slist;
	table_size = num_strings = prime_index = 0;
}




int StringTable::insert(const string &input, int value) { // returns 0 if already exists

	unsigned int index;
	stringlist *new_str;
			
	if (search(input) != default_st_val)
		return 0; // duplicate entry
		
	new_str = new stringlist;
	if (new_str == NULL) {
		out_of_memory();
		return -1;
	}
	if (num_strings/table_size > max_sload_factor) // resize / rehash
		rehash();
		
	index = hash_string(input, table_size);
	++num_strings;
	new_str->value = value;
	new_str->str   = input;
	new_str->next  = slist[index];
	slist[index]   = new_str;
	max_val        = max(max_val, value);
		
	return 1;
}




void StringTable::rehash() {

	unsigned int i, index, old_table_size(table_size);
	stringlist *head, *temp, **slist2 = NULL;
		
	if (prime_index < NUM_PRIMES)
		++prime_index;
		
	table_size = PRIMES[prime_index];
		
	slist2 = memAlloc(slist2, table_size);	// create new, larger hashtable
	if(slist2 == NULL) 
		return;

	for (i = 0; i < table_size; ++i)
		slist2[i] = NULL;

	for (i = 0; i < old_table_size; ++i) { // copy old hashtable into array
		head = slist[i];	
		while (head != 0) {
			index = hash_string(head->str, table_size);
			temp  = head->next;
			head->next    = slist2[index];
			slist2[index] = head;
			head = temp;
		}
	}	
	delete [] slist;
	slist = slist2;
}




int StringTable::update(const string &input, int value) { // updates value, returns 1 if updated and 0 if not found

	unsigned int index;
	stringlist *head;
	
	if (table_size == 0)
		return 0;
		
	index = hash_string(input, table_size);
	head  = slist[index];
		
	while (head != 0) {
		if (head->str == input) {
			head->value = value;
			max_val     = max(max_val, value);
			return 1;
		}
		head = head->next;
	}	
	return 0; // not found
}




int StringTable::search(const string &input) {

	unsigned int index;
	stringlist *head;
	
	if (table_size == 0)
		init_table(0);
		
	index = hash_string(input, table_size);
	head  = slist[index];
		
	while (head != 0) {
		if (head->str == input)
			return head->value;
		head = head->next;
	}	
	return default_st_val; // not found
}



// doesn't update max_val
int StringTable::remove(const string &input) {

	unsigned int index;
	stringlist *temp, *head;
	
	if (table_size == 0)
		return 0;
		
	index = hash_string(input, table_size);
	head  = slist[index];
	
	if (head == 0)
		return 0;
		
	if (head->str == input) {
		temp = head->next;
		delete head;
		slist[index] = temp;
		--num_strings;
		return 1;
	}
	temp = head;
	head = head->next;
	
	while (head != 0) {
		if (head->str == input) {
			temp->next = head->next;
			delete head;
			--num_strings;
			return 1;
		}
		temp = head;
		head = head->next;
	}	
	return 0; // not found
}




int StringTable::search_and_insert(const string &input, int val) {

	int search_val = search(input);

	if (search_val == default_st_val) {
		insert(input, val);
		return val;
	}
	return search_val;
}




string StringTable::find_string(int val) {

	if (num_strings == 0 || val > max_val) {
		return (string)"";
	}
	unsigned i;
	stringlist *head;

	for (i = 0; i < table_size; ++i) {
		head = slist[i];
		while (head != NULL) {
			if (head->value == val) {
				return head->str;
			}
			head = head->next;
		}
	}
	return (string)"";
}



// doesn't work for negative values
string *StringTable::make_string_array(unsigned &arr_size) {

	if (num_strings == 0 || table_size == 0 || max_val < 0) {
		arr_size = 0;
		return NULL;
	}
	int val;
	unsigned i;
	string *str_arr = NULL;
	stringlist *head;
	char temp_str[20] = {0};

	arr_size = (unsigned)(max_val+1);
	str_arr  = memAlloc(str_arr, arr_size);

	for (i = 0; i < arr_size; ++i) {
		str_arr[i] = ""; // default NULL string
	}
	for (i = 0; i < table_size; ++i) {
		head = slist[i];
		while (head != NULL) {
			val = head->value;
			if (val >= 0 && val <= max_val) { // val should always be <= max_val
				str_arr[val] = head->str;
			}
			head = head->next;
		}
	}
	for (i = 0; i < arr_size; ++i) {
		if (str_arr[i] == "") {
			sprintf(temp_str, "_cs_temp_%i", i);
			str_arr[i] = (string)temp_str;
		}
	}
	return str_arr;
}




void StringTable::destroy() {

	if (table_size > 0) {
		for (unsigned int i = 0; i < num_strings; ++i) 
			destroyL(slist[i]);
		if (slist != NULL) {
			delete [] slist;
			slist = NULL;
		}
	}
	table_size  = 0;
	num_strings = 0;
	prime_index = 0;
}








