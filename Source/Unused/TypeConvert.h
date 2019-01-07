#ifndef _TYPE_CONVERT_H_
#define _TYPE_CONVERT_H_


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, TypeConvert.h, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Drawing Function int->double type converter
// By Frank Gennari
void MoveTo(double x, double y);
void LineTo(double x, double y);
void LineDraw(double x1, double y1, double x2, double y2);
void RectangleDraw(double x1, double y1, double x2, double y2);
void RectangleFrame(double x1, double y1, double x2, double y2);
void CircleDraw(double x, double y, double rad);
void CircleFrame(double x, double y, double rad);
void ArcFrame(double x, double  y, double radius, double startDegrees, double NumDegrees);



void MoveTo(double x, double y) {MoveTo((int)x, (int)y);}
void LineTo(double x, double y) {LineTo((int)x, (int)y);}
void LineDraw(double x1, double y1, double x2, double y2) {LineDraw((int)x1, (int)y1, (int)x2, (int)y2);}
void RectangleDraw(double x1, double y1, double x2, double y2) {RectangleDraw((int)x1, (int)y1, (int)x2, (int)y2);}
void RectangleFrame(double x1, double y1, double x2, double y2) {RectangleFrame((int)x1, (int)y1, (int)x2, (int)y2);}
void CircleDraw(double x, double y, double rad) {CircleDraw((int)x, (int)y, (int)rad);}
void CircleFrame(double x, double y, double rad) {CircleFrame((int)x, (int)y, (int)rad);}
void ArcFrame(double x, double  y, double radius, double startDegrees, double NumDegrees) {ArcFrame((int)x, (int)y, (int)radius, (int)startDegrees, (int)NumDegrees);}


#endif