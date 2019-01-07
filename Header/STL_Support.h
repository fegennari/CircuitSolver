#ifndef _STL_SUPPORT_H_
#define _STL_SUPPORT_H_


#include <cmath>
#include <fstream>

// necessary for MS Visual C++
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <ctime>
#include <algorithm>


#ifdef CS_SET_NAMESPACE
using namespace std; // need for MS Visual C++
#endif


#include <vector>

// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, STL_Support.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver STL/vector supporting header
// By Frank Gennari
template<class T> void vector_delete(vector<T> &vect);
template<class T> void vector_delete_2d(vector<vector<T> > &vect2d);
template<class T> void array_vector_delete(vector<T> *&arr_vect, unsigned size);



template<class T> void vector_delete(vector<T> &vect) {

  vect.erase(vect.begin(), vect.end());
}


template<class T> void vector_delete_2d(vector<vector<T> > &vect2d) {

  //for (unsigned i = 0; i < vect2d.size(); ++i) {
    //vector_delete(vect2d[i]);
  //}
  vector_delete(vect2d);
}


template<class T> void array_vector_delete(vector<T> *&arr_vect, unsigned size) {

	//for (unsigned i = 0; i < size; ++i) {
		//vector_delete(arr_vect[i]);
	//}
	if (arr_vect != NULL && size > 0) {
		delete [] arr_vect;
	}
}


#endif

