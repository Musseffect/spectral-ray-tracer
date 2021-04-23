#pragma once
#include "common.h"
#include "Ray.h"

#include <vector>

struct BBox3D {
	vec3 _min;
	vec3 _max;

	BBox3D(const glm::vec3& min, const glm::vec3& max);
	BBox3D();
	vec3 min() const;
	vec3 max() const;
	vec3 center() const;
	vec3 size() const;
	bool isEmpty() const;
	bool isInBall(const vec3& center, float radiusSqr) const;
	template<class Point>
	bool intersect(const Point& p) const;
	bool contains(const BBox3D& box) const;
	// clamped positive distance to box
	static float outerDistToBox(vec3 p, vec3 size);
	static float outerSqrDistToBox(vec3 p, vec3 size);
	// signed distance to box
	static float distToBox(vec3 p, vec3 size);
	template<size_t Size>
	BBox3D(const std::array<glm::vec3, Size> array);
	float outerDistance(vec3 point) const;
	float outerSqrDistance(vec3 point) const;
	float distance(vec3 point) const;
	BBox3D inflate(const float r) const;
	BBox3D inflate(const glm::vec3& scale) const;
	BBox3D(const BBox3D& a, const BBox3D& b);
	template<class Iterator>
	BBox3D(const Iterator& begin, const Iterator& end);
	BBox3D(const std::vector<glm::vec3>& points);
	BBox3D append(const BBox3D& box);
	BBox3D append(const glm::vec3& p);
	bool intersectInclusive(const Ray& ray) const;
	bool intersectInclusive(const Ray& ray, float& t) const;
	bool intersect(const Ray& ray) const;
	int maxExtentDirection() const;
	float area() const;
};

using AABB = BBox3D;

BBox3D unionOp(const BBox3D& a, const BBox3D& b);
BBox3D intersectionOp(const BBox3D& a, const BBox3D& b);
BBox3D subtractionOp(const BBox3D& a, const BBox3D& b);

template<size_t Size>
BBox3D::BBox3D(const std::array<glm::vec3, Size> array)
	:_min(std::numeric_limits<float>::max())
	, _max(std::numeric_limits<float>::min()) {
	for (int i = 0; i < Size; i++) {
		const auto& element = array[i];
		_min = glm::min(_min, element);
		_max = glm::max(_max, element);
	}
}

template<class Iterator>
BBox3D::BBox3D(const Iterator& begin, const Iterator& end)
	:_min(std::numeric_limits<float>::max())
	, _max(std::numeric_limits<float>::min()) {
	for (auto it = begin; it != end; ++it) {
		_min = glm::min(*it, _min);
		_max = glm::max(*it, _max);
	}
}


template<class Point>
bool BBox3D::intersect(const Point& p) const {
	vec3 position = p.position();
	return position.x >= _min.x && position.x <= _max.x &&
		position.y >= _min.y && position.y <= _max.y &&
		position.z >= _min.z && position.z <= _max.z;
}

template<>
bool BBox3D::intersect<vec3>(const vec3& p) const;