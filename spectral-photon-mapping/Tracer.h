#pragma once
#include "Scene.h"
#include "Color.h"
#include "Camera.h"
#include "Image.h"
#include "Progress.h"
#include "Distribution1D.h"

namespace photon_mapping {
	const float ShadowEps = 0.001f;
	const float OffsetEps = 0.001f;
	float powerHeuristic(int nf, float fPdf, int ng, float gPdf) {
		float f = nf * fPdf, g = ng * gPdf;
		return (f * f) / (f * f + g * g);
	}
	struct Settings {
		// todo: Not implemented
		int threads = 8;
		// todo: Not implemented
		int tileSize = 16;
		int photonsPerIteration = 10000;
		int iterations = 200;
		int maxDepth = 5;
		float initialRadius = 1.0f;
	};

	struct PixelInfo {
		float radius;
		vec3 directLight = vec3(0.0f);
		vec3 indirectLight = vec3(0.0f);
		float n = 0.0f;
		int m = 0;
	};

	class Tracer {
		std::shared_ptr<Scene> scene;
		std::shared_ptr<Camera> camera;
		Settings settings;
	private:
		float sampleLight(int index, const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const {
			const spLight light = scene->light(index);
			//sample light
			float lightPdf;
			vec3 lightPosition;
			float li = light->sampleLi(hitInfo, wavelength, lightPosition, lightPdf);
			float ld = 0.0f;
			if (li > 0.0f && lightPdf > 0.0f) {
				vec3 wi = lightPosition - hitInfo.globalPosition;
				float distance = glm::max(glm::length(wi) - ShadowEps, OffsetEps);
				wi = glm::normalize(wi);
				float f = bsdf->f(wo, wi, hitInfo.normal, BxDF::All, wavelength) * std::abs(glm::dot(wi, hitInfo.normal));
				float scatteringPdf = bsdf->pdf(wo, wi, hitInfo.normal, BxDF::All);
				if (f > 0.0f && scatteringPdf > 0.0f) {
					// if sampled point is visible compute bsdf f()
					if (scene->testVisibility(Ray(hitInfo.globalPosition, wi, OffsetEps, distance))) {
						li = 0.0f;
					} else {
						if (light->isDelta())
							ld += f * li / lightPdf;
						else {
							float weight = powerHeuristic(1, lightPdf, 1, scatteringPdf);
							ld += f * li * weight / lightPdf;
						}
					}
				}
			}
			// sample bsdf
			if (!light->isDelta()) {
				vec3 wi;
				float scatteringPdf;
				int sampledType;
				float f = bsdf->sampleF(wo, wi, hitInfo.normal, scatteringPdf, BxDF::All, sampledType, wavelength);
				f *= glm::max(glm::dot(wi, hitInfo.normal), 0.0f);
				if (f > 0.0f && scatteringPdf > 0.0f) {
					float weight = 1.0f;
					if ((sampledType & BxDF::Specular) == 0) {
						lightPdf = light->pdfLi(hitInfo, wi, scene.get());
						if (lightPdf == 0.0f)
							return ld;
						weight = powerHeuristic(1, scatteringPdf, 1, lightPdf);
					}
					HitInfo lightHitInfo;
					Ray ray(hitInfo.globalPosition, wi);
					float li = 0.0f;
					if (scene->intersect(ray, lightHitInfo)) {
						if (lightHitInfo.light == light.get()) {
							li = lightHitInfo.lightEmitted(-wi, wavelength);
						}
					} else // area ambient light
						li = light->lightEmitted(ray, wavelength);
					if (li > 0.0f)
						ld += f * li * weight / scatteringPdf;
				}
			}
			return ld;
		}
		float sampleOneLight(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const {
			if (scene->numLights() == 0)
				return 0.0f;
			// sample light
			int lightIndex = Random::random(scene->numLights() - 1);
			return sampleLight(lightIndex, wo, hitInfo, bsdf, wavelength) * scene->numLights();
		}
		/*
		float sampleAllLights(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const {
			if (scene->numLights() == 0)
				return 0.0f;
			float ld = 0.0f
			for (int i = 0; i < scene->numLights(); ++i) {
				ld += sampleLight(i, wo, hitInfo, bsdf, wavelength);
			}
			return ld;
		}*/
		std::vector<SpectralPhoton> emitPhotons(float wavelength) {
			Distribution1D lightPowerDistribution = scene->computeSpectralLightPowerDistribution(wavelength);
			std::vector<SpectralPhoton> photons;
			for (int i = 0; i < settings.photonsPerIteration; i++) {
				// choose light
				float lightPdf = 0.0f;
				int lightIndex = lightPowerDistribution.sampleDiscrete(Random::random(), lightPdf);
				float pdfPos;
				float pdfDir;
				Ray ray;
				vec3 lightNormal;
				float le = scene->light(lightIndex)->sampleLe(ray, lightNormal, pdfPos, pdfDir, wavelength);
				if (le == 0.0f || pdfPos == 0.0f || pdfDir == 0.0f)
					continue;
				float intensity = glm::abs(glm::dot(lightNormal, ray.rd)) * le / (lightPdf * pdfPos * pdfDir);
				if (intensity == 0.0f)
					continue;
				for (int depth = 0; depth < settings.maxDepth; depth++)
				{
					HitInfo hitInfo;
					if (!scene->intersect(ray, hitInfo))
						break;
					if (!hitInfo.sceneObject) {
						break;
					}
					std::shared_ptr<BSDF> bsdf = hitInfo.sceneObject->getMaterial()->bsdf(hitInfo);
					bool isDiffuse = bsdf->hasType(BxDF::Diffuse);
					bool isSpecular = bsdf->hasType(BxDF::Specular);
					bool isGlossy = bsdf->hasType(BxDF::Glossy);
					if (isDiffuse || isGlossy && depth > 0) {
						// leave photon if diffuse
						photons.push_back(SpectralPhoton{ wavelength, hitInfo.globalPosition, -ray.rd, hitInfo.normal, intensity, hitInfo.sceneObject });
					}
					float pdf;
					vec3 wo;
					int sampledType;
					float lightOut = bsdf->sampleF(-ray.rd, wo, hitInfo.normal, pdf, BxDF::All, sampledType, wavelength);
					if (lightOut == 0.0f || pdf == 0.0f)
						break;
					float newIntensity = intensity * lightOut * glm::abs(glm::dot(wo, hitInfo.normal)) / pdf;
					float q = glm::max(0.0f, 1.0f - newIntensity / intensity);
					if (Random::random() < q)
						break;
					intensity = newIntensity / (1.0f - q);
					ray.ro = hitInfo.globalPosition;
					ray.rd = wo;
					if (intensity == 0.0f)
						break;
				}
			}
#ifdef DEBUG
			/*if (photons.empty())
				fprintf(stderr, "Couldn't find any photon intersection\n");*/
#endif
			return photons;
		}
		void gather(const vec2& ndc, float wavelength, const KdTree<SpectralPhoton>& pointLocator, PixelInfo& pixel) {
			float luminocity = 1.0f;
			Ray ray = camera->generateRay(ndc);
			pixel.m = 0;
			bool specularBounce = false;
			float phi = 0.0f;
			for (int depth = 0; depth < settings.maxDepth; depth++)
			{
				HitInfo hitInfo;
				if (!scene->intersect(ray, hitInfo))
				{
					for (int i = 0; i < scene->numLights(); ++i)
						pixel.directLight += wavelengthToRGB(wavelength, luminocity * scene->light(i)->lightEmitted(ray, wavelength));
					break;
				}
				vec3 wo = -ray.rd;
				if (depth == 0 || specularBounce)
					pixel.directLight += wavelengthToRGB(wavelength, luminocity * hitInfo.lightEmitted(wo, wavelength));
				if (!hitInfo.sceneObject) {
					break;
				}
				std::shared_ptr<BSDF> bsdf = hitInfo.sceneObject->getMaterial()->bsdf(hitInfo);
				pixel.directLight += wavelengthToRGB(wavelength, luminocity * sampleOneLight(wo, hitInfo, bsdf, wavelength));

				bool isDiffuse = bsdf->hasType(BxDF::Diffuse);
				bool isGlossy = bsdf->hasType(BxDF::Glossy);
				if (isDiffuse || (isGlossy && depth == settings.maxDepth - 1)) {
					// accumulate indirect
					std::vector<int> indices;
					pointLocator.indicesWithinRadius(hitInfo.globalPosition, pixel.radius, indices);
					for (const auto& index : indices)
					{
						const auto& particle = pointLocator.pointAt(index);
						if (glm::dot(particle.normal, hitInfo.normal) < 0.0 || hitInfo.sceneObject != particle.object)
							continue;
						phi += particle.power * bsdf->f(wo, particle.wi, particle.normal, BxDF::Type::All, wavelength);
						pixel.m++;
					}
					break;
				}
				// bounce ray
				if (depth < settings.maxDepth - 1) {
					float pdf;
					vec3 wi;
					int type;
					float f = bsdf->sampleF(wo, wi, hitInfo.normal, pdf, BxDF::All, type, wavelength);
					if (f == 0.0f || pdf == 0.0f)
						break;
					specularBounce = (type & BxDF::Specular);
					luminocity *= f * glm::max(0.0f, glm::dot(wi, hitInfo.normal)) / pdf;
					if (luminocity == 0.0f)
						break;
					ray = Ray(hitInfo.globalPosition, wi);
				}
			}
			if (pixel.m == 0)
				return;
			const float Gamma = 2.0f / 3.0f;
			float newN = pixel.n + Gamma * pixel.m;
			float newRadius = pixel.radius * std::sqrt(newN / glm::max(pixel.n + pixel.m, 1.0f));
			pixel.indirectLight = (pixel.indirectLight + wavelengthToRGB(wavelength, luminocity * phi)) * (newRadius * newRadius) / (pixel.radius * pixel.radius);
			pixel.radius = newRadius;
			pixel.n = newN;
		}
	public:
		void setScene(const std::shared_ptr<Scene>& scene) {
			this->scene = scene;
		}
		void setSettings(const Settings& settings) {
			this->settings = settings;
		}
		void setCamera(const std::shared_ptr<Camera>& camera) {
			this->camera = camera;
		}
#if 0
		Image<rgb> renderForward(int width, int height) {
			vec2 resolution(width, height);
			struct VisibilityPoint {
				vec3 position;
				glm::ivec2 screenCoords;
			};
			for (int k = 0; k < settings.iterations; k++) {
				float wavelength = Random::random(380.f, 720.f);

				Distribution1D lightPowerDistribution = scene->computeLightPowerDistribution(wavelength);
				std::vector<VisibilityPoint> visibilityPoints(width * height);
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 ndc = (2.0f * vec2(i, j) - resolution + vec2(1.0f)) / resolution;
						ndc = ndc + Sampling::sampleDisk(0.5f);
						// create visibility points


					}
				}
				KdTree<VisibilityPoint> kdTree(visibilityPoints.begin(), visibilityPoints.end());
				for (int i = 0; i < settings.photonsPerIteration; ++i) {
					float lightPdf = 0.0f;
					int lightIndex = lightPowerDistribution.sampleDiscrete(Random::random(), lightPdf);
					SpectralRay ray = scene->light(lightIndex)->emitPhoton();;
				}
				// trace photons and change visibility points data
			}
			throw std::runtime_error("Not implemented");
		}
#endif
		Image<rgb> renderBackward(int width, int height, const std::unique_ptr<Progress>& progress) {
			Image<rgb> image(width, height, rgb(0.0));
			std::unique_ptr<PixelInfo[]> pixelInfos(new PixelInfo[width * height]);
			for (int i = 0; i < width*height; i++)
				pixelInfos[i].radius = settings.initialRadius;

			vec2 resolution(width, height);
			for (int k = 0; k < settings.iterations; k++) {
				//float wavelength = Random::random(400.f, 700.f);
				// stratified sampling
				float wavelength = glm::mix(400.f, 700.0f, Sampling::sampleStratified(60));
				auto photons = emitPhotons(wavelength);
				KdTree<SpectralPhoton> pointLocator(photons.begin(), photons.end(), 1, 8);
				for (int j = 0; j < height; j++)
				{
					for (int i = 0; i < width; i++)
					{
						vec2 aaShift = Sampling::sampleDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						gather(ndc, wavelength, pointLocator, pixelInfos[i + j * width]);
					}
				}
				progress->emitProgress(k / float(settings.iterations));
			}
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					float N = settings.iterations * std::max(settings.photonsPerIteration, 1);
					const PixelInfo& pixelInfo = pixelInfos[i + j * width];
					image(i, j) = pixelInfo.directLight / settings.iterations +
						pixelInfo.indirectLight / (N * pixelInfo.radius * pixelInfo.radius * glm::pi<float>());
				}
			}
			progress->emitProgress(1.0f);
			return image;
		}
	};
};