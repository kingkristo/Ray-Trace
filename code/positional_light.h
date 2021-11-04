#pragma once

#include "vertex.h"
#include "vector.h"
#include "colour.h"

class PositionalLight {
public:
	Vertex position;
	PositionalLight*next;
	Colour intensity;
	PositionalLight();
	PositionalLight(Vertex pos, Colour col);
	bool get_direction(Vertex& surface, Vector& dir);
	void get_intensity(Vertex& surface, Colour& intensity);
};
