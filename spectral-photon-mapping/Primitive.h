#pragma once
#include <vector>
#include <memory>

#include "Intersectable.h"
#include "Shape.h"
#include "Transform.h"
#include "Sampling.h"
#include "Material.h"

class Primitive;

using spPrimitive = std::shared_ptr<Primitive>;


class Primitive: public Intersectable {
public:
	Primitive(const std::shared_ptr<Shape>& shape, const std::shared_ptr<Spectral::Material>& material, const Affine& transform)
		:transform(transform), shape(shape), material(material) {}
	virtual BBox3D bbox() const override {
		return transform.transform(shape->bbox());
	}
	virtual bool intersect(const Ray& ray) const override {
		const Ray rayLocal = transform.transformInverse(ray);
		return shape->intersect(rayLocal);
	}
	virtual bool intersect(Ray& ray, HitInfo& hitInfo) const override {
		const Ray rayLocal = transform.transformInverse(ray);
		bool result = shape->intersect(rayLocal, hitInfo);
		if (!result)
			return false;
		hitInfo.globalPosition = transform.transformPoint(hitInfo.localPosition);//local to world
		hitInfo.normal = transform.transformNormal(hitInfo.normal);
		ray.tMax = hitInfo.t;
		hitInfo.primitive = this;
		hitInfo.light = nullptr;
		return true;
	}
	const std::shared_ptr<Spectral::Material>& getMaterial() const {
		return material;
	}
protected:
	std::shared_ptr<Shape> shape;
	Affine transform;
	std::shared_ptr<Spectral::Material> material;
};