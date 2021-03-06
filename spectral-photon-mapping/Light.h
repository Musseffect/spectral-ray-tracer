#pragma once
#include "Transform.h"
#include "Ray.h"
#include "Color.h"
#include "Disk.h"
#include "Rect.h"
#include <memory>

class Light;

using spLight = std::shared_ptr<Light>;


class Light{
protected:
	Affine lightToWorld;
public:
	virtual void preprocess() {};
	Light(const Affine& lightToWorld)
		:lightToWorld(lightToWorld) {}
	virtual float sampleLe(Ray& ray, vec3& lightNormal, float& pdfPos, float& pdfDir, float wavelength) const = 0;
	virtual float sampleLi(const HitInfo& hitInfo, float wavelength, vec3& worldPosition, float& pdf) const = 0;
	virtual BBox3D bbox() const = 0;
	virtual float power(float wavelength) const = 0;
	virtual float power() const = 0;
	virtual bool intersect(const Ray& ray) const = 0;
	virtual bool isDelta() const = 0;
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const = 0;
	// for infinite area lights
	virtual float lightEmitted(const Ray& ray, float wavelength) const {
		return 0.0f;
	}
	// for area lights
	virtual float lightEmitted(const vec3& wo, const vec3& normal, float wavelength) const {
		return 0.0f;
	}
	virtual float pdfLi(const HitInfo& hi, const vec3& wi, const Scene* scene) const = 0;
};

// DONE
class PointLight : public Light {
	spColorSampler intensity;
	float totalPower;
	vec3 worldCenter;
public:
	PointLight(const Affine& transform, const spColorSampler& intensity)
		:Light(transform), intensity(intensity), worldCenter(transform.transformPoint(vec3(0.0)))
	{
		totalPower = intensity->integral(400, 700, 60);
	}
	virtual bool isDelta() const override {
		return true;
	}
	virtual float power(float wavelength) const override {
		return 4.0f * glm::pi<float>() * intensity->sample(wavelength);
	}
	virtual float power() const override {
		return 4.0f * glm::pi<float>() * totalPower;
	}
	virtual float sampleLe(Ray& ray, vec3& lightNormal, float& pdfPos, float& pdfDir, float wavelength) const override {
		vec3 dir = Sampling::sampleSphere();
		ray.ro = worldCenter;
		ray.rd = dir;
		lightNormal = dir;
		pdfPos = 1.0f;
		pdfDir = Sampling::uniformSpherePdf();
		return intensity->sample(wavelength);
	}
	virtual float sampleLi(const HitInfo& hitInfo, float wavelength, vec3& worldPosition, float& pdf) const override{
		float dist = glm::l2Norm(hitInfo.globalPosition - worldCenter);
		worldPosition = worldCenter;
		pdf = 1.0f;
		return intensity->sample(wavelength) / (dist * dist);
	}
	virtual bool intersect(const Ray& ray) const override {
		return false;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		return false;
	}
	virtual float pdfLi(const HitInfo& hi, const vec3& wi, const Scene* scene) const override {
		return 0.0f;
	}
};

//default direction
class DirectionalLight : public Light {
	spColorSampler intensity;
	float totalPower;
protected:
	virtual vec2 samplePosition() const = 0;
	virtual bool isInBounds(vec2 localPosition) const = 0;
	virtual float surfaceArea() const = 0;
public:
	DirectionalLight(const Affine& transform,
		const spColorSampler& intensity)
		:Light(transform), intensity(intensity) {
		totalPower = intensity->integral(400, 700, 60);
	}
	virtual bool isDelta() const override {
		return true;
	}
	virtual float power(float wavelength) const override {
		return intensity->sample(wavelength) * surfaceArea();
	}
	virtual float power() const override {
		return totalPower * surfaceArea();
	}
	virtual float sampleLe(Ray& ray, vec3& lightNormal, float& pdfPos, float& pdfDir, float wavelength) const override {
		vec2 sample = samplePosition();
		ray.ro = lightToWorld.transformPoint(vec3(sample.x, 0.0f, sample.y));
		ray.rd = glm::normalize(lightToWorld.transformNormal(vec3(0.f, 1.f, 0.f)));
		lightNormal = ray.rd;
		pdfPos = 1.0f / surfaceArea();
		pdfDir = 1.0f;
		return intensity->sample(wavelength);
	}
	virtual float sampleLi(const HitInfo& hitInfo, float wavelength, vec3& worldPosition, float& pdf) const override {
		vec3 localPosition = lightToWorld.transformInversePoint(hitInfo.globalPosition);
		if (localPosition.y < 0.0f)
			return 0.0f;
		localPosition.y = 0.0f;
		if (!isInBounds(vec2(localPosition.x, localPosition.z)) )
			return 0.0f;
		worldPosition = lightToWorld.transformPoint(localPosition);
		pdf = 1.0f;
		return intensity->sample(wavelength);
	}
	virtual bool intersect(const Ray& ray) const override {
		const Ray rayLocal = lightToWorld.transformInverse(ray);
		float t = -rayLocal.ro.y / rayLocal.rd.y;
		if (t < ray.tMin || t > ray.tMax)
			return false;
		vec2 p = vec2(rayLocal.ro.x, rayLocal.ro.z) + vec2(rayLocal.rd.x, rayLocal.rd.z) * t;
		if (!isInBounds(p))
			return false;
		return true;
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		const Ray rayLocal = lightToWorld.transformInverse(ray);
		float t = -rayLocal.ro.y / rayLocal.rd.y;
		if (t < ray.tMin || t > ray.tMax)
			return false;
		vec2 p = vec2(rayLocal.ro.x, rayLocal.ro.z) + vec2(rayLocal.rd.x, rayLocal.rd.z) * t;
		if (!isInBounds(p))
			return false;
		hitInfo.t = t;
		hitInfo.localPosition = vec3(p.x, 0.0, p.y);
		hitInfo.normal = lightToWorld.transformNormal(vec3(0.f, 1.0f, 0.f));
		hitInfo.globalPosition = lightToWorld.transformPoint(hitInfo.localPosition);
		return true;
	}
	virtual float pdfLi(const HitInfo& hi, const vec3& wi, const Scene* scene) const override {
		return 0.0f;
	}
};


