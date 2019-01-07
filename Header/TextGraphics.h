#ifndef _TEXT_GRAPHICS_H_
#define _TEXT_GRAPHICS_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, TextGraphics.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Text Based Graphics Header
// By Frank Gennari
char const text_NULL  = ' ';
char const text_VLINE = '|';
char const text_HLINE = '-';
char const text_VPIN  = '|';
char const text_HPIN  = '-';
char const text_INTER = '+';
char const text_NODE  = '*';
char const text_PIN   = 'O';
char const text_SEPAR = '.';
char const text_IC_S  = ')';
char const text_IC_M  = '+';
char const text_IC_E  = '|';


const string text_NO 		 = " ";
const string text_BUFFER	 = " |\\  .-| >-. |/  ";
const string text_NOT		 = " |\\  .-| O-. |/  ";
const string text_AND		 = "|-\\  .|  |-.|-/  ";
const string text_NAND		 = "|-\\  .|  |O.|-/  ";
const string text_OR		 = "|-\\  .)  |-.|-/  ";
const string text_NOR		 = "|-\\  .)  |O.|-/  ";
const string text_XOR		 = "|-\\  .)) |-.|-/  ";
const string text_XNOR		 = "|-\\  .)) |O.|-/  ";
const string text_TAND		 = "|-\\  .|  |-.|-/  ";
const string text_TNAND		 = "|-\\  .|  |O.|-/  ";
const string text_TOR		 = "|-\\  .)  |-.|-/  ";
const string text_TNOR		 = "|-\\  .)  |O.|-/  ";
const string text_TXOR		 = "|-\\  .)) |-.|-/  ";
const string text_TXNOR		 = "|-\\  .)) |O.|-/  ";
const string text_FF 		 = "-+-+-.-| |-.-+-+-";
const string text_OPAMP		 = "-|\\  .-| >-.+|/  ";
const string text_IC 		 = "| | | | | | |.+-+-+-+-+-+-+.)           |.+-+-+-+-+-+-+.| | | | | | |";
const string text_TRANSISTOR = " |/--.-|   . |\\>-";
const string text_CAP 		 = "-||--";
const string text_RES		 = "-\\/\\-";
const string text_DIODE		 = " |\\| .-| >-. |/| ";
const string text_INPUT		 = "O-";
const string text_OUTPUT	 = "-O";
const string text_POWER		 = "^+.| ";
const string text_GROUND	 = " | .---. = ";
const string text_CUSWIRE	 = "-";
const string text_BATTERY	 = "  |+ .-||--.  |  ";
const string text_INDUCTOR	 = "-@@@-";
const string text_ST_SEL	 = "1";
const string text_V_SOURCE	 = " /-\\+.-| |-. \\-/ ";
const string text_I_SOURCE	 = " /-\\ .-|>|-. \\-/ ";
const string text_VCVS		 = "+O O-.  _ +.-<_>-";
const string text_CCVS		 = "-->--.  _ +.-<_>-";
const string text_VCCS		 = "+O O-.  _ >.-<_>-";
const string text_CCCS		 = "-->--.  _ >.-<_>-";
const string text_T_ST_BUF	 = "--|  . |\\  .-| >-. |/  ";
const string text_T_ST_INV	 = "--|  . |\\  .-| >O. |/  ";
const string text_MUX		 = "--|  .-|-\\ . |  |.-|-/ ";
const string text_ADDER		 = "--|  .-|-\\ . >  |.-|-/ .  |--";
const string text_NFET		 = " ||--.-||  . ||--";
const string text_PFET		 = " ||--.-O|  . ||--";
const string text_XFMR		 = "-@|@-. @|@ .-@|@-";
const string text_QAND		 = "|    .|-\\  .|  |-.|-/  ";
const string text_QNAND		 = "|    .|-\\  .|  |O.|-/  ";
const string text_QOR		 = "|    .|-\\  .)  |-.|-/  ";
const string text_QNOR		 = "|    .|-\\  .)  |O.|-/  ";
const string text_QXOR		 = "|    .|-\\  .)) |-.|-/  ";
const string text_QXNOR		 = "|    .|-\\  .)) |O.|-/  ";
const string text_DELAY		 = ">[]";



