#ifndef _COMPONENT_ID_H_
#define _COMPONENT_ID_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, ComponentID.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Component ID Header
// By Frank Gennari


// component ID allocations
int const Component_max_id =	59;
int const FF_base_id =			60;
int const FF_max_id	=			99;
int const Q_base_id	=			100;
int const Q_max_id =			999;
int const IC_base_id =			1000;
int const IC_max_id	=			49999;
int const Wire_base_id =		50000;
int const Wire_max_id =			1000000000;
int const num_ffs = 			7;

#define NO_ID			0
#define BUFFER_ID		1	
#define NOT_ID			2
#define AND_ID			3
#define NAND_ID			4
#define OR_ID			5
#define NOR_ID			6
#define XOR_ID			7
#define XNOR_ID			8
#define TAND_ID			9
#define TNAND_ID		10
#define TOR_ID			11
#define TNOR_ID			12
#define TXOR_ID			13
#define TXNOR_ID		14
#define FF_ID			15
#define OPAMP_ID		16
#define IC_ID			17
#define NPN_ID			18
#define TRANSISTOR_ID	18
#define CAP_ID			19
#define RES_ID			20
#define DIODE_ID		21
#define INPUT_ID		22
#define OUTPUT_ID		23
#define POWER_ID		24
#define GROUND_ID		25
#define CUSWIRE_ID		26
#define BATTERY_ID		27
#define INDUCTOR_ID		28
#define ST_SEL_ID		29
#define V_SOURCE_ID		30
#define I_SOURCE_ID		31
#define DEP_SRC_ID		32
#define VCVS_ID			32
#define CCVS_ID			33
#define VCCS_ID			34
#define CCCS_ID			35
#define MAX_DEP_SRC_ID	35
#define T_ST_BUF_ID		36
#define T_ST_INV_ID		37
#define MUX_ID			38
#define ADDER_ID		39
#define FET_ID			40
#define NFET_ID			40
#define PFET_ID			41
#define XFMR_ID			42
#define QAND_ID			43
#define QNAND_ID		44
#define QOR_ID			45
#define QNOR_ID			46
#define QXOR_ID			47
#define QXNOR_ID		48
#define DELAY_ID        49
#define TLINE_ID        50

#define PNP_ID			111 // a temporary ID

#define FF_SEL			25
#define XFMR_SEL		34
#define DEP_SRC_SEL		39
#define FUNCTION_SEL	47

#define NO_FF			-100


enum {NO_FF_ID, RSL_ID, RSFF_ID, DFF_ID, TDFF_ID, JKFF_ID, MSFF_ID, TFF_ID};

int const PRSL_ID =		FF_base_id + RSL_ID;
int const PRSFF_ID =	FF_base_id + RSFF_ID;
int const PDFF_ID =		FF_base_id + DFF_ID;
int const PTDFF_ID =	FF_base_id + TDFF_ID;
int const PJKFF_ID =	FF_base_id + JKFF_ID;
int const PMSFF_ID =	FF_base_id + MSFF_ID;
int const PTFF_ID =		FF_base_id + TFF_ID;
int const NRSL_ID =		FF_base_id + num_ffs + RSL_ID;
int const NRSFF_ID =	FF_base_id + num_ffs + RSFF_ID;
int const NDFF_ID =		FF_base_id + num_ffs + DFF_ID;
int const NTDFF_ID =	FF_base_id + num_ffs + TDFF_ID;
int const NJKFF_ID =	FF_base_id + num_ffs + JKFF_ID;
int const NMSFF_ID =	FF_base_id + num_ffs + MSFF_ID;
int const NTFF_ID =		FF_base_id + num_ffs + TFF_ID;

int const MAX_USED_ID     = 50;
int const num_gate_types  = 6;
int const MAX_NEW_USED_ID = MAX_USED_ID + num_gate_types;

int const T_GATE_OFFSET = TAND_ID - AND_ID;
int const Q_GATE_OFFSET = QAND_ID - AND_ID;


#endif


