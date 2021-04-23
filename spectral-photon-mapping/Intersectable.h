#pragma once
#include "HitInfo.h"
#include "Ray.h"
#include "BBox3D.h"

class Intersectable {
public:
	virtual bool intersect(const Ray& ray) const = 0;
	virtual bool intersect(Ray& ray, HitInfo& hitInfo) const = 0;
	virtual BBox3D bbox() const = 0;
};