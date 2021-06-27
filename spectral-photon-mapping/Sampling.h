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
	static vec3 uniformTriangle(const vec3& p0, const vec3& p1, const vec3& p2);
	static float uniformTrianglePdf(const vec3& p0, const vec3& p1, const vec3& p2);
	static vec3 uniformTriangle(const vec3& p0, const vec3& p1, const vec3& p2, float& pdf);
	static vec2 uniformExponential2D();
	static vec2 uniformExponential2D(float m, float sigma);
	// http://vcg.isti.cnr.it/jgt/tetra.htm
	static vec3 uniformTetrahedron(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3);
};