/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2018.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Material is the base class for materials.

#pragma once

#include "vector.h"
#include "colour.h"

class Material {
public:
	bool reflective;
	bool transparent;
	float refractiveIndex;
	Colour kr;
	float pr;
	float pt;
	bool useTexture = false;

	virtual void compute_base_colour(Colour &result)
	{
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
	}
	virtual void compute_light_colour(Vector &viewer, Vector &normal, Vector &ldir, Colour lightCol, Colour &result)
	{
		result.r = 0.0f;
		result.g = 0.0f;
		result.b = 0.0f;
	}
};
