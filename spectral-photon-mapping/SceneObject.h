#pragma once
#include <vector>
#include <memory>

#include "Shape.h"
#include "Transform.h"
#include "KDTree.h"
#include "Sampling.h"
#include "Material.h"

class SceneObject;

using spSceneObject = std::shared_ptr<SceneObject>;


class SceneObject {
public:
	SceneObject(const std::shared_ptr<Shape>& shape, const std::shared_ptr<Material>& material, const Affine& transform)
		:transform(transform), shape(shape), material(material) {}
	BBox3D bbox() const {
		return transform.transform(shape->bbox());
	}
	bool intersect(const Ray& ray) const {
		const Ray rayLocal = transform.transformInverse(ray);
		return shape->intersect(rayLocal);
	}
	bool intersect(const Ray& ray, HitInfo& hitInfo) const {
		const Ray rayLocal = transform.transformInverse(ray);
		bool result = shape->intersect(rayLocal, hitInfo);
		if (!result)
			return false;
		if (hitInfo.t < ray.tMin || hitInfo.t > ray.tMax) {
			return false;
		}
		hitInfo.globalPosition = transform.transformPoint(hitInfo.localPosition);//local to world
		hitInfo.normal = transform.transformNormal(hitInfo.normal);
		return true;
	}
	const std::shared_ptr<Material>& getMaterial() const {
		return material;
	}
protected:
	std::shared_ptr<Shape> shape;
	Affine transform;
	std::shared_ptr<Material> material;
};