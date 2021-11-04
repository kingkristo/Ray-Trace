/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <math.h>

class Vector {
public:
	float x;
	float y;
	float z;

	Vector(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	Vector()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	void normalise()
	{
		float len = (float)sqrt((double)(x*x + y*y + z*z));
		x = x / len;
		y = y / len;
		z = z / len;
	}

	float len_sqr()
	{
	  return(x*x + y*y + z*z);
	}

	float length()
	{
	  return(float)sqrt((double)(x*x + y*y + z*z));
	}
	/*
	float dot(Vector &other)
	{
		return x*other.x + y*other.y + z*other.z;
	}
	*/
	float dot(Vector other)
	{
		return x*other.x + y*other.y + z*other.z;
	}

	float distanceFrom(Vector v) {
		return (this->x - v.x) * (this->x - v.x) + (this->y - v.y) * (this->y - v.y) + (this->z - v.z) * (this->z - v.z);
	}


	void reflection(Vector initial, Vector &reflect)
	{
		float d;

		d = dot(initial);
		d = d * 2.0f;

		reflect.x = initial.x - d * x;
		reflect.y = initial.y - d * y;
		reflect.z = initial.z - d * z;
	}

	void negate()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	void multiply(float num)
	{
		x = num * x;
		y = num * y;
		z = num * z;
	}

	void cross(Vector &other, Vector &result)
	{
	  result.x = y*other.z - z*other.y;
	  result.y = z*other.x - x*other.z;
	  result.z = x*other.y - y*other.x;
	}

	void cross(Vector &other)
	{
	  Vector result;
	  result.x = y*other.z - z*other.y;
	  result.y = z*other.x - x*other.z;
	  result.z = x*other.y - y*other.x;
	  x = result.x;
	  y = result.y;
	  z = result.z;
	}

	void add(Vector &other)
	{
	  x += other.x;
	  y += other.y;
	  z += other.z;
	}

	void sub(Vector &other)
	{
	  x -= other.x;
	  y -= other.y;
	  z -= other.z;
	}

	Vector& operator=(Vector other)
	{
	  x = other.x;
	  y = other.y;
	  z = other.z;

	  return *this;
	}

	friend Vector operator+(const Vector &a, const Vector &b)
	{
	  Vector t;
	  t.x = a.x + b.x;
	  t.y = a.y + b.y;
	  t.z = a.z + b.z;
	  return t;
	}
	friend Vector operator-(const Vector &a, const Vector &b)
	{
	  Vector t;
	  t.x = a.x - b.x;
	  t.y = a.y - b.y;
	  t.z = a.z - b.z;
	  return t;
	}

        friend Vector operator*(const Vector &a, const Vector &b)
		{
	  Vector t;
	  t.x = a.x * b.x;
	  t.y = a.y * b.y;
	  t.z = a.z * b.z;
	  return t;
	}

	        friend Vector operator*(const float a, const Vector &b)
		{
	  Vector t;
	  t.x = a * b.x;
	  t.y = a * b.y;
	  t.z = a * b.z;
	  return t;
	}

	
};

#endif
