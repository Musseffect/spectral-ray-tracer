#pragma once
#include "Scene.h"
#include "Color.h"
#include "Camera.h"
#include "Image.h"
#include "Progress.h"
#include "Distribution1D.h"


namespace Debug {
	template<class RayTraceAccel>
	class Tracer {
		const float ShadowEps = 0.001f;
		const float OffsetEps = 0.001f;
		std::shared_ptr<Scene<RayTraceAccel>> scene;
		std::shared_ptr<Camera> camera;
	public:
		void setScene(const std::shared_ptr<Scene<RayTraceAccel>>& scene) {
			this->scene = scene;
		}
		void setCamera(const std::shared_ptr<Camera>& camera) {
			this->camera = camera;
		}
		// http://casedefault.com/articles/why-every-raytracer-needs-a-heatmap.html
		Image<float> renderHeatmap(int width, int height, int samples) {
			Image<float> image(width, height, rgb(0.0));
			vec2 resolution(width, height);
			float maxValue = 0.0;
			for (int k = 0; k < samples; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						image(i, j) += scene->intersectionCost(ray);
						maxValue = std::max(maxValue, image(i, j));
					}
				}
			}
			image.multiply(1.0f / maxValue);
			return image;
		}
		Image<rgb> renderObjectColor(int width, int height, int iteration) const {
			Image<rgb> image(width, height, rgb(0.0));
			std::vector<vec3> colors;
			colors.reserve(scene->numObjects() + scene->numLights());
			for (int i = 0; i < scene->numObjects(); ++i) {
				colors.push_back(vec3(Random::random(), Random::random(), Random::random()));
			}
			for (int i = 0; i < scene->numLights(); ++i) {
				colors.push_back(vec3(Random::random(), Random::random(), Random::random()));
			}
			vec2 resolution(width, height);
			for (int k = 0; k < iteration; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						HitInfo hitInfo;
						if (scene->intersect(ray, hitInfo)) {
							if (hitInfo.primitive) {
								for (int l = 0; l < scene->numObjects(); ++l) {
									if (hitInfo.primitive == scene->primitive(l).get()) {
										image(i, j) += colors[l];
										break;
									}
								}
							}
							else if (hitInfo.light) {
								for (int l = 0; l < scene->numLights(); ++l) {
									if (hitInfo.light == scene->light(l).get()) {
										image(i, j) += colors[l + scene->numObjects()];
										break;
									}
								}
							}
						}
					}
				}
			}
			image.multiply(1.0f / iteration);
			return image;
		}
		Image<rgb> renderDiffuse(int width, int height, int iterations) const {
			Image<rgb> image(width, height, rgb(0.0));
			vec2 resolution(width, height);
			for (int k = 0; k < iterations; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						HitInfo hitInfo;
						if (scene->intersect(ray, hitInfo)) {
							image(i, j) += vec3(glm::max(glm::dot(hitInfo.normal, ray.rd), 0.0f), glm::max(glm::dot(hitInfo.normal, -ray.rd), 0.0f), 0.0f);
						}
					}
				}
			}
			image.multiply(1.0f / iterations);
			return image;
		}
		Image<rgb> renderNormals(int width, int height, int iterations) const {
			Image<rgb> image(width, height, rgb(0.0));
			vec2 resolution(width, height);
			for (int k = 0; k < iterations; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						HitInfo hitInfo;
						if (scene->intersect(ray, hitInfo)) {
							image(i, j) += glm::abs(hitInfo.normal);
						}
					}
				}
			}
			image.multiply(1.0f / iterations);
			return image;
		}
		Image<rgb> renderDistance(int width, int height, int iterations) const {
			Image<rgb> image(width, height, rgb(0.0));
			vec2 resolution(width, height);
			for (int k = 0; k < iterations; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						HitInfo hitInfo;
						if (scene->intersect(ray, hitInfo)) {
							image(i, j) += vec3(1.0f / (glm::distance(ray.ro, hitInfo.globalPosition) + 1.0f));
						}
					}
				}
			}
			image.multiply(1.0f / iterations);
			return image;
		}
		Image<rgb> renderAmbientOcclusion(int width, int height, int iterations, double radius) const {
			Image<rgb> image(width, height, rgb(0.0));
			vec2 resolution(width, height);
			for (int k = 0; k < iterations; k++) {
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						Ray ray = camera->generateRay(ndc);
						HitInfo hitInfo;
						if (scene->intersect(ray, hitInfo)) {
							vec3 wi = Sampling::uniformHemisphere(hitInfo.normal);
							if (glm::dot(-ray.rd, hitInfo.normal) < 0.0f)
								wi *= -1.0f;
							if (!scene->testVisibility(Ray(hitInfo.globalPosition, wi, ShadowEps, radius))) {
								image(i, j) += vec3(1.0f);
							}
						}
					}
				}
			}
			image.multiply(1.0f / static_cast<float>(iterations));
			return image;
		}
	};
}