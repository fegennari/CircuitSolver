// This file contains general purpose functions that are frequently used by ACCPM
// Written by:    Hector Mora and Olivier Péton
//                Logilab, HEC Geneva, Section of Management Studies, 
//                40, Bd du pont d'Arve, 1211 Geneva, Switzerland.
// Last modified: november 29, 1999
//---------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> 
#include <time.h>

// This file contains the following routines :
// odd:        determines if a number is odd(1) or even (0)
// max:        returns the max of two integers
// maxd:       returns the maximum of two doubles
// scalar:     returns the scalar product of two double vectors with specified dimension
// scalar_int: idem with integer vectors
// norm2:      euclidian norm of a vector
// dist2:      euclidian distance between two points
// displayVector:    displays vector on two columns
// display_i :       idem with integer vectors




//----------------------------------------------------------
// MATHEMATICAL FUNCTIONS
//----------------------------------------------------------

// determines if a number is odd
int odd(int n)
{
	int o = (n>0 && int(n/2) == int((n-1)/2));
	return o;
}

// maximum of two integers 
int max(int a, int b)
{
	if (a>b) return a;
	else return b;
}

// maximum of two doubles
double maxd(double a, double b)
{
	if (a>=b) return a;
	else return b;
}


// scalar product (with double vectors)
double scalar(double *x1, double *x2, int n)
{
    double *px1,*px2, *px9, sum=0.0;

    if( n <= 0 ) return 0.0;
    px9 = x1 + n-1;
    for( px1 = x1, px2 = x2; px1 <= px9; px1++, px2++ ) sum += (*px1)*(*px2);
	return sum;
}

// scalar product (with integer vectors)
double scalar_int( double *x1, int *x2, int n)
{
     int *px2;
	 double *px1, *px9, sum=0.0;

    if( n <= 0 ) return 0.0;
    px9 = x1 + n-1;
    for( px1 = x1, px2 = x2; px1 <= px9; px1++, px2++ ) sum += (*px1)*(*px2);
	return sum;
}



// Euclidean norm
double norm2( double *x, int n)
{
	double *p, *p9, sum2 = 0., xi;

	p9 = x+n-1;
	for( p=x; p<=p9; p++){
		xi = *p;
		sum2 += xi*xi;
	}
	return sqrt(sum2);
}


// Euclidean distance
double dist2( double *x, double *y, int n)
{
	double *p, *pp, *p9, sum2 = 0., xiyi;

	p9 = x+n-1;
	for( p=x, pp=y; p<=p9; p++, pp++){
		xiyi = *p - *pp;
		sum2 += xiyi*xiyi;
	}
	return sqrt(sum2);
}


// DISPLAY FUNCTIONS

// writes a vector with two columns
void displayVector(char *var, double *y, int d)
{
	int z;
	char key;
	printf("\n");
	for (z=0; z<d; z++)
	{
		printf(" %s[%i] = %10.8f   ",var,z,y[z]);
		if (odd(z)) printf("\n");
		if (floor((z+1)/20) > floor(z/20)) scanf("%c",&key);
	}
}

void display_i(char *var, int *a, int d)
{
	int z;
	char key;
	printf("\n");
	for (z=0; z<d; z++)
	{
		printf(" %s[%i] = %i   ",var,z,a[z]);
		if (odd(z)) printf("\n");
		if (floor((z+1)/20) > floor(z/20)) scanf("%c",&key);
	}
}
