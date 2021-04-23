#pragma once
#include "common.h"
#include "Random.h"



class Sampling {
public:
	static float uniformStratified(int count);
	static vec3 uniformHemisphere(vec3 dir);
	static float uniformHemispherePdf();
	static vec2 uniformDisk(float radius);
	static vec3 uniformSphere();
	static float uniformSpherePdf();
	static vec3 cosWeightedHemisphere(vec3 dir);
	static vec3 cosWeightedHemisphere(vec3 dir, float& pdf);
	static vec3 uniformTriangle(const vec3& a, const vec3& b, const vec3& c);
	static float uniformTrianglePdf(const vec3& a, const vec3& b, const vec3& c);
	static vec3 uniformTriangle(const vec3& a, const vec3& b, const vec3& c, float& pdf);
	static vec2 uniformExponential2D();
	static vec2 uniformExponential2D(float m, float sigma);
};