#include "BxDF.h"
#include "Sampling.h"


bool sameHemisphere(const vec3& a, const vec3& b, const vec3 normal) {
	return ((glm::dot(a, normal) >= 0.0f) == (glm::dot(b, normal) >= 0.0f));
}