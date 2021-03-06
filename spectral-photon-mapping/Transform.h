#pragma once
#include "common.h"
#include "BBox3D.h"
#include "Ray.h"

#include <array>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;
using quat = glm::quat;

vec3 ortho(vec3 v);

// Done
struct Rigid {
	vec3 translation;
	quat rotation;
	vec3 scale;
	Rigid(vec3 t, quat r = quat(), vec3 s = vec3(1.0f));
	vec3 transform(const vec3& v) const;
	vec3 inverseTransform(const vec3& v) const;
	mat4 toMat4() const;
};

Rigid interpolate(const Rigid& a, const Rigid& b, float t);

class Affine {
	mat4 _direct;
	mat4 _inverse;
public:
	static mat3 buildCoordSystemTransform(vec3 zDir);
	static mat4 buildCoordSystemTransform(vec3 translation, vec3 zDir);
	friend Affine interpolate(const Affine& a, const Affine& b, float t);
	Affine(const Rigid& transform);
	Affine(const mat4& direct);
	Affine(const mat4& direct, const mat4& inverse);
	Affine inverse() const;
	static Affine lookAt(vec3 eye, vec3 target, vec3 up);
	Ray transformInverse(const Ray& ray) const;
	vec3 transformInversePoint(const vec3& point) const;
	vec3 transformInverseVector(const vec3& vector) const;
	Ray transform(const Ray& ray) const;
	vec3 transformNormal(const vec3& normal) const;
	vec3 transformPoint(const vec3& point) const;
	vec3 transformVector(const vec3& vector) const;
	BBox3D transform(const BBox3D& bbox) const;
};

Affine interpolate(const Affine& a, const Affine& b, float t);
