#pragma once
#include "Shape.h"
#include "Triangle.h"


// todo: add indexed mesh
// todo: NOT IMPLEMENTED
// todo: add accelerator structure for intersections
template<class PrimitiveAccelerator>
class Mesh : public Shape {
	std::vector<Triangle> triangles;
	std::unique_ptr<PrimitiveAccelerator> accelerator;
	float _area;
public:
	Mesh();
	Mesh(std::initializer_list<Triangle> list);
	template<typename Iterator>
	Mesh(const Iterator& begin, const Iterator& end);
	virtual HitInfo sample() const override;
	virtual BBox3D bbox() const override;
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual float area() const override;
};



template<class Accelerator>
template<typename Iterator>
Mesh<Accelerator>::Mesh(const Iterator& begin, const Iterator& end) {
	_area = 0.0f;
	for (auto it = begin; it != end; ++it) {
		triangles.push_back(*it);
		_area += it->area();
	}
	accelerator(triangles.begin(), triangles.end());
}


template<class Accelerator>
Mesh<Accelerator>::Mesh() {
}

template<class Accelerator>
Mesh<Accelerator>::Mesh(std::initializer_list<Triangle> list) {
	_area = 0.0f;
	for (auto triangle : list) {
		triangles.push_back(triangle);
		_area += triangle.area();
	}
	accelerator = std::make_unique<Accelerator>(triangles.begin(), triangles.end());
}

template<class Accelerator>
HitInfo Mesh<Accelerator>::sample() const {
	throw std::runtime_error("Not implemented");
}

template<class Accelerator>
BBox3D Mesh<Accelerator>::bbox() const {
	return accelerator->bbox();
}

template<class Accelerator>
bool Mesh<Accelerator>::intersect(const Ray& ray) const {
	return accelerator->intersect(ray);
	/*if (!bbox().intersect(ray))
		return false;
	for (auto& triangle : triangles)
	{
		if (triangle.intersect(ray))
			return true;
	}
	return false;*/
}

template<class Accelerator>
bool Mesh<Accelerator>::intersect(const Ray& ray, HitInfo& hitInfo) const {
	Ray tRay = ray;
	return accelerator->intersect(tRay, hitInfo);
	/*if (!bbox().intersect(ray))
		return false;
	Ray tRay = ray;
	bool intersected = false;
	for (auto& triangle : triangles) {
		if (triangle.intersect(tRay, hitInfo)) {
			tRay.tMax = hitInfo.t;
			intersected = true;
		}
	}
	return intersected;*/
}

template<class Accelerator>
float Mesh<Accelerator>::area() const {
	return _area;
}