#pragma once
#include "PlanarShape.h"

// Done
// rect at (0, 0, 0) with normal (0, 1, 0)
class Rect :public PlanarShape {
	vec2 size;
protected:
	virtual bool isInBounds(vec2 p) const override;
public:
	Rect(const vec2& size = vec2(1.0f));
	virtual BBox3D bbox() const override;
	virtual HitInfo sample() const override;
	virtual float area() const override;
};