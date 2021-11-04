/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

// Ray is a class to store and maniplulate 3D rays.

#pragma once

#include <iostream>
using namespace std;

#include "vertex.h"
#include "vector.h"
#include "colour.h"

class Ray {
public:
	Vertex position;
	Vector direction;
	bool direct;
	bool indirect;
	bool shadow;
	//added colour to ray for use when building photon map
	Colour colour;

	Ray()
	{
	}


	Ray(Vertex p, Vector d)
	{
		position = p;
		direction = d;
	}

	friend ostream& operator<<(ostream& os, const Ray& r)
	{
	  os << "Ray{[" << r.position.x << "," << r.position.y << "," << r.position.z << "],[" << r.direction.x << "," << r.direction.y << "," << r.direction.z << "]}\n"; 
          return os;
	}
};
