#include "Shape.h"
#include "Scene.h"


float Shape::pdf(const vec3& pos, const vec3& wi, const Affine& transform) const {
	HitInfo tHit;
	Ray localRay = transform.transformInverse(Ray(pos, wi));
	if (!intersect(localRay, tHit))
		return 0.0f;
	float pdf = glm::pow2(glm::dot(localRay.ro - tHit.localPosition, localRay.rd)) /
		(area() * glm::abs(glm::dot(tHit.normal, -glm::normalize(localRay.rd))));
	return pdf;
}