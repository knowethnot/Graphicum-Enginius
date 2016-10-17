#ifndef __SOURCE_H__
#define __SOURCE_H__

#include "vect.h"
#include "color.h"

class Source {
	public:
		Source();

		virtual Vect getLightPosition() { return Vect(0, 0, 0); }
		virtual Color getLightColor() { return Color(1, 1, 1, 0); }
};

#endif // __SOURCE_H__