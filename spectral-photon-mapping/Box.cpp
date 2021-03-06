#include "Box.h"

vec3 Box::getNormal(vec3 localPos) const {
	vec3 p = (localPos - center) / glm::max(size, vec3(0.0001f));
	vec3 s = glm::abs(p);
	return glm::sign(p) * glm::step(vec3(s.y, s.z, s.x), s) * glm::step(vec3(s.z, s.x, s.y), s);
}
Box::Box(const vec3& center, const vec3& size)
	:center(center),
	size(size),
	surfAreaDist({
		size.x * size.y, size.x * size.y,
		size.x * size.z, size.x * size.z,
		size.y * size.z, size.y * size.z
		})
{
}
BBox3D Box::bbox() const {
	return BBox3D(center - size * 0.5f, center + size * 0.5f);
}
bool Box::intersect(const Ray& ray) const {
	vec3 inv = vec3(1.0f) / ray.rd;
	vec3 s = size * 0.5f * glm::abs(inv);
	vec3 o = (center - ray.ro) * inv;
	vec3 tmin = -s + o;
	vec3 tmax = s + o;
	float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
	float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);
	if (tNear > tFar)
		return false;
	if ((tNear >= ray.tMin && tNear <= ray.tMax) || (tFar >= ray.tMin && tFar <= ray.tMax))
		return true;
	return false;
}
bool Box::intersect(const Ray& ray, HitInfo& hitInfo) const {
	vec3 inv = vec3(1.0f) / ray.rd;
	vec3 s = size * 0.5f * glm::abs(inv);
	vec3 o = (center - ray.ro) * inv;
	vec3 tmin = -s + o;
	vec3 tmax = s + o;
	float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
	float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);
	if (tNear > tFar || tFar < ray.tMin)
		return false;
	float t = tNear;
	if (tNear < ray.tMin)
		t = tFar;
	if (t > ray.tMax)
		return false;
	hitInfo.t = t;
	hitInfo.localPosition = ray.ro + ray.rd * hitInfo.t;
	hitInfo.normal = getNormal(hitInfo.localPosition);
	return true;
}
std::vector<Interval> Box::intersectionList(const Ray& ray) const {
	vec3 inv = vec3(1.0f) / ray.rd;
	vec3 s = size * 0.5f * glm::abs(inv);
	vec3 o = (center - ray.ro) * inv;
	vec3 tmin = -s + o;
	vec3 tmax = s + o;
	float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
	float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);
	if (tNear > tFar)
		return {};
	return { Interval{tNear, getNormal(ray.ro + ray.rd * tNear), tFar, getNormal(ray.ro + ray.rd * tFar)} };
}
HitInfo Box::sample() const {
	int index = surfAreaDist.sampleDiscrete(Random::random());
	static const vec3 normal[6] = {
		vec3(0.0, 0.0, -1.0), vec3(0.0, 0.0, 1.0),
		vec3(0.0, -1.0, 0.0), vec3(0.0, 1.0, 0.0),
		vec3(-1.0, 0.0, 0.0), vec3(1.0, 0.0, 0.0)
	};
	static const vec3 tangentX[3] = {
		vec3(1.0, 0.0, 0.0),
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0)
	};
	static const vec3 tangentY[3] = {
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 0.0, 1.0)
	};
	HitInfo result;
	result.normal = normal[index];
	result.localPosition = center + (normal[index] * 0.5f +
		tangentX[index / 2] * (Random::random() - 0.5f) +
		tangentY[index / 2] * (Random::random() - 0.5f)) * size;
	return result;
}
float Box::area() const {
	return 2.0f * (size.x * size.z + size.y * size.z + size.x * size.y);
}