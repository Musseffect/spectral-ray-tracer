#pragma once
#include "common.h"

struct Ray {
	vec2 ro;
	vec2 rd;
	float tMin = 0.001f;
	float tMax = std::numeric_limits<float>::max();
	Ray() :ro(0.0), rd(0.0, 0.0, -1.0) {}
	Ray(const vec2& ro, const vec2& rd, float tMin = 0.001f, float tMax = std::numeric_limits<float>::max()) :ro(ro), rd(rd), tMin(tMin), tMax(tMax) {}
	vec2 operator()(float t) const {
		return ro + rd * t;
	}
};