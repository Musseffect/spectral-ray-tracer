#pragma once
#include "Shape.h"
#include "Distribution1D.h"

class Box : public Shape {
	vec3 center;
	vec3 size;
	Distribution1D surfAreaDist;
public:
	vec3 getNormal(vec3 localPos) const;
	Box(const vec3& center = vec3(0.0f), const vec3& size = vec3(1.0f));
	virtual BBox3D bbox() const override;
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual std::vector<Interval> intersectionList(const Ray& ray) const override;
	virtual HitInfo sample() const override;
	virtual float area() const override;
};
