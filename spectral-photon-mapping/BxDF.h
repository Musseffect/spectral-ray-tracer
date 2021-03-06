#pragma once
#include "Color.h"

class BxDF {
public:
	enum Type {
		Diffuse = 1,
		Reflection = 2,
		Glossy = 4,
		Specular = 8,
		Transmission = 16,
		All = Diffuse | Reflection | Glossy | Specular | Transmission
	};
protected:
	int type;
public:
	int getType() const;
	BxDF(int type);
	bool hasType(int type) const;
	bool hasExactType(int type) const;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const;
	virtual float f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const = 0;
	virtual float sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const;
};


class LambertianReflection : public BxDF {
	spColorSampler color;
public:
	LambertianReflection(const spColorSampler& color);
	virtual float f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const override;
};

class SpecularReflection : public BxDF {
	spColorSampler color;
public:
	SpecularReflection(const spColorSampler& color);
	virtual float f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const override;
	virtual float sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const override;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const override;
};

class IdealGlass : public BxDF {
	spColorSampler kr;
	spColorSampler kt;
	spColorSampler refractionIndex;
public:
	IdealGlass(const spColorSampler& kr, const spColorSampler& kt, const spColorSampler& refractionIndex);
	virtual float f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const override;
	virtual float sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const override;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const override;

};

// todo: cook-torrance
// specular transmission
// fresnel
// lambertian transmission


class BSDF {
	std::vector<std::shared_ptr<BxDF>> bxdfs;
public:
	BSDF();
	BSDF(const std::vector<std::shared_ptr<BxDF>>& bxdfs);
	void add(const std::shared_ptr<BxDF>& bxdf);
	int numComponents(int type) const;
	bool hasType(int type) const;
	float sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int matchTypes, int& sampledType, float wavelength) const;
	float f(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes, float wavelength) const;
	float pdf(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const;
};