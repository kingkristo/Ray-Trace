/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "polymesh.h"

using namespace std;

PolyMesh::PolyMesh(char *file)
{
  Transform *transform = new Transform();

  this->do_construct(file, transform);
}

PolyMesh::PolyMesh(char *file, Transform *transform)
{
  this->do_construct(file, transform);
}

void PolyMesh::do_construct(char *file, Transform *transform)
{
  int count;
  ifstream meshfile(file);

  cerr << "Opening meshfile: " << file << endl;
  
  if (!meshfile.is_open())
  {
    cerr << "Problem reading meshfile (not found)." << endl;
    meshfile.close();
    exit(-1);
  }

  string line;

  try {
    getline(meshfile, line);
  } catch(ifstream::failure e)
  {
    cerr << "Problem reading meshfile (getline failed)." << endl;
  }

  if (line.compare("kcply") != 0)
  {
    cerr << "Problem reading meshfile (not kcply). [" << line << "]" << endl;
    meshfile.close();
    exit(-1);
  }

  try {
    getline(meshfile, line);
  } catch(ifstream::failure e)
  {
    cerr << "Problem reading meshfile (getline failed)." << endl;
    exit(-1);
  }

  istringstream vertex_iss(line);
  string vertex_element;
  string vertex_label;

  vertex_iss >> vertex_element >> vertex_label >> vertex_count;

  if ((vertex_element.compare("element") != 0)||(vertex_label.compare("vertex") != 0))
  {
    cerr << "Problem reading meshfile (element vertex)."<< endl;
    meshfile.close();
    exit(-1);
  }

  cerr << "Expect " << vertex_count << " vertices." << endl;
  
  try {
    getline(meshfile, line);
  } catch(ifstream::failure e)
  {
    cerr << "Problem reading meshfile (getline failed)." << endl;
    exit(-1);
  }

  istringstream triangle_iss(line);
  string triangle_element;
  string triangle_label;

  triangle_iss >> triangle_element >> triangle_label >> triangle_count;

  if ((triangle_element.compare("element") != 0)||(triangle_label.compare("face") != 0))
  {
    cerr << "Problem reading meshfile (element triangle)."<< endl;
    meshfile.close();
    exit(-1);
  }

  cerr << "Expect " << triangle_count << " triangles." << endl;

  vertex = new Vertex[vertex_count];
  
  triangle = new TriangleIndex[triangle_count];
  face_normal = new Vector[triangle_count];
  vertex_normal = new Vector[vertex_count];

  int i;

  for (i = 0; i < vertex_count; i += 1)
  {
    try {
      getline(meshfile, line);
    } catch(ifstream::failure e)
    {
      cerr << "Problem reading meshfile (getline failed)." << endl;
      exit(-1);
    }

    vertex_iss.clear();
    vertex_iss.str(line);

    vertex_iss >> vertex[i].x >> vertex[i].y >>vertex[i].z;

    vertex[i].w = 1.0f;

    transform->apply(vertex[i]);
  }

  for (i = 0; i < triangle_count; i += 1)
  {
    try {
      getline(meshfile, line);
    } catch(ifstream::failure e)
    {
      cerr << "Problem reading meshfile (getline failed)." << endl;
      exit(-1);
    }

    triangle_iss.clear();
    triangle_iss.str(line);
    
    triangle_iss >> count >> triangle[i][0] >> triangle[i][1] >> triangle[i][2];
    /*    triangle[i][0] -= 1;
    triangle[i][1] -= 1;
    triangle[i][2] -= 1;*/

    if (count != 3)
    {
      cerr << "Problem reading meshfile (non-triangle present)." << endl;
      exit(-1);
    }

    compute_face_normal(i, face_normal[i]);
  }

  compute_vertex_normals();
  
  meshfile.close();
  cerr << "Meshfile read." << endl;
  next = 0;
}


