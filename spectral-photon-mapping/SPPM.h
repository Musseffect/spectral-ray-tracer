#pragma once
#include "Scene.h"
#include "Color.h"
#include "Camera.h"
#include "Image.h"
#include "Progress.h"
#include "Distribution1D.h"

namespace Spectral {
	namespace SPPM {
		vec3 toGrid(const vec3& p, const AABB& bounds, const glm::ivec3& gridRes);
		vec3 toGridClamped(const vec3& p, const AABB& bounds, const glm::ivec3& gridRes);

		struct PixelInfo {
			float radius;
			vec3 directLight = vec3(0.0f);
			vec3 indirectLight = vec3(0.0f);
			float phi = 0.0f;
			float n = 0.0f;
			int m = 0;
		};
		struct SpectralPhoton {
			vec3 center;
			vec3 wi;
			vec3 normal;
			float power;
			const Primitive* primitive;
			vec3 position() const {
				return center;
			}
		};
		struct VisibilityPoint {
			vec3 center;
			PixelInfo* pi;
			vec3 wo;
			float luminocity;
			vec3 normal;
			Spectral::spBSDF bsdf;
			const Primitive* primitive;
			vec3 position() const {
				return center;
			}
			bool intersect(const vec3& point) const {
				return glm::distance2(point, center) <= pi->radius * pi->radius;
			}
			bool intersect(const AABB& aabb) const {
				return aabb.outerSqrDistance(center) <= pi->radius;
			}
			bool intersect(const Ray& ray) const {
				throw std::runtime_error("");
			}
			bool intersect(Ray& ray, HitInfo& hitInfo) const {
				throw std::runtime_error("");
			}
			AABB bbox() const {
				return AABB(center - vec3(pi->radius), center + vec3(pi->radius));
			}
		};
		const float ShadowEps = 0.001f;
		const float OffsetEps = 0.001f;
		float powerHeuristic(int nf, float fPdf, int ng, float gPdf);
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
		template<class RayTracerAccel>
		class Tracer {
			std::shared_ptr<Scene<RayTracerAccel>> scene;
			std::shared_ptr<Camera> camera;
			Settings settings;
		private:
			float sampleLight(int index, const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const;
			float sampleOneLight(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const;
			float sampleAllLights(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<BSDF>& bsdf, float wavelength) const;
			std::vector<SpectralPhoton> emitPhotons(float wavelength);
			template<class PointLocator>
			void gather(const vec2& ndc, float wavelength, const PointLocator& pointLocator, PixelInfo& pixel);
		public:
			void setScene(const spScene<RayTracerAccel>& scene);
			void setSettings(const Settings& settings);
			void setCamera(const std::shared_ptr<Camera>& camera);
			std::vector<VisibilityPoint> getVisibilityPoints(int width, int height, float wavelength, PixelInfo* pixelInfos);
			template<class SearchAccel, class...Params>
			Image<rgb> renderForward(int width, int height, const std::unique_ptr<Progress>& progress, Params...params);
			template<class PointLocator>
			Image<rgb> renderBackward(int width, int height, const std::unique_ptr<Progress>& progress);
		};

		template<class RayTraceAccel>
		void Tracer<RayTraceAccel>::setScene(const spScene<RayTraceAccel>& scene) {
			this->scene = scene;
		}
		template<class RayTraceAccel>
		void Tracer<RayTraceAccel>::setSettings(const Settings& settings) {
			this->settings = settings;
		}
		template<class RayTraceAccel>
		void Tracer<RayTraceAccel>::setCamera(const std::shared_ptr<Camera>& camera) {
			this->camera = camera;
		}
		template<class RayTraceAccel>
		std::vector<VisibilityPoint> Tracer<RayTraceAccel>::getVisibilityPoints(int width, int height, float wavelength, PixelInfo* pixelInfos) {
			vec2 resolution(width, height);
			std::vector<VisibilityPoint> visibilityPoints;
			visibilityPoints.reserve(width * height);
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					vec2 aaShift = Sampling::uniformDisk(1.0f);
					vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
					float luminocity = 1.0f;
					Ray ray = camera->generateRay(ndc);
					bool specularBounce = false;
					float phi = 0.0f;
					PixelInfo& pi = pixelInfos[i + j * width];
					for (int depth = 0; depth < settings.maxDepth; depth++)
					{
						HitInfo hitInfo;
						if (!scene->intersect(ray, hitInfo)) {
							for (int i = 0; i < scene->numLights(); ++i)
								pi.directLight += wavelengthToRGB(wavelength, luminocity * scene->light(i)->lightEmitted(ray, wavelength));
							break;
						}
						vec3 wo = -ray.rd;
						if (depth == 0 || specularBounce)
							pi.directLight += wavelengthToRGB(wavelength, luminocity * hitInfo.lightEmitted(wo, wavelength));
						if (!hitInfo.primitive) {
							break;
						}
						std::shared_ptr<Spectral::BSDF> bsdf = hitInfo.primitive->getMaterial()->bsdf(hitInfo, wavelength);
						pi.directLight += wavelengthToRGB(wavelength, luminocity * sampleOneLight(wo, hitInfo, bsdf, wavelength));

						bool isDiffuse = bsdf->hasType(BxDF::Diffuse);
						bool isGlossy = bsdf->hasType(BxDF::Glossy);
						if (isDiffuse || (isGlossy && depth == settings.maxDepth - 1)) {
							// accumulate indirect
							visibilityPoints.push_back(VisibilityPoint{
								hitInfo.globalPosition,
								&pi,
								wo,
								luminocity,
								hitInfo.normal,
								bsdf,
								hitInfo.primitive });
							break;
						}
						// bounce ray
						if (depth < settings.maxDepth - 1) {
							float pdf;
							vec3 wi;
							int type;
							float f = bsdf->sampleF(wo, wi, hitInfo.normal, pdf, BxDF::All, type);
							if (f == 0.0f || pdf == 0.0f)
								break;
							specularBounce = (type & BxDF::Specular);
							luminocity *= f * glm::abs(glm::dot(wi, hitInfo.normal)) / pdf;
							if (luminocity == 0.0f)
								break;
							ray = Ray(hitInfo.globalPosition, wi);
						}
					}
				}
			}
			return visibilityPoints;
		}
		template<class RayTraceAccel>
		template<class SearchAccel, class...Params>
		Image<rgb> Tracer<RayTraceAccel>::renderForward(int width, int height, const std::unique_ptr<Progress>& progress, Params...params) {
			std::unique_ptr<PixelInfo[]> pixelInfos(new PixelInfo[width * height]);
			for (int i = 0; i < width * height; i++)
				pixelInfos[i].radius = settings.initialRadius;

			float maxRadius = settings.initialRadius;
			for (int k = 0; k < settings.iterations; k++) {
				float wavelength = Random::random(Config::get().spectrumMin(), Config::get().spectrumMax());

				std::vector<VisibilityPoint> visibilityPoints = getVisibilityPoints(width, height, wavelength, pixelInfos.get());
//#define GRID
#ifndef GRID
				SearchAccel searchAccel(visibilityPoints.begin(), visibilityPoints.end(), params...);
#else
				struct Node {
					int next;
					VisibilityPoint* vp;
					Node(int next, VisibilityPoint* vp) : next(next), vp(vp) {}
				};
				glm::ivec3 size;
				AABB gridBounds;
				float maxRadius = 0.0f;
				for (const auto& vp : visibilityPoints) {
					maxRadius = std::max(maxRadius, vp.pi->radius);
					gridBounds.append(vp.bbox());
				}
				vec3 diag = gridBounds.max() - gridBounds.min();
				float maxDiag = glm::max(diag.x, glm::max(diag.y, diag.z));
				int res = (int)(maxDiag / maxRadius);
				glm::ivec3 gridRes = glm::max(glm::ivec3(1), glm::ivec3(res * diag / maxDiag));
				gridRes = glm::min(gridRes, glm::ivec3(10000));
				std::vector<int> grid(gridRes.x * gridRes.y * gridRes.z, -1);
				auto toIndex = [&gridRes](int i, int j, int k) {
					return i + gridRes.x * (j + gridRes.y * k);
				};
				std::vector<Node> nodes;
				nodes.reserve(gridRes.x * gridRes.y * gridRes.z);
				for (auto& vp : visibilityPoints) {
					AABB bbox = vp.bbox();
					glm::ivec3 min = toGridClamped(bbox.min(), gridBounds, gridRes);
					glm::ivec3 max = toGridClamped(bbox.max(), gridBounds, gridRes);
					for (int z = min.z; z <= max.z; ++z) {
						for (int y = min.y; y <= max.y; ++y) {
							for (int x = min.x; x <= max.x; ++x) {
								int index = toIndex(x, y, z);
								nodes.push_back(Node(grid[index], &vp));
								grid[index] = nodes.size() - 1;
							}
						}
					}
				}
#endif
				Distribution1D lightPowerDistribution = scene->computeSpectralLightPowerDistribution(wavelength);
				for (int i = 0; i < settings.photonsPerIteration; ++i) {
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
						if (!hitInfo.primitive) {
							break;
						}
						if (depth > 0) {
#ifndef GRID
							std::vector<int> indices = searchAccel.intersectedIndicies(hitInfo.globalPosition);
							for (auto index : indices) {
								const VisibilityPoint& vp = visibilityPoints[index];
								if (glm::length2(vp.center - hitInfo.globalPosition) > vp.pi->radius * vp.pi->radius || glm::dot(hitInfo.normal, vp.normal) < 0.0
									|| hitInfo.primitive != vp.primitive) {
									continue;
								}
								vp.pi->phi += vp.luminocity * intensity * vp.bsdf->f(vp.wo, -ray.rd, hitInfo.normal, BxDF::Type::All);
								vp.pi->m++;
							}
#else
							glm::ivec3 index = toGrid(hitInfo.globalPosition, gridBounds, gridRes);
							int node = -1;
							if (index.x < 0 || index.y < 0 || index.z < 0 || index.x >= gridRes.x || index.y >= gridRes.y || index.z >= gridRes.z)
								node = -1;
							else
								node = grid[toIndex(index.x, index.y, index.z)];
							while (node != -1) {
								auto& nodeRef = nodes[node];
								VisibilityPoint& vp = *(nodeRef.vp);
								if (glm::length2(vp.center - hitInfo.globalPosition) > vp.pi->radius * vp.pi->radius || glm::dot(hitInfo.normal, vp.normal) < 0.0
									|| hitInfo.primitive != vp.primitive) {
									node = nodeRef.next;
									continue;
								}
								vp.pi->phi += vp.luminocity * intensity * vp.bsdf->f(vp.wo, -ray.rd, hitInfo.normal, BxDF::Type::All);
								vp.pi->m++;
								node = nodeRef.next;
							}
#endif
						}
						float pdf;
						vec3 wo;
						int sampledType;
						std::shared_ptr<Spectral::BSDF> bsdf = hitInfo.primitive->getMaterial()->bsdf(hitInfo, wavelength);
						float lightOut = bsdf->sampleF(-ray.rd, wo, hitInfo.normal, pdf, BxDF::All, sampledType, false);
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
				maxRadius = 0.0f;
				// change visibility points data
				for (int i = 0; i < width * height; ++i) {
					PixelInfo& pi = pixelInfos[i];
					if (pi.m != 0) {
						const float Gamma = 2.0f / 3.0f;
						float newN = pi.n + Gamma * pi.m;
						float newRadius = pi.radius * std::sqrt(newN / glm::max(pi.n + pi.m, 1.0f));
						pi.indirectLight = (pi.indirectLight + wavelengthToRGB(wavelength, pi.phi)) * (newRadius * newRadius) / (pi.radius * pi.radius);
						pi.radius = newRadius;
						pi.n = newN;
					}
					pi.m = 0;
					pi.phi = 0.0f;
					maxRadius = std::max(maxRadius, pi.radius);
				}
				progress->emitProgress(float(k) / settings.iterations);
			}
			Image<rgb> image(width, height, rgb(0.0));
			for (int j = 0; j < height; j++)
			{
				for (int i = 0; i < width; i++)
				{
					float N = std::max(settings.iterations * settings.photonsPerIteration, 1);
					const PixelInfo& pixelInfo = pixelInfos[i + j * width];
					image(i, j) = pixelInfo.directLight / settings.iterations +
						pixelInfo.indirectLight / (N * pixelInfo.radius * pixelInfo.radius * glm::pi<float>());
				}
			}
			progress->emitProgress(1.0f);
			return image;
		}
		template<class RayTraceAccel>
		template<class PointLocator>
		Image<rgb> Tracer<RayTraceAccel>::renderBackward(int width, int height, const std::unique_ptr<Progress>& progress) {
			Image<rgb> image(width, height, rgb(0.0));
			std::unique_ptr<PixelInfo[]> pixelInfos(new PixelInfo[width * height]);
			for (int i = 0; i < width*height; i++)
				pixelInfos[i].radius = settings.initialRadius;

			vec2 resolution(width, height);
			for (int k = 0; k < settings.iterations; k++) {
				//float wavelength = Random::random(400.f, 700.f);
				// stratified sampling
				float wavelength = glm::mix(Config::get().spectrumMin(), Config::get().spectrumMax(), Sampling::uniformStratified(60));
				std::vector<SpectralPhoton> photons = emitPhotons(wavelength);
				PointLocator pointLocator(photons.begin(), photons.end(), 1, 8);
				for (int j = 0; j < height; j++) {
					for (int i = 0; i < width; i++) {
						vec2 aaShift = Sampling::uniformDisk(1.0f);
						vec2 ndc = (2.0f * vec2(i, j) + aaShift - resolution + vec2(1.0f)) / resolution;
						gather<PointLocator>(ndc, wavelength, pointLocator, pixelInfos[i + j * width]);
					}
				}
				progress->emitProgress(k / float(settings.iterations));
			}
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					float N = std::max(settings.iterations * settings.photonsPerIteration, 1);
					const PixelInfo& pixelInfo = pixelInfos[i + j * width];
					image(i, j) = pixelInfo.directLight / settings.iterations +
						pixelInfo.indirectLight / (N * pixelInfo.radius * pixelInfo.radius * glm::pi<float>());
				}
			}
			progress->emitProgress(1.0f);
			return image;
		}
		template<class RayTraceAccel>
		float Tracer<RayTraceAccel>::sampleLight(int index, const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<Spectral::BSDF>& bsdf, float wavelength) const {
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
				float f = bsdf->f(wo, wi, hitInfo.normal, Spectral::BxDF::All) * std::abs(glm::dot(wi, hitInfo.normal));
				float scatteringPdf = bsdf->pdf(wo, wi, hitInfo.normal, BxDF::All);
				if (f > 0.0f && scatteringPdf > 0.0f) {
					// if sampled point is visible compute bsdf f()
					if (scene->testVisibility(Ray(hitInfo.globalPosition, wi, OffsetEps, distance))) {
						li = 0.0f;
					}
					else {
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
				float f = bsdf->sampleF(wo, wi, hitInfo.normal, scatteringPdf, BxDF::All, sampledType);
				f *= glm::abs(glm::dot(wi, hitInfo.normal));
				if (f > 0.0f && scatteringPdf > 0.0f) {
					float weight = 1.0f;
					if ((sampledType & BxDF::Specular) == 0) {
						lightPdf = light->pdfLi(hitInfo, wi);
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
					}
					else // area ambient light
						li = light->lightEmitted(ray, wavelength);
					if (li > 0.0f)
						ld += f * li * weight / scatteringPdf;
				}
			}
			return ld;
		}
		template<class RayTraceAccel>
		float Tracer<RayTraceAccel>::sampleOneLight(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<Spectral::BSDF>& bsdf, float wavelength) const {
			if (scene->numLights() == 0)
				return 0.0f;
			// sample light
			int lightIndex = Random::random(scene->numLights() - 1);
			return sampleLight(lightIndex, wo, hitInfo, bsdf, wavelength) * scene->numLights();
		}
		template<class RayTraceAccel>
		float Tracer<RayTraceAccel>::sampleAllLights(const vec3& wo, const HitInfo& hitInfo, const std::shared_ptr<Spectral::BSDF>& bsdf, float wavelength) const {
			if (scene->numLights() == 0)
				return 0.0f;
			float ld = 0.0f;
			for (int i = 0; i < scene->numLights(); ++i) {
				ld += sampleLight(i, wo, hitInfo, bsdf, wavelength);
			}
			return ld;
		}
		template<class RayTraceAccel>
		std::vector<SpectralPhoton> Tracer<RayTraceAccel>::emitPhotons(float wavelength) {
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
					if (!hitInfo.primitive) {
						break;
					}
					std::shared_ptr<Spectral::BSDF> bsdf = hitInfo.primitive->getMaterial()->bsdf(hitInfo, wavelength);
					bool isDiffuse = bsdf->hasType(Spectral::BxDF::Diffuse);
					bool isSpecular = bsdf->hasType(Spectral::BxDF::Specular);
					bool isGlossy = bsdf->hasType(Spectral::BxDF::Glossy);
					if ((isDiffuse || isGlossy) && depth > 0) {
						// leave photon if diffuse
						photons.push_back(SpectralPhoton{ hitInfo.globalPosition, -ray.rd, hitInfo.normal, intensity, hitInfo.primitive });
					}
					float pdf;
					vec3 wo;
					int sampledType;
					float lightOut = bsdf->sampleF(-ray.rd, wo, hitInfo.normal, pdf, Spectral::BxDF::All, sampledType, false);
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
			return std::move(photons);
		}
		template<class RayTraceAccel>
		template<class PointLocator>
		void Tracer<RayTraceAccel>::gather(const vec2& ndc, float wavelength, const PointLocator& pointLocator, PixelInfo& pixel) {
			float luminocity = 1.0f;
			Ray ray = camera->generateRay(ndc);
			pixel.m = 0;
			bool specularBounce = false;
			float phi = 0.0f;
			for (int depth = 0; depth < settings.maxDepth; depth++) {
				HitInfo hitInfo;
				if (!scene->intersect(ray, hitInfo)) {
					for (int i = 0; i < scene->numLights(); ++i)
						pixel.directLight += wavelengthToRGB(wavelength, luminocity * scene->light(i)->lightEmitted(ray, wavelength));
					break;
				}
				vec3 wo = -ray.rd;
				if (depth == 0 || specularBounce)
					pixel.directLight += wavelengthToRGB(wavelength, luminocity * hitInfo.lightEmitted(wo, wavelength));
				if (!hitInfo.primitive) {
					break;
				}
				std::shared_ptr<Spectral::BSDF> bsdf = hitInfo.primitive->getMaterial()->bsdf(hitInfo, wavelength);
				pixel.directLight += wavelengthToRGB(wavelength, luminocity * sampleOneLight(wo, hitInfo, bsdf, wavelength));

				bool isDiffuse = bsdf->hasType(BxDF::Diffuse);
				bool isGlossy = bsdf->hasType(BxDF::Glossy);
				if (isDiffuse || (isGlossy && depth == settings.maxDepth - 1)) {
					// accumulate indirect
					std::vector<int> indices = pointLocator.indicesWithinRadius(hitInfo.globalPosition, pixel.radius);
					for (const auto& index : indices) {
						const auto& particle = pointLocator.pointAt(index);
						if (glm::dot(particle.normal, hitInfo.normal) < 0.0 || hitInfo.primitive != particle.primitive)
							continue;
						phi += luminocity * particle.power * bsdf->f(wo, particle.wi, particle.normal, BxDF::Type::All);
						pixel.m++;
					}
					break;
				}
				// bounce ray
				if (depth < settings.maxDepth - 1) {
					float pdf;
					vec3 wi;
					int type;
					float f = bsdf->sampleF(wo, wi, hitInfo.normal, pdf, BxDF::All, type);
					if (f == 0.0f || pdf == 0.0f)
						break;
					specularBounce = (type & BxDF::Specular);
					luminocity *= f * glm::abs(glm::dot(wi, hitInfo.normal)) / pdf;
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
			pixel.indirectLight = (pixel.indirectLight + wavelengthToRGB(wavelength, phi)) * (newRadius * newRadius) / (pixel.radius * pixel.radius);
			pixel.radius = newRadius;
			pixel.n = newN;
		}
	}

}