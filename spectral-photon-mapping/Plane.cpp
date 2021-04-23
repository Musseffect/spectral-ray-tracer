#include "Plane.h"


BBox3D Plane::bbox() const {
	return BBox3D(vec3(std::numeric_limits<float>::infinity()),
		vec3(std::numeric_limits<float>::infinity()));
}
bool Plane::intersect(const Ray& ray) const {
	float t = (d - glm::dot(ray.ro, normal)) / glm::dot(ray.rd, normal);
	if (t < ray.tMin || t > ray.tMax)
		return false;
	return true;
}
bool Plane::intersect(const Ray& ray, HitInfo& hitInfo) const {
	float t = (d - glm::dot(ray.ro, normal)) / glm::dot(ray.rd, normal);
	if (t < ray.tMin || t > ray.tMax)
		return false;
	hitInfo.localPosition = ray(t);
	hitInfo.t = t;
	hitInfo.normal = normal;
	return true;
}
std::vector<Interval> Plane::intersectionList(const Ray& ray) const {
	float t = (d - glm::dot(ray.ro, normal)) / glm::dot(ray.rd, normal);
	return { Interval{t, normal, t, normal} };
}
float Plane::area() const {
	return std::numeric_limits<float>::infinity();
}
float Plane::pdf() const { return 0.0f; }
HitInfo Plane::sample() const {
	throw std::runtime_error("Cannot sample inifinite plane");
}
float Plane::pdf(const vec3& pos, const vec3& wi, const Affine& transform) const {
	return 0.0f;
}

