#include "Rect.h"

bool Rect::isInBounds(vec2 p) const {
	return std::abs(p.x) <= size.x * 0.5f && std::abs(p.y) <= size.y * 0.5f;
}
Rect::Rect(const vec2& size) : size(size) {
}
BBox3D Rect::bbox() const {
	return BBox3D(vec3(-size.x, 0.0, -size.y) * 0.5f, vec3(size.x, 0.0, size.y) * 0.5f);
}
HitInfo Rect::sample() const {
	HitInfo result;
	result.normal = vec3(0.0, 1.0, 0.0);
	result.localPosition =
		vec3(size.x * (Random::random() - 0.5f), 0.0, size.y * (Random::random() - 0.5f));
	return result;
}
float Rect::area() const {
	return size.x * size.y;
}