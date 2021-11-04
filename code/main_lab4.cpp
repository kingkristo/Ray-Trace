/***************************************************************************
 *
 * krt - Kens Raytracer - Coursework Edition. (C) Copyright 1997-2019.
 *
 * Do what you like with this code as long as you retain this comment.
 */

 /* This is the entry point function for the program you need to create for lab two.
  * You should not need to modify this code.
  * It creates a framebuffer, loads an triangle mesh object, calls the drawing function to render the object and then outputs the framebuffer as a ppm file.
  *
  * On linux.bath.ac.uk:
  *
  * Compile the code using g++ -o lab2executable main_lab2.cpp framebuffer.cpp linedrawer.cpp polymesh.cpp -lm
  *
  * Execute the code using ./lab2executable
  *
  * This will produce an image file called test.ppm. You can convert this a png file for viewing using
  *
  * pbmropng test.ppm > test.png
  *
  * You are expected to fill in the missing code in polymesh.cpp.
  */

#include "framebuffer.h"
#include "ray.h"
#include "hit.h"
#include "polymesh.h"
#include "sphere.h"
#include "positional_light.h"
#include "material.h"
#include "phong.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <random>
#include "positional_light.h"
#include "PmNode.h"
#include "kdTree.h"
#include "stb_image.h"
#include "sphere_texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace std;

void object_test(Ray ray, Object* objects, Hit& best_hit)
{
	Object* obj = objects;

	best_hit.flag = false;


	while (obj != 0)
	{
		
			Hit obj_hit;
			obj_hit.flag = false;

			obj->intersection(ray, obj_hit);


			if (obj_hit.flag)
			{
				if (obj_hit.t > 0.0f)
				{
					if (best_hit.flag == false)
					{
						best_hit = obj_hit;
					}
					else if (obj_hit.t < best_hit.t)
					{
						best_hit = obj_hit;
					}
				}
			}

			obj = obj->next;
		
	}

	return;
}

//Refraction method adapted from: https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
Vector refract(Vector& normal, Vector& direction, float ri1, float ri2) {
	float cosi = direction.dot(normal);
	float eta = ri1 / ri2;
	float k = eta * eta * (1 - cosi * cosi);
	return Vector((direction.x * eta) - normal.x * (sqrt(1 - k) + (eta * cosi)), (direction.y * eta) - normal.y * (sqrt(1 - k) + (eta * cosi)), (direction.z * eta) - normal.z * (sqrt(1 - k) + (eta * cosi)));
}

//code taken directly from https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel;
void fresnel(Vector I, Vector N, const float& ior, float& kr)
{
	float cosi = I.dot(N);
	if (cosi < 1.0f) {
		cosi = 1.0f;
	}
	else if (cosi < -1.0f) {
		cosi = -1.0f;
	}
	float etai = 1, etat = ior;
	if (cosi > 0) { std::swap(etai, etat); }
	// Compute sini using Snell's law
	float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
	// Total internal reflection
	if (sint >= 1) {
		kr = 1;
	}
	else {
		float cost = sqrtf(std::max(0.f, 1 - sint * sint));
		cosi = fabsf(cosi);
		float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
		float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
		kr = (Rs * Rs + Rp * Rp) / 2;
	}
	// As a consequence of the conservation of energy, transmittance is given by:
	// kt = 1 - kr;
}

