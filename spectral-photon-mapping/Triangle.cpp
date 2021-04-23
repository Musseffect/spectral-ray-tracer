#include "Triangle.h"


Triangle::Triangle(const vec3& p0, const vec3& p1, const vec3& p2) : _v0(p0), _v1(p1), _v2(p2)
{
	vec3 v20 = p2 - p0;
	vec3 v10 = p1 - p0;
	_normal = glm::cross(v10, v20);
	_area = glm::length(_normal);
	_normal = _normal / _area;
}
const vec3& Triangle::v0() const {
	return _v0;
}
const vec3& Triangle::v1() const {
	return _v1;
}
const vec3& Triangle::v2() const {
	return _v2;
}
BBox3D Triangle::bbox() const {
	vec3 min(glm::min(_v0, glm::min(_v1, _v2)));
	vec3 max(glm::max(_v0, glm::max(_v1, _v2)));
	return BBox3D(min, max);
}

bool Triangle::intersect(const Ray& ray) const {
	const float Epsilon = 0.0000001f;
	vec3 v10 = _v1 - _v0;
	vec3 v20 = _v2 - _v0;
	vec3 h = glm::cross(ray.rd, v20);
	float a = glm::dot(v10, h);
	if (a > -Epsilon && a < Epsilon)
		return false;
	float f = 1.0f / a;
	vec3 s = ray.ro - _v0;
	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;
	vec3 q = glm::cross(s, v10);
	float v = f * glm::dot(ray.rd, q);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	float t = f * glm::dot(v20, q);
	if (t >= ray.tMin && t <= ray.tMax) {
		return true;
	}
	return false;
}
bool Triangle::intersect(const Ray& ray, HitInfo& hitInfo) const {
	const float Epsilon = 0.0000001f;
	vec3 v10 = _v1 - _v0;
	vec3 v20 = _v2 - _v0;
	vec3 h = glm::cross(ray.rd, v20);
	float a = glm::dot(v10, h);
	if (a > -Epsilon && a < Epsilon)
		return false;
	float f = 1.0f / a;
	vec3 s = ray.ro - _v0;
	float u = f * glm::dot(s, h);
	if (u < 0.0f || u > 1.0f)
		return false;
	vec3 q = glm::cross(s, v10);
	float v = f * glm::dot(ray.rd, q);
	if (v < 0.0f || u + v > 1.0f)
		return false;
	float t = f * glm::dot(v20, q);
	if (t >= ray.tMin && t <= ray.tMax) {
		hitInfo.t = t;
		hitInfo.localPosition = ray.ro + ray.rd * t;
		hitInfo.normal = _normal;
		return true;
	}
	return false;
}
float Triangle::area() const {
	return _area;
}
HitInfo Triangle::sample() const {
	HitInfo result;
	result.localPosition = Sampling::uniformTriangle(_v0, _v1, _v2);
	result.normal = _normal;
	return result;
}