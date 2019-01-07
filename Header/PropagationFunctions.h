#ifndef _PROPAGATION_FUNCTIONS_H_
#define _PROPAGATION_FUNCTIONS_H_


enum {cvNO = 0, cvAND, cvOR};



// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, PropagationFunctions.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Propagation Functions for Logic Engine header
// includes both logic propagation and delay propagation function arrays
// By Frank Gennari

// Propagation Function Mappings
static const propValue_func propValue[FF_max_id+1] = {pNONE, pBUF, pINV, pAND, pNAND, 
			 pOR, pNOR, pXOR, pXNOR, pAND3, pNAND3, pOR3, pNOR3, pXOR3, pXNOR3,
			 pNONE, pOPAMP, pNONE, pNONE, pCAP, pRES, pNONE, pNONE, pNONE, pPOWER, 
			 pGROUND, pNONE, pNONE, pINDUCTOR, pSTATE, pNONE, pNONE, pNONE, pNONE,
			 pNONE, pNONE, pTSTBUF, pTSTINV, pMUX, pADDER, pNONE, pNONE, pNONE,
			 pAND4, pNAND4, pOR4, pNOR4, pXOR4, pXNOR4, pDELAY, pRES, 
			 pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE,
			 pRSL, pRSFF, pDFF, pTDFF, pJKFF, pMSFF, pTFF, pRSL, pRSFF, pDFF,
			 pTDFF, pJKFF, pMSFF, pTFF, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE,
			 pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE,
			 pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE, pNONE}; // 99


// Delay Function Mappings
static const propDelay_func propDelay[FF_max_id+1] = {dNONE, dBUF, dBUF, dAND, dAND, dOR, dOR, 
			 dXOR, dXOR, dAND3, dAND3, dOR3, dOR3, dXOR3, dXOR3, dNONE, dNONE, 
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, 
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE,
			 dTSTBUF, dTSTBUF, dMUX, dADDER, dMOSFET, dMOSFET, dNONE, dAND4, dAND4,
			 dOR4, dOR4, dXOR4, dXOR4, dDELAY, dNONE, 
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE,
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE,
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE,
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE,
			 dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE, dNONE}; // 99

// Controlling Value Functions
static const int isControllingValue[MAX_USED_ID+1] = {cvNO, cvNO, 
			 cvNO, cvAND, cvAND, cvOR, cvOR, cvNO, cvNO, cvAND, cvAND, cvOR, 
			 cvOR, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, 
			 cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, 
			 cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, cvNO, 
			 cvNO, cvAND, cvAND, cvOR, cvOR, cvNO, cvNO, cvNO, cvNO};
	
			 
			 
#endif
