#ifndef __COLOR_H__
#define __COLOR_H__

class Color {
	double red, green, blue, special;

	public:
		Color();
		Color(double r, double g, double b, double s);

		double getColorRed() { return red; }
		double getColorGreen() { return green; }
		double getColorBlue() { return blue; }
		double getColorSpecial() { return special; }

		double setColorRed(double redValue) { return red = redValue; };
		double setColorGreen(double greenValue) { return green = greenValue; };
		double setColorBlue(double blueValue) { return blue = blueValue; };
		double setColorSpecial(double specialValue) { return special = specialValue; };

		double brightness() { return(red + green + blue) / 3; }

		Color colorScalar(double scalar);
		Color colorAdd(Color color);
		Color colorMultiply(Color color);
		Color colorAvg(Color color);

		Color clip();
};

#endif // __COLOR_H__