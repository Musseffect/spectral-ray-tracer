#include "PlanarShape.h"


bool PlanarShape::intersect(const Ray& ray) const {
	float t = -ray.ro.y / ray.rd.y;
	if (t < ray.tMin || t > ray.tMax)
		return false;
	vec2 p = vec2(ray.ro.x, ray.ro.z) + vec2(ray.rd.x, ray.rd.z) * t;
	if (!isInBounds(p))
		return false;
	return true;
}

bool PlanarShape::intersect(const Ray& ray, HitInfo& hitInfo) const {
	float t = -ray.ro.y / ray.rd.y;
	if (t < ray.tMin || t > ray.tMax)
		return false;
	vec2 p = vec2(ray.ro.x, ray.ro.z) + vec2(ray.rd.x, ray.rd.z) * t;
	if (!isInBounds(p))
		return false;
	hitInfo.t = t;
	hitInfo.localPosition = vec3(p.x, 0.0f, p.y);
	hitInfo.normal = vec3(0.f, 1.0f, 0.f);
	return true;
}