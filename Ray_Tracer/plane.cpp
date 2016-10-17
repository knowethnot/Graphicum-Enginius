#include "plane.h"

Plane::Plane()
{
	normal = Vect(1,0,0);
	distance = 0.0;
	color = Color(0.5,0.5,0.5, 0);
}

Plane::Plane(Vect normalValue, double distanceValue, Color colorValue)
{
	normal = normalValue;
	distance = distanceValue;
	color = colorValue;
}