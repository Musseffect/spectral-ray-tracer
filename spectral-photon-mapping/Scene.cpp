#include "Scene.h"

void Scene::clearObjects() {
	objects.clear();
}

void Scene::clearLights() {
	lights.clear();
}

void Scene::addObject(const spSceneObject& object) {
	objects.push_back(object);
}

void Scene::addLight(const spLight& light) {
	lights.push_back(light);
}

const spSceneObject& Scene::object(int index) const {
	assert(index >= 0 && index < objects.size());
	return objects[index];
}

const spLight& Scene::light(int index) const {
	assert(index >= 0 && index < lights.size());
	return lights[index];
}

int Scene::numLights() const {
	return lights.size();
}

int Scene::numObjects() const {
	return objects.size();
}

Distribution1D Scene::computeSpectralLightPowerDistribution(float wavelength) const {
	std::vector<float> lightPower;
	for (const auto& light : lights)
		lightPower.push_back(light->power(wavelength));
	return Distribution1D(lightPower.begin(), lightPower.end());
}

Distribution1D Scene::computeLightPowerDistribution() const {
	std::vector<float> lightPower;
	for (const auto& light : lights)
		lightPower.push_back(light->power());
	return Distribution1D(lightPower.begin(), lightPower.end());
}

bool Scene::testVisibility(const Ray& ray) const {
	for (const auto& object : objects) {
		if (object->intersect(ray)) {
			return true;
		}
	}
	for (const auto& light : lights) {
		if (light->intersect(ray)) {
			return true;
		}
	}
	return false;
}

bool Scene::intersect(const Ray& ray, HitInfo& hitInfo) const {
	Ray tRay = ray;
	HitInfo tHitInfo;
	tHitInfo.t = -1.0;
	hitInfo.sceneObject = nullptr;
	hitInfo.light = nullptr;
	for (const auto& object : objects) {
		if (object->intersect(tRay, tHitInfo)) {
			hitInfo= tHitInfo;
			hitInfo.sceneObject = object.get();
			hitInfo.light = nullptr;
			tRay.tMax = tHitInfo.t;
		}
	}
	for (const auto& light : lights) {
		if (light->intersect(tRay, tHitInfo)) {
			hitInfo = tHitInfo;
			hitInfo.sceneObject = nullptr;
			hitInfo.light = light.get();
			tRay.tMax = tHitInfo.t;
			break;
		}
	}
	return hitInfo.t >= 0.0;
}