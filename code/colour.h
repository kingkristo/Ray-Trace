/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2018.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Colour is a class to store and maniplulate an rgba colour.

#pragma once

class Colour {
public:
	float r;
	float g;
	float b;
	float a;

	Colour()
	{
		r = 0.0f;
		g = 0.0f;
		b = 0.0f;
		a = 1.0f;
	}

	Colour(float pr, float pg, float pb, float pa)
	{
		r = pr;
		g = pg;
		b = pb;
		a = pa;
	}

	void scale(Colour &scaling)
	{
		r *= scaling.r;
		g *= scaling.g;
		b *= scaling.b;
		a *= scaling.a;
	}

	void add(Colour &adjust)
	{
		r += adjust.r;
		g += adjust.g;
		b += adjust.b;
	}
};
