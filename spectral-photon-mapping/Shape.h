#pragma once
#include <glm\common.hpp>
#include <glm\geometric.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>

#include "Ray.h"
#include "BBox3D.h"
#include "Random.h"
#include "Distribution.h"
#include "Sampling.h"
#include "HitInfo.h"
#undef min
#undef max

#define EPSILON 0.00001f

class SceneObject;
class Light;

struct Interval {
	float min;
	vec3 minNormal;
	float max;
	vec3 maxNormal;
	bool isValid() const {
		return max - min < EPSILON;
	}
	bool overlap(const Interval& other) const {
		return other.min < max && other.min > min || other.max > min && other.max < max;
	}
	bool intervalIsIn(const Interval& other) const {
		return other.min >= min && other.max <= max && other.isValid() && isValid();
	}
};

class Shape {
public:
	virtual BBox3D bbox() const = 0;
	virtual bool intersect(const Ray& ray) const = 0;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const = 0;
	virtual std::vector<Interval> intersectionList(const Ray& ray) const {
		throw std::runtime_error("Cannot return intervals");
	}
	virtual float area() const = 0;
	virtual HitInfo sample() const = 0;
};

class Sphere: public Shape {
	vec3 center;
	float radius;
public:
	Sphere(const vec3& center, float radius) : center(center), radius(radius) {
	}
	virtual BBox3D bbox() const override {
		return BBox3D(glm::vec3(center - radius * 0.5f), glm::vec3(center + radius * 0.5f));
	}
	virtual bool intersect(const Ray& ray) const override {
		vec3 ro = ray.ro - center;
		float c = glm::dot(ro, ro);
		float b = glm::dot(ro, ray.rd);
		float a = glm::dot(ray.rd, ray.rd);
		float d = (b  - c) / a;
		if (d < 0.0)
			return false;
		d = std::sqrt(d);
		float t0 = -b - d;
		float t1 = -b + d;
		if ((t0 >= ray.tMin && t0 <= ray.tMax) || (t1 >=ray.tMin && t1 <= ray.tMax))
			return true;
		return false;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		vec3 ro = ray.ro - center;
		float c = glm::dot(ro, ro);
		float b = glm::dot(ro, ray.rd);
		float a = glm::dot(ray.rd, ray.rd);
		float d = (b - c) / a;
		if (d < 0.0)
			return false;
		d = std::sqrt(d);
		float t0 = -b - d;
		float t1 = -b + d;
		float t = t0 > 0.0 ? t0 : t1;
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
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override {
		vec3 ro = ray.ro - center;
		float c = glm::dot(ro, ro);
		float b = glm::dot(ro, ray.rd);
		float a = glm::dot(ray.rd, ray.rd);
		float d = (b - c) / a;
		if (d < 0.0)
			return {};
		d = std::sqrt(d);
		float t0 = -b - d;
		float t1 = -b + d;
		return { Interval{t0, ray.ro + ray.rd * t0 - center,
			t1, ray.ro + ray.rd * t1 - center} };
	}
	virtual HitInfo sample() const override {
		HitInfo result;
		result.localPosition = radius * Sampling::sampleSphere() + center;
		result.normal = glm::normalize(result.localPosition - center);
		return result;

	}
	virtual float area() const override {
		return 4.0f * radius * glm::pi<float>();
	}
};

// Done
class Box : public Shape {
	vec3 center;
	vec3 size;
	Distribution1D surfAreaDist;
public:
	vec3 getNormal(vec3 localPos) const {
		vec3 p = (localPos - center);
		vec3 s = glm::abs(p);
		return  glm::sign(p) * glm::step(vec3(s.y, s.z, s.x), s) * glm::step(vec3(s.z, s.x, s.y), s);
	}
	Box(const vec3& center = vec3(0.0f), const vec3& size = vec3(1.0f))
		:center(center),
		size(size),
		surfAreaDist({
			size.x * size.y, size.x * size.y,
			size.x * size.z, size.x * size.z,
			size.y * size.z, size.y * size.z
			})
	{
	}
	virtual BBox3D bbox() const override {
		return BBox3D(center - size * 0.5f, center + size * 0.5f);
	}
	virtual bool intersect(const Ray& ray) const override {
		vec3 inv = vec3(1.0) / ray.rd;
		vec3 s = size * 0.5f * glm::abs(inv);
		vec3 o = (center - ray.ro) * inv;
		vec3 tmin = -s + o;
		vec3 tmax = s + o;
		float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
		float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);
		if (tNear > tFar || tFar < ray.tMin)
			return false;
		return true;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		vec3 inv = vec3(1.0) / ray.rd;
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
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override {
		vec3 inv = vec3(1.0) / ray.rd;
		vec3 s = size * 0.5f * glm::abs(inv);
		vec3 o = (center - ray.ro) * inv;
		vec3 tmin = -s + o;
		vec3 tmax = s + o;
		float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
		float tFar = std::min(std::min(tmax.x, tmax.y), tmax.z);
		if (tNear > tFar)
			return {};
		return {Interval{tNear, getNormal(ray.ro + ray.rd * tNear), tFar, getNormal(ray.ro + ray.rd * tFar)}};
	}
	virtual HitInfo sample() const override {
		int index = surfAreaDist.sampleDiscrete(Random::random());
		static const vec3 normal[6] = {
			vec3(0.0, 0.0, -1.0), vec3(0.0, 0.0, 1.0),
			vec3(0.0, -1.0, 0.0), vec3(0.0, 1.0, 0.0),
			vec3(-1.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0)
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
		result.localPosition = normal[index] + center +
			tangentX[index / 2] * (Random::random() - 0.5f) +
			tangentY[index / 2] * (Random::random() - 0.5f);
		return result;
	}
	virtual float area() const override {
		return 2.0f * (size.x * size.z + size.y * size.z + size.x * size.y);
	}
};

// todo: NOT IMPLEMENTED
class Cylinder : public Shape {
	vec3 center;
	float radius;
	float height;
public:
};

// todo: NOT IMPLEMENTED
class Plane : public Shape {
	vec3 normal;
	float d;
public:
};


// todo: NOT IMPLEMENTED
// https://mrl.cs.nyu.edu/~dzorin/rend05/lecture2.pdf
// https://www.geometrictools.com/Documentation/IntersectionLineCone.pdf
class Cone : public Shape {
	vec3 center;
	float radius;
	float height;
public:
};


// todo: NOT IMPLEMENTED
class Torus : public Shape {
	vec3 center;
	float r1;
	float r2;
public:
};

// center at (0, 0, 0) with normal (0, 1, 0)
class Shape2D :public Shape {
public:
	virtual bool intersect(const Ray& ray) const override {
		float t = -ray.ro.y / ray.rd.y;
		if (t < ray.tMin || t > ray.tMax)
			return false;
		vec2 p = vec2(ray.ro.x, ray.ro.z) + vec2(ray.rd.x, ray.rd.z) * t;
		if (!isInBounds(p))
			return false;
		return true;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
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
protected:
	virtual bool isInBounds(vec2 p) const = 0;
};

// Done
// rect at (0, 0, 0) with normal (0, 1, 0)
class Rect :public Shape2D {
	vec2 size;
protected:
	virtual bool isInBounds(vec2 p) const {
		return std::abs(p.x) < size.x * 0.5f && std::abs(p.y) < size.y * 0.5f;
	}
public:
	Rect(const vec2& size = vec2(1.0f)) : size(size) {
	}
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-size.x, 0.0, -size.y) * 0.5f, vec3(size.x, 0.0, size.y) * 0.5f);
	}
	virtual HitInfo sample() const override {
		HitInfo result;
		result.normal = vec3(0.0, 1.0, 0.0);
		result.localPosition =
			vec3(size.x * (Random::random() - 0.5f), 0.0, size.y * (Random::random() - 0.5f));
		return result;
	}
	virtual float area() const override {
		return size.x * size.y;
	}
};

