# include "CircuitSolver.h"
# include "Complex.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.ComplexNumbers.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Complex Number Analysis
// By Frank Gennari
extern complex Z_storage, Z_temp;


unsigned char *get_units(int id, double value, int for_drawing);
double range_value(double value1);




complex get_j() {
	
	complex j;
	
	j.Im = 1;
	j.Re = 0;
	
	return j;
}





istream& operator>>(ifstream& in, complex& z) {

	return in >> z.Re >> z.Im;
}





ostream& operator<<(ostream& out, complex& z) {
	
	if (fabs(z.Im) < 1e-12) 
		return out << z.Re;
	
	if (z.Im > 0)
		return out << z.Re << " + j" << z.Im;
		
	return out << z.Re << " - j" << (-z.Im);
}





complex operator+(complex num1, complex num2) {
	
	complex sum;
	
	sum.Re = num1.Re + num2.Re;
	sum.Im = num1.Im + num2.Im;
		
	return sum;
}




complex operator-(complex num1, complex num2) {
	
	complex difference;
	
	difference.Re = num1.Re - num2.Re;
	difference.Im = num1.Im - num2.Im;
		
	return difference;
}




complex operator*(complex num1, complex num2) {
	
	complex product;
	
	product.Re = num1.Re*num2.Re - num1.Im*num2.Im;
	product.Im = num1.Re*num2.Im + num2.Re*num1.Im;
		
	return product;
}




complex operator/(complex num1, complex num2) {
	
	complex z;
	double denom;
	
	if (num2.Re == 0 && num2.Im == 0) {
		z.Re = z.Im = bound;
		return z;
	}
	denom = (num2.Re*num2.Re + num2.Im*num2.Im);
	z.Re = (num1.Re*num2.Re + num1.Im*num2.Im)/denom;
	z.Im = (num2.Re*num1.Im - num1.Re*num2.Im)/denom;
	
	return z;
}



// All of the following (+, -, *, /) are repeated for both (complex, real) and (real, complex)
// so that the user doesn't need to use a specific ordering
complex operator+(complex num2, double num1) {

	complex z;
	
	z.Re = num2.Re + num1;
	z.Im = num2.Im;
	
	return z;
}


complex operator+(double num1, complex num2) {

	complex z;
	
	z.Re = num2.Re + num1;
	z.Im = num2.Im;
	
	return z;
} 




complex operator-(complex num2, double num1) {

	complex z;
	
	z.Re = num2.Re - num1;
	z.Im = num2.Im;
	
	return z;
}


complex operator-(double num1, complex num2) {

	complex z;
	
	z.Re = num2.Re - num1;
	z.Im = num2.Im;
	
	return z;
}




complex operator*(complex num2, double num1) {

	complex z;
	
	z.Re = num2.Re*num1;
	z.Im = num2.Im*num1;
	
	return z;
}


complex operator*(double num1, complex num2) {

	complex z;
	
	z.Re = num2.Re*num1;
	z.Im = num2.Im*num1;
	
	return z;
}




complex operator/(complex num2, double num1) {

	complex z;
	
	z.Re = num2.Re/num1;
	z.Im = num2.Im/num1;
	
	return z;
}


complex operator/(double num1, complex num2) {

	complex z;
	
	z.Re = num2.Re/num1;
	z.Im = num2.Im/num1;
	
	return z;
}





void out(complex Z) {

	out(Z.Re);
	if (Z.Im == 0)
		cout << "½";
	else if (Z.Im > 0) {
		cout << "½ + j";
		out(Z.Im);
		cout << "½";
	}
	else if (Z.Im < 0) {
		cout << "½ - j";
		out(fabs(Z.Im));
		cout << "½";
	}
	else
		cout << "½";
}




void out2(complex Z) {
	
	double range = magnitude(Z)/range_value(magnitude(Z));
	
	if (magnitude(Z) == 0)
		range = 1;
	
	cout << Z.Re/range;
	if (Z.Im > 0) {
		cout << " + j";
		cout << Z.Im/range;
	}
	else if (Z.Im < 0) {
		cout << " - j";
		cout << fabs(Z.Im)/range;
	}
	print_ch(get_units(NO_ID, magnitude(Z), 0));
}




