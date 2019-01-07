# include "CircuitSolver.h"
# include "Complex.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Transforms.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Transforms - FFT
// By Frank Gennari
int const use_fast_transform = 1;


extern int Count, num_points, x_limit, y_limit;
extern double domain, range;
extern point origin, *graph_data;


void conv();
complex *conv(complex *x, complex *y, complex *h, long M, long N);
complex *convfast(complex *x, complex *y, complex *h, long M, long N);
complex *dft(complex *x, complex *X, long N, int inverse);
complex *fft(complex *x, complex *X, long N, int inverse);
void split_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old);
void merge_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old);
void fft_on_graph(int inverse);
long pow2(long x);
int ceil_log2(long N);
long switch_bits(long x, unsigned int n);

void fft(double xr[], double xi[], int nu,int ie);




void conv() {

	complex *X = NULL, *x = NULL, *h = NULL, *y = NULL;
	long N, M, L;
	unsigned long n;
	
	while (1) {
		cout << "\nEnter the number of elements in x(0 to quit):  ";
		M = in();
		
		if (M <= 0)
			return;
		
		X = memAlloc(X, M);
		x = memAlloc(x, M);
		
		cout << "\nEnter the elements in x:  ";
		
		for (n = 0; n < (unsigned long)M; ++n) {	
			x[n].Re = ind();
			x[n].Im = 0;
		} 
		X = dft(x, X, M, 0);
		
		cout << "X: " << endl;
		
		for (n = 0; n < (unsigned long)M; ++n) 
			cout << X[n] << endl;
			
		/*x = idft(X, x, M);
		
		cout << "x: " << endl;
		
		for (n = 0; n < M; ++n) 
			cout << x[n] << endl;*/
					
		cout << "\nEnter the number of elements in h:  ";
		N = in();
		
		L = N + M - 1;
		
		if (N > 0 && L > 0) {
			y = memAlloc(y, L);
			h = memAlloc(h, M);
			
			cout << "\nEnter the elements in h:  ";
		
			for (n = 0; n < (unsigned long)N; ++n) {	
				h[n].Re = ind();
				h[n].Im = 0;
			} 		
			y = conv(x, y, h, M, N);
			//y = convfast(x, y, h, M, N);	
			
			cout << "y: " << endl;
		
			for (n = 0; n < (unsigned long)L; ++n) 
				cout << y[n] << endl;
			
			delete [] h;
			delete [] y;
		}
		delete [] x;
		delete [] X;
	}	
}




complex *conv(complex *x, complex *y, complex *h, long M, long N) {

	long L;
	
	L = M + N - 1;
	
	for (unsigned long n = 0; (long)n < L; ++n) {
		y[n].Re = y[n].Im = 0;
		for (unsigned long k = 0; (long)k < N && k <= n; ++k) 
			if ((long)(n-k) < M)
				y[n] = y[n] + h[k]*x[n-k];
	}		
	return y;
}





complex *convfast(complex *x, complex *y, complex *h, long M, long N) { // currently this is slower
	
	unsigned long P, i, L;
	complex *H = NULL, *X = NULL, *apow2 = NULL; 
	
	if (M <= 0 || N <= 0)  // M = length(x), N = length(h)
		return y;  // y must be length L
	
	L = M + N - 1;  // size of convolution
	P = pow2(L);  // P = smallest power of 2 >= L
	
	apow2 = memAlloc(apow2, P);
	
	for (i = 0; i < (unsigned long)N; ++i) 
		apow2[i] = h[i];  // apow2 = h + 2^n - length(h) zeros
		
	for (i = N; i < P; ++i) 
		apow2[i].Re = apow2[i].Im = 0;
	
	H = memAlloc(H, P);
	H = fft(apow2, H, P, 0);
	
	for (i = 0; i < (unsigned long)M; ++i) 
		apow2[i] = x[i];  // apow2 = x + 2^n - length(x) zeros
		
	for (i = M; i < P; ++i) 
		apow2[i].Re = apow2[i].Im = 0;
	
	X = memAlloc(X, P);	
	X = fft(apow2, X, P, 0);
	
	for (i = 0; i < P; ++i)
		X[i] = X[i]*H[i];  // Actually this is Y
	
	apow2 = fft(X, apow2, P, 1);  // This is really ypow2 = fft(Y, ypow2, Npow2, 1);
		
	for (i = 0; i < L; ++i) 
		y[i] = apow2[i];
	
	delete [] apow2;
	delete [] H;
	delete [] X;
	
	return y;
}




