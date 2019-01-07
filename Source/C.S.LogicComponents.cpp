#include "CircuitSolver.h"
#include "Logic.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LogicComponents.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Logic Components
// By Frank Gennari
extern int PREVIOUS_CLOCK, EDGE, AUTOTRIGGER, LOOP, NOCLOCK, FAST, Q_POLARITY, Q_warning;
extern char INITIAL_OUTPUT, CARRY_OUT;
extern double supply_voltage, ground;




inline char BooleanNot(char in) {

	if (in == 1)
		return 0;
	else if (in == 0)
		return 1;
		
	return 2;
}




inline char AtoD(double NUM) {

	if (NUM > 0.5*(supply_voltage - ground))
		return 1;
	else if (NUM < 0.5*(supply_voltage - ground))
		return 0;
		
	return 2;
}



inline double DtoA(char NUM) {

	if (NUM == 1)
		return supply_voltage;
		
	if (NUM == 0)
		return ground;
		
	return 0.5*(supply_voltage - ground);
}



inline char DIODE(char A) {

	if (A == 1)
		return 1;
		
	return -2;
		
}


inline char DIODE2(char A) {

	if (A == 0)
		return 0;
		
	return -2;
		
}



inline double DIODEf(double A, double Vd_ON) {
	
	if (Vd_ON > 0)
		return A - Vd_ON;
	
	return A - diode_vdrop;
}



inline char NTRANSISTOR(char B, char C, char E) {
		
	if (B == 1) {
		if (E == 0) {
			Q_POLARITY = 0;
			return 0;
		}
		if (C != -2 || FAST) {
			Q_POLARITY = 1;
			if (E == 1) {
				Q_warning = 1;
				return -2;
			}
			return 1;
		}
	}
	return -2;
}


inline char PTRANSISTOR(char B, char E, char C) {

	if (B == 0) {
		if (E == 1) {
			Q_POLARITY = 1;
			return 1;
		}
		if (C != -2 || FAST) {
			Q_POLARITY = 0;
			if (E == 0) {
				Q_warning = 1;
				return -2;
			}
			return 0;
		}
	}
	return -2;
}



inline char NFET(char G, char D, char S) {
			
	if (G == 1) {
		if (known(S)) {
			Q_POLARITY = 0;
			return S;
		}
		if (known(D)) {
			Q_POLARITY = 1;
			return D;
		}
	}
	return -2;
}



inline char PFET(char G, char S, char D) {
				
	if (G == 0) {
		if (known(S)) {
			Q_POLARITY = 1;
			return S;
		}
		if (known(D)) {
			Q_POLARITY = 0;
			return D;
		}
	}
	return -2;
}





inline double OPAMPd(double N, double P) {
	
	double vout = A*(P - N);
	
	if (vout > supply_voltage)
		return supply_voltage;
		
	if (vout < ground)
		return ground;
		
	return vout;
}


inline char OPAMP(char N, char P) {

	if (P == 1 && N != 1)
		return 1;
	else if (N == 1 && P != 1)
		return 0;
		
	return 2;
}




inline char SAME(char A, char B, char C, char D) {

	if (A == B && B == C && C == D)
		return 1;

	return 0;
}




inline char TRIGGER_CK(char CK) {
	
	if (LOOP)
		PREVIOUS_CLOCK = CK;
	
	if (AUTOTRIGGER)
		return 1;
		
	if (EDGE) {
		if (!PREVIOUS_CLOCK && CK) {
			return 1;
		}
		return 0;
	}
	if (PREVIOUS_CLOCK && !CK) {
		return 1;
	}
	return 0;		
}




inline char DELAY_ELEM(char A) {

	if (A == 1)
		return 1;

	if (A == 0)
		return 0;

	return 2;
}




inline char ITE(char I, char T, char E) {

	if (I == 0) {
		return T;
	}
	else if (I == 1) {
		return E;
	}
	return 2;
}




inline char BUFFER(char A) {
		
	if (A == 1)
		return 1;
	
	if (A == 0)
		return 0;

	return 2;
}

inline char NOT(char A) {

	if (A == 1)
		return 0;
	
	if (A == 0)
		return 1;

	return 2;
}

inline char AND(char A, char B) {
	
	if (A == 0 || B == 0) 
		return 0;

	if (A == 1 && B == 1)
		return 1;
	
	return 2;
}

inline char OR(char A, char B) {

	if (A == 1 || B == 1) 
		return 1;

	if (A == 0 && B == 0)
		return 0;
	
	return 2;
}

