#include "color.h"

Color::Color()
{
	red = 0.5;
	green = 0.5;
	blue = 0.5;
}

Color::Color(double r, double g, double b, double s)
{
	red = r;
	green = g;
	blue = b;
	special = s;
}

Color Color::colorScalar(double  scalar)
{
	return Color(red*scalar,
				 green*scalar,
				 blue*scalar,
				 special);
}

Color Color::colorAdd(Color color)
{
	return Color(red + color.getColorRed(),
		green + color.getColorGreen(),
		blue + color.getColorBlue(), special);
}

Color Color::colorMultiply(Color color)
{
	return Color(red * color.getColorRed(),
		green * color.getColorGreen(),
		blue * color.getColorBlue(), special);
}

Color Color::colorAvg(Color color)
{
	return Color((red + color.getColorRed()) / 2,
		(green + color.getColorGreen()) / 2, 
		(blue + color.getColorBlue()) / 2,
		special);
}

Color Color::clip()
{
	double allLight = red + green + blue;
	double excessLight = allLight - 3;

	if (excessLight > 0)
	{
		red = red + excessLight * (red / allLight);
		green = green + excessLight * (green / allLight);
		blue = blue + excessLight * (blue / allLight);
	}

	if (red > 1) { red = 1; }
	if (green > 1) { green = 1; }
	if (blue > 1) { blue = 1; }
	if (red < 0) { red = 0; }
	if (green < 0 ) { green = 0; }
	if (blue < 0 ) { blue = 0; }

	return Color(red, green, blue, special);
}