//get a random vector using spherical coordinates, based on https://stackoverflow.com/questions/5408276/sampling-uniformly-distributed-random-points-inside-a-spherical-volume
Vector randomVector() {
	float pi = 3.14159265359;
	float theta = 2 * pi * ((double)rand() / (RAND_MAX));
	float phi = 2 * pi * ((double)rand() / (RAND_MAX));
	return Vector(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
}

//tracks the path of a single photon (non caustic) records shadow or direct lighting
void raytracePhoton(Ray ray, Object* objects, PositionalLight* lights, int level, kdTree& pm)
{
	// first step, find the closest primitive
	Hit best_hit;
	object_test(ray, objects, best_hit);

	// check we've not recursed too far.
	level = level - 1;
	if (level < 0)
	{
		return;
	}

	//primative found
	if (best_hit.flag)
	{
		//insert shadow ray if nessaccary
		if (ray.shadow) {
			ray.position = best_hit.position;
			ray.indirect = false;
			ray.direct = false;
			pm.insert(ray);
		}

		//if the ray is not a shadow ray, insert it into the photon map as direct lighting
		if (!ray.shadow) {
			//assign ray position to hit position for photon calculations
			ray.position = best_hit.position;
			ray.indirect = false;
			ray.direct = true;
			pm.insert(ray);
		}

		//extend ray after a hit to find shadow areas
		ray.shadow = true;
		raytracePhoton(ray, objects, lights, level, pm);
		return;

	}
	else {
		return;
	}
}

//tracks the path of a single photon (caustic, only records rays that have reflected or refracted at least once)
void raytracePhotonCaustic(Ray ray, Object* objects, PositionalLight* lights, int level, kdTree& pm)
{
	Hit best_hit;
	object_test(ray, objects, best_hit);

	// check we've not recursed too far.
	level = level - 1;
	if (level < 0)
	{
		return;
	}
	
	//primative found
	if (best_hit.flag)
	{
		//offset ray pos slightly
		ray.position = Vertex(best_hit.position.x + 0.0001f * best_hit.normal.x, best_hit.position.y + 0.0001f * best_hit.normal.y, best_hit.position.z + 0.0001f * best_hit.normal.z);
		
		//insert any indirect ray into the photon map 
		if (ray.indirect) {
			pm.insert(ray);
		}
		
		//gnerate probability for russian roulette
		float randomProb = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

		//reflect case
		if (randomProb < best_hit.what->material->pr) {
			ray.indirect = true;
			Colour materialCol;
			best_hit.what->material->compute_base_colour(materialCol);
			//ray colour scaled by the material it reflects from
			ray.colour.scale(materialCol);

			//generate random reflection
			//FUTURE WORK: USE BRDF TO CONSTRAIN REFLECTION TO CONTRIBUTE MORE
			bool internalDirection = true;
			Vector reflectDir;
			while (internalDirection) {
				internalDirection = false;
				reflectDir = randomVector();
				//reflection is unusable
				if (reflectDir.dot(best_hit.normal) < 0.0f) {
					internalDirection = true;
				}
			}

			ray.direction = reflectDir;
			//ray trace with this new reflected ray
			raytracePhoton(ray, objects, lights, level, pm);

		}
		else if (randomProb < (best_hit.what->material->pr + best_hit.what->material->pt)) {
			ray.indirect = true;
			//generate internal refraction ray
			Ray internalRay;
			//flip the refractive index for internal/external rays
			internalRay.direction = refract(best_hit.normal, ray.direction, 1.0f, best_hit.what->material->refractiveIndex);
			//offset position
			internalRay.position = Vertex(best_hit.position.x + ray.direction.x * -0.0001f, best_hit.position.y + ray.direction.y * -0.0001f, best_hit.position.z + ray.direction.z * -0.0001f);
			Hit internalHit;
			object_test(internalRay, objects, internalHit);
			//generate external ray when outside edge hit
			if (internalHit.flag) {
				//record photon on other side of transparent object
				internalRay.position = internalHit.position;
				pm.insert(internalRay);
				Ray externalRay;
				internalHit.normal.negate();
				externalRay.position = Vertex(internalHit.position.x + externalRay.direction.x * 0.0001f, internalHit.position.y + externalRay.direction.y * 0.0001f, internalHit.position.z + externalRay.direction.z * 0.0001f);
				externalRay.direction = refract(internalHit.normal, internalRay.direction, best_hit.what->material->refractiveIndex, 1.0f);
				//continue ray tracing using external ray
				raytracePhoton(externalRay, objects, lights, level, pm);
			}

		}
		else {
			if (ray.indirect) {
				Colour materialCol;
				best_hit.what->material->compute_base_colour(materialCol);
				//ray colour scaled by the material it reflects from
				ray.colour.scale(materialCol);
				//record indirect photon at the point of being absorbed
				//don't insert any direct photons as they will not contibue usefully. 
				pm.insert(ray);
			}
			return;
		}
	}
	else {
		return;
	}
}

void buildPhotonMap(int photonCount, Object* objects, PositionalLight* light, int level, kdTree& pm, kdTree& pmCaustic) {
	// for each photon
	for (int i = 0; i < photonCount; i++) {
		//new photon sent from light source in a random direction
		Ray photonRay;
		photonRay.direction = randomVector();
		photonRay.position = Vertex(light->position.x, light->position.y, light->position.z);
		//ray is direct by default
		photonRay.direct = true;
		photonRay.indirect = false;
		photonRay.shadow = false;
		photonRay.colour = light->intensity;
		raytracePhoton(photonRay, objects, light, level, pm);

		//caustic map
		Ray causticPhotonRay;
		causticPhotonRay.direction = randomVector();
		causticPhotonRay.position = Vertex(light->position.x, light->position.y, light->position.z);
		causticPhotonRay.direct = true;
		causticPhotonRay.indirect = false;
		causticPhotonRay.shadow = false;
		causticPhotonRay.colour = light->intensity;
		raytracePhotonCaustic(causticPhotonRay, objects, light, level, pmCaustic);
	}
}

void raytraceRender(Ray ray, Object* objects, PositionalLight* light, Colour& colour, int level, int totalSamples, kdTree& pm, kdTree& photonMapCaustic, sphere_texture texture)
{
	//allocate memory for photon samples
	Ray* photonSamples = (Ray*)malloc(totalSamples * sizeof(Ray));
	Ray* photonSamplesCaustic = (Ray*)malloc(totalSamples * sizeof(Ray));

	// first step, find the closest primitive
	//Hit shadow_hit;
	Hit best_hit;
	object_test(ray, objects, best_hit);

	// check we've not recursed too far.
	level = level - 1;
	if (level < 0)
	{
		return;
	}

	// if we found a primitive then compute the colour we should see
	if (best_hit.flag)
	{
		//added this boolean return as in some edge cases photon map was not being populated (eg. empty scene)
		bool photonMapExists = pm.getNearestPhotons(best_hit.position, photonSamples, totalSamples);
		bool pohtonMapCausticExists = photonMapCaustic.getNearestPhotons(best_hit.position, photonSamplesCaustic, totalSamples);
		Colour indirect;
		Colour direct;
		float radius = 0;
		float shadowCount = 0;
		float directCount = 0;
		float indirectCount = totalSamples;

		//count the number of direct and shadow photons
		if (photonMapExists) {
			for (int i = 0; i < totalSamples; i++) {
				if (photonSamples[i].direct) {
					directCount++;
				}
				else if (photonSamples[i].shadow) {
					shadowCount++;
				}
			}
		}
		
		Vector viewer;
		Vector ldir;

		viewer.x = -best_hit.position.x;
		viewer.y = -best_hit.position.y;
		viewer.z = -best_hit.position.z;
		viewer.normalise();

		bool lit;
		lit = light->get_direction(best_hit.position, ldir);
		ldir.normalise();

		//if light hits the visible side of surface
		if (ldir.dot(best_hit.normal) < 0)
		{

			//all shadow photons
			if (shadowCount / totalSamples == 1) {
				lit = false;
			}
			else if (shadowCount > 0) {
				//mix of shadow, generate new shadow ray to make sure
				Ray shadow_ray;
				Hit shadow_hit;
				shadow_ray.direction.x = -ldir.x;
				shadow_ray.direction.y = -ldir.y;
				shadow_ray.direction.z = -ldir.z;
				shadow_ray.position.x = best_hit.position.x + (0.0001f * shadow_ray.direction.x);
				shadow_ray.position.y = best_hit.position.y + (0.0001f * shadow_ray.direction.y);
				shadow_ray.position.z = best_hit.position.z + (0.0001f * shadow_ray.direction.z);

				//find diff between light pos and hit pos
				float lightDistance = light->position.distanceFrom(best_hit.position);

				object_test(shadow_ray, objects, shadow_hit);

				if (shadow_hit.flag == true)
				{
					//if shadow hit is closer than the light source, then light is blocked
					if (shadow_hit.t < lightDistance)
					{
						lit = false;
					}
				}
			}
			else {
				//no shadow
				lit = true;
			}

			if (lit) {
				Colour materialCol;

				best_hit.what->material->compute_light_colour(viewer, best_hit.normal, ldir, light->intensity, materialCol);
				
				direct.add(materialCol);
			}
		}

		//only do this if a caustic map exists to prevent crash
		if (pohtonMapCausticExists) {
			//look at indirect samples

			for (int i = 0; i < totalSamples; i++) {
				float distance = best_hit.position.distanceFrom(photonSamplesCaustic[i].position);
				//radius of the points to be considered is the distance from the furthest one
				if (distance > radius) {
					radius = distance;
				}
			}

			//
			for (int i = 0; i < totalSamples; i++) {
				//take the colour from nearby caustic sample
				Colour materialCol;
				materialCol.add(photonSamplesCaustic[i].colour);

				float distance = best_hit.position.distanceFrom(photonSamplesCaustic[i].position);
				
				//this used to be distance / radius, changed so that it would favour the closest points rather than the furthest away
				float scale = 1 - distance / radius;

				//scale colour by 1 - distance/radius to favour the closest points
				materialCol.r = materialCol.r * scale;
				materialCol.g = materialCol.g * scale;
				materialCol.b = materialCol.b * scale;

				indirect.add(materialCol);
			}

			//divide by total amount of indirect photons to get an average
			indirect.r = indirect.r / indirectCount;
			indirect.g = indirect.g / indirectCount;
			indirect.b = indirect.b / indirectCount;
		}
	
		//compute frensel term
		float fresenelKr;
		fresnel(ray.direction, best_hit.normal, best_hit.what->material->refractiveIndex, fresenelKr);
		Colour reflectionColour;
		Colour refractionColour;

		// TODO: compute reflection ray if material supports it.
		if (best_hit.what->material->reflective == true)
		{
			Ray reflectedRay;
			//offset by small amount needed for same reason as shadow 
			reflectedRay.position = Vertex(best_hit.position.x + best_hit.normal.x * 0.0001f, best_hit.position.y + best_hit.normal.y * 0.0001f, best_hit.position.z + best_hit.normal.z * 0.0001f);
			reflectedRay.direction = ray.direction - Vector(best_hit.normal.x * 2.0f * ray.direction.dot(best_hit.normal), best_hit.normal.y * 2.0f * ray.direction.dot(best_hit.normal), best_hit.normal.z * 2.0f * ray.direction.dot(best_hit.normal));
			reflectedRay.direction.normalise();
			raytraceRender(reflectedRay, objects, light, reflectionColour, level, indirectCount, pm, photonMapCaustic, texture);
		}

		// TODO: compute refraction ray if material supports it. (fresenelKr >=1 means total internal reflection)
		if (best_hit.what->material->transparent == true && fresenelKr < 1)
		{
			//generate internal refraction ray
			Ray internalRay;
			//flip the refractive index for internal/external rays
			internalRay.direction = refract(best_hit.normal, ray.direction, 1.0f, best_hit.what->material->refractiveIndex);
			//offset position
			internalRay.position = Vertex(best_hit.position.x + internalRay.direction.x * 0.0001f, best_hit.position.y + internalRay.direction.y * 0.0001f, best_hit.position.z + internalRay.direction.z * 0.0001f);
			Hit internalHit;
			object_test(internalRay, objects, internalHit);
			//generate external ray
			if (internalHit.flag) {
				Ray externalRay;
				internalHit.normal.negate();
				externalRay.direction = refract(internalHit.normal, internalRay.direction, best_hit.what->material->refractiveIndex, 1.0f);
				externalRay.position = Vertex(internalHit.position.x + externalRay.direction.x * 0.0001f, internalHit.position.y + externalRay.direction.y * 0.0001f, internalHit.position.z + externalRay.direction.z * 0.0001f);
				raytraceRender(externalRay, objects, light, refractionColour, level, indirectCount, pm, photonMapCaustic, texture);
			}

		}
		if (best_hit.what->material->transparent == true && best_hit.what->material->reflective == true) {
			Colour reflectiveKr;
			Colour refractiveKr;
			reflectiveKr.r = fresenelKr;
			reflectiveKr.g = fresenelKr;
			reflectiveKr.b = fresenelKr;

			float kt = 1 - fresenelKr;
			refractiveKr.r = kt;
			refractiveKr.g = kt;
			refractiveKr.b = kt;

			//if reflective and refractive use frenel term
			refractionColour.scale(refractiveKr);
			reflectionColour.scale(reflectiveKr);

		}
		else if (best_hit.what->material->transparent == true) {
			refractionColour.scale(best_hit.what->material->kr);
		}
		else if (best_hit.what->material->reflective == true) {
			reflectionColour.scale(best_hit.what->material->kr);
		}

		//texture
		Colour textureCol;
		if (best_hit.what->material->useTexture == true) {

			Vertex spherePos = best_hit.position;
			//offset by sphere position
			spherePos.x = spherePos.x - 0.025f;
			spherePos.y = spherePos.y - 1.5f;
			spherePos.z = spherePos.z - 2.0f;
			 
			float pi = 3.14159265359;
			float phi = atan2(spherePos.z, spherePos.x);
			float theta = asin(spherePos.y);
			float u = 1 - (phi + pi) / (2 * pi);
			float v = (theta + pi / 2) / pi;

			textureCol = texture.pixelCol(u, v);
		}

		free(photonSamples);

		colour.add(refractionColour);
		colour.add(reflectionColour);
		colour.add(direct);
		colour.add(indirect);
		//texture
		if (best_hit.what->material->useTexture == true) {
			colour.scale(textureCol);
		}

		//prevent overflow
		if (colour.r > 1) {
			colour.r = 1;
		}
		if (colour.g > 1) {
			colour.g = 1;
		}
		if (colour.b > 1) {
			colour.b = 1;
		}
	}
	else
	{
		//free allocated memory
		free(photonSamples);
		colour.r = 0.0f;
		colour.g = 0.0f;
		colour.b = 0.0f;
	}
}

int main(int argc, char* argv[])
{
	int width = 512;
	int height = 512;
	// Create a framebuffer
	FrameBuffer* fb = new FrameBuffer(width, height);

	// The following transform allows 4D homogeneous coordinates to be transformed. It moves the supplied teapot model to somewhere visible.
	Transform* transform = new Transform(0.4f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.4f, -1.0f,
	0.0f, 0.4f, 0.0f, 2.5f,
	0.0f, 0.0f, 0.0f, 3.5f);

	//  Read in the teapot model.
	PolyMesh* pm = new PolyMesh((char*)"teapot_smaller.ply", transform);
	PolyMesh* pmRight = new PolyMesh((char*)"rig.ply");
	PolyMesh* pmBottom = new PolyMesh((char*)"bot.ply");
	PolyMesh* pmBack = new PolyMesh((char*)"bac.ply");
	PolyMesh* pmLeft = new PolyMesh((char*)"lef.ply");

	Vertex v;
	v.x = 0.25f;
	v.y = 1.0f;
	v.z = 2.0f;
	Sphere* sphere = new Sphere(v, 0.5f);

	v.x = -0.6f;
	v.y = -0.7f;
	v.z = 1.25f;
	Sphere* sphere2 = new Sphere(v, 0.3f);

	v.x = 0.6f;
	v.y = -0.7f;
	v.z = 1.25f;
	Sphere* sphere3 = new Sphere(v, 0.3f);

	Ray ray;
	ray.position.x = 0.0001f;
	ray.position.y = 0.0f;
	ray.position.z = 0.0f;

	PositionalLight* pl = new PositionalLight(Vertex(0.0f, 0.5f, 0.0f), Colour(1.0f, 1.0f, 1.0f, 0.0f));

	//texture
	int textureWidth = 1000;
	int textureHeight = 500;
	int nn = 3;
	unsigned char* texture_data = stbi_load("moon.jpg", &textureWidth, &textureHeight, &nn, 0);
	sphere_texture moonTexture = sphere_texture(texture_data, textureWidth, textureHeight);

	Phong bp1;
	bp1.ambient.r = 0.2f;
	bp1.ambient.g = 0.0f;
	bp1.ambient.b = 0.2f;
	bp1.diffuse.r = 0.4f;
	bp1.diffuse.g = 0.0f;
	bp1.diffuse.b = 0.4f;
	bp1.specular.r = 0.4f;
	bp1.specular.g = 0.4f;
	bp1.specular.b = 0.4f;
	bp1.power = 40.0f;
	//kr is the same as ks but kept seperate to allow for subtle effects
	bp1.kr = bp1.specular;
	bp1.pr = bp1.specular.r;
	bp1.pt = 40.0;
	bp1.reflective = true;
	bp1.transparent = false;
	bp1.refractiveIndex = 1.5;

	Phong bp2;
	bp2.ambient.r = 0.0f;
	bp2.ambient.g = 0.0f;
	bp2.ambient.b = 0.0f;
	bp2.diffuse.r = 0.0f;
	bp2.diffuse.g = 0.0f;
	bp2.diffuse.b = 0.0f;
	bp2.specular.r = 1.0f;
	bp2.specular.g = 1.0f;
	bp2.specular.b = 1.0f;
	bp2.power = 40.0f;
	bp2.kr = bp2.specular;
	bp2.pr = bp1.specular.r;
	bp2.pt = 0.4;
	bp2.reflective = true;
	bp2.transparent = true;
	bp2.refractiveIndex = 1.55;

	Phong bp3;
	bp3.ambient.r = 0.0f;
	bp3.ambient.g = 0.0f;
	bp3.ambient.b = 0.4f;
	bp3.diffuse.r = 0.0f;
	bp3.diffuse.g = 0.0f;
	bp3.diffuse.b = 0.6f;
	bp3.specular.r = 0.0f;
	bp3.specular.g = 0.0f;
	bp3.specular.b = 0.2f;
	bp3.power = 40.0f;
	bp3.kr = bp2.specular;
	bp3.pr = bp3.specular.r;
	bp3.pt = 0.0;
	bp3.reflective = true;
	bp3.transparent = false;
	bp3.refractiveIndex = 1.05;

	Phong bp4;
	bp4.ambient.r = 0.2f;
	bp4.ambient.g = 0.2f;
	bp4.ambient.b = 0.2f;
	bp4.diffuse.r = 0.3f;
	bp4.diffuse.g = 0.3f;
	bp4.diffuse.b = 0.3f;
	bp4.specular.r = 0.0f;
	bp4.specular.g = 0.0f;
	bp4.specular.b = 0.0f;
	bp4.power = 40.0f;
	bp4.kr = bp2.specular;
	bp4.pr = bp4.specular.r;
	bp4.pt = 0.0;
	bp4.reflective = false;
	bp4.transparent = false;
	bp4.refractiveIndex = 1.05;
	bp4.useTexture = true;

	Phong bp5;
	bp5.ambient.r = 0.2f;
	bp5.ambient.g = 0.0f;
	bp5.ambient.b = 0.0f;
	bp5.diffuse.r = 0.4f;
	bp5.diffuse.g = 0.0f;
	bp5.diffuse.b = 0.0f;
	bp5.specular.r = 0.4f;
	bp5.specular.g = 0.0f;
	bp5.specular.b = 0.0f;
	bp5.power = 40.0f;
	bp5.kr = bp5.specular;
	bp5.pr = bp5.specular.r;
	bp5.pt = 0.0;
	bp5.reflective = false;
	bp5.transparent = false;
	bp5.refractiveIndex = 1.5;

	Phong bp6;
	bp6.ambient.r = 0.0f;
	bp6.ambient.g = 0.2f;
	bp6.ambient.b = 0.0f;
	bp6.diffuse.r = 0.0f;
	bp6.diffuse.g = 0.4f;
	bp6.diffuse.b = 0.0f;
	bp6.specular.r = 0.0f;
	bp6.specular.g = 0.4f;
	bp6.specular.b = 0.0f;
	bp6.power = 40.0f;
	bp6.kr = bp6.specular;
	bp6.pr = bp6.specular.r;
	bp6.pt = 0.0;
	bp6.reflective = false;
	bp6.transparent = false;
	bp6.refractiveIndex = 1.5;

	Phong bp7;
	bp7.ambient.r = 0.2f;
	bp7.ambient.g = 0.2f;
	bp7.ambient.b = 0.0f;
	bp7.diffuse.r = 0.4f;
	bp7.diffuse.g = 0.4f;
	bp7.diffuse.b = 0.4f;
	bp7.specular.r = 0.4f;
	bp7.specular.g = 0.4f;
	bp7.specular.b = 0.0f;
	bp7.power = 40.0f;
	bp7.kr = bp7.specular;
	bp7.pr = bp7.specular.r;
	bp7.pt = 0.0;
	bp7.reflective = false;
	bp7.transparent = false;
	bp7.refractiveIndex = 1.5;

	pm->material = &bp1;
	sphere->material = &bp4;
	sphere2->material = &bp2;
	sphere3->material = &bp3;
	pmBottom->material = &bp7;
	pmBack->material = &bp7;
	pmLeft->material = &bp6;
	pmRight->material = &bp5;

	pm->next = sphere;
	pm->next->next = pmBottom;
	pm->next->next->next = pmBack;
	pm->next->next->next->next = pmRight;
	pm->next->next->next->next->next = pmLeft;
	pm->next->next->next->next->next->next = sphere2;
	pm->next->next->next->next->next->next->next = sphere3;
	
	int photonCount = 7000000;
	int sampleCount = 50;
	int maxLevel = 4;
	kdTree photonMap;
	kdTree photonMapCaustic;

	photonMap.nodes = new PmNode[photonCount];
	photonMapCaustic.nodes = new PmNode[photonCount];

    buildPhotonMap(photonCount, pm, pl, maxLevel, photonMap, photonMapCaustic);

	for (int y = 0; y < height; y += 1)
	{
		for (int x = 0; x < width; x += 1)
		{
			float fx = (float)x / (float)width;
			float fy = (float)y / (float)height;

			Vector direction;
			ray.direction.x = (fx - 0.5f);
			ray.direction.y = (0.5f - fy);
			ray.direction.z = 0.5f;
			ray.direction.normalise();
			Colour colour;

			raytraceRender(ray, pm, pl, colour, maxLevel, sampleCount, photonMap, photonMapCaustic, moonTexture);

			fb->plotPixel(x, y, colour.r, colour.g, colour.b);
		}
		cerr << "*" << flush;
	}

	// Output the framebuffer.
	fb->writeRGBFile((char*)"test.ppm");
	return 0;

}
