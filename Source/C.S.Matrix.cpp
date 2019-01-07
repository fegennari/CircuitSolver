#ifndef _MATRIX_CC_
#define _MATRIX_CC_


// Because of the templates, this file works like a header file and is not
// included in the project.
//#include "CircuitSolver.h"
#include "Complex.h"
#include "Matrix.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Matrix.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Matrix Functions - Templates
// by Frank Gennari
extern int dimension_error, inconsistent, arbitrary, USE_FAST_BACK_SUBSTITUTE;
extern double min_scale;





// All templated functions work with both doubles and complex numbers.
template<class T> T **create_matrix(T **matrix, int m, int n) {
	
	matrix = memAlloc(matrix, m);

	for (int i = 0; i < m; ++i) {
		matrix[i] = memAlloc(matrix[i], n);
		for (int j = 0; j < n; ++j) {
			zero(matrix[i][j]);
		}
	}
	return matrix;	
}





template<class T> void delete_matrix(T **matrix, int m) {

	if (matrix == NULL) {
		return;
	}
	for (int i = 0; i < m; ++i) {
		delete [] matrix[i];
	}
	delete [] matrix;
}




template<class T> const void print_matrix(T **matrix, int m, int n) {

	T elem;

	if (matrix == NULL) {
		return;
	}
	for(int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			elem = matrix[i][j];
			cout << elem<< "\t";
			//if (matrix[i][j] == 0)
				//cout << "\t";
		}
		cout << endl;
	}
}




template<class T> void identity(T **matrix, int m, int n) {
		
	if (m != n)
		dimension_error = 1;
	if (matrix == NULL) {
		return;
	}
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			if (i == j)
				one(matrix[i][j]);
			else
				zero(matrix[i][j]);
		}
	}
}





template<class T> void number(T **matrix, int m, int n) {
	
	if (matrix == NULL) {
		return;
	}
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = (i + 1) + 10*(j + 1);	
		}
	}
}





template<class T> void zero(T **matrix, int m, int n) {
	
	if (matrix == NULL) {
		return;
	}
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			zero(matrix[i][j]);
		}
	}
}





template<class T> T **add(T **matrix1, T **matrix2, int m, int n) {

	T **matrix = NULL;
	
	matrix = create_matrix(matrix, m, n);
	
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = matrix1[i][j] + matrix2[i][j];
		}
	}
	return matrix;
}





template<class T> T **subtract(T **matrix1, T **matrix2, int m, int n) {

	T **matrix = NULL;
	
	matrix = create_matrix(matrix, m, n);
	
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = matrix1[i][j] - matrix2[i][j];
		}
	}
	return matrix;
}





template<class T> T **multiply(T **matrix1, T **matrix2, int m, int n, int s, int t) {

	T **matrix = NULL;
	T sum;
	
	matrix = create_matrix(matrix, m, t);
	
	if (n != s) {
		zero(matrix, m, t);
		dimension_error = 1;
		return matrix;
	}
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < t; ++j) {
			zero(sum);
			for (int k = 0; k < n; ++k) {
				sum = sum + matrix1[i][k]*matrix2[k][j];
			}
			matrix[i][j] = sum;
		}
	}
	return matrix;
}





template<class T> T **multiply(T **matrix1, T k, int m, int n) {

	T **matrix = NULL;
	
	matrix = create_matrix(matrix, m, n);
	
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = k*matrix1[i][j];
		}
	}
	return matrix;
}





template<class T> void multiply(T **matrix1, T *matrix2, T *matrix3, int m, int n) {
	
	T sum;
	
	for (int i = 0; i < m; ++i) {
		zero(sum);
		for (int j = 0; j < n; ++j) {
			sum = sum + matrix1[i][j]*matrix2[j];
		}
		matrix3[i] = sum;	
	}				
}




template<class T> T matrix_det(T **matrix, int m, int n) {
	
	int i;
	T ***matrix_data = NULL, result;

	if (m != n) {
		dimension_error = 1;
		cerr << "Error in determinant of (" << m << " x " << n << ") matrix: Matrix must be square." << endl;
		zero(result);
		return result;
	}
	matrix_data    = memAlloc(matrix_data, m);
	matrix_data[0] = matrix_data[1] = NULL;

	for (i = 2; i < m; ++i) {
		matrix_data[i] = create_matrix(matrix_data[i], i, i);
	}
	result = matrix_det_recur(matrix, matrix_data, m);

	for (i = 2; i < m; ++i) {
		delete_matrix(matrix_data[i], i);
	}
	delete [] matrix_data;

	return result;
}




