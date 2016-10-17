#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "ray.h"
#include "vect.h"
#include "color.h"

class Object {

	public:
		Object();

		virtual Color getColor() { return Color(0.0, 0.0, 0.0, 0); }
		virtual Vect getNormalAt(Vect intersection_position){ return Vect (0, 0, 0); }
		virtual double findIntersection(Ray ray) { return 0; }
};

#endif // __OBJECT_H__