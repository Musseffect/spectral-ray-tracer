#pragma once
#include "Shape.h"


class InfiniteCylinder : public Shape {
	vec2 center;
	float radius;
public:
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-radius, -std::numeric_limits<float>::infinity(), -radius),
			vec3(radius, std::numeric_limits<float>::infinity(), radius));
	}
	virtual bool intersect(const Ray& ray) const override {
		vec2 ro(ray.ro.x - center.x, ray.ro.z - center.y);
		vec2 rd(ray.rd.x, ray.rd.z);
		float c = glm::dot(ro, ro) - radius * radius;
		float b = glm::dot(ro, rd);
		float a = glm::dot(rd, rd);
		float d = (b * b - c * a);
		if (d < 0.0f)
			return {};
		d = std::sqrt(d);
		float t0 = (-b - d) / a;
		float t1 = (-b + d) / a;
		if ((t0 >= ray.tMin && t0 <= ray.tMax) || (t1 >= ray.tMin && t1 <= ray.tMax))
			return true;
		return false;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		vec2 ro(ray.ro.x - center.x, ray.ro.z - center.y);
		vec2 rd(ray.rd.x, ray.rd.z);
		float c = glm::dot(ro, ro) - radius * radius;
		float b = glm::dot(ro, rd);
		float a = glm::dot(rd, rd);
		float d = (b * b - c * a);
		if (d < 0.0f)
			return {};
		d = std::sqrt(d);
		float t0 = (-b - d) / a;
		float t1 = (-b + d) / a;
		float t = t0 >= ray.tMin ? t0 : t1;
		if (t >= ray.tMin && t <= ray.tMax)
		{
			hitInfo.localPosition = ray.ro + ray.rd * t;
			hitInfo.normal = glm::normalize(vec3(hitInfo.localPosition.x - center.x, 0.0f, hitInfo.localPosition.z - center.y));
			hitInfo.t = t;
			hitInfo.uv = vec2(0.);
			return true;
		}
		return false;
	}
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override {
		vec2 ro(ray.ro.x - center.x, ray.ro.z - center.y);
		vec2 rd(ray.rd.x, ray.rd.z);
		float c = glm::dot(ro, ro) - radius * radius;
		float b = glm::dot(ro, rd);
		float a = glm::dot(rd, rd);
		float d = (b * b - c * a);
		if (d < 0.0f)
			return {};
		d = std::sqrt(d);
		float t0 = (-b - d) / a;
		float t1 = (-b + d) / a;
		vec3 pNear = ray.ro + ray.rd * t0;
		vec3 pFar = ray.ro + ray.rd * t1;
		return { Interval{t0, glm::normalize(pNear - vec3(center.x, pNear.y, center.y)),
			t1, glm::normalize(pFar - vec3(center.x, pFar.y, center.y))} };
	}
	virtual float area() const override {
		return std::numeric_limits<float>::infinity();
	}
	virtual float pdf() const override {
		throw std::runtime_error("Cannot sample infinite shape");
	}
	virtual HitInfo sample() const override {
		throw std::runtime_error("Cannot sample infinite shape");
	}
	virtual float pdf(const vec3& pos, const vec3& wi, const Affine& transform) const override {
		throw std::runtime_error("Cannot sample infinite shape");
	}
};

// todo: NOT IMPLEMENTED
class Cylinder : public Shape {
protected:
	vec3 center;
	float height;
	float radius;
public:
	Cylinder(float height = 1.0f, float radius = 0.5f, vec3 center = vec3(0.0f))
		:height(height), radius(radius), center(center)
	{
	}
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-radius, -height * 0.5, -radius) + center, vec3(radius, 0.5 * height, radius) + center);
	}
	virtual float area() const override {
		return radius * glm::two_pi<float>() * height;
	}
};

class CappedCylinder : public Cylinder {
public:
	virtual float area() const override {
		return radius * glm::two_pi<float>() * (height + radius);
	}
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override {
		vec2 ro(ray.ro.x - center.x, ray.ro.z - center.z);
		vec2 rd(ray.rd.x, ray.rd.z);
		float c = glm::dot(ro, ro) - radius * radius;
		float b = glm::dot(ro, rd);
		float a = glm::dot(rd, rd);
		float d = (b * b - c * a);
		if (d < 0.0f)
			return {};
		d = std::sqrt(d);
		float tCylinderMin = (-b - d) / a;
		float tCylinderMax = (-b + d) / a;

		float inv = 1.0f / ray.rd.y;
		float s = height * 0.5f * glm::abs(inv);
		float o = (center.y - ray.ro.y) * inv;
		float tPlaneMin = -s + o;
		float tPlaneMax = s + o;
		float tNear = -1.0f;
		float tFar = -1.0f;
		vec3 minNormal;
		if (tCylinderMin > tPlaneMin) {
			tNear = tCylinderMin;
			vec3 p = ray.ro + ray.rd * tNear;
			minNormal = normalize(p - vec3(center.x, p.y, center.z));
		} else {
			tNear = tPlaneMin;
			vec3 p = ray.ro + ray.rd * tNear;
			minNormal = vec3(0.0f, glm::sign(p.y - center.y), 0.0f);
		}
		vec3 maxNormal;
		if (tCylinderMax < tPlaneMax) {
			tFar = tCylinderMax;
			vec3 p = ray.ro + ray.rd * tFar;
			maxNormal = normalize(p - vec3(center.x, p.y, center.z));

		} else {
			tFar = tPlaneMax;
			vec3 p = ray.ro + ray.rd * tFar;
			maxNormal = vec3(0.0f, glm::sign(p.y - center.y), 0.0f);
		}

		return { Interval{tNear, minNormal,
			tFar, maxNormal} };
	}
};