void outp(complex z) {
	
	double mag = magnitude(z);
	
	mag = range_value(mag);
	
	cout << mag << "<" << phase(z);
	print_ch(get_units(NO_ID, magnitude(z), 0));
}




void show_equation(complex z, double f, char units, char function) {

	double w = two_pi*f;
	
	out(magnitude(z));
	
	cout << units;
	
	if (comp_char(function, 'c'))
		cout << " cos(";
	else if (comp_char(function, 's'))
		cout << " sin(";
	
	cout << w << "t";
	
	if (phase(z) > 0)
		cout << " + " << phase(z) << ")";
	else if (phase(z) < 0)
		cout << " - " << -phase(z) << ")";
	else
		cout << ")";
}





int get_complex_form() {
	
	int form(0);
	
	cout << "\nUse Rectangular form?  ";
	if (decision())
		form += 1;
	
	cout << "\nUse Polar form?  ";
	if (decision())
		form += 2;
	
	if (form == 0)
		return 4;
	
	cout << "\nShow actual equation?  ";
	if (decision())
		form += 4;
		
	return form;
}





complex real_to_complex(double real) {

	complex z;
	
	z.Re = real;
	z.Im = 0;
	
	return z;
}




double magnitude(complex z) {

	return sqrt(z.Re*z.Re + z.Im*z.Im);
}




double ri_sum(complex z) {

	return z.Re + z.Im;
}




double phase(complex z) {
		
	double angle = get_angle(z.Re, z.Im);
	
	if (angle > 180)
		return angle - 360;
		
	return angle;	
}




complex phase_shift(complex z, double phase) {

	if (phase == 0)
		return z;
	
	polar P = RtoP(z);
	P.phase += pi_over_180*phase;
		
	return PtoR(P);
}




double get_angle(double x, double y) {
	
	if (x == 0) {
		if (y > 0)
			return 90;
		if (y < 0)
			return 270;
		return 0;
	}
	if (y == 0) {
		if (x > 0)
			return 0;
		if (x < 0)
			return 180;
		return 0;
	}
	if (x >= 0) {
		if (y >= 0)
			return pi_under_180*atan(y/x);
		else
			return pi_under_180*atan(y/x) + 360;
	}
	else {
		return pi_under_180*atan(y/x) + 180;
	}
	return 0;
}




complex expc(double theta) {

	complex z;
	
	z.Re = cos(theta);
	z.Im = sin(theta);
	
	return z;
}



complex exp(complex z) {

	complex Z;
	
	Z.Re = cos(z.Im);
	Z.Im = sin(z.Im);
	
	return Z*exp(z.Re);
}




polar RtoP(complex z) {

	polar p;
	
	p.phase = phase(z);
	p.mag = sqrt(z.Re*z.Re + z.Im*z.Im);
	
	return p;
}




complex PtoR(polar p) {

	complex z;
	
	z.Re = p.mag*cos(p.phase);
	z.Im = p.mag*sin(p.phase);
	
	return z;
}




complex parallel(complex num1, complex num2) {
	
	complex z = num1 + num2;
	
	if (z.Re == 0 && z.Im == 0) {
		z.Re = z.Im = bound;
		return z;
	}
	return (num1*num2)/z;
}




complex impedance(int ID, double f, double Value) {

	complex z;
	
	if (Value < 0)
		Value = -Value;
		
	switch (ID) {
	
		case RES_ID:  // resistor
			z.Re = Value;
			z.Im = 0;
			return z;
			
		case CAP_ID:  // capacitor
			z.Re = 0;
			if (f == 0 || Value == 0)
				z.Im = bound;
			else
				z.Im = -1/(two_pi*f*Value);
			return z;
		
		case XFMR_ID:      // transformer
		case INDUCTOR_ID:  // inductor
			z.Re = 0;
			z.Im = two_pi*f*Value;
			return z;
			
		default:  // ???
			internal_error();
			cerr << "Error: Cannot calculate impedance of type " << ID << "." << endl;
			z.Re = 0;
			z.Im = 0;
	}
	return z;
}




