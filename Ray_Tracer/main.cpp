#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "vect.h"
#include "ray.h"
#include "camera.h"
#include "color.h"
#include "source.h"
#include "light.h"
#include "object.h"
#include "sphere.h"
#include "plane.h"
#include "triangle.h"

struct rgbType {
	double r;
	double g;
	double b;	
};

void saveBMP(const char *filename, int w, int h, int dpi, rgbType *data)
{
	FILE *f;
	int k = w * h;
	int s = 4 * k;
	int filesize = 54 + s;

	double factor = 39.375;
	int m = static_cast<int>(factor);

	int ppm = dpi * m;

	unsigned char bmpFileHeader[14] = { 'B', 'M', 0,0,0,0, 0,0,0,0, 54,0,0,0 };
	unsigned char bmpInfoHeader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,24,0 };

	bmpFileHeader[2] = (unsigned char)(filesize);
	bmpFileHeader[3] = (unsigned char)(filesize >> 8);
	bmpFileHeader[4] = (unsigned char)(filesize >> 16);
	bmpFileHeader[5] = (unsigned char)(filesize >> 24);

	bmpInfoHeader[4] = (unsigned char)(w);
	bmpInfoHeader[5] = (unsigned char)(w >> 8);
	bmpInfoHeader[6] = (unsigned char)(w >> 16);
	bmpInfoHeader[7] = (unsigned char)(w >> 24);

	bmpInfoHeader[8]  = (unsigned char)(h);
	bmpInfoHeader[9]  = (unsigned char)(h >> 8);
	bmpInfoHeader[10] = (unsigned char)(h >> 16);
	bmpInfoHeader[11] = (unsigned char)(h >> 24);

	bmpInfoHeader[21] = (unsigned char)(s);
	bmpInfoHeader[22] = (unsigned char)(s >> 8);
	bmpInfoHeader[23] = (unsigned char)(s >> 16);
	bmpInfoHeader[24] = (unsigned char)(s >> 24);

	bmpInfoHeader[25] = (unsigned char)(ppm);
	bmpInfoHeader[26] = (unsigned char)(ppm >> 8);
	bmpInfoHeader[27] = (unsigned char)(ppm >> 16);
	bmpInfoHeader[28] = (unsigned char)(ppm >> 24);

	bmpInfoHeader[29] = (unsigned char)(ppm);
	bmpInfoHeader[30] = (unsigned char)(ppm >> 8);
	bmpInfoHeader[31] = (unsigned char)(ppm >> 16);
	bmpInfoHeader[32] = (unsigned char)(ppm >> 24);

	f = fopen(filename,"wb");

	fwrite(bmpFileHeader,1,14,f);
	fwrite(bmpInfoHeader,1,40,f);

	for (int i = 0; i < k; ++i)
	{
		rgbType rgb;
		rgb = data[i];

		double red = (data[i].r) * 255;
		double green = (data[i].g) * 255;
		double blue = (data[i].b) * 255;

		unsigned char color[3] = { static_cast<unsigned char>(floor(blue)), static_cast<unsigned char>(floor(green)), static_cast<unsigned char>(floor(red)) };

		fwrite(color,1,3,f);
	}
	fclose(f);
}

int winningObjectIndex(std::vector<double> object_intersections)
{
	int index_of_minimum_value;	   // Return winning intersection

	if (object_intersections.size() == 0)
	{
		return -1;	// There are no Intersections
	}
	else if (object_intersections.size() == 1)
	{
		if (object_intersections.at(0) > 0)
		{
			return 0;	// Win intersect is > 0
		}
		else { return -1; }
	}
	else
	{
		double max = 0;		// Find maximum value

		for (int i = 0; i < object_intersections.size(); ++i)
		{
			if (max < object_intersections.at(i))
			{
				max = object_intersections.at(i);
			}
		}
		// Then Starting from the Maximum val find the Minimum Positive
		if (max > 0)
		{
			for (int index = 0; index < object_intersections.size(); ++index)
			{
				if (object_intersections.at(index) > 0 && object_intersections.at(index) <= max)
				{
					max = object_intersections.at(index);
					index_of_minimum_value = index;
				}
			}
			return index_of_minimum_value;
		}
		else {
			return -1;	// all the intersections were Negativa
		}
	}
}