complex *dft(complex *x, complex *X, long N, int inverse) {

	double num1, num2;
	
	num1 = -two_pi/N;
	
	if (inverse)
		num1 = -num1;

	for (unsigned long k = 0; (long)k < N; ++k) {
		num2 = num1*k;	
		X[k].Re = X[k].Im = 0;
		for (unsigned long n = 0; (long)n < N; ++n)
			X[k] = X[k] + x[n]*expc(n*num2);
			
		if (inverse)
			X[k] = X[k]/N;
	}
	return X;	
}




complex *fft(complex *x, complex *X, long N, int inverse) {
	
	if (!use_fast_transform)
		return dft(x, X, N, inverse); 
	
	if (N < 2)
		return X;
		
	double *Re = NULL, *Im = NULL;
	
	split_complex(x, Re, Im, N, 0);	
	fft(Re, Im, ceil_log2(N), (2*inverse-1));	
	merge_complex(X, Re, Im, N, 1);
		
	return X;
}




void split_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old) {

	if (C == NULL) {
		Re = NULL;
		Im = NULL;
		return;
	}
	if (Re == NULL)
		Re = memAlloc(Re, size);
			
	if (Im == NULL)
		Im = memAlloc(Im, size);
			
	for (unsigned long i = 0; (long)i < size; ++i) {
		Re[i] = C[i].Re;
		Im[i] = C[i].Im;
	}
	if (delete_old) {
		delete [] C;
		C = NULL;
	}
}




void merge_complex(complex *&C, double *&Re, double *&Im, long size, int delete_old) {

	if (Re == NULL || Im == NULL) {
		C = NULL;
		return;
	}
	if (C == NULL)
		C = memAlloc(C, size);
	
	for (unsigned long i = 0; (long)i < size; ++i) {
		C[i].Re = Re[i];
		C[i].Im = Im[i];
	}
	if (delete_old) {
		delete [] Re;
		delete [] Im;
		Re = NULL;
		Im = NULL;
	}
}




void fft_on_graph(int inverse) { // still working on this

	unsigned int size = pow2(Count), i;
	double *Re = NULL, *Im = NULL;

	if (inverse == -1) {
		cout << "\nCompute fft (ifft otherwise)?  ";
		inverse = decision();
	}
	cout << "Truncate graph to closest power of two <= the number of points?  ";
	if (decision()) {
		size = size >> 1;
		Count = num_points = size;
	}	
	cout << "Use hanning window to eliminate spectral leakage?  ";
	if (decision()) {
		for (i = 0; (int)i < Count; ++i)
			graph_data[i].y *= 0.5*(1-cos(two_pi*(graph_data[i].x/domain - origin.x/x_limit)));
	}
	Re = memAlloc(Re, size);
	Im = memAlloc(Im, size);
	
	if (inverse) 
		cout << "Computing ifft for " << size << " points." << endl;
	else
		cout << "Computing fft for " << size << " points." << endl;
			
	for (i = 0; i < size; ++i) {
		Im[i] = 0;
		if ((int)i < Count)
			Re[i] = graph_data[i].y;
		else
			Re[i] = 0;
	}
	fft(Re, Im, ceil_log2(size), (2*inverse-1));
	
	for (i = 0; (int)i < Count; ++i) {
		graph_data[i].x = i;
		graph_data[i].y = sqrt(Re[i]*Re[i] + Im[i]*Im[i]); // magnitude
	}
	delete [] Re;
	delete [] Im;
}




long pow2(long x) { // returns smallest 2^N >= x
	
	/*long pow2x(1);
		
	--x;
	
	while (x) {
		x = x >> 1;
		pow2x = pow2x << 1;
	}
	return pow2x;*/
	
	return (long)pow(2, (double)ceil_log2(x));	
}




int ceil_log2(long N) {

	return (int)ceil(log(N-1.0)/log(2.0));
}




long switch_bits(long x, unsigned int n) { // not used here

	unsigned int k(n/2), i, val;
	
	for (i = 0; i < k; ++i) {
		val = 1 << (n - i - 1);
		if (((x & (1 << i)) >> i) != ((x & (long)val) >> (n - i - 1))) {
			if (x & (1 << i))
				x -= (1 << i);
			else
				x += (1 << i);
				
			if (x & val)
				x -= val;
			else
				x += val;
		}	
	}
	return x;
}