class Disc : public Shape2D {
	float radius;
protected:
	virtual bool isInBounds(vec2 p) const {
		return glm::dot(p, p) < radius * radius;
	}
public:
	Disc(float radius = 1.0f) : radius(radius) {
	}
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-radius, -radius, 0.0f), vec3(radius, radius, 0.0f));
	}
	virtual HitInfo sample() const override {
		HitInfo result;
		result.normal = vec3(0.0f, 1.0f, 0.0f);
		vec2 sample = Sampling::sampleDisk(radius);
		result.localPosition = vec3(sample.x, 0.0f, sample.y);
		return result;
	}
	virtual float area() const override {
		return glm::pi<float>() * radius * radius / 2.0f;
	}
};

// multiple shapes together
template<class Accelerator>
class Compound : public Shape {
	Accelerator accelerator;
public:
	virtual bool intersect(const Ray& ray) const override {
		return accelerator.intersect(ray);
	}
	virtual BBox3D bbox() const override {
		return accelerator.bbox();
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		return accelerator.intersect(ray, hitInfo);
	}
};

class Triangle : public Shape {
	vec3 _v0;
	vec3 _v1;
	vec3 _v2;
	vec3 _normal;
	float _area;
public:
	Triangle(const vec3& p0, const vec3& p1, const vec3& p2) : _v0(p0), _v1(p1), _v2(p2)
	{
		vec3 v20 = p2 - p0;
		vec3 v10 = p1 - p0;
		_normal = glm::cross(v20, v10);
		_area = glm::length(_normal);
		_normal = _normal / _area;
	}
	const vec3& v0() const {
		return _v0;
	}
	const vec3& v1() const {
		return _v1;
	}
	const vec3& v2() const {
		return _v2;
	}
	virtual BBox3D bbox() const override {
		vec3 min(glm::min(_v0, glm::min(_v1, _v2)));
		vec3 max(glm::max(_v0, glm::max(_v1, _v2)));
		return BBox3D(min, max);
	}
	//Möller–Trumbore intersection algorithm
	virtual bool intersect(const Ray& ray) const override {
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
		if (t > ray.tMin && t < ray.tMax) {
			return true;
		}
		return false;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
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
		if (t > ray.tMin && t < ray.tMax) {
			hitInfo.t = t;
			hitInfo.localPosition = ray.ro + ray.rd * t;
			hitInfo.normal = _normal;
			return true;
		}
		return false;
	}
	virtual float area() const override {
		return _area;
	}
	virtual HitInfo sample() const override {
		HitInfo result;
		result.localPosition = Sampling::randomTriangle(_v0, _v1, _v2);
		result.normal = _normal;
		return result;
	}
};


