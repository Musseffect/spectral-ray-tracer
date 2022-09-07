#include "Transform.h"

float diff(const mat4& a, const mat4& b) {
	float result = 0.0f;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j)
			result += glm::pow(a[i][j] - b[i][j], 2.0f);
	}
	return result;
}

vec3 ortho(vec3 v) {
	//  See : http://lolengine.net/blog/2013/09/21/picking-orthogonal-vector-combing-coconuts
	return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0) : vec3(0.0, -v.z, v.y);
}

Transform::Transform(vec3 t, quat r, vec3 s) : translation(t), rotation(r), scale(s) {}

vec3 Transform::transform(const vec3& v) const {
	return (rotation * (v * scale) * inverse(rotation)) + translation;
}

vec3 Transform::inverseTransform(const vec3& v) const {
	return (inverse(rotation) * (v - translation) * rotation) / scale;
}

mat4 Transform::toMat4() const {
	return glm::translate(mat4(1.0f), translation) * mat4_cast(rotation) * glm::scale(mat4(1.0f), scale);
}

Transform interpolate(const Transform& a, const Transform& b, float t) {
	return Transform(glm::mix(a.translation, b.translation, t),
		glm::slerp(a.rotation, b.rotation, t),
		glm::mix(a.scale, b.scale, t));
}


Affine::Affine(const Transform& transform) {
	_direct = transform.toMat4();
	_inverse = glm::inverse(_direct);
}

Affine::Affine(const mat4& direct) :_direct(direct), _inverse(glm::inverse(direct)) {
}

Affine::Affine(const mat4& direct, const mat4& inverse) : _direct(direct), _inverse(inverse) {
	assert(diff(direct * inverse, mat4(1.0f)) < 0.001f);
}

Affine Affine::inverse() const {
	return Affine(_inverse, _direct);
}

Affine Affine::lookAt(vec3 eye, vec3 target, vec3 up) {
	mat4 res = glm::lookAt(eye, target, up);
	return Affine(res, glm::inverse(res));
}

Ray Affine::transformInverse(const Ray& ray) const {
	return Ray(transformInversePoint(ray.ro), transformInverseVector(ray.rd), ray.tMin, ray.tMax);
}

vec3 Affine::transformInversePoint(const vec3& point) const {
	vec4 result = _inverse * vec4(point, 1.0);
	return vec3(result) / result.w;
}

vec3 Affine::transformInverseVector(const vec3& vector) const {
	return vec3(_inverse * vec4(vector, 0.0));
}

Ray Affine::transform(const Ray& ray) const {
	return Ray(transformPoint(ray.ro), transformVector(ray.rd), ray.tMin, ray.tMax);
}

vec3 Affine::transformNormal(const vec3& normal) const {
	return normalize(vec3(glm::transpose(_inverse) * vec4(normal, 0.0)));
}

vec3 Affine::transformPoint(const vec3& point) const {
	vec4 result = _direct * vec4(point, 1.0);
	return vec3(result) / result.w;
}

vec3 Affine::transformVector(const vec3& vector) const {
	return vec3(_direct * vec4(vector, 0.0));
}

BBox3D Affine::transform(const BBox3D& bbox) const {
	std::array<vec3, 8> points;
	points[0] = vec3(_direct * vec4(bbox._min, 1.0));
	points[1] = vec3(_direct * vec4(bbox._min.x, bbox._min.y, bbox._max.z, 1.0));
	points[2] = vec3(_direct * vec4(bbox._min.x, bbox._max.y, bbox._min.z, 1.0));
	points[3] = vec3(_direct * vec4(bbox._min.x, bbox._max.y, bbox._max.z, 1.0));
	points[4] = vec3(_direct * vec4(bbox._min.x, bbox._max.y, bbox._min.z, 1.0));
	points[5] = vec3(_direct * vec4(bbox._max.x, bbox._min.y, bbox._min.z, 1.0));
	points[6] = vec3(_direct * vec4(bbox._max.x, bbox._max.y, bbox._min.z, 1.0));
	points[7] = vec3(_direct * vec4(bbox._max, 1.0));
	return BBox3D(points);
}

Affine interpolate(const Affine& a, const Affine& b, float t) {
	mat4 result;
	for (int i = 0; i < 4; i++) {
		result[i] = glm::mix(a._direct[i], b._direct[i], t);
	}
	return result;
}

mat3 Affine::buildCoordSystemTransform(vec3 zDir) {
	vec3 xDir = ortho(zDir);
	vec3 yDir = glm::cross(xDir, zDir);
	return mat3(
		xDir,
		yDir,
		zDir);
}

mat4 Affine::buildCoordSystemTransform(vec3 translation, vec3 zDir) {
	vec3 xDir = ortho(zDir);
	vec3 yDir = glm::cross(xDir, zDir);
	return mat4(
		vec4(xDir, 0.0),
		vec4(yDir, 0.0),
		vec4(zDir, 0.0),
		vec4(translation, 1.0));
}