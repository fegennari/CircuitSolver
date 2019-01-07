#ifndef _MATRIX_H_
#define _MATRIX_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Matrix.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Matrix Header
// By Frank Gennari


#include "CircuitSolver.h"


#define SPECIAL_ALLOC_MAX_SIZE 1000




// All templated functions work with both doubles and complex numbers.
void solve_matrix();
int memory_warning(int size);
template<class T> T **create_matrix(T **matrix, int m, int n);
void get_matrix(double **matrix, int m, int n);
void get_matrixC(complex **matrix, int m, int n);
template<class T> void delete_matrix(T **matrix, int m);
template<class T> const void print_matrix(T **matrix, int m, int n);
template<class T> void identity(T **matrix, int m, int n);
template<class T> void number(T **matrix, int m, int n);
template<class T> void zero(T **matrix, int m, int n);
template<class T> T **add(T **matrix1, T **matrix2, int m, int n);
template<class T> T **subtract(T **matrix1, T **matrix2, int m, int n);
template<class T> T **multiply(T **matrix1, T **matrix2, int m, int n, int s, int t);
template<class T> T **multiply(T **matrix1, T k, int m, int n);
template<class T> void multiply(T **matrix1, T *matrix2, T *matrix3, int m, int n);
template<class T> T matrix_det(T **matrix, int m, int n);
template<class T> T matrix_det_recur(T **matrix, T ***matrix_data, int size);
template<class T> T **transpose(T **matrix, int m, int n);
template<class T> T **augument(T **matrix, T **A, T *b, int m, int n);
template<class T> T **AI(T **A, int m, int n);
template<class T> T **split_AI(T **ai, int m, int n);
template<class T> T *gauss_solve(T **matrix, T *b, T *x, int m, int n);
template<class T> T **gauss_eliminate(T **matrix, int m, int n);
template<class T> T **inverse(T **matrix, int m, int n);
template<class T> int square_inverse_no_alloc(T **matrix, T **ai, T **inv, int n);
template<class T> T **lu_decomp(T **matrix, int m, int n);
void show_errors();
void get_matrix(complex **matrix, int m, int n);

complex** gauss_eliminate(complex **matrix, int m, int n);

inline void zero(complex &matrix);
inline void zero(double &matrix);
inline void one(complex &matrix);
inline void one(double &matrix);


#ifdef USE_CC
#include "../Source/C.S.Matrix.cc"
#else
#include "../Source/C.S.Matrix.cpp"
#endif



#endif


