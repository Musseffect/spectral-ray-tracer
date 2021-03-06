#pragma once
#include "SceneObject.h"
#include "Light.h"

class Scene {
	std::vector<spSceneObject> objects;
	std::vector<spLight> lights;
public:
	void clearObjects();
	void clearLights();
	void addObject(const spSceneObject& object);
	void addLight(const spLight& light);
	const spSceneObject& object(int index) const;
	const spLight& light(int index) const;
	int numLights() const;
	int numObjects() const;
	Distribution1D computeSpectralLightPowerDistribution(float wavelength) const;
	Distribution1D computeLightPowerDistribution() const;
	bool testVisibility(const Ray& ray) const;
	//todo: add aabb tree or something like that
	bool intersect(const Ray& ray, HitInfo& hitInfo) const;
};