// todo: add indexed mesh
// todo: NOT IMPLEMENTED
// todo: add accelerator structure for intersections
class Mesh : public Shape {
	std::vector<Triangle> triangles;
	BBox3D bounds;
public:
	Mesh() {
	}
	Mesh(std::initializer_list<Triangle> list) {
		for (auto triangle : list) {
			bounds.append(triangle.bbox());
			triangles.push_back(triangle);
		}
	}
	template<typename Iterator>
	Mesh(const Iterator& begin, const Iterator& end) {
		for (auto it = begin; it != end; ++it) {
			bounds.append(it->bbox());
			triangles.push_back(*it);
		}
	}
	Mesh& append(const Triangle& triangle) {
		triangles.push_back(triangle);
		bounds.append(triangle.bbox());
		return *this;
	}
	virtual HitInfo sample() const override {
		throw std::runtime_error("Not implemented");
	}
	virtual BBox3D bbox() const override {
		return bounds;
	}
	virtual bool intersect(const Ray& ray) const override {
		if (!bbox().intersect(ray))
			return false;
		for (auto& triangle : triangles)
		{
			if (triangle.intersect(ray))
				return true;
		}
		return false;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		if (!bbox().intersect(ray))
			return false;
		Ray tRay = ray;
		bool intersected = false;
		for (auto& triangle : triangles)
		{
			if (triangle.intersect(tRay, hitInfo)) {
				tRay.tMax = hitInfo.t;
				intersected = true;
			}
		}
		return intersected;
	}
	virtual float area() const override {
		float totalArea = 0.0;
		for (const auto& triangle : triangles) {
			totalArea += triangle.area();
		}
		return totalArea;
	}
};