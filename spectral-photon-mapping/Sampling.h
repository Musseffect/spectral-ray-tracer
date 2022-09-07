#pragma once
#include "common.h"
#include "Random.h"



namespace Sampling
{
	float uniformStratified(int count);
	vec3 uniformHemisphere(vec3 dir);
	float uniformHemispherePdf();
	vec2 uniformDisk(float radius);
	vec3 uniformSphere();
	float uniformSpherePdf();
	vec3 cosWeightedHemisphere(vec3 dir);
	vec3 cosWeightedHemisphere(vec3 dir, float& pdf);
	vec3 uniformTriangle(const vec3& p0, const vec3& p1, const vec3& p2);
	float uniformTrianglePdf(const vec3& p0, const vec3& p1, const vec3& p2);
	vec3 uniformTriangle(const vec3& p0, const vec3& p1, const vec3& p2, float& pdf);
	vec2 uniformExponential2D();
	vec2 uniformExponential2D(float m, float sigma);
	// http://vcg.isti.cnr.it/jgt/tetra.htm
	vec3 uniformTetrahedron(const vec3& p0, const vec3& p1, const vec3& p2, const vec3& p3);


	float balanceHeuristic(int nf, float fPdf, int ng, float gPdf);
	float powerHeuristic(int nf, float fPdf, int ng, float gPdf);
};