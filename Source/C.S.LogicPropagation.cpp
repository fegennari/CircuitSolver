#include "CircuitSolver.h"
#include "Logic.h"
#include "Propagations.h"
#include "GridArray.h"

#include "C.S.LogicComponents.cpp"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.LogicPropagations.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Gate Propagations for Logic Engine 
// includes both logic propagations and delay propagations
// By Frank Gennari
extern int FAST, AC1DC0, LOOP, loop_num, force_init_states, no_inputs, mos_delay_not_wl;
extern char CARRY_OUT, PRESET, CLEAR;

void add_conflict(int location, int type);



// Propagation Function Definitions
char pNONE(char *NODEVALUE, comp_sim &comp) {

	NODEVALUE; // just so I don't get the "unused argument" warning
	comp;

	return no_val;
}



char pBUF(char *NODEVALUE, comp_sim &comp) {

	return BUFFER(NODEVALUE[comp.conn[1]]);
}



char pINV(char *NODEVALUE, comp_sim &comp) {

	return NOT(NODEVALUE[comp.conn[1]]);
}



char pAND(char *NODEVALUE, comp_sim &comp) {

	return AND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pNAND(char *NODEVALUE, comp_sim &comp) {

	return NAND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pOR(char *NODEVALUE, comp_sim &comp) {

	return OR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pNOR(char *NODEVALUE, comp_sim &comp) {

	return NOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pXOR(char *NODEVALUE, comp_sim &comp) {

	return XOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pXNOR(char *NODEVALUE, comp_sim &comp) {

	return XNOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pAND3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_AND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pNAND3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_NAND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pOR3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_OR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pNOR3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_NOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pXOR3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_XOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pXNOR3(char *NODEVALUE, comp_sim &comp) {

	return TRIPLE_XNOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]]);
}



char pAND4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_AND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pNAND4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_NAND(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pOR4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_OR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pNOR4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_NOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pXOR4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_XOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pXNOR4(char *NODEVALUE, comp_sim &comp) {

	return QUAD_XNOR(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[3]]);
}



char pOPAMP(char *NODEVALUE, comp_sim &comp) {

	return OPAMP(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]]);
}



char pCAP(char *NODEVALUE, comp_sim &comp) {

	if (AC1DC0 && (LOOP || loop_num == 0)) { 
		if (known(NODEVALUE[comp.conn[1]]) && NODEVALUE[comp.conn[4]] == -2) 
			comp.state = NODEVALUE[comp.conn[4]] = NODEVALUE[comp.conn[1]];
		else if (known(NODEVALUE[comp.conn[4]]) && NODEVALUE[comp.conn[1]] == -2) 
			comp.state = NODEVALUE[comp.conn[1]] = NODEVALUE[comp.conn[4]];
	}
	return def_val;
}



char pRES(char *NODEVALUE, comp_sim &comp) {

	char vi2 = NODEVALUE[comp.conn[1]];
	char vo2 = NODEVALUE[comp.conn[4]];
	
	if (known(vo2))
		comp.state = vo2;
	else if (known(vi2))
		comp.state = vi2;
		
	if (FAST && unknown(vo2) && known(vi2)) 
		comp.state = NODEVALUE[comp.conn[4]] = vi2;
	else if (FAST && unknown(vi2) && known(vo2)) 
		comp.state = NODEVALUE[comp.conn[1]] = vo2;
		
	return no_val;
}



char pPOWER(char *NODEVALUE, comp_sim &comp) {

	if (NODEVALUE[comp.conn_loc] == 0)
		add_conflict(comp.location, 0); 
		
	comp.state = NODEVALUE[comp.conn_loc] = 1;
	
	return def_val;
}



char pGROUND(char *NODEVALUE, comp_sim &comp) {

	if (NODEVALUE[comp.conn_loc] == 1)
		add_conflict(comp.location, 0); 
		
	comp.state = NODEVALUE[comp.conn_loc] = 0;
	
	return def_val;
}



char pINDUCTOR(char *NODEVALUE, comp_sim &comp) {

	comp.state = NODEVALUE[comp.conn[4]];
	
	return def_val;
}



char pSTATE(char *NODEVALUE, comp_sim &comp) {

	if ((FAST || force_init_states || no_inputs) && !known(NODEVALUE[comp.conn[4]])) { // initial state
		if (comp.flags & ROTATED)
			comp.state = NODEVALUE[comp.conn[4]] = 0;
		else 
			comp.state = NODEVALUE[comp.conn[4]] = 1;
	}
	return def_val;
}



char pTSTBUF(char *NODEVALUE, comp_sim &comp) {

	return TRI_STATE_BUFFER(NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[0]]);
}



