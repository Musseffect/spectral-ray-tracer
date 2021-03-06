#include "Sphere.h"

Sphere::Sphere(const vec3& center, float radius) : center(center), radius(radius) {
}

BBox3D Sphere::bbox() const {
	return BBox3D(glm::vec3(center - radius), glm::vec3(center + radius));
}

bool Sphere::intersect(const Ray& ray) const {
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
	if ((t0 >= ray.tMin && t0 <= ray.tMax) || (t1 >= ray.tMin && t1 <= ray.tMax))
		return true;
	return false;
}

bool Sphere::intersect(const Ray& ray, HitInfo& hitInfo) const {
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
	float t = t0 >= ray.tMin ? t0 : t1;
	if (t >= ray.tMin && t <= ray.tMax)
	{
		hitInfo.localPosition = ray.ro + ray.rd * t;
		hitInfo.normal = glm::normalize(hitInfo.localPosition - center);
		hitInfo.t = t;
		hitInfo.uv = vec2(0.);
		return true;
	}
	return false;
}

std::vector<Interval> Sphere::intersectionList(const Ray& ray) const {
	vec3 ro = ray.ro - center;
	float c = glm::dot(ro, ro) - radius * radius;
	float b = glm::dot(ro, ray.rd);
	float a = glm::dot(ray.rd, ray.rd);
	float d = (b * b - c * a);
	if (d < 0.0f)
		return {};
	d = std::sqrt(d);
	float t0 = (-b - d) / a;
	float t1 = (-b + d) / a;
	return { Interval{t0, glm::normalize(ray.ro + ray.rd * t0 - center),
		t1, glm::normalize(ray.ro + ray.rd * t1 - center)} };
}

HitInfo Sphere::sample() const {
	HitInfo result;
	result.localPosition = radius * Sampling::sampleSphere() + center;
	result.normal = glm::normalize(result.localPosition - center);
	return result;
}

float Sphere::area() const {
	return 4.0f * radius * glm::pi<float>();
}