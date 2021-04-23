#pragma once
#include "common.h"
#include "HitInfo.h"
#include "Ray.h"

class BSphere3D {
	vec3 center;
	float radius;
public:
	BSphere3D(vec3 center, float radius) :center(center), radius(radius)
	{}
	void append(vec3 point) {
		vec3 cp = center - point;
		float squaredLength = glm::length2(cp);
		if (squaredLength > radius * radius) {
			vec3 newCenter = 0.5 * (center + cp * radius / std::sqrt(squaredLength) + point);
			radius = glm::length(point - center);
		}
	}
	template<class Point>
	bool intersect(const Point& p) const;
	bool intersect(const Ray& ray) const;
	bool intersect(const Ray& ray, HitInfo& hitInfo) const;
};


template<class Point>
bool BSphere3D::intersect(const Point& p) const {
	vec3 position = p.position();
	return glm::length2(position - center) <= radius * radius;
}

bool BSphere3D::intersect(const Ray& ray) const {
	vec3 ro = ray.ro - center;
	float c = glm::dot(ro, ro) - radius * radius;
	float b = glm::dot(ro, ray.rd);
	float a = glm::dot(ray.rd, ray.rd);
	float d = (b * b - c * a);
	if (d < 0.0f)
		return false;
	d = std::sqrt(d);
	float t0 = (-b - d) / a;
	float t1 = (-b + d) / a;
	if (t1 < ray.tMin || t0 > ray.tMax)
		return false;
	return true;
}

bool BSphere3D::intersect(const Ray& ray, HitInfo& hitInfo) const {
	return intersect(ray);
}