class RectDirectionalLight : public DirectionalLight {
	vec2 size;
protected:
	virtual vec2 samplePosition() const override {
		return vec2(Random::random() - 0.5f, Random::random() - 0.5f) * size;
	}
	virtual bool isInBounds(vec2 localPosition) const override {
		return std::abs(localPosition.x) <= 0.5f * size.x && std::abs(localPosition.y) <= 0.5f * size.y;
	}
	virtual float surfaceArea() const override {
		return size.x * size.y;
	}
public:
	RectDirectionalLight(const Affine& transform,
		const spColorSampler& intensity,
		const vec2& size)
		:DirectionalLight(transform, intensity), size(size)
	{
	}
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-size.x, 0.0f, -size.y) * 0.5f, vec3(size.x, 0.0f, size.y) * 0.5f);
	}
};

class DiscDirectionalLight : public DirectionalLight {
	float radius;
protected:
	virtual vec2 samplePosition() const override {
		return Sampling::sampleDisk(radius);
	}
	virtual bool isInBounds(vec2 localPosition) const override {
		return glm::dot(localPosition, localPosition) < radius * radius;
	}
	virtual float surfaceArea() const override {
		return glm::pi<float>() * radius * radius;
	}
public:
	DiscDirectionalLight(const Affine& transform,
		const spColorSampler& intensity,
		float radius)
		:DirectionalLight(transform, intensity), radius(radius)
	{
	}
	virtual BBox3D bbox() const override {
		return BBox3D(vec3(-radius, 0.0f, -radius), vec3(radius, 0.0f, radius));
	}
};

class DiffuseAreaLight : public Light {
	spColorSampler intensity;
	std::shared_ptr<Shape> shape;
	const float area;
	float totalPower;
public:
	// Make sure lightToWorld has uniform scaling
	DiffuseAreaLight(const Affine& lightToWorld,
		const spColorSampler& intensity,
		const std::shared_ptr<Shape>& shape)
		:Light(lightToWorld)
		, shape(shape)
		, intensity(intensity)
		, area(shape->area())
	{
		totalPower = intensity->integral(400.0f, 700.0f, 60);
	}
	virtual bool isDelta() const override {
		return false;
	}
	virtual float power(float wavelength) const override {
		return intensity->sample(wavelength) * area * 2.0f * glm::pi<float>();
	}
	virtual float power() const override {
		return totalPower * area * 2.0f * glm::pi<float>();
	}
	virtual float sampleLe(Ray& ray, vec3& lightNormal, float& pdfPos, float& pdfDir, float wavelength) const override {
		HitInfo hitInfo = shape->sample();
		lightNormal = lightToWorld.transformNormal(hitInfo.normal);
		ray.ro = lightToWorld.transformPoint(hitInfo.localPosition);
		ray.rd = Sampling::sampleHemisphere(lightNormal);
		pdfPos = shape->pdf();
		pdfDir = Sampling::uniformHemispherePdf();
		return intensity->sample(wavelength);
	}
	virtual bool intersect(const Ray& ray) const override {
		const Ray rayLocal = lightToWorld.transformInverse(ray);
		return shape->intersect(rayLocal);
	}
	virtual bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
		const Ray rayLocal = lightToWorld.transformInverse(ray);
		bool result = shape->intersect(rayLocal, hitInfo);
		if (!result)
			return false;
		hitInfo.globalPosition = lightToWorld.transformPoint(hitInfo.localPosition); //local to world
		hitInfo.normal = lightToWorld.transformNormal(hitInfo.normal);
		return true;
	}
	virtual float lightEmitted(const vec3& wo, const vec3& normal, float wavelength) const {
		return glm::dot(wo, normal) > 0.0f ? intensity->sample(wavelength): 0.0f;
	}
	virtual float sampleLi(const HitInfo& hitInfo, float wavelength, vec3& worldPosition, float& pdf) const override {
		// sample in localCoords
		HitInfo shapeSample = shape->sample(lightToWorld.transformInversePoint(hitInfo.globalPosition), lightToWorld, pdf);
		worldPosition = lightToWorld.transformPoint(shapeSample.localPosition);
		shapeSample.normal = lightToWorld.transformNormal(shapeSample.normal);
		if (glm::dot(hitInfo.globalPosition - worldPosition, shapeSample.normal) < 0.0f)
			return 0.0f;
		return intensity->sample(wavelength);
	}
	virtual BBox3D bbox() const override {
		return lightToWorld.transform(shape->bbox());
	}
	virtual float pdfLi(const HitInfo& hit, const vec3& wi, const Scene* scene) const override {
		return shape->pdf(hit.globalPosition, wi, lightToWorld, scene);
	}
};