char pTSTINV(char *NODEVALUE, comp_sim &comp) {

	return TRI_STATE_INVERTER(NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[0]]);
}



char pMUX(char *NODEVALUE, comp_sim &comp) {

	return MUX2to1(NODEVALUE[comp.conn[1]], NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]]);
}



char pADDER(char *NODEVALUE, comp_sim &comp) {

	char Value = FULL_ADDER(NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[1]]);
	
	if (known(Value))
		NODEVALUE[comp.conn[5]] = CARRY_OUT; 
		
	return Value;
}



char pRSL(char *NODEVALUE, comp_sim &comp) {

	return RS_LATCH(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]], PRESET, CLEAR);
}



char pRSFF(char *NODEVALUE, comp_sim &comp) {

	return RS_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}



char pDFF(char *NODEVALUE, comp_sim &comp) {

	return D_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}



char pTDFF(char *NODEVALUE, comp_sim &comp) {

	return TRANSPARENT_D_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}



char pJKFF(char *NODEVALUE, comp_sim &comp) {

	return JK_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[2]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}



char pMSFF(char *NODEVALUE, comp_sim &comp) {

	return MS_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}



char pTFF(char *NODEVALUE, comp_sim &comp) {

	return T_FF(NODEVALUE[comp.conn[3]], NODEVALUE[comp.conn[0]], NODEVALUE[comp.conn[1]], PRESET, CLEAR);
}




char pDELAY(char *NODEVALUE, comp_sim &comp) {

	return DELAY_ELEM(NODEVALUE[comp.conn[1]]);
}





// Delay Function Definitions
double dNONE(char *NODEVALUE, int *conn, double *net_delay) {

	NODEVALUE; // just so I don't get the "unused argument" warning
	conn;
	net_delay;

	return 0;
}



double dBUF(char *NODEVALUE, int *conn, double *net_delay) { // same as inverter
	
	NODEVALUE;
	
	return net_delay[conn[1]];
}



double dAND(char *NODEVALUE, int *conn, double *net_delay) {
	
	char A(conn[0]), B(conn[2]);
	double delay(-1);
			
	if (NODEVALUE[A] == 0) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 0) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (delay == -1) 
		delay = max(net_delay[A], net_delay[B]);
	
	return delay;
}



double dOR(char *NODEVALUE, int *conn, double *net_delay) {

	char A(conn[0]), B(conn[2]);
	double delay(-1);
			
	if (NODEVALUE[A] == 1) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 1) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (delay == -1) 
		delay = max(net_delay[A], net_delay[B]);
	
	return delay;
}



double dXOR(char *NODEVALUE, int *conn, double *net_delay) {

	NODEVALUE;

	return max(net_delay[conn[0]], net_delay[conn[2]]);
}



double dAND3(char *NODEVALUE, int *conn, double *net_delay) {

	char A(conn[0]), B(conn[1]), C(conn[2]);
	double delay(-1);
			
	if (NODEVALUE[A] == 0) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 0) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (NODEVALUE[C] == 0) {
		if (delay == -1)
			delay = net_delay[C];
		else
			delay = min(delay, net_delay[C]);			
	}
	if (delay == -1) {
		delay = max(net_delay[A], net_delay[B]);
		delay = max(delay, net_delay[C]);
	}
	return delay;
}