// rotated
const string text_rNO 		 = " ";
const string text_rBUFFER	 = "  /| .-> |-.  \\| ";
const string text_rNOT		 = "  /| .-O |-.  \\| ";
const string text_rAND		 = "  /-|.-|  |.  \\-|";
const string text_rNAND		 = "  /-|.O|  |.  \\-|";
const string text_rOR		 = "  /-|.-|  (.  \\-|";
const string text_rNOR		 = "  /-|.O|  (.  \\-|";
const string text_rXOR		 = "  /-|.-| ((.  \\-|";
const string text_rXNOR		 = "  /-|.O| ((.  \\-|";
const string text_rTAND		 = "  /-|.-|  |.  \\-|";
const string text_rTNAND	 = "  /-|.O|  |.  \\-|";
const string text_rTOR		 = "  /-|.-|  (.  \\-|";
const string text_rTNOR		 = "  /-|.O|  (.  \\-|";
const string text_rTXOR		 = "  /-|.-| ((.  \\-|";
const string text_rTXNOR	 = "  /-|.O| ((.  \\-|";
const string text_rFF 		 = "-+-+-.-| |-.-+-+-";
const string text_rOPAMP	 = "  /|+.-< |-.  \\|_";
const string text_rIC 		 = "| | | | | | |.+-+-+-+-+-+-+.)           |.+-+-+-+-+-+-+.| | | | | | |";
const string text_rTRANSISTOR= "-<\\| .   |-.--/| ";
const string text_rCAP 		 = "|.-.-.|.|";
const string text_rRES		 = "|./.\\./.|";
const string text_rDIODE	 = " |/| .-< |-.  \\| ";
const string text_rINPUT	 = "-O";
const string text_rOUTPUT	 = "O-";
const string text_rPOWER	 = "| ._+";
const string text_rGROUND	 = " = .---. | ";
const string text_rCUSWIRE	 = "|";
const string text_rBATTERY	 = "  |+ .-||--.  |  ";
const string text_rINDUCTOR	 = "|.@.@.@.|";
const string text_rST_SEL	 = "0";
const string text_rV_SOURCE	 = " |+./-\\.| |.\\-/. | ";
const string text_rI_SOURCE	 = " | ./-\\.|^|.\\-/. | ";
const string text_rVCVS		 = "++|.O ^. ||.O -.| |";
const string text_rCCVS		 = "|^|.| ^.|||.| -.|/|";
const string text_rVCCS		 = "++|.O ^. ||.O -.| |";
const string text_rCCCS		 = "|^|.| ^.|||.| -.|/|";
const string text_rT_ST_BUF	 = "  /|.-< |-.  \\|.  |--";
const string text_rT_ST_INV	 = "  /|.-O |-.  \\|.  |--";
const string text_rMUX		 = " /-|-.|  | . \\-|-.  |--";
const string text_rADDER	 = "--| . /-|-.|  < . \\-|-.  |--";
const string text_rNFET		 = "--|| .  ||-.--|| ";
const string text_rPFET		 = "--|| .  |O-.--|| ";
const string text_rXFMR		 = "| |.@@@.===.@@@.| |";
const string text_rQAND		 = "  /-|.-|  |.  \\-|.    |"; // got here
const string text_rQNAND	 = "  /-|.O|  |.  \\-|.    |";
const string text_rQOR		 = "  /-|.-|  (.  \\-|.    |";
const string text_rQNOR		 = "  /-|.O|  (.  \\-|.    |";
const string text_rQXOR		 = "  /-|.-| ((.  \\-|.    |";
const string text_rQXNOR	 = "  /-|.O| ((.  \\-|.    |";
const string text_rDELAY	 = "/.~.~";