// Moller-Trumbore
bool PolyMesh::rayTriangleIntersect(const Ray& ray, const Vector &v0, const Vector &v1, const Vector &v2, float &t)
{
  Vector p;
  Vector d;
  Vector e1,e2,h,s,q;
  float a,f,u,v;

  p.x = ray.position.x;
  p.y = ray.position.y;
  p.z = ray.position.z;
  d = ray.direction;

  e1 = v1 - v0;
  e2 = v2 - v0;

  d.cross(e2,h);
  a = e1.dot(h);

  if (a > -0.00001f && a < 0.00001f)
  {
    return false ;
  }

  f = 1/a;
  s = p - v0;
  u = f * s.dot(h);

  if (u < 0.0f || u > 1.0f)
  {
    return false;
  }

  s.cross(e1,q);
  v = f * d.dot(q);

  if ((v < 0.0f) || ((u + v) > 1.0f))
  {
    return false;
  }

  // compute t
 
  t = f * e2.dot(q);

  if (t > 0.00001f)
  {
    return true; // it's in front ray start
  }

  // it's behind you
  return false;
}

void PolyMesh::compute_vertex_normals(void)
{
  int i,j;

  // The vertex_normal array is already zeroed.

  for (i = 0; i < triangle_count; i += 1)
  {
    for (j = 0; j < 3; j += 1)
    {
      vertex_normal[triangle[i][j]].add(face_normal[i]);
    }
  }

  for (i = 0; i < vertex_count; i += 1)
  {
    vertex_normal[i].normalise();
  }
}

void PolyMesh::compute_face_normal(int which_triangle, Vector &normal)
{
  Vector v0v1, v0v2;
  v0v1.x = vertex[triangle[which_triangle][1]].x - vertex[triangle[which_triangle][0]].x;
  v0v1.y = vertex[triangle[which_triangle][1]].y - vertex[triangle[which_triangle][0]].y;
  v0v1.z = vertex[triangle[which_triangle][1]].z - vertex[triangle[which_triangle][0]].z;

  v0v1.normalise();

  v0v2.x = vertex[triangle[which_triangle][2]].x - vertex[triangle[which_triangle][0]].x;
  v0v2.y = vertex[triangle[which_triangle][2]].y - vertex[triangle[which_triangle][0]].y;
  v0v2.z = vertex[triangle[which_triangle][2]].z - vertex[triangle[which_triangle][0]].z;

  v0v2.normalise();

  v0v1.cross(v0v2, normal);
  normal.normalise();
}

void PolyMesh::triangle_intersection(Ray ray, Hit &hit, int which_triangle)
{
  hit.flag = false;

  float ndotdir = face_normal[which_triangle].dot(ray.direction);

  if (fabs(ndotdir) < 0.000000001f)
  {
    // ray is parallel to triangle so does not intersect
    return;
  }

  Vector v0,v1,v2;
  v0.x = vertex[triangle[which_triangle][0]].x;
  v1.x = vertex[triangle[which_triangle][1]].x;
  v2.x = vertex[triangle[which_triangle][2]].x;

  v0.y = vertex[triangle[which_triangle][0]].y;
  v1.y = vertex[triangle[which_triangle][1]].y;
  v2.y = vertex[triangle[which_triangle][2]].y;

  v0.z = vertex[triangle[which_triangle][0]].z;
  v1.z = vertex[triangle[which_triangle][1]].z;
  v2.z = vertex[triangle[which_triangle][2]].z;


  Vector o;

  o.x = ray.position.x;
  o.y = ray.position.y;
  o.z = ray.position.z;
  float t,u,v;

  hit.flag =  rayTriangleIntersect(ray, v0, v1, v2, t) ;

   if (hit.flag == false) return;
   
  if (t <= 0.0f)
  {
    // intersection is behind start of ray
    return;
  }

  Vertex p;

  p.x = ray.position.x + t * ray.direction.x;
  p.y = ray.position.y + t * ray.direction.y;
  p.z = ray.position.z + t * ray.direction.z;

  hit.t = t;
  hit.what = this;
  hit.position = p;

  hit.normal = face_normal[which_triangle];

  hit.normal.normalise();

  return;
}

void PolyMesh::intersection(Ray ray, Hit &hit)
{
  Hit current_hit;
  int i;

  hit.flag = false;

  // Check each triangle, find closest if any intersecion

  for(i = 0; i < triangle_count; i += 1)
  {
    triangle_intersection(ray, current_hit, i);

    if (current_hit.flag != false)
    {
      if (hit.flag == false)
      {
	hit = current_hit;

      } else if (current_hit.t < hit.t)
      {
        hit = current_hit;
      }
    }
  }

  if (hit.flag == true)
  {
    if(hit.normal.dot(ray.direction) > 0.0)
    {
      hit.normal.negate();
    }
  }
}
