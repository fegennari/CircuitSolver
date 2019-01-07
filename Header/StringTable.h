#ifndef _STRING_TABLE_H_
#define _STRING_TABLE_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, StringTable.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver String Table: Hash Table for strings header
// By Frank Gennari
#define default_st_val -1


int const init_stable_size = 10;
float const max_sload_factor = 2.0;


unsigned int hash_string(const string &str, int tsize);




struct stringlist {

	int value;
	string str;
	stringlist *next;
};





class StringTable {

	public:
		StringTable(unsigned int table_size_index = 0);
		~StringTable();
		int insert(const string &input, int value);
		int update(const string &input, int value);
		int search(const string &input);
		int remove(const string &input);
		int search_and_insert(const string &input, int val);
		inline unsigned get_num_strings() {return num_strings;};
		string  find_string(int val);
		string *make_string_array(unsigned &arr_size);
		void destroy();
		
	private:
		void init_table(unsigned int tabls_size_index);
		void rehash();
		
		unsigned int table_size, num_strings, prime_index;
		int max_val;
		stringlist **slist; 
};




#endif
