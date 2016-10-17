#include "sphere.h"

Sphere::Sphere()
{
	center = Vect(0,0,0);
	radius = 1.0;
	color = Color(0.5, 0.5, 0.5, 0);	
}

Sphere::Sphere(Vect centerValue, double radiusValue, Color colorValue)
{
	center = centerValue;
	radius = radiusValue;
	color = colorValue;
}