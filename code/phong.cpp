/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2018.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#include "phong.h"

#include <math.h>

// A simple Phong based lighting model

void Phong::compute_base_colour(Colour &result)
{
	result.r = ambient.r;
	result.g = ambient.g;
	result.b = ambient.b;
}

void Phong::compute_light_colour(Vector &viewer, Vector &normal, Vector &ldir, Colour lightCol, Colour &result)
{

	float diff;

	Vector tolight;
	Vector toviewer;

	result.r=0.0f;
	result.g=0.0f;
	result.b=0.0f;

	tolight = ldir;
	tolight.negate();

	toviewer = viewer;
	toviewer.negate();

	diff = normal.dot(tolight);
	
	if (diff < 0.0f) // light is behind surface
	{
		return;
	}

	// diffuse

	result.r += diffuse.r * diff;
	result.g += diffuse.g * diff;
	result.b += diffuse.b * diff;

	// the specular component

	Vector r;
	
	normal.reflection(tolight, r);
	r.normalise();

	float h;

	h = r.dot(toviewer);

	if (h > 0.0f)
	{
		float p = (float)pow(h, power);

		result.r += specular.r * p;
		result.g += specular.g * p;
		result.b += specular.b * p;
	}
	
	//scale the result by the light colour for coloured lights
	result.scale(lightCol);
}

