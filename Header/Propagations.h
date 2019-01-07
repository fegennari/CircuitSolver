#ifndef _PROPAGATION_H_
#define _PROPAGATION_H_



// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, Propagation.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Gate Propagations for Logic Engine header
// includes both logic propagations and delay propagations
// By Frank Gennari
#define def_val 7
#define no_val  8


typedef char   (*propValue_func)(char *, comp_sim&);
typedef double (*propDelay_func)(char *, int *, double *);



// Propagation Function Prototypes
char pNONE(char     *NODEVALUE, comp_sim &comp);

char pBUF(char      *NODEVALUE, comp_sim &comp);
char pINV(char      *NODEVALUE, comp_sim &comp);

char pAND(char      *NODEVALUE, comp_sim &comp);
char pNAND(char     *NODEVALUE, comp_sim &comp);
char pOR(char       *NODEVALUE, comp_sim &comp);
char pNOR(char      *NODEVALUE, comp_sim &comp);
char pXOR(char      *NODEVALUE, comp_sim &comp);
char pXNOR(char     *NODEVALUE, comp_sim &comp);

char pAND3(char     *NODEVALUE, comp_sim &comp);
char pNAND3(char    *NODEVALUE, comp_sim &comp);
char pOR3(char      *NODEVALUE, comp_sim &comp);
char pNOR3(char     *NODEVALUE, comp_sim &comp);
char pXOR3(char     *NODEVALUE, comp_sim &comp);
char pXNOR3(char    *NODEVALUE, comp_sim &comp);

char pAND4(char     *NODEVALUE, comp_sim &comp);
char pNAND4(char    *NODEVALUE, comp_sim &comp);
char pOR4(char      *NODEVALUE, comp_sim &comp);
char pNOR4(char     *NODEVALUE, comp_sim &comp);
char pXOR4(char     *NODEVALUE, comp_sim &comp);
char pXNOR4(char    *NODEVALUE, comp_sim &comp);

char pOPAMP(char    *NODEVALUE, comp_sim &comp);
char pCAP(char      *NODEVALUE, comp_sim &comp);
char pRES(char      *NODEVALUE, comp_sim &comp);
char pPOWER(char    *NODEVALUE, comp_sim &comp);
char pGROUND(char   *NODEVALUE, comp_sim &comp);
char pINDUCTOR(char *NODEVALUE, comp_sim &comp);
char pSTATE(char    *NODEVALUE, comp_sim &comp);
char pTSTBUF(char   *NODEVALUE, comp_sim &comp);
char pTSTINV(char   *NODEVALUE, comp_sim &comp);
char pMUX(char      *NODEVALUE, comp_sim &comp);
char pADDER(char    *NODEVALUE, comp_sim &comp);

char pRSL(char      *NODEVALUE, comp_sim &comp);
char pRSFF(char     *NODEVALUE, comp_sim &comp);
char pDFF(char      *NODEVALUE, comp_sim &comp);
char pTDFF(char     *NODEVALUE, comp_sim &comp);
char pJKFF(char     *NODEVALUE, comp_sim &comp);
char pMSFF(char     *NODEVALUE, comp_sim &comp);
char pTFF(char      *NODEVALUE, comp_sim &comp);

char pDELAY(char    *NODEVALUE, comp_sim &comp);


// Delay Function Prototypes
double dNONE(char   *NODEVALUE, int *conn, double *net_delay);

double dBUF(char    *NODEVALUE, int *conn, double *net_delay);

double dAND(char    *NODEVALUE, int *conn, double *net_delay);
double dOR(char     *NODEVALUE, int *conn, double *net_delay);
double dXOR(char    *NODEVALUE, int *conn, double *net_delay);

double dAND3(char   *NODEVALUE, int *conn, double *net_delay);
double dOR3(char    *NODEVALUE, int *conn, double *net_delay);
double dXOR3(char   *NODEVALUE, int *conn, double *net_delay);

double dAND4(char   *NODEVALUE, int *conn, double *net_delay);
double dOR4(char    *NODEVALUE, int *conn, double *net_delay);
double dXOR4(char   *NODEVALUE, int *conn, double *net_delay);

double dTSTBUF(char *NODEVALUE, int *conn, double *net_delay);
double dMUX(char    *NODEVALUE, int *conn, double *net_delay);
double dADDER(char  *NODEVALUE, int *conn, double *net_delay);

double dMOSFET(char *NODEVALUE, int *conn, double *net_delay);

double dDELAY(char  *NODEVALUE, int *conn, double *net_delay);



#endif