template<class T> T matrix_det_recur(T **matrix, T ***matrix_data, int size) {

	int i, j, k, l;
	double mult(1.0);
	T **matrix2 = NULL, *matrix2_l, *matrix_j, result;
	
	if (size == 0) {
		zero(result);
		return result;
	}
	if (size == 1) {
		return matrix[0][0];
	}
	if (size == 2) {
		return matrix[0][0]*matrix[1][1] - matrix[1][0]*matrix[0][1];
	}
	zero(result);
	matrix2 = matrix_data[size-1];

	for (i = 0; i < size; ++i) {
		for (j = 0, l = 0; j < size; ++j) { // rows
			matrix_j = matrix[j];
			if (i != j) {
				matrix2_l = matrix2[l];
				for (k = 0; k < size-1; ++k) { // columns
					matrix2_l[k] = matrix_j[k+1];
				}
				++l;
			}
		}
		result = result + mult*matrix[i][0]*matrix_det_recur(matrix2, matrix_data, size-1);
		mult   = -1.0*mult;
	}
	return result;
}




template<class T> T **transpose(T **matrix, int m, int n) {

	int i, j;
	T **matrixT = NULL;

	if (matrix == NULL) {
		return matrixT;
	}
	matrixT = create_matrix(matrixT, n, m);

	for (i = 0; i < m; ++i) {
		for (j = 0; j < n; ++j) {
			matrixT[j][i] = matrix[i][j];
		}
	}
	return matrixT;
}





template<class T> T **augument(T **matrix, T **A, T *b, int m, int n) {
	
	if (matrix == NULL) {
		matrix = create_matrix(matrix, m, n+1);
	}
	for (int i = 0; i < m; ++i) {
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = A[i][j];
		}
	}
	for (int k = 0; k < m; ++k) {
		matrix[k][n] = b[k];
	}
	return matrix;
}





template<class T> T **AI(T **A, int m, int n) {

	T **ai = NULL;
	int i, j, c(2*n);
	
	ai = create_matrix(ai, m, c);
	
	if (m != n) {
		zero(ai, m, c);
		dimension_error = 1;
		return ai;
	}
	for (i = 0; i < m; ++i) {
		for (j = 0; j < n; ++j) {
			ai[i][j] = A[i][j];
		}
		for (j; j < c; ++j) {
			if (i == (j-n)) {
				one(ai[i][j]);
			}
			else {
				zero(ai[i][j]);
			}
		}
	}
	return ai;
}





template<class T> T **split_AI(T **ai, int m, int n) {

	T **inv = NULL;
	int i, j, c(n/2);
	
	inv = create_matrix(inv, m, c);
	
	if (m != c) {
		zero(inv, m, c);
		dimension_error = 1;
		return inv;
	}
	for (i = 0; i < m; ++i) {
		for (j = c; j < n; ++j) {
			inv[i][j - c] = ai[i][j];
		}
	}
	return inv;
}



template<class T> T *gauss_solve(T **matrix, T *b, T *x, int m, int n) {

	T **matrixaug = NULL;
	int c(n + 1), mn(m*n);

	static T** mt = NULL;
	static int m2 = 0, n2 = 0;

	USE_FAST_BACK_SUBSTITUTE = 1;

	if (mn < SPECIAL_ALLOC_MAX_SIZE) { // small matrix
		if (m != m2 || n != n2) { // size is different from last time
			if (mt != NULL) {
				delete_matrix(mt, m2);
			}
			matrixaug = create_matrix(matrixaug, m, n+1);
			mt = matrixaug;
			m2 = m;
			n2 = n;
		}
		else {
			matrixaug = mt; // use previous matrix
		}
	}
	else {
		if (mt != NULL) { // make sure to delete previous matrix
			delete_matrix(mt, m2);
			mt = NULL;
		}
		m2 = 0;
		n2 = 0;
		matrixaug = create_matrix(matrixaug, m, n+1);
	}
	matrixaug = augument(matrixaug, matrix, b, m, n);	
	matrixaug = gauss_eliminate(matrixaug, m, c);
	
	for(int i = 0; i < m; ++i) {
		x[i] = matrixaug[i][n];
	}
	if (mn >= SPECIAL_ALLOC_MAX_SIZE) {
		delete_matrix(matrixaug, m);
	}
	USE_FAST_BACK_SUBSTITUTE = 0;

	return x;
}