double dOR3(char *NODEVALUE, int *conn, double *net_delay) {

	char A(conn[0]), B(conn[1]), C(conn[2]);
	double delay(-1);
			
	if (NODEVALUE[A] == 1) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 1) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (NODEVALUE[C] == 1) {
		if (delay == -1)
			delay = net_delay[C];
		else
			delay = min(delay, net_delay[C]);			
	}
	if (delay == -1) {
		delay = max(net_delay[A], net_delay[B]);
		delay = max(delay, net_delay[C]);
	}
	return delay;
}



double dXOR3(char *NODEVALUE, int *conn, double *net_delay) {

	double delay(max(net_delay[conn[0]], net_delay[conn[1]]));
	
	NODEVALUE;
	
	return max(delay, net_delay[conn[2]]);
}



double dAND4(char *NODEVALUE, int *conn, double *net_delay) {

	char A(conn[0]), B(conn[1]), C(conn[2]), D(conn[3]);
	double delay(-1);
			
	if (NODEVALUE[A] == 0) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 0) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (NODEVALUE[C] == 0) {
		if (delay == -1)
			delay = net_delay[C];
		else
			delay = min(delay, net_delay[C]);			
	}
	if (NODEVALUE[D] == 0) {
		if (delay == -1)
			delay = net_delay[D];
		else
			delay = min(delay, net_delay[D]);			
	}
	if (delay == -1) {
		delay = max(net_delay[A], net_delay[B]);
		delay = max(delay, net_delay[C]);
		delay = max(delay, net_delay[D]);
	}
	return delay;
}



double dOR4(char *NODEVALUE, int *conn, double *net_delay) {

	char A(conn[0]), B(conn[1]), C(conn[2]), D(conn[3]);
	double delay(-1);
			
	if (NODEVALUE[A] == 1) 
		delay = net_delay[A];
			
	if (NODEVALUE[B] == 1) {
		if (delay == -1)
			delay = net_delay[B];
		else
			delay = min(delay, net_delay[B]);			
	}
	if (NODEVALUE[C] == 1) {
		if (delay == -1)
			delay = net_delay[C];
		else
			delay = min(delay, net_delay[C]);			
	}
	if (NODEVALUE[D] == 1) {
		if (delay == -1)
			delay = net_delay[D];
		else
			delay = min(delay, net_delay[D]);			
	}
	if (delay == -1) {
		delay = max(net_delay[A], net_delay[B]);
		delay = max(delay, net_delay[C]);
		delay = max(delay, net_delay[D]);
	}
	return delay;
}



double dXOR4(char *NODEVALUE, int *conn, double *net_delay) {

	double delay(max(net_delay[conn[0]], net_delay[conn[1]]));
	
	NODEVALUE;
	
	delay = max(delay, net_delay[conn[2]]);
	
	return max(delay, net_delay[conn[3]]);
}



double dTSTBUF(char *NODEVALUE, int *conn, double *net_delay) {

	if (NODEVALUE[conn[0]] == 1)
		return net_delay[conn[0]];

	return max(net_delay[conn[0]], net_delay[conn[1]]);
}



double dMUX(char *NODEVALUE, int *conn, double *net_delay) {
	
	if (NODEVALUE[conn[0]] == 0)
		return max(net_delay[conn[0]], net_delay[conn[1]]);
	
	return max(net_delay[conn[0]], net_delay[conn[2]]);
}



double dADDER(char *NODEVALUE, int *conn, double *net_delay) {

	double delay(max(net_delay[conn[0]], net_delay[conn[1]]));
	
	NODEVALUE;
	
	return max(delay, net_delay[conn[2]]);
}




double dMOSFET(char *NODEVALUE, int *conn, double *net_delay) {

	if (!mos_delay_not_wl) {
		return 0;
	}
	// don't know if signal propagates from S->D or from D->S, but whichever is to be assigned to should have
	// net_delay[x] = 0 (init value), so max should work and is quicker than determining direction of signal propagation
	double delay(max(net_delay[conn[3]], net_delay[conn[4]]));

	NODEVALUE;
	
	return max(delay, net_delay[conn[1]]);
}




double dDELAY(char *NODEVALUE, int *conn, double *net_delay) {

	NODEVALUE;

	return net_delay[conn[1]];
}





