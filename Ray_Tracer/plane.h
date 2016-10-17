#ifndef __PLANE_H__
#define __PLANE_H__

#include <cmath>
#include "object.h"
#include "vect.h"
#include "color.h"

class Plane : public Object {
	Vect normal;
	double distance;
	Color color;

	public:
		Plane();
		Plane(Vect normalValue, double distanceValue, Color colorValue);
		
		Vect getPlaneNormal() { return normal; }
		double getPlaneDistance() { return distance; }
		virtual Color getColor() { return color; }

		Vect getNormalAt(Vect point) { return normal; }

		virtual double findIntersection(Ray ray)
		{
			Vect ray_direction = ray.getRayDirection();
			
			double a = ray_direction.dotProduct(normal);

			if (a == 0)
			{
				return -1;	// Ray is Parrelel to the plane
			}
			else {
				double b = normal.dotProduct(ray.getRayOrigin().vectAdd(normal.vectMultiply(distance).negative()));
				return -1 * b / a;	// Distance from the ray origin to the intersection
			}
		}
};

#endif // __PLANE_H__