inline char NAND(char A, char B) {
		
	if (A == 0 || B == 0) 
		return 1;

	if (A == 1 && B == 1)
		return 0;

	return 2;
}

inline char NOR(char A, char B) {
		
	if (A == 1 || B == 1) 
		return 0;

	if (A == 0 && B == 0)
		return 1;
	
	return 2;
}

inline char XOR(char A, char B) {

	if (unknown(A) || unknown(B)) 
		return 2;
			
	if (A != B)
		return 1;
		
	return 0;
}

inline char XNOR(char A, char B) {

	if (unknown(A) || unknown(B))
		return 2;
				
	if (A == B)
		return 1;
		
	return 0;
}

inline char TRIPLE_AND(char A, char B, char C) {

	if (A == 0 || B == 0 || C == 0) 
		return 0;

	if (A == 1 && B == 1 && C == 1)
		return 1;
	
	return 2;
}

inline char TRIPLE_OR(char A, char B, char C) {

	if (A == 1 || B == 1 || C == 1) 
		return 1;

	if (A == 0 && B == 0 && C == 0)
		return 0;

	return 2;
}

inline char TRIPLE_NAND(char A, char B, char C) {

	if (A == 0 || B == 0 || C == 0) 
		return 1;

	if (A == 1 && B == 1 && C == 1)
		return 0;

	return 2;
}

inline char TRIPLE_NOR(char A, char B, char C) {

	if (A == 1 || B == 1 || C == 1) 
		return 0;

	if (A == 0 && B == 0 && C == 0)
		return 1;

	return 2;
}

inline char TRIPLE_XOR(char A, char B, char C) {

	if (unknown(A) || unknown(B) || unknown(C))
		return 2;
				
	if (((A + B + C) & 0x01) == 0)
		return 0;
		
	return 1;
}

inline char TRIPLE_XNOR(char A, char B, char C) {

	if (unknown(A) || unknown(B) || unknown(C))
		return 2;
				
	if (((A + B + C) & 0x01) == 0)
		return 1;
		
	return 0;
}


inline char QUAD_AND(char A, char B, char C, char D) {

	if (A == 0 || B == 0 || C == 0 || D == 0) 
		return 0;

	if (A == 1 && B == 1 && C == 1 && D == 1)
		return 1;

	return 2;
}

inline char QUAD_OR(char A, char B, char C, char D) {

	if (A == 1 || B == 1 || C == 1 || D == 1) 
		return 1;

	if (A == 0 && B == 0 && C == 0 && D == 0)
		return 0;

	return 2;
}

inline char QUAD_NAND(char A, char B, char C, char D) {

	if (A == 0 || B == 0 || C == 0 || D == 0) 
		return 1;

	if (A == 1 && B == 1 && C == 1 && D == 1)
		return 0;

	return 2;
}

inline char QUAD_NOR(char A, char B, char C, char D) {

	if (A == 1 || B == 1 || C == 1 || D == 1) 
		return 0;

	if (A == 0 && B == 0 && C == 0 && D == 0)
		return 1;

	return 2;
}

inline char QUAD_XOR(char A, char B, char C, char D) {

	if (unknown(A) || unknown(B) || unknown(C) || unknown(D))
		return 2;
				
	if (((A + B + C + D) & 0x01) == 0)
		return 0;
	
	return 1;
}

inline char QUAD_XNOR(char A, char B, char C, char D) {

	if (unknown(A) || unknown(B) || unknown(C) || unknown(D))
		return 2;
				
	if (((A + B + C + D) & 0x01) == 0)
		return 1;
	
	return 0;
}




inline char TRI_STATE_BUFFER(char A, char C) {
	
	if (C == 1)
		return -2;

	if (C == 0) {
		if (A == 1)
			return 1;
		if (A == 0)
			return 0;
	}
	return 2;
}


inline char TRI_STATE_INVERTER(char A, char C) {
	
	if (C == 1)
		return -2;

	if (C == 0) {
		if (A == 1)
			return 0;
		if (A == 0)
			return 1;
	}
	return 2;
}





inline char RS_LATCH(char Q, char R, char S, char P, char C) {
		
	if (P && C)
		return 2;
		
	if (P == 1) 
		return 1;
		
	if (C == 1) 
		return 0;
	
	if (unknown(R) || unknown(S) || unknown(P) || unknown(C))
		return 2;
			
	//if (!(P || C))
		//PROPAGATION_DELAY += 3;
		
	if (!S && R)
		return 0;
		
	if (S && !R)
		return 1;
		
	if (!S && !R) {
		if (Q == 2)
			Q = INITIAL_OUTPUT;
		return Q;
	}		
	return 3; // R = 1 and S = 1  =>  Q = Q` = 0
}



