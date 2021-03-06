#pragma once
#include "Shape.h"


class Triangle : public Shape {
	vec3 _v0;
	vec3 _v1;
	vec3 _v2;
	vec3 _normal;
	float _area;
public:
	Triangle(const vec3& p0, const vec3& p1, const vec3& p2);
	const vec3& v0() const;
	const vec3& v1() const;
	const vec3& v2() const;
	virtual BBox3D bbox() const override;
	//Moller–Trumbore intersection algorithm
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual float area() const override;
	virtual HitInfo sample() const override;
};