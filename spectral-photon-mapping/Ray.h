#pragma once
#include "common.h"

struct Ray {
	vec3 ro;
	vec3 rd;
	float tMin = 0.001f;
	float tMax = std::numeric_limits<float>::max();
	Ray():ro(0.0), rd(0.0, 0.0, -1.0) {}
	Ray(vec3 ro, vec3 rd, float tMin = 0.001f, float tMax = std::numeric_limits<float>::max()) :ro(ro), rd(rd), tMin(tMin), tMax(tMax) {}
};


struct SpectralRay {
	float wavelength;
	float intensity;
	Ray ray;
};