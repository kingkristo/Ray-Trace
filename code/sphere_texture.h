#ifndef SPHERE_TEXTURE_H
#define SPHERE_TEXTURE_H
#include "vector.h"
#include "colour.h"

class sphere_texture {
public:
	sphere_texture(unsigned char* pixels, int x, int y) {
		imageData = pixels;
		width = x;
		height = y;
	}

	virtual Colour pixelCol(float u, float v) const;

	unsigned char* imageData;

	int width, height;
};

Colour sphere_texture::pixelCol(float u, float v) const {
	int i = (u)* width;
	int j = (1 - v) * height - 0.001;

	if (i < 0) {
		i = 0;
	}
	if (j < 0) {
		j = 0;
	}

	if (i > width - 1) {
		i = width - 1;
	}
	
	if (j > height - 1) {
		j = height - 1;
	}

	float r = int(imageData[3 * i + 3 * width * j]) / 255.0;
	float g = int(imageData[3 * i + 3 * width * j + 1]) / 255.0;
	float b = int(imageData[3 * i + 3 * width * j + 2]) / 255.0;

	return Colour(r, g, b, 0);
}
#endif