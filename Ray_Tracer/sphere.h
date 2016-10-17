#ifndef __SPHERE_H__
#define __SPHERE_H__

#include <cmath>

#include "object.h"
#include "vect.h"
#include "color.h"

class Sphere : public Object {
	Vect center;
	double radius;
	Color color;

	public:
		Sphere();
		Sphere(Vect centerValue, double radiusValue, Color colorValue);
		
		Vect getSphereCenter() { return center; }
		double getSphereRadius() { return radius; }
		virtual Color getColor() { return color; }

		Vect getNormalAt(Vect point)
		{
			// Normaly points away from the center of the sphere
			Vect noraml_Vect = point.vectAdd(center.negative()).normalize();
			return noraml_Vect;
		}

		virtual double findIntersection(Ray ray)
		{
			Vect ray_origin = ray.getRayOrigin();
			double ray_origin_x = ray_origin.getVectX();
			double ray_origin_y = ray_origin.getVectY();
			double ray_origin_z = ray_origin.getVectZ();

			Vect ray_direction = ray.getRayDirection();
			double ray_direction_x =  ray_direction.getVectX();
			double ray_direction_y = ray_direction.getVectY();
			double ray_direction_z = ray_direction.getVectZ();
		
			Vect sphere_center = center;
			double sphere_center_x = sphere_center.getVectX();
			double sphere_center_y = sphere_center.getVectY();
			double sphere_center_z = sphere_center.getVectZ();

			double a = 1;	// Normalized
			double b = (2 * (ray_origin_x - sphere_center_x) * ray_direction_x) + (2 * (ray_origin_y - sphere_center_y) * ray_direction_y) + (2 * (ray_origin_z - sphere_center_z) * ray_direction_z);
			double c = pow(ray_origin_x - sphere_center_x, 2) + pow(ray_origin_y - sphere_center_y, 2) + pow(ray_origin_z - sphere_center_z, 2) - (radius * radius);

			double discriminant = b * b - 4 * c;

			if (discriminant > 0)
			{
				// The Ray Intersects the Sphere
				double root_1 = ((-1 * b - sqrt(discriminant)) / 2) - 0.0001;	// The First Root

				if (root_1 > 0)
				{
					return root_1;	// first is the smallest positive root
				}
				else {
					double root_2 = ((sqrt(discriminant) - b) / 2) - 0.0001;  // the second root is the smallest positive root
					return root_2;
				}
			}
			else {
				// the ray missed the sphere;
				return a - 2;	// Or Return -1
			}
		}
};

#endif // __SPHERE_H__