#include "Sampling.h"
#include "Transform.h"


float Sampling::sampleStratified(int count) {
	int bin = Random::random(count - 1);
	return (float(bin) + Random::random()) / float(count);
}

vec3 Sampling::sampleHemisphere(vec3 dir) {
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r = vec2(Random::random(), Random::random());
	float p = sqrt(1.0f - r.y * r.y);
	return dir * r.y + cos(r.x) * p * o1 + sin(r.x) * p * o2;
}

float Sampling::uniformHemispherePdf() {
	return glm::one_over_two_pi<float>();
}

vec2 Sampling::sampleDisk(float radius) {
	float r = sqrt(Random::random()*radius);
	float angle = Random::random()*glm::two_pi<float>();
	return vec2(r*cos(angle), r*sin(angle));
}

vec3 Sampling::sampleSphere() {
	vec2 r = vec2(Random::random(), Random::random());
	r.y = 2.0f * r.y - 1.0f;
	float p = sqrt(1.0f - r.y * r.y);
	return vec3(cos(r.x) * p, sin(r.x) * p, r.y);
}

float Sampling::uniformSpherePdf() {
	return glm::one_over_two_pi<float>() / 2.0f;
}

vec3 Sampling::sampleCosWeighted(vec3 dir) {
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r = vec2(Random::random(), Random::random());
	r.x = r.x * glm::two_pi<float>();
	float p = sqrt(1.0f - r.y);
	r.y = std::sqrt(r.y);
	return cos(r.x) * p * o1 + sin(r.x) * p * o2 + r.y * dir;
}

vec3 Sampling::sampleCosWeighted(vec3 dir, float& pdf) {
	dir = normalize(dir);
	vec3 o1 = normalize(ortho(dir));
	vec3 o2 = normalize(cross(dir, o1));
	vec2 r = vec2(Random::random(), Random::random());
	r.x = r.x * glm::two_pi<float>();
	float p = sqrt(1.0f - r.y);
	r.y = std::sqrt(r.y);
	pdf = r.y / glm::pi<float>();
	return cos(r.x) * p * o1 + sin(r.x) * p * o2 + r.y * dir;
}

vec3 Sampling::randomTriangle(const vec3& a, const vec3& b, const vec3& c) {
	vec2 r = vec2(Random::random(), Random::random());
	float s = 1.0f - glm::sqrt(1.0f - r.x);
	float t = (1.0f - s) * r.y;
	return a + (b - a) * s + (c - a);
}

float Sampling::uniformTrianglePdf(const vec3& a, const vec3& b, const vec3& c) {
	return glm::length(glm::cross(b - a, b - c));
}

vec3 Sampling::randomTriangle(const vec3& a, const vec3& b, const vec3& c, float& pdf) {
	vec2 r = vec2(Random::random(), Random::random());
	float s = 1.0f - glm::sqrt(1.0f - r.x);
	float t = (1.0f - s) * r.y;
	pdf = uniformTrianglePdf(a, b, c);
	return a + (b - a) * s + (c - a);
}

vec2 Sampling::uniformExponential2D() {
	vec2 r = vec2(Random::random(), Random::random());
	return vec2(glm::cos(glm::two_pi<float>() * r.y), glm::sin(glm::two_pi<float>() * r.y)) * glm::sqrt(-2.0 * glm::log(r.x));
}

vec2 Sampling::uniformExponential2D(float m, float sigma) {
	vec2 r = vec2(Random::random(), Random::random());
	return vec2(glm::cos(glm::two_pi<float>() * r.y), glm::sin(glm::two_pi<float>() * r.y)) * glm::sqrt(-2.0 * glm::log(r.x)) * sigma + m;
}