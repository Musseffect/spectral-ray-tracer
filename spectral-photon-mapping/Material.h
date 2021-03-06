#pragma once
#include "Color.h"
#include "BxDF.h"
#include <vector>



template<class T>
class Texture {
public:
	virtual const T sample(const HitInfo& hitInfo) const = 0;
	virtual ~Texture() {}
};

template <class T>
using spTexture = std::shared_ptr<Texture<T>>;

template<class T>
class ConstantTexture : public Texture<T> {
	T value;
public:
	ConstantTexture(const T& value) :value(value)
	{
	}
	virtual const T sample(const HitInfo& hitInfo) const override {
		return value;
	}
};

template <class T>
using spConstTexture = std::shared_ptr<ConstantTexture<T>>;

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

class Material {

public:
	virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const = 0;
};


class DiffuseMaterial: public Material{
	std::shared_ptr<Texture<spColorSampler>> kd;
public:
	DiffuseMaterial(const std::shared_ptr<Texture<spColorSampler>>& kd) :kd(kd)
	{
	}
	virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const override {
		std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(
			std::vector<std::shared_ptr<BxDF>>({ std::make_shared<LambertianReflection>(kd->sample(hitInfo)) })
		);
		return bsdf;
	}
};


class SpecularMirrorMaterial: public Material {
	spTexture<spColorSampler> kr;
public:
	SpecularMirrorMaterial(const spTexture<spColorSampler>& kr) :kr(kr)
	{
	}
	std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const {
		std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(
			std::vector<std::shared_ptr<BxDF>>({ std::make_shared<SpecularReflection>(kr->sample(hitInfo)) })
		);
		return bsdf;
	}
};

// todo:
class DiffuseGlossyMaterial {
public:
};


class IdealGlassMaterial: public Material {
	spTexture<spColorSampler> kr;
	spTexture<spColorSampler> kt;
	spTexture<spColorSampler> refraction;
public:
	IdealGlassMaterial(const spTexture<spColorSampler>& kr, spTexture<spColorSampler>& kt, spTexture<spColorSampler>& refraction)
	:kr(kr), kt(kt), refraction(refraction)
	{
	}
	virtual std::shared_ptr<BSDF> bsdf(const HitInfo& hitInfo) const override {
		std::shared_ptr<BSDF> bsdf = std::make_shared<BSDF>(
			std::vector<std::shared_ptr<BxDF>>({ std::make_shared<IdealGlass>(kr->sample(hitInfo), kt->sample(hitInfo), refraction->sample(hitInfo)) })
			);
		return bsdf;
	}
};