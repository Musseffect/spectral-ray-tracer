#pragma once
#include "Shape.h"
#include "Triangle.h"


// todo: add indexed mesh
// todo: NOT IMPLEMENTED
// todo: add accelerator structure for intersections
class Mesh : public Shape {
	std::vector<Triangle> triangles;
	BBox3D bounds;
public:
	Mesh();
	Mesh(std::initializer_list<Triangle> list);
	template<typename Iterator>
	Mesh(const Iterator& begin, const Iterator& end);
	Mesh& append(const Triangle& triangle);
	virtual HitInfo sample() const override;
	virtual BBox3D bbox() const override;
	virtual bool intersect(const Ray& ray) const override;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override;
	virtual float area() const override;
};



template<typename Iterator>
Mesh::Mesh(const Iterator& begin, const Iterator& end) {
	for (auto it = begin; it != end; ++it) {
		bounds.append(it->bbox());
		triangles.push_back(*it);
	}
}