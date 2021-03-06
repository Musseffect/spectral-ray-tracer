#include "BBox3D.h"

#include <algorithm>


BBox3D::BBox3D(const glm::vec3& min, const glm::vec3& max) : _min(min), _max(max) {
}

BBox3D::BBox3D()
	: _min(std::numeric_limits<float>::max())
	, _max(std::numeric_limits<float>::lowest()) {
}

vec3 BBox3D::min() const {
	return _min;
}

vec3 BBox3D::max() const {
	return _max;
}

vec3 BBox3D::center() const {
	return (_max + _min) * 0.5f;
}

vec3 BBox3D::size() const {
	return _max - _min;
}

bool BBox3D::isEmpty() const {
	return _max.x > _min.x || _max.y > _min.y || _max.z > _min.z;
}

bool BBox3D::isInBall(const vec3& center, float radiusSqr) const {
	vec3 diff = glm::max(glm::abs(_min - center), glm::abs(_max - center));
	return glm::length2(diff) <= radiusSqr;
}

float BBox3D::outerDistToBox(vec3 p, vec3 size) {
	vec3 r = glm::abs(p) - size;
	return length(glm::max(r, vec3(0.0)));
}

float BBox3D::outerSqrDistToBox(vec3 p, vec3 size) {
	vec3 r = glm::abs(p) - size;
	return glm::length2(glm::max(r, vec3(0.0)));
}

float BBox3D::distToBox(vec3 p, vec3 size) {
	vec3 r = glm::abs(p) - size;
	return glm::length(glm::max(r, vec3(0.0))) + glm::min(glm::max(r.x, glm::max(r.y, r.z)), 0.0f);
}

float BBox3D::outerSqrDistance(vec3 point) const {
	return outerSqrDistToBox(point - 0.5f * (_min + _max), 0.5f * (_max - _min));
}

float BBox3D::outerDistance(vec3 point) const {
	return outerDistToBox(point - 0.5f * (_min + _max), 0.5f * (_max - _min));
}

float BBox3D::distance(vec3 point) const {
	return distToBox(point - 0.5f * (_min + _max), 0.5f * (_max - _min));
}

BBox3D BBox3D::inflate(const float r) const {
	return BBox3D(_min * r, _max * r);
}

BBox3D BBox3D::inflate(const glm::vec3& scale) const {
	return BBox3D(_min * scale, _max * scale);
}

BBox3D::BBox3D(const BBox3D& a, const BBox3D& b) :
	_min(glm::min(a._min, b._min)),
	_max(glm::max(a._max, b._max))
{
}

BBox3D::BBox3D(const std::vector<glm::vec3>& points) {
	_min = glm::vec3(std::numeric_limits<float>::max());
	_max = glm::vec3(std::numeric_limits<float>::lowest());
	for (const auto& point : points) {
		_min = glm::min(point, _min);
		_max = glm::max(point, _max);
	}
}

BBox3D BBox3D::append(const BBox3D& box) {
	_min = glm::min(box._min, _min);
	_max = glm::max(box._max, _max);
	return *this;
}

BBox3D BBox3D::append(const glm::vec3& p) {
	_min = glm::min(p, _min);
	_max = glm::max(p, _max);
	return *this;
}

bool BBox3D::intersect(const Ray& ray) const {
	vec3 inv = vec3(1.0) / ray.rd;
	vec3 s = (_max - _min) * 0.5f * glm::abs(inv);
	vec3 o = ((_max + _min) * 0.5f - ray.ro) * inv;
	vec3 tmin = -s + o;
	vec3 tmax = s + o;
	float tNear = std::max(std::max(tmin.x, tmin.y), tmin.z);
	float tFar = std::max(std::max(tmax.x, tmax.y), tmax.z);
	if (tNear > tFar)
		return false;
	if ((tNear < ray.tMin || tNear > ray.tMax) && (tFar < ray.tMin || tFar > ray.tMax))
		return false;
	return true;
}

BBox3D unionOp(const BBox3D& a, const BBox3D& b) {
	return BBox3D(a, b);
}

BBox3D intersectionOp(const BBox3D& a, const BBox3D& b) {
	return BBox3D(glm::max(a._min, b._min), glm::min(a._max, b._max));
}

BBox3D subtractionOp(const BBox3D& a, const BBox3D& b) {
	glm::vec3 min = b._max;
	glm::vec3 max = b._min;
	for (int i = 0; i < 3; i++) {
		if (a._min[i] < b._min[i] || b._max[i] < a._min[i])
			min[i] = a._min[i];
		else if (b._max[i] < a._max[i])
			min[i] = b._max[i];
		if (a._max[i] > b._max[i] || b._min[i] > a._max[i])
			max[i] = a._max[i];
		else if (b._min[i] > a._min[i])
			max[i] = b._min[i];
	}
	return BBox3D(min, max);
}

int BBox3D::maxExtentDirection() const {
	vec3 _size = size();
	if (_size.x > _size.y) {
		if (_size.x > _size.z)
			return 0;
		return 2;
	}
	if (_size.y > _size.z)
		return 1;
	return 2;
}

float BBox3D::area() const {
	vec3 _size = size();
	return 2.0f * (_size.x * _size.y + _size.y * _size.z + _size.x * _size.z);
}