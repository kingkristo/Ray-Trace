/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2018.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Phong is a child class of Material and implement the simple Phong
// surface illumination model.

#pragma once

#include "material.h"
#include "colour.h"
#include "vector.h"

class Phong : public Material {
public:
	Colour ambient;
	Colour diffuse;
	Colour specular;

	float  power;

	void compute_base_colour(Colour &result);

	void compute_light_colour(Vector &viewer, Vector &normal, Vector &ldir, Colour lightCol, Colour &result);
};
