#include "HitInfo.h"
#include "Light.h"


float HitInfo::lightEmitted(const vec3& wo, float wavelength) const  {
	if (light)
		return light->lightEmitted(wo, normal, wavelength);
	return 0.0f;
}