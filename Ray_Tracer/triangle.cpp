#include "triangle.h"

Triangle::Triangle()
{
	A = Vect(1, 0, 0);
	B = Vect(0, 1, 0);
	C = Vect(0, 0, 1);
	color = Color(0.5, 0.5, 0.5, 0);
}

Triangle::Triangle(Vect pointA, Vect pointB, Vect pointC, Color colorValue)
{
	A = pointA;
	B = pointB;
	C = pointC;
	color = colorValue;
}