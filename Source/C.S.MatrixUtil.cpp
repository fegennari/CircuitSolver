#include "Complex.h"
#include "Matrix.h"


// Circuit Solver is copyright © 1998-2002 by Frank Gennari.
// Circuit Solver, C.S.MatrixUtil.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Matrix Functions and Utilities
// by Frank Gennari
extern int function_id, useDIALOG;
extern double min_scale;


int dimension_error, inconsistent, arbitrary, USE_FAST_BACK_SUBSTITUTE = 0;




void solve_matrix() {
	
	double **matrix = NULL, **matrixinv;
	double *b = NULL, *x = NULL;
	complex **matrixC = NULL, **matrixinvC = NULL;
	complex *bC = NULL, *xC = NULL;
	complex det_c;
	int i, m, n, calcINV(0), useCOMPLEX(0);
	
	function_id = 22;	
	dimension_error = inconsistent = arbitrary = 0;
	
	cout << "\nWould you like to work with a matrix of complex numbers?  ";
	if (decision())
		useCOMPLEX = 1;
	
	cout << "\nEnter the number of equations:  ";
	m = inp();
	
	cout << "\nEnter the number of variables:  ";
	n = inp();
	
	if (m == 0 || n == 0)
		return;
	
	if (memory_warning(max(m, n)))
		return;
	
	if (useCOMPLEX)
		matrixC = create_matrix(matrixC, m, n);	
	else
		matrix = create_matrix(matrix, m, n);
	
	cout << "\nEnter the coefficients in matrix format:" << endl;
	if (useCOMPLEX)
		get_matrixC(matrixC, m, n);
	else
		get_matrix(matrix, m, n);
	
	cout << "\nMatrix = " << endl;
	if (useCOMPLEX)
		print_matrix(matrixC, m, n);
	else
		print_matrix(matrix, m, n);
		
	cout << "\nEnter b, the constants:" << endl;
	
	if (useCOMPLEX)
		bC = memAlloc(bC, m);
	else
		b = memAlloc(b, m);
	
	for (i = 0; i < m; ++i) {
		if (useCOMPLEX) {
			bC[i].Re = ind();
			bC[i].Im = ind();
		}
		else
			b[i] = ind();
	}
	cout << "\nb = " << endl;
	
	for (i = 0; i < m; ++i) {
		if (useCOMPLEX) 
			cout << bC[i] << endl;
		else
			cout << b[i] << endl;
	}
	cout << "\nDo you want to calculate the determinant of the coefficient matrix?  ";
	if (decision()) {
		cout << "The determinant of the matrix = ";
		if (useCOMPLEX) {
			det_c = matrix_det(matrixC, m, n);
			cout << det_c;
		}
		else
			cout << matrix_det(matrix, m, n);
		cout << "." << endl;
	}
	cout << "\nDo you want to calculate the inverse of the coefficient matrix?  ";
	calcINV = decision();
	
	if (calcINV) {
		cout << "\nThe inverse of the matrix = " << endl;
	
		if (useCOMPLEX) {
			matrixinvC = inverse(matrixC, m, n);
			show_errors();
			print_matrix(matrixinvC, m, n);
		}
		else {
			matrixinv = inverse(matrix, m, n);
			show_errors();
			print_matrix(matrixinv, m, n);
		}
		cout << "\nPress Enter to continue." << endl;
		xin(1);
	}
	cout << "\nx = " << endl;
	
	if (useCOMPLEX) {
		xC = memAlloc(xC, m);
		xC = gauss_solve(matrixC, bC, xC, m, n);
	}
	else {
		x = memAlloc(x, m);
		x = gauss_solve(matrix, b, x, m, n);
	}
	show_errors();
	
	for (i = 0; i < m; ++i) {
		if (useCOMPLEX)
			cout << xC[i] << endl;
		else
			cout << x[i] << endl;
	}
	if (useCOMPLEX) {
		delete_matrix(matrixC, m);
		if (calcINV)
			delete_matrix(matrixinvC, m);
		delete [] xC;
		delete [] bC;
	}
	else {	
		delete_matrix(matrix, m);
		if (calcINV)
			delete_matrix(matrixinv, m);
		delete [] x;
		delete [] b;
	}
}




int memory_warning(int size) {

	if (size > max_matrix_size) {
		if (useDIALOG) {
			if (memory_error()%2) 
				return 1;
		}
		else {
			cout << "\nWarning: Circuit Solver may not have enough RAM to solve this many equations. Continue?  ";
			if (!decision()) 
				return 1;
		}
	}
	return 0;
}




void get_matrix(double **matrix, int m, int n) {
	
	for (int i = 0; i < m; ++i) {
		cout << "Enter row " << (i + 1) << " of the matrix:" << endl;
		for (int j = 0; j < n; ++j) {
			matrix[i][j] = ind();
		}
	}
}