inline char RS_FF(char Q, char R, char S, char CK, char P, char C) {
		
	if (P && C) 
		return 2;
				
	if (P == 1) 
		return 1;
		
	if (C == 1) 
		return 0;
		
	if (unknown(R) || unknown(S) || unknown(P) || unknown(C))
		return 2;
			
	if (!LOOP && CK == 2) {
		CK = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	if (!CK) {
		if (Q == 2)
			Q = INITIAL_OUTPUT;
		return Q; // loop?
	}		
	return RS_LATCH(Q, R, S, 0, 0);
}



inline char TRANSPARENT_D_FF(char Q, char D, char E, char S, char R) {
		
	if (S && R)
		return 2;
		
	if (unknown(D) || unknown(R) || unknown(S))
		return 2;
			
	if (!LOOP && E == 2) {
		E = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	//if (R || S)
		//--PROPAGATION_DELAY;
	
	return RS_FF(Q, NOT(D), D, E, R, S);
}
	


inline char D_FF(char Q, char D, char CK, char S, char R) {
		
	if (S && R)
		return 2;
	
	if (R == 1) 
		return 0;
		
	if (S == 1) 
		return 1;
		
	if (unknown(D) || unknown(R) || unknown(S))
		return 2;
			
	if (!LOOP && CK == 2) {
		CK = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	//if (!(R || S))
		//PROPAGATION_DELAY += 4;
		
	if (TRIGGER_CK(CK))
		return D;
	
	if (Q == 2)
		Q = INITIAL_OUTPUT;
		
	return Q;
}



inline char JK_FF(char Q, char J, char K, char CK, char S, char R) {
		
	if (S && R)
		return 2;
	
	if (R == 1) 
		return 0;
		
	if (S == 1) 
		return 1;
		
	if (unknown(J) || unknown(K) || unknown(R) || unknown(S))
		return 2;
			
	if (!LOOP && CK == 2) {
		CK = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	//if (!(R || S))
		//PROPAGATION_DELAY += 5;
		
	if (!TRIGGER_CK(CK) || (!J && !K)) {
		if (Q == 2)
			Q = INITIAL_OUTPUT;
		return Q;
	}		
	if (!J && K)
		return 0;
		
	if (J && !K)
		return 1;
		
	if (Q == 2)
		Q = INITIAL_OUTPUT; // NOT?
	
	return NOT(Q);	
}



inline char T_FF(char Q, char T, char CK, char S, char R) {
		
	if (S && R)
		return 2;
	
	if (R == 1) 
		return 0;
		
	if (S == 1) 
		return 1;
		
	if (unknown(T) || unknown(R) || unknown(S))
		return 2;
			
	if (!LOOP && CK == 2) {
		CK = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	//if (!(R || S))
		//PROPAGATION_DELAY += 5;
		
	if (TRIGGER_CK(CK) && T) {
		if (Q == 2)
			Q = INITIAL_OUTPUT; // NOT?
		return NOT(Q);
	}
	if (Q == 2)
		Q = INITIAL_OUTPUT;
	
	return Q;
}



inline char MS_FF(char Q, char D, char CK, char S, char R) {
		
	if (S && R)
		return 2;
		
	if (unknown(D) || unknown(R) || unknown(S))
		return 2;
			
	if (!LOOP && CK == 2) {
		CK = PREVIOUS_CLOCK;
		NOCLOCK = 1;
	}
	//if (R || S)
		//--PROPAGATION_DELAY;
		
	char Q0 = TRANSPARENT_D_FF(Q, D, NOT(CK), R, S);
	
	return TRANSPARENT_D_FF(Q, Q0, CK, R, S);
}




inline char HALF_ADDER(char A, char B) {
	
	if (unknown(A) || unknown(B))
		return 2;
		
	CARRY_OUT = AND(A, B);
	
	return XOR(A, B);
}


inline char FULL_ADDER(char A, char B, char CARRY_IN) { 
	
	if (unknown(A) || unknown(B) || unknown(CARRY_IN))
		return 2;
		
	if (A + B + CARRY_IN > 1)
		CARRY_OUT = 1;
	else
		CARRY_OUT = 0;
	
	if (((A + B + CARRY_IN) & 0x01) == 0)
		return 0;
		
	return 1; 
}



inline char MUX2to1(char A0, char A1, char S) {

	if (S == 0)
		return A0;

	if (S == 1)
		return A1;
			
	return 2;
}














		
