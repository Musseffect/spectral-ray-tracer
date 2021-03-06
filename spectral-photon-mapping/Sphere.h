#pragma once
#include "Shape.h"

class Sphere : public Shape {
	vec3 center;
	float radius;
public:
	Sphere(const vec3& center = vec3(0.0f), float radius = 0.5f);
	virtual BBox3D bbox() const override;
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override;
	virtual HitInfo sample() const override;
	virtual float area() const override;
};