#pragma once
#include "Shape.h"

// center at (0, 0, 0) with normal (0, 1, 0)
class PlanarShape :public Shape {
public:
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
protected:
	virtual bool isInBounds(vec2 p) const = 0;
};