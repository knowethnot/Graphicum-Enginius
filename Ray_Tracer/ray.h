#ifndef __RAY_H__
#define __RAY_H__

#include "vect.h"

class Ray {
	Vect origin, direction;

	public:
		Ray();
		Ray(Vect o, Vect d);

		Vect getRayOrigin() { return origin; }
		Vect getRayDirection() { return direction; }
};

#endif // __RAY_H__