const string text_NPN        = " |/--.-|   . |\\>-";
const string text_PNP 		 = " |/--.-|   . |\\<-";

const string text_rNPN		 = "-<\\| .   |-.--/| ";
const string text_rPNP		 = "->\\| .   |-.--/| ";

const string text_IC_START   = "|.+.).+.|";
const string text_FF_BASE    = " +-+ . | | . +-+ ";

int const NPN_TEXT_ID = MAX_USED_ID + 1;
int const PNP_TEXT_ID = MAX_USED_ID + 2;
int const FF_TEXT_ID  = MAX_USED_ID + 3;
int const IC_TEXT_ID  = MAX_USED_ID + 4;
int const MAX_TEXT_ID = MAX_USED_ID + 4;


// fix IC - resize, flipflop pins

const string text_comps[MAX_USED_ID+5] = {text_NO, text_BUFFER, text_NOT, text_AND, text_NAND,
	  text_OR, text_NOR, text_XOR, text_XNOR, text_TAND, text_TNAND, text_TOR,
	  text_TNOR, text_TXOR, text_TXNOR, text_FF, text_OPAMP, text_IC, text_TRANSISTOR,
	  text_CAP, text_RES, text_DIODE, text_INPUT, text_OUTPUT, text_POWER,
	  text_GROUND, text_CUSWIRE, text_BATTERY, text_INDUCTOR, text_ST_SEL,
	  text_V_SOURCE, text_I_SOURCE, text_VCVS, text_CCVS, text_VCCS, text_CCCS,
	  text_T_ST_BUF, text_T_ST_INV, text_MUX, text_ADDER, text_NFET, text_PFET,
	  text_XFMR, text_QAND, text_QNAND, text_QOR, text_QNOR, text_QXOR, text_QXNOR,
	  text_DELAY, text_NPN, text_PNP, text_FF_BASE, text_IC_START};
	  
const string text_comps_rot[MAX_USED_ID+5] = {text_rNO, text_rBUFFER, text_rNOT, text_rAND, text_rNAND,
	  text_rOR, text_rNOR, text_rXOR, text_rXNOR, text_rTAND, text_rTNAND, text_rTOR,
	  text_rTNOR, text_rTXOR, text_rTXNOR, text_rFF, text_rOPAMP, text_rIC, text_rTRANSISTOR,
	  text_rCAP, text_rRES, text_rDIODE, text_rINPUT, text_rOUTPUT, text_rPOWER,
	  text_rGROUND, text_rCUSWIRE, text_rBATTERY, text_rINDUCTOR, text_rST_SEL,
	  text_rV_SOURCE, text_rI_SOURCE, text_rVCVS, text_rCCVS, text_rVCCS, text_rCCCS,
	  text_rT_ST_BUF, text_rT_ST_INV, text_rMUX, text_rADDER, text_rNFET, text_rPFET,
	  text_rXFMR, text_rQAND, text_rQNAND, text_rQOR, text_rQNOR, text_rQXOR, text_rQXNOR,
	  text_rDELAY, text_rNPN, text_rPNP, text_FF_BASE, text_IC_START};
	  
const int text_comp_x_offset[MAX_USED_ID+5] = {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	  2, 2, 2, 2, 2, 0, 1, 2, 2, 2, 2, -1, 0, 1, 0, 2, 2, 0, 2, 2, 2, 2, 2, 2,
	  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 0};
	  
const int text_comp_y_offset[MAX_USED_ID+5] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1,
	  2, 2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1, 1, 1, 2};

const int text_comp_x_offset_rot[MAX_USED_ID+5] = {0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	  2, 2, 2, 2, 2, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1,
	  2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 0};
	  
const int text_comp_y_offset_rot[MAX_USED_ID+5] = {0, 2, 1, 2, 2, 2, 2, 2, 2, 1, 1, 1,
	  1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 1, 0, 0, 2, 0, 0, 1, 1, 1, 1, 1, 1,
	  1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2};



#endif