template<class T> T **gauss_eliminate(T **matrix, int m, int n) {
	
	int i, j, k, p, q;
	T num, num2, num3, min, largest = 0, matrix_element, *matrix_i, *matrix_k;
	char less_than_min;

	for (i = 0; i < m; ++i) {  // find the largest coefficient and use it to calculate
		matrix_i = matrix[i];  // the maximum error and minimum useable matrix element
		for (j = 0; j < n; ++j)  
			largest = max(fabs(matrix_i[j]), largest);
	}				
	min = minimum*min_scale*largest; // numbers less than min are thought of as zeros
	
	for (i = 0; i < m; ++i) { // swap rows to establish nonzeros on the main
		matrix_i       = matrix[i]; // diagonal if posssible
		matrix_element = matrix_i[i];
		less_than_min  = (matrix_element <= min && matrix_element >= -min);

		if (i < m-1) { // not the last element
			if (less_than_min) {
				for (j = (i + 1); j < m; ++j) {   
					if (matrix[j][i] >= min || matrix[j][i] <= -min) {      
						swap(matrix[i], matrix[j]); // threshold partial pivoting
						matrix_i       = matrix[i];
						matrix_element = matrix_i[i];
						less_than_min  = 0;
						break;
					}
				}  
			}
			if (matrix_element != 0) {			           // if successful, use the 
				for (k = (i + 1); k < m; ++k) {		       // top row to eliminate
					matrix_k = matrix[k];                  // numbers not on the main
					num      = matrix_k[i]/matrix_element; // diagonal from lower rows
					if (num != 0) {
						matrix_k[i] = 0.0;
						for (p = (i + 1); p < n; ++p)
							matrix_k[p] -= matrix_i[p]*num;
					}
				}
			}
		}
		if (less_than_min && !inconsistent) {
			USE_FAST_BACK_SUBSTITUTE = 0;
			if (m == n)           // test to see if a number on the main diagonal equals 0
				inconsistent = 1; // if so, matrix is singular
			else {
				for (q = m; q < n; ++q) {
					if (matrix_i[q] >= min || matrix_i[q] <= -min) {
						inconsistent = 1;
						break;
					}
				}
			}
			if (!inconsistent)  // if the bottom row is zeros, the last variable is arbitrary
				arbitrary = 1; 	// if not, the matrix is inconsistent
		}
	}
	for (i = (m-1); i >= 0; --i) {  // back-substitute the lower row into the row above it
		matrix_i = matrix[i];

		if (/*m == n &&*/ USE_FAST_BACK_SUBSTITUTE) {
			num2 = 0.0;
			for (j = i+1; j < m; ++j) {
				num2 += matrix[j][n-1]*matrix_i[j];
			}
			matrix_i[n-1] = (matrix_i[n-1] - num2)/matrix_i[i];
		}
		else {
			num2 = matrix_i[i];
			if (num2 != 0) {
				for (j = i; j < n; ++j)	
					matrix_i[j] /= num2; 		 // divide the row by the value of the main
				for (k = (i - 1); k >= 0; --k) { // diagonal to establish ones (or zeros) on
					matrix_k = matrix[k];        // the main diagonal
					num3     = matrix_k[i];            
					if (num3 != 0) {		
						for (p = i; p < n; ++p) 
							matrix_k[p] -= matrix_i[p]*num3;
					}
				}
			}
		}
	}
	for(i = 0; i < m; ++i) {
		matrix_i = matrix[i];
		for (j = 0; j < (n + 1); ++j) {
			if (matrix_i[j] == -0) // doesn't know that -0 = 0, and this causes problems
				matrix_i[j] = 0;
		}
	}			
	if (inconsistent && arbitrary)
		arbitrary = 0; // cannot be both inconsistent and arbitrary
		
	return matrix;
}




template<class T> T **inverse(T **matrix, int m, int n) {
	
	T **ai, **inv;
	int c(2*n);

	if (m != n) {
		dimension_error = 1;
	}
	ai  = AI(matrix, m, n);
	ai  = gauss_eliminate(ai, m, c);
	inv = split_AI(ai, m, c);
	delete_matrix(ai, m);
	
	return inv;
}



template<class T> int square_inverse_no_alloc(T **matrix, T **ai, T **inv, int n) {

	int i, j, c(2*n);

	if (matrix == NULL || ai == NULL || inv == NULL || n <= 0) {
		return 0;
	}
	for (i = 0; i < n; ++i) {
		for (j = 0; j < n; ++j) {
			ai[i][j] = matrix[i][j];
		}
		for (j; j < c; ++j) {
			if (i == (j-n)) {
				one(ai[i][j]);
			}
			else {
				zero(ai[i][j]);
			}
		}
	}
	ai = gauss_eliminate(ai, n, c);
	
	for (i = 0; i < n; ++i) {
		for (j = n; j < c; ++j) {
			inv[i][j - n] = ai[i][j];
		}
	}
	return 1;
}


// similar to Gaussian elimination
template<class T> T **lu_decomp(T **matrix, int m, int n) {

	// *** WRITE ***

	return matrix;
}




inline void zero(complex &matrix) {

	matrix.Im = matrix.Re = 0;
}


inline void zero(double &matrix) {

	matrix = 0;
}


inline void one(complex &matrix) {

	matrix.Im = matrix.Re = 1;
}


inline void one(double &matrix) {

	matrix = 1;
}


#endif






