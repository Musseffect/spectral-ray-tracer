#include "Shape.h"
#include "Light.h"
#include "SceneObject.h"
#include "Scene.h"




HitInfo Shape::sample(const vec3& localPos, const Affine& transform, float& pdf) const {
	HitInfo sampleHit = sample();
	vec3 wi = glm::normalize(sampleHit.localPosition - localPos);
	vec3 t_wi = transform.transformInverseVector(glm::normalize(transform.transformVector(wi)));
	pdf = this->pdf() * glm::pow2(glm::dot(sampleHit.localPosition - localPos, t_wi)) / glm::abs(glm::dot(sampleHit.normal, -wi));
	if (std::isinf(pdf)) pdf = 0.0f;
	return sampleHit;
}

float Shape::pdf(const vec3& pos, const vec3& wi, const Affine& transform, const Scene* scene) const {
	HitInfo tHit;
	Ray localRay = transform.transformInverse(Ray(pos, wi));
	if (!intersect(localRay, tHit))
		return 0.0f;
	float pdf = glm::pow2(glm::dot(localRay.ro - tHit.localPosition, localRay.rd)) /
		(area() * glm::abs(glm::dot(tHit.normal, -glm::normalize(localRay.rd))));
	return pdf;
}