Color getColorAt(Vect intersection_position, Vect intersection_ray_direction, std::vector<Object*> scene_objects, int index_of_winning_object, std::vector<Source*> light_sources, double accuracy, double ambientLight)
{
	Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
	Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);

	if (winning_object_color.getColorSpecial() == 2)
	{
		// Checkered/tile floor pattern

		int square = (int)floor(intersection_position.getVectX()) + (int)floor(intersection_position.getVectZ());
	
		if ((square % 2) == 0)
		{
			// black tile
			winning_object_color.setColorRed(0);
			winning_object_color.setColorGreen(0);
			winning_object_color.setColorBlue(0);
		}
		else
		{
			// white tile
			winning_object_color.setColorRed(1);
			winning_object_color.setColorGreen(1);
			winning_object_color.setColorBlue(1);
		}
	}

	Color final_color = winning_object_color.colorScalar(ambientLight);

	if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1)
	{
		// Reflecion from objects with specular intensity
		double dot1 = winning_object_normal.dotProduct(intersection_ray_direction.negative());
		Vect scalar1 = winning_object_normal.vectMultiply(dot1);
		Vect add1 = scalar1.vectAdd(intersection_ray_direction);
		Vect scalar2 = add1.vectMultiply(2);
		Vect add2 = intersection_ray_direction.negative().vectAdd(scalar2);
		Vect reflection_direction = add2.normalize();

		Ray reflection_ray(intersection_position, reflection_direction);

		// determine what the ray intersects with first

		std::vector<double> reflection_intersections;

		for (int reflection_index = 0; reflection_index < scene_objects.size(); ++reflection_index)
		{
			reflection_intersections.push_back(scene_objects.at(reflection_index)->findIntersection(reflection_ray));
		}

		int index_of_winning_object_with_reflection = winningObjectIndex(reflection_intersections);

		if (index_of_winning_object_with_reflection != -1)
		{
			// reflection ray missed everything else
			if (reflection_intersections.at(index_of_winning_object_with_reflection) > accuracy)
			{
				// determine the position and direction at the point of intersection

				Vect reflection_intersection_position = intersection_position.vectAdd(reflection_direction.vectMultiply(reflection_intersections.at(index_of_winning_object_with_reflection)));
				Vect reflection_intersection_ray_direction = reflection_direction;

				Color reflection_intersection_color = getColorAt(reflection_intersection_position, reflection_intersection_ray_direction, scene_objects, index_of_winning_object_with_reflection, light_sources, accuracy, ambientLight);
				final_color = final_color.colorAdd(reflection_intersection_color.colorScalar(winning_object_color.getColorSpecial()));
			}
		}
	}

	for (int light_index = 0; light_index < light_sources.size(); ++light_index)
	{
		Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();

		float cosine_angle = winning_object_normal.dotProduct(light_direction);

		if (cosine_angle > 0)
		{
			// test for shadows
			bool shadowed = false;

			Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
			float distance_to_light_magnitude = distance_to_light.magnitude();

			Ray shadow_ray(intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize());

			std::vector<double> secondary_intersections;

			for (int objects_index = 0; objects_index < scene_objects.size() && shadowed == false; ++objects_index)
			{
				secondary_intersections.push_back(scene_objects.at(objects_index)->findIntersection(shadow_ray));
			}

			for (int c = 0; c < secondary_intersections.size(); ++c)
			{
				if (secondary_intersections.at(c) > accuracy)
				{
					if (secondary_intersections.at(c) <= distance_to_light_magnitude)
					{
						shadowed = true;
					}
					break;
				}
			}

			if (shadowed == false)
			{
				final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));

				if (winning_object_color.getColorSpecial() > 0 && winning_object_color.getColorSpecial() <= 1)
				{
					// special [0-1]
					double dot1 = winning_object_normal.dotProduct(intersection_ray_direction.negative());
					Vect scalar1 = winning_object_normal.vectMultiply(dot1);
					Vect add1 = scalar1.vectAdd(intersection_ray_direction);
					Vect scalar2 = add1.vectMultiply(2);
					Vect add2 = intersection_ray_direction.negative().vectAdd(scalar2);
					Vect reflection_direction = add2.normalize();

					double specular = reflection_direction.dotProduct(light_direction);

					if (specular > 0)
					{
						specular = pow(specular, 10);
						final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular*winning_object_color.getColorSpecial()));
					}
				}
			}
		}
	}
	return final_color.clip();
}

std::vector<Object*> scene_objects;

