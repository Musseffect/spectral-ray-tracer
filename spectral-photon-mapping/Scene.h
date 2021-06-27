#pragma once
#include "Primitive.h"
#include "Light.h"

template<class RayTraceAccel>
class Scene {
	std::vector<spPrimitive> primitives;
	std::vector<spLight> lights;
	std::unique_ptr<RayTraceAccel> accel;
public:
	void clearPrimitives();
	void clearLights();
	void addPrimitive(const spPrimitive& primitive);
	void addLight(const spLight& light);
	const spPrimitive& primitive(int index) const;
	const spLight& light(int index) const;
	int numLights() const;
	int numObjects() const;
	Distribution1D computeSpectralLightPowerDistribution(float wavelength) const;
	Distribution1D computeLightPowerDistribution() const;
	bool testVisibility(const Ray& ray) const;
	//todo: add aabb tree or something like that
	bool intersect(Ray ray, HitInfo& hitInfo) const;
	template<class...Args>
	void buildAccelerator(Args...args);
	void print() const;
};

template<class RayTraceAccel>
using spScene = std::shared_ptr<Scene<RayTraceAccel>>;

template<class RayTraceAccel>
void Scene<RayTraceAccel>::clearPrimitives() {
	primitives.clear();
}

template<class RayTraceAccel>
void Scene<RayTraceAccel>::clearLights() {
	lights.clear();
}

template<class RayTraceAccel>
void Scene<RayTraceAccel>::addPrimitive(const spPrimitive& primitive) {
	primitives.push_back(primitive);
}

template<class RayTraceAccel>
void Scene<RayTraceAccel>::addLight(const spLight& light) {
	lights.push_back(light);
}

template<class RayTraceAccel>
const spPrimitive& Scene<RayTraceAccel>::primitive(int index) const {
	assert(index >= 0 && index < primitives.size());
	return primitives[index];
}

template<class RayTraceAccel>
const spLight& Scene<RayTraceAccel>::light(int index) const {
	assert(index >= 0 && index < lights.size());
	return lights[index];
}

template<class RayTraceAccel>
int Scene<RayTraceAccel>::numLights() const {
	return lights.size();
}

template<class RayTraceAccel>
int Scene<RayTraceAccel>::numObjects() const {
	return primitives.size();
}

template<class RayTraceAccel>
Distribution1D Scene<RayTraceAccel>::computeSpectralLightPowerDistribution(float wavelength) const {
	std::vector<float> lightPower;
	for (const auto& light : lights)
		lightPower.push_back(light->power(wavelength));
	return Distribution1D(lightPower.begin(), lightPower.end());
}

template<class RayTraceAccel>
Distribution1D Scene<RayTraceAccel>::computeLightPowerDistribution() const {
	std::vector<float> lightPower;
	for (const auto& light : lights)
		lightPower.push_back(light->power());
	return Distribution1D(lightPower.begin(), lightPower.end());
}

template<class RayTraceAccel>
bool Scene<RayTraceAccel>::testVisibility(const Ray& ray) const {
	assert(accel != nullptr);
	if (accel)
		return accel->intersect(ray);
	// fallback to brute force
	for (const auto& object : primitives) {
		if (object->intersect(ray))
			return true;
	}
	for (const auto& light : lights) {
		if (light->intersect(ray))
			return true;
	}
	return false;
}

template<class RayTraceAccel>
bool Scene<RayTraceAccel>::intersect(Ray ray, HitInfo& hitInfo) const {
	// fallback to brute force
	hitInfo.t = -1.0;
	hitInfo.primitive = nullptr;
	hitInfo.light = nullptr;
	if (accel)
		return accel->intersect(ray, hitInfo);
	for (const auto& object : primitives) {
		object->intersect(ray, hitInfo);
	}
	for (const auto& light : lights) {
		light->intersect(ray, hitInfo);
	}
	return hitInfo.t >= 0.0;
}

template<class RayTraceAccel>
template<class...Args>
void Scene<RayTraceAccel>::buildAccelerator(Args...args) {
	std::vector<Intersectable*> objects;
	objects.reserve(primitives.size() + lights.size());
	for (auto primitive : primitives)
		objects.push_back(primitive.get());
	for (auto light : lights) {
		if (!light->hasZeroArea())
			objects.push_back(light.get());
	}
	std::function<Intersectable*(typename std::vector<Intersectable*>::iterator&)> get = [](typename std::vector<Intersectable*>::iterator& it)->Intersectable* {return *it; };
	accel = std::make_unique<RayTraceAccel, Args...>(objects.begin(), objects.end(), get, args...);
}

template<class RayTraceAccel>
void Scene<RayTraceAccel>::print() const {
	accel->print();
}