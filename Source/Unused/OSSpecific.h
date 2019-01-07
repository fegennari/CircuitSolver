#ifndef _OS_SPECIFIC_H_
#define _OS_SPECIFIC_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, OSSpecific.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver OS Specific Header
// Mac OS uses the SIOUX graphics library, C.S. Graphics.cc, and C.S. Interface.cc
// Windows 95/98 uses WNDCLASS and other windows stuff
// By Frank Gennari
void MoveTo(int x, int y);
void LineTo(int x, int y);





struct Point {
	int h, v;
};


#endif