#include "Mesh.h"



Mesh::Mesh() {
}

Mesh::Mesh(std::initializer_list<Triangle> list) {
	for (auto triangle : list) {
		bounds.append(triangle.bbox());
		triangles.push_back(triangle);
	}
}

Mesh& Mesh::append(const Triangle& triangle) {
	triangles.push_back(triangle);
	bounds.append(triangle.bbox());
	return *this;
}
HitInfo Mesh::sample() const {
	throw std::runtime_error("Not implemented");
}

BBox3D Mesh::bbox() const {
	return bounds;
}

bool Mesh::intersect(const Ray& ray) const {
	if (!bbox().intersect(ray))
		return false;
	for (auto& triangle : triangles)
	{
		if (triangle.intersect(ray))
			return true;
	}
	return false;
}

bool Mesh::intersect(const Ray& ray, HitInfo& hitInfo) const {
	if (!bbox().intersect(ray))
		return false;
	Ray tRay = ray;
	bool intersected = false;
	for (auto& triangle : triangles)
	{
		if (triangle.intersect(tRay, hitInfo)) {
			tRay.tMax = hitInfo.t;
			intersected = true;
		}
	}
	return intersected;
}

float Mesh::area() const {
	float totalArea = 0.0;
	for (const auto& triangle : triangles) {
		totalArea += triangle.area();
	}
	return totalArea;
}