complex impedance(char comp, double f, double value) {

	complex z;
	
	switch (comp) {	
		case 'r':  // resistor
		case 'R':
			z.Re = value;
			z.Im = 0;
			return z;
			
		case 'c':  // capacitor
		case 'C':
			z.Re = 0;
			if (f == 0 || value == 0)
				z.Im = bound;
			else
				z.Im = -1/(two_pi*f*value);
			return z;
		
		case 'x':  // transformer
		case 'X':	
		case 'i':  // inductor
		case 'I':
			z.Re = 0;
			z.Im = two_pi*f*value;
			return z;
			
		default:  // ???
			internal_error();
			cerr << "Error: Cannot calculate impedance of type " << comp << "." << endl;
			z.Re = 0;
			z.Im = 0;
	}
	return z;
}




complex admittance(int ID, double f, double Value) {
	
	complex z;
	
	switch (ID) {	
		case RES_ID:  // resistor
			if (Value == 0)
				z.Re = bound;
			else
				z.Re = 1/Value;
			z.Im = 0;
			return z;
			
		case CAP_ID:  // capacitor
			z.Re = 0;
			z.Im = two_pi*f*Value;
			return z;
		
		case XFMR_ID:      // transformer	
		case INDUCTOR_ID:  // inductor
			z.Re = 0;
			if (f == 0 || Value == 0)
				z.Im = bound;
			else
				z.Im = -1/(two_pi*f*Value);
			return z;
			
		default:  // ???
			internal_error();
			cerr << "Error: Cannot calculate impedance of type " << ID << "." << endl;
			z.Re = 0;
			z.Im = 0;
	}
	return z;
}




void Z_equiv() {
	
	complex Z, Z2;
	double f, value(1);
	int needtoask(0);
	char comp, sp;
	
	Z.Re = Z.Im = Z2.Re = Z2.Im = 0;
	
	cout << "\nEnter the frequency(use a positive number for Hz, and a negative number for rad/sec):  ";
	f = ind();
	
	if (f < 0)
		f /= -two_pi;
	
	do {	
		do {
			cout << "\nWould you like add a (r)esistor, (c)apacior, (i)nductor, (z)=complex Inpedance, (p)reviously calculated complex impedance, or (n)othing?  ";
			comp = inc();
		
			if (!comp_char(comp, 'r') && !comp_char(comp, 'c') && !comp_char(comp, 'i') && !comp_char(comp, 'z') && !comp_char(comp, 'p') && !comp_char(comp, 'n'))
				beep();
			
		} while (!comp_char(comp, 'r') && !comp_char(comp, 'c') && !comp_char(comp, 'i') && !comp_char(comp, 'z') && !comp_char(comp, 'p') && !comp_char(comp, 'n'));
		
		if (!comp_char(comp, 'n')) {
			if (comp_char(comp, 'z') || comp_char(comp, 'p')) {
				value = 1;
				
				if (comp_char(comp, 'z')) {
					cout << "\nEnter real part:  ";
					Z.Re = ind();
					cout << "Enter imaginary part:  ";
					Z.Im = ind();
				}
				else if (comp_char(comp, 'p')) {
					Z = Z_storage;
					cout << "\nThe stored impedance is ";
					out(Z);
					cout << " or ";
					outp(Z);
					cout << " ½";
				}	
				if (needtoask) {
					do {
						cout << "(S)eries or (P)arallel?  ";
						sp = inc();
		
						if (comp_char(sp, 's'))
							Z2 = Z2 + Z;
					
						else if (comp_char(sp, 'p'))
							Z2 = parallel(Z, Z2);
					
						else
							beep();
			
					} while (!comp_char(sp, 's') && !comp_char(sp, 'p'));	
				}	
				else
					Z2 = Z;	
			}
			else {
				cout << "Enter a value(+# for series; -# for parallel; 0 to end): "; 
				value = ind();
				
				Z = impedance(comp, f, fabs(value));
				
				if (!needtoask)
					Z2 = Z;
				else {
					if (value < 0) 
						Z2 = parallel(Z, Z2);
					else 
						Z2 = Z2 + Z;
					}
			}
		}
		needtoask = 1;
					
	} while (value && !comp_char(comp, 'n'));
	
	cout << "\nThe equivalent impedance is ";
	out(Z2);
	cout << " or ";
	outp(Z);
	cout << " ½";
	
	Z_temp = Z_storage;
	Z_storage = Z2;
}




complex *memAlloc_complex_zero(complex *data, int size) {

	data = memAlloc(data, size);

	memset(data, 0, size*sizeof(complex));

	return data;
}






