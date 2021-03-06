#pragma once
#include "PlanarShape.h"

class Disc : public PlanarShape {
	float radius;
protected:
	virtual bool isInBounds(vec2 p) const override;
public:
	Disc(float radius = 1.0f);
	virtual BBox3D bbox() const override;
	virtual HitInfo sample() const override;
	virtual float area() const override;
};