void get_matrixC(complex **matrix, int m, int n) {
	
	for (int i = 0; i < m; ++i) {
		cout << "Enter row " << (i + 1) << " of the matrix in the form(real, complex):" << endl;
		for (int j = 0; j < n; ++j) {
			matrix[i][j].Re = ind();
			matrix[i][j].Im = ind();
		}
	}
}




void show_errors() {

	if (dimension_error)
		cout << "\nWarning: Matrix or Matrices have incorrect dimensions and the selected operation cannot be performed." << endl;
	
	if (inconsistent)
		cout << "\nThis matrix or system of equations is inconsistent." << endl;
		
	if (arbitrary)
		cout << "\nSome of the solutions of this matrix or system of equations are arbitrary." << endl;
		
	dimension_error = inconsistent = arbitrary = 0;
}



// complex matrices


void get_matrix(complex **matrix, int m, int n) {
	
	for (int i = 0; i < m; ++i) {
		cout << "Enter row " << (i + 1) << " of the matrix(Real, Imaginary):" << endl;
		for (int j = 0; j < n; ++j) {
			matrix[i][j].Re = ind();
			matrix[i][j].Im = ind();
		}
	}
}



complex** gauss_eliminate(complex **matrix, int m, int n) {
	
	int i, j, k, p, q;
	complex num, num2, num3, matrix_element, *matrix_i, *matrix_k;
	double min, largest = 0, mag_element;
	char mag_less_than_min;
	
	for (i = 0; i < m; ++i) { // find the largest coefficient and use it to calculate
		matrix_i = matrix[i]; // the maximum error and minimum useable matrix element
		for (j = 0; j < n; ++j)  
			largest = max(ri_sum(matrix_i[j]), largest); 
	}			
	min = minimum*min_scale*largest; // numbers less than min are thought of as zeros
		
	for (i = 0; i < m; ++i) { // swap rows to establish nonzeros on the main
		matrix_i = matrix[i]; // diagonal if posssible
		matrix_element = matrix_i[i];
		mag_element    = ri_sum(matrix_element);
		mag_less_than_min = (mag_element < min);

		if (i < m-1) { // not the last element
			if (mag_less_than_min) {
				for (j = (i + 1); j < m; ++j) {   
					if (ri_sum(matrix[j][i]) >= min) {      
						swap(matrix[i], matrix[j]); // threshold partial pivoting
						matrix_i          = matrix[i];
						matrix_element    = matrix_i[i];
						mag_element       = ri_sum(matrix_element);
						mag_less_than_min = 0;
						break;
					}
				} 
			}
			if (mag_element != 0) {	                       // if successful, use the 
				for (k = (i + 1); k < m; ++k) {	           // top row to eliminate
					matrix_k = matrix[k];                  // numbers not on the main
					num      = matrix_k[i]/matrix_element; // diagonal from lower rows
					if (num.Re != 0 || num.Im != 0) {
						matrix_k[i].Re = 0.0;
						matrix_k[i].Im = 0.0;
						for (p = (i + 1); p < n; ++p)
							matrix_k[p] = matrix_k[p] - matrix_i[p]*num;
					}
				}
			}
		}
		if (mag_less_than_min && !inconsistent) {
			USE_FAST_BACK_SUBSTITUTE = 0;
			if (m == n)           // test to see if a number on the main diagonal equals 0
				inconsistent = 1; // if so, matrix is singular
			else {
				for (q = m; q < n; ++q) {
					if (ri_sum(matrix_i[q]) < min) {
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
			zero(num2);
			for (j = i+1; j < m; ++j) {
				num2 = num2 + matrix[j][n-1]*matrix_i[j];
			}
			matrix_i[n-1] = (matrix_i[n-1] - num2)/matrix_i[i];
		}
		else {
			num2     = matrix_i[i];
			if (num2.Re != 0 || num2.Im != 0) {
				for (j = i; j < n; ++j)	
					matrix_i[j] = matrix_i[j]/num2; // divide the row by the value of the main
				for (k = (i - 1); k >= 0; --k) {	// diagonal to establish ones (or zeros) on
					matrix_k = matrix[k];           // the main diagonal
					num3     = matrix_k[i];			 
					if (num3.Re != 0 || num3.Im != 0) {
						for (p = i; p < n; ++p)
							matrix_k[p] = matrix_k[p] - matrix_i[p]*num3;
					}
				}
			}
		}
	}
	for(i = 0; i < m; ++i) {
		matrix_i = matrix[i];
		for (j = 0; j < (n + 1); ++j) {
			if (matrix_i[j].Re == -0) // doesn't know that -0 = 0, and this causes problems
				matrix_i[j].Re = 0;
			if (matrix_i[j].Im == -0)
				matrix_i[j].Im = 0;
		}
	}		
	if (inconsistent && arbitrary)
		arbitrary = 0; // cannot be both inconsistent and arbitrary
			
	return matrix;
}




