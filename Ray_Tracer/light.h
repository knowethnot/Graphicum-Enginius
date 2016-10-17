#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "source.h"
#include "vect.h"
#include "color.h"

class Light : public Source {
	Vect position;
	Color color;

	public:
		Light();
		Light(Vect p, Color c);

		virtual Vect getLightPosition() { return position; }
		virtual Color getLightColor() { return color; }
};

#endif // __LIGHT_H__