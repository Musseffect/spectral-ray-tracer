#include "Disk.h"

bool Disc::isInBounds(vec2 p) const {
	return glm::dot(p, p) <= radius * radius;
}
Disc::Disc(float radius) : radius(radius) {
}
BBox3D Disc::bbox() const {
	return BBox3D(vec3(-radius, -radius, 0.0f), vec3(radius, radius, 0.0f));
}
HitInfo Disc::sample() const {
	HitInfo result;
	result.normal = vec3(0.0f, 1.0f, 0.0f);
	vec2 sample = Sampling::sampleDisk(radius);
	result.localPosition = vec3(sample.x, 0.0f, sample.y);
	return result;
}
float Disc::area() const {
	return glm::pi<float>() * radius * radius;
}
