#pragma once
#include "common.h"

class SceneObject;
class Light;


struct HitInfo {
	vec3 localPosition;
	vec3 globalPosition;
	vec3 normal;
	vec2 uv;
	float t;
	SceneObject* sceneObject = nullptr;
	Light* light = nullptr;
	float lightEmitted(const vec3& wo, float wavelength) const;
};