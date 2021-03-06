#pragma once
#include "Ray.h"
#include "Transform.h"
#include "Sampling.h"

#define up vec3(0.0, 1.0, 0.0)
#define right vec3(1.0, 0.0, 0.0)
#define forward vec3(0.0, 0.0, -1.0)

class Camera {
protected:
	Affine cameraToWorld;
	float aspectRatio;
public:
	Camera(const Affine& cameraToWorld, float aspectRatio)
		:cameraToWorld(cameraToWorld), aspectRatio(aspectRatio)
	{}
	virtual Ray generateRay(const vec2& ndc) const = 0;
};

// todo: test
class Pinhole: public Camera {
	float focus;
	float aperture;
	float fov;
public:
	Pinhole(float fov, float focus, float aperture, const Affine& cameraToWorld, float aspectRatio)
		:Camera(cameraToWorld, aspectRatio), fov(fov), focus(focus), aperture(aperture) {}
	Ray generateRay(const vec2& ndc) const override {
		Ray ray;
		ray.ro = vec3(0.0);
		ray.rd = glm::normalize(forward + glm::tan(fov) * (right * ndc.x / aspectRatio + up * ndc.y));
	    vec3 fp = ray.ro + ray.rd * focus;
		vec2 shift = Sampling::sampleDisk(aperture);
		ray.ro = ray.ro + right * shift.x +  up * shift.y;
		ray.rd = glm::normalize(fp - ray.ro);
		return cameraToWorld.transform(ray);
	}
};

// todo: test
class OrthoProjectionCamera : public Camera {
	float focus;
	float aperture;
	float h;
public:
	OrthoProjectionCamera(float height, float focus, float aperture,
		const Affine& cameraToWorld, float aspectRatio)
		:Camera(cameraToWorld, aspectRatio), focus(focus), aperture(aperture), h(height) {}
	Ray generateRay(const vec2& ndc) const override {
		Ray ray;
		ray.ro = h * (up * ndc.y + right * ndc.x / aspectRatio);
		ray.rd = forward;
		vec3 fp = ray.ro + ray.rd * focus;
		vec2 shift = Sampling::sampleDisk(aperture);
		ray.ro = ray.ro + right * shift.x + up * shift.y;
		ray.rd = glm::normalize(fp - ray.ro);
		return cameraToWorld.transform(ray);
	}
};

// todo: test
class CustomFisheye : public Camera {
	virtual float radiusToTheta(float r) const = 0;
public:
	Ray generateRay(const vec2& ndc) const override {
		Ray ray;
		float theta = radiusToTheta(glm::length(ndc));
		float phi = glm::atan(ndc.y, ndc.x / aspectRatio);
		ray.ro = vec3(0.0);
		ray.rd = glm::normalize(forward * cos(theta) + up * sin(theta)*sin(phi) + right * sin(theta)*cos(phi));
		return cameraToWorld.transform(ray);
	}
};

// todo: test
class Fisheye : public Camera {
	enum class Type {
		Stereographic,
		Equidistant,
		Equisolid,
		Orthographic
	};
	Type type;
	float fov;
	Fisheye(const Type type, float fov):Camera(cameraToWorld, aspectRatio), type(type), fov(fov) {
		switch (type) {
			case Type::Stereographic:
				break;
			case Type::Equidistant:
				break;
			case Type::Equisolid:
				this->fov = std::min(this->fov, glm::pi<float>());
				break;
			case Type::Orthographic:
				this->fov = std::min(this->fov, glm::half_pi<float>());
				break;
		}
	}
	Ray generateRay(const vec2& ndc) const override {
		float theta = 0.0f;
		float r = glm::length(ndc);
		// todo: check correctness of this formulas
		switch (type) {
			case Type::Stereographic:
				theta = 2.0f * glm::atan(r * tan(fov / 4.0f));
				break;
			case Type::Equidistant:
				theta = r * fov / 2.0f;
				break;
			case Type::Equisolid:
				theta = 2.0f * glm::asin(r * glm::sin(fov / 4.0f));
				break;
			case Type::Orthographic:
				theta = glm::asin(r * glm::sin(fov / 2.0f));
				break;
		}
		float phi = glm::atan(ndc.y, ndc.x / aspectRatio);
		Ray ray;
		ray.ro = vec3(0.0);
		ray.rd = glm::normalize(forward * cos(theta) + up * sin(theta)*sin(phi) + right * sin(theta)*cos(phi));
		return cameraToWorld.transform(ray);
	}
};


// todo: film camera
// generate mapping from film surface with normalized coordinates
// to a ray comping out of camera objective

// todo: alternative film camera