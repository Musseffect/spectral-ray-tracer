#pragma once
#include "common.h"

class Primitive;
class Light;


struct HitInfo {
	vec3 localPosition;
	vec3 globalPosition;
	vec3 normal;
	vec2 uv;
	float t;
	const Primitive* primitive = nullptr;
	const Light* light = nullptr;
	float lightEmitted(const vec3& wo, float wavelength) const;
};