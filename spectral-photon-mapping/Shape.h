#pragma once
#include <glm\common.hpp>
#include <glm\geometric.hpp>
#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>

#include "Ray.h"
#include "BBox3D.h"
#include "Random.h"
#include "Distribution1D.h"
#include "Sampling.h"
#include "HitInfo.h"
#include "Transform.h"
#undef min
#undef max

#define EPSILON 0.00001f

class Primitive;
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
	virtual float pdf() const { return 1.0f / area(); }
	virtual HitInfo sample() const = 0;
	virtual float pdf(const vec3& pos, const vec3& wi, const Affine& transform) const;
};

using spShape = std::shared_ptr<Shape>;


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



// multiple shapes together
template<class Accelerator>
class Compound : public Shape {
	std::shared_ptr<Accelerator> accelerator;
	float _area;
	std::unique_ptr<Distribution1D> areaDistribution;
public:
	Compound(const std::shared_ptr<Accelerator> accelerator):accelerator(accelerator)
	{
		_area = 0.0f;
		std::vector<float> areas;
		for (auto primitive : accelerator.primitives()) {
			float value = primitive.area();
			_area += value;
			areas.push_back(value);
		}
		areaDistribution = std::make_unique(areas.begin(), areas.end());
	}
	virtual bool intersect(const Ray& ray) const override {
		return accelerator.intersect(ray);
	}
	virtual BBox3D bbox() const override {
		return accelerator.bbox();
	}
	virtual float area() const override {
		return _area;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		return accelerator.intersect(ray, hitInfo);
	}
};