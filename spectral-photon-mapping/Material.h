#pragma once
#include <vector>

#include "BSDF.h"
#include "HitInfo.h"
#include "Texture.h"


class TextureMapping2D {
public:
	virtual vec2 get(const HitInfo& hitInfo) const = 0;
};

class UVTextureMapping2D: public TextureMapping2D {
public:
	virtual vec2 get(const HitInfo& hitInfo) const override {
		return hitInfo.uv;
	}
};

namespace Spectral {
	class Material {
	public:
		virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo, float wavelength) const = 0;
	};
	class DiffuseMaterial : public Material {
		spTex<spColorSampler> R;
	public:
		DiffuseMaterial(const spTex<spColorSampler>& R) :R(R)
		{
		}
		virtual spBSDF bsdf(const HitInfo& hitInfo, float wavelength) const override {
			spBSDF bsdf = std::make_shared<BSDF>(
				std::vector<spBxDF>({ std::make_shared<LambertianReflection>(R->sample(hitInfo)->sample(wavelength)) })
				);
			return bsdf;
		}
	};


	class SpecularMirrorMaterial : public Material {
		spTex<spColorSampler> R;
	public:
		SpecularMirrorMaterial(const spTex<spColorSampler>& R) :R(R)
		{
		}
		virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo, float wavelength) const override {
			std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(
				std::vector<spBxDF>({ std::make_shared<SpecularReflection>(R->sample(hitInfo)->sample(wavelength)) })
				);
			return bsdf;
		}
	};

	// todo:
	class DiffuseGlossyMaterial {
	public:
	};


	class IdealGlassMaterial : public Material {
		spTex<spColorSampler> R;
		spTex<spColorSampler> T;
		spTex<spColorSampler> refraction;
	public:
		IdealGlassMaterial(const spTex<spColorSampler>& R, const spTex<spColorSampler>& T, const spTex<spColorSampler>& refraction)
			:R(R), T(T), refraction(refraction)
		{
		}
		virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo, float wavelength) const override {
			spBSDF bsdf = std::make_shared<BSDF>(
				std::vector<spBxDF>({ std::make_shared<IdealGlass>(R->sample(hitInfo)->sample(wavelength),
					T->sample(hitInfo)->sample(wavelength),
					refraction->sample(hitInfo)->sample(wavelength)) })
				);
			return bsdf;
		}
	};

	using spMaterial = std::shared_ptr<Material>;
	using spIdealGlassMat = std::shared_ptr<IdealGlassMaterial>;
	using spDiffuseMat = std::shared_ptr<DiffuseMaterial>;
	constexpr auto makeDiffuseMat = std::make_shared<DiffuseMaterial, const spTex<spColorSampler>&>;
	template<class... Args>
	constexpr auto makeGlassMat = std::make_shared<IdealGlassMaterial, Args>;
}


namespace Tristimulus {
	class Material {
	public:
		virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const = 0;
	};
	class DiffuseMaterial : public Material {
		spTex<Color> R;
	public:
		DiffuseMaterial(const std::shared_ptr<Texture<Color>>& R) :R(R)
		{
		}
		virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const override {
			std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(
				std::vector<spBxDF>({ std::make_shared<LambertianReflection>(R->sample(hitInfo)) })
				);
			return bsdf;
		}
	};
	class IdealGlassMaterial : public Material {
		spTex<Color> R;
		spTex<Color> T;
		spTex<float> refraction;
	public:
		IdealGlassMaterial(const spTex<Color>& R, const spTex<Color>& T, const spTex<float>& refraction)
			:R(R), T(T), refraction(refraction)
		{
		}
		virtual spBSDF bsdf(const HitInfo& hitInfo) const override {
			spBSDF bsdf = std::make_shared<BSDF>(
				std::vector<spBxDF>({ std::make_shared<IdealGlass>(R->sample(hitInfo), T->sample(hitInfo),
					refraction->sample(hitInfo)) })
				);
			return bsdf;
		}
	};

	using spMaterial = std::shared_ptr<Material>;
	using spIdealGlassMat = std::shared_ptr<IdealGlassMaterial>;
	using spDiffuseMat = std::shared_ptr<DiffuseMaterial>;

	constexpr auto makeDiffuseMat = std::make_shared<DiffuseMaterial, const std::shared_ptr<Texture<Color>>&>;
	template<class... Args>
	constexpr auto makeGlassMat = std::make_shared<IdealGlassMaterial, Args>;
}