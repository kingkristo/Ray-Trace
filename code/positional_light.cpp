#include "positional_light.h"

PositionalLight::PositionalLight()
{
	next = (PositionalLight*)0;
}

PositionalLight::PositionalLight(Vertex pos, Colour col)
{
	PositionalLight();
	position = pos;
	intensity = col;
}

bool PositionalLight::get_direction(Vertex &surface, Vector &dir)
{
	dir = Vector(surface.x - position.x, surface.y - position.y,  surface.z - position.z);
	return true;
}

void PositionalLight::get_intensity(Vertex &surface, Colour &level)
{
	level = intensity;
}
