#pragma once
#include "Shape.h"


// (ro + rd * t) * n - d = 0
class Plane : public Shape {
	vec3 normal;
	float d;
public:
	virtual BBox3D bbox() const override;
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override;
	virtual float area() const override;
	virtual float pdf() const override;
	virtual HitInfo sample() const override;
	virtual float pdf(const vec3& pos, const vec3& wi, const Affine& transform) const override;
};
