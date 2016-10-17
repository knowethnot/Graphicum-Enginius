#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vect.h"

class Camera {
	Vect campos, camdir, camright, camdown;

	public:
		Camera();
		Camera(Vect pos, Vect dir, Vect right, Vect camdown);

		Vect getCameraPosition() { return campos; }
		Vect getCameraDirection() { return camdir; }
		Vect getCameraRight() { return camright; }
		Vect getCameraDown() { return camdown; }
};

#endif // __CAMERA_H__