void makeCube(Vect corner1, Vect corner2, Color color)
{
	double c1x = corner1.getVectX();
	double c1y = corner1.getVectY();
	double c1z = corner1.getVectZ();

	double c2x = corner2.getVectX();
	double c2y = corner2.getVectY();
	double c2z = corner2.getVectZ();

	Vect A(c2x, c1y, c1z);
	Vect B(c2x, c1y, c2z);
	Vect C(c1x, c1y, c2z);

	Vect D(c2x, c2y, c1z);
	Vect E(c1x, c2y, c1z);
	Vect F(c1x, c2y, c2z);

	scene_objects.push_back(new Triangle(D, A, corner1, color));
	scene_objects.push_back(new Triangle(corner1, E, D, color));

	scene_objects.push_back(new Triangle(corner2, B, A, color));
	scene_objects.push_back(new Triangle(A, D, corner2, color));

	scene_objects.push_back(new Triangle(F, C, B, color));
	scene_objects.push_back(new Triangle(B, corner2, F, color));

	scene_objects.push_back(new Triangle(E, corner1, C, color));
	scene_objects.push_back(new Triangle(C, F, E, color));

	scene_objects.push_back(new Triangle(D, E, F, color));
	scene_objects.push_back(new Triangle(F, corner2, corner1, color));

	scene_objects.push_back(new Triangle(corner1, B, C, color));
	scene_objects.push_back(new Triangle(B, C, corner1, color));
}

int main(int argc, char *argv[])
{
	std::cout << "\n[RENDERING]..................................................." << std::endl;

	clock_t t1, t2;
	t1 = clock();

	int dpi = 72;
	int width = 640;
	int height = 480;
	int n = width * height;
	rgbType *pixels = new rgbType[n];

	double accuracy = 0.00000001;
	double ambientLight = 0.2;
	int aadepth = 2;
	// double aathreshold = 0.1;
	double aspectRatio = (double)width / (double)height;

	Vect O1(0,0,0);
	Vect X1(1,0,0);
	Vect Y1(0,1,0);
	Vect Z1(0,0,1);

	Vect new_sphere_location(1.75, -0.25, 0);

	Vect campos(3, 1.5, -4);

	Vect look_at(0,0,0);
	Vect diff_btw(campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY(), campos.getVectZ() - look_at.getVectZ());

	Vect camdir = diff_btw.negative().normalize();
	Vect camright = Y1.crossProduct(camdir).normalize();
	Vect camdown = camright.crossProduct(camdir);

	Camera scene_cam(campos, camdir, camright, camdown);

	Color white_light(1.0, 1.0, 1.0, 0);
	Color pretty_green(0.5, 1.0, 0.5, 0.3);
	Color maroon(0.5, 0.25, 0.25, 0);
	Color orange(0.94, 0.75, 0.31, 0);
	Color gray(0.5, 0.5, 0.5, 0);
	Color black(0.0, 0.0, 0.0, 0.0);
	Color tile_floor(1.0, 1.0, 1.0, 2); // was red and black 1, 1, 1, 2

	// Sphere scene_sphere(O1, 1, pretty_green);
	Sphere scene_sphere2(new_sphere_location, 0.5, orange);
	Plane scene_plane(Y1, -1, tile_floor);
	// Triangle scene_triangle(Vect(3, 0, 0), Vect(0, 3, 0), Vect(0, 0, 3), orange);

	// std::vector<Object*> scene_objects;
	// scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_sphere2));
	scene_objects.push_back(dynamic_cast<Object*>(&scene_plane));
	// scene_objects.push_back(dynamic_cast<Object*>(&scene_triangle));

	Vect light_position(-7,10,-10);  // -7,10,-10
	Light scene_light(light_position, white_light);
	std::vector<Source*> light_sources;
	light_sources.push_back(dynamic_cast<Source*>(&scene_light));

	makeCube(Vect(1, 1, 1), Vect(-1, -1, -1), orange);

	int thisone, aa_index;
	double xamnt, yamnt;

//	double tempRed, tempGreen, tempBlue;

	for (int x = 0; x < width; ++x)
	{
		for (int y = 0; y < height; ++y)
		{
			thisone = y * width + x;

			// Start With Blank Pixelation

			double tempRed[aadepth*aadepth];
			double tempGreen[aadepth*aadepth];
			double tempBlue[aadepth*aadepth];

			for (int aax = 0; aax < aadepth; ++aax)
			{
				for (int aay = 0; aay < aadepth; ++aay)
				{	
					aa_index = aay * aadepth + aax;

					srand(time(0));

					// create the ray from the camera to this pixel
					
					if (aadepth == 1)
					{
						// Start with no anti-aliasing
						
						if (width > height)
						{
							// the img is wider than its height
							xamnt = ((x + 0.5) / width) * aspectRatio - (((width-height) / (double)height) / 2);
							yamnt = ((height - y) + 0.5) / height;
						}
						else if (height > width)
						{
							xamnt = (x + 0.5) / width;
							yamnt = (((height - y) + 0.5) / height) / aspectRatio - (((height - width) / (double)width) / 2);
						}
						else
						{
							// the image is square
							xamnt = (x + 0.5) / width;
							yamnt = ((height - y) + 0.5) / height;
						}
					}
					else
					{
						// Anti-Aliasing

						if (width > height)
						{
							// the image is wider than it is tall
							xamnt = ((x + (double)aax/((double)aadepth - 1))/width) * aspectRatio - (((width-height)/(double)height)/2);
							yamnt = ((height - y) + (double)aax/((double)aadepth - 1)) / height;
						}
						else if (height > width)
						{
							// the image is taller than it is wide
							xamnt = (x + (double)aax / ((double)aadepth - 1)) / width;
							yamnt = (((height - y) + (double)aax/((double)aadepth - 1))/height) / aspectRatio - (((height-width)/(double)width)/2);
						}
						else
						{
							// the image is square
							xamnt = (x + (double)aax/((double)aadepth - 1)) / width;
							yamnt = ((height - y) + (double)aax/((double)aadepth - 1)) / height;
						}

						Vect cam_ray_origin = scene_cam.getCameraPosition();
						Vect cam_ray_direction = camdir.vectAdd(camright.vectMultiply(xamnt - 0.5).vectAdd(camdown.vectMultiply(yamnt - 0.5))).normalize();

						Ray cam_ray(cam_ray_origin, cam_ray_direction);

						std::vector<double> intersections;

						for (int index = 0; index < scene_objects.size(); ++index)
						{
							intersections.push_back(scene_objects.at(index)->findIntersection(cam_ray));
						}

						int index_of_winning_object = winningObjectIndex(intersections);

						// if ((x > 200 && x < 440) && (y > 200 && y < 280))
						
						if (index_of_winning_object == -1)
						{
							tempRed[aa_index] = 0.0;
							tempGreen[aa_index] = 0.0;
							tempBlue[aa_index] = 0.0;
						}
						else
						{
							if (intersections.at(index_of_winning_object) > accuracy)
							{
								// Determine the position and the directional vectors at point of intersection
									
								Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMultiply(intersections.at(index_of_winning_object)));
								Vect intersection_ray_direction = cam_ray_direction;

								Color intersection_color = getColorAt(intersection_position, intersection_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientLight);

								tempRed[aa_index] = intersection_color.getColorRed();
								tempGreen[aa_index] = intersection_color.getColorGreen();
								tempBlue[aa_index] = intersection_color.getColorBlue();
							}
						}
					}
					
				}

			}
			// averagign the pixelative color

			double totalRed = 0;
			double totalGreen = 0;
			double totalBlue = 0;

			for (int iRed = 0; iRed < aadepth * aadepth; ++iRed)
			{
				totalRed = totalRed + tempRed[iRed];
			}
			for (int iGreen = 0; iGreen < aadepth * aadepth; ++iGreen)
			{
				totalGreen = totalGreen + tempGreen[iGreen];
			}
			for (int iBlue = 0; iBlue < aadepth * aadepth; ++iBlue)
			{
				totalBlue = totalBlue + tempBlue[iBlue];
			}

			double avgRed = totalRed / (aadepth*aadepth);
			double avgGreen = totalGreen / (aadepth*aadepth);
			double avgBlue = totalBlue / (aadepth*aadepth);

			pixels[thisone].r = avgRed;
			pixels[thisone].r = avgGreen;
			pixels[thisone].r = avgBlue;
		}
	}
	saveBMP("scene.bmp", width,height,dpi,pixels);

	t2 = clock();
	float diff = ((float)t2 - (float)t1) / 1000;

	std::cout << diff << " seconds" << std::endl;

	return 0;
}