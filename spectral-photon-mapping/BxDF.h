#pragma once
#include "Color.h"
#include "Sampling.h"

#include <stack>


template<class Color>
class BxDF {
public:
	enum Type : int {
		Diffuse = 1 << 0,
		Reflection = 1 << 1,
		Glossy = 1 << 2,
		Specular = 1 << 3,
		Transmission = 1 << 4,
		All = Diffuse | Reflection | Glossy | Specular | Transmission
	};
protected:
	int type;
public:
	int getType() const;
	BxDF(int type);
	bool hasType(int type) const;
	bool hasExactType(int type) const;
	virtual Color f(const vec3& wi, const vec3& wo, const vec3& normal) const = 0;
	virtual Color sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward = true) const;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const;
};
template<class Color>
class LambertianReflection : public BxDF<Color> {
	Color R;
public:
	LambertianReflection(const Color& R);
	virtual Color f(const vec3& wi, const vec3& wo, const vec3& normal) const override;
};

template<class Color>
class SpecularReflection : public BxDF<Color> {
	Color R;
public:
	SpecularReflection(const Color& R);
	virtual Color f(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	virtual Color sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward = true) const override;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const override;
};

template<class Color>
class SpecularTransmission : public BxDF<Color> {
	Color T;
	float refractionIndex;
public:
	SpecularTransmission(const Color& T);
	virtual Color f(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	virtual Color sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward = true) const override;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const override;
};

template<class Color>
class IdealGlass : public BxDF<Color> {
	Color R;
	Color T;
	float refractionIndex;
public:
	IdealGlass(const Color& R, const Color& T, float refractionIndex);
	virtual Color f(const vec3& wi, const vec3& wo, const vec3& normal) const override;
	virtual Color sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward = true) const override;
	virtual float pdf(const vec3& wo, const vec3& wi, const vec3& normal) const override;
};
template<class Color>
using spBxDF = std::shared_ptr<BxDF<Color>>;

bool sameHemisphere(const vec3& a, const vec3& b, const vec3 normal);

template<class Color>
BxDF<Color>::BxDF(int type) :type(type) {}
template<class Color>
int BxDF<Color>::getType() const { return type; }
template<class Color>
bool BxDF<Color>::hasType(int type) const {
	return (this->type & type) != 0;
}
template<class Color>
bool BxDF<Color>::hasExactType(int type) const {
	return this->type == type;
}
template<class Color>
float BxDF<Color>::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return sameHemisphere(wo, wi, normal)? glm::abs(glm::dot(wi, normal)) * glm::one_over_pi<float>() : 0.0f;
}
template<class Color>
Color BxDF<Color>::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward) const {
	sampledType = this->type;
	wi = Sampling::cosWeightedHemisphere(normal);
	if (glm::dot(wo, normal) < 0.0f)
		wi *= -1.0f;
	pdf = BxDF<Color>::pdf(wo, wi, normal);
	return f(wi, wo, normal);
}
template<class Color>
LambertianReflection<Color>::LambertianReflection(const Color& R)
	: BxDF<Color>(BxDF<Color>::Diffuse | BxDF<Color>::Reflection)
	, R(R)
{
}
template<class Color>
Color LambertianReflection<Color>::LambertianReflection::f(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return R / glm::pi<float>();
}

template<class Color>
SpecularReflection<Color>::SpecularReflection(const Color& R)
	: BxDF<Color>(BxDF<Color>::Specular | BxDF<Color>::Reflection)
	, R(R)
{
}
template<class Color>
Color SpecularReflection<Color>::f(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return Color(0.0);
}
template<class Color>
Color SpecularReflection<Color>::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward) const {
	sampledType = this->type;
	vec3 tNormal = normal;
	if (glm::dot(wo, normal) < 0.0f)
		tNormal *= -1.0f;
	wi = glm::reflect(-wo, normal);
	pdf = 1.0f;
	return R / glm::abs(glm::dot(wi, tNormal));
}
template<class Color>
float SpecularReflection<Color>::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return 0.0f;
}


template<class Color>
SpecularTransmission<Color>::SpecularTransmission(const Color& T)
	: BxDF<Color>(BxDF<Color>::Specular | BxDF<Color>::Transmission)
	, T(T)
{}
template<class Color>
Color SpecularTransmission<Color>::f(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return Color(0.0f);
}
template<class Color>
Color SpecularTransmission<Color>::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward) const {
	float cosTheta = glm::dot(wo, normal);
	sampledType = BxDF<Color>::Specular | BxDF<Color>::Reflection;
	pdf = 1.0;
	vec3 tempNormal = normal;
	float eta = 1.0f / refractionIndex;
	if (cosTheta < 0.0f) {
		tempNormal *= -1.0f;
		eta = refractionIndex;
	}
	wi = glm::refract(-wo, tempNormal, eta);
	Color result = T * (1.0f - f) / glm::abs(glm::dot(wi, normal));
	if (!isBackward)
		result *= eta * eta;
	return result;
}

template<class Color>
float SpecularTransmission<Color>::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return 0.0f;
}


template<class Color>
IdealGlass<Color>::IdealGlass(const Color& R, const Color& T, float refractionIndex)
	: BxDF<Color>(BxDF<Color>::Specular | BxDF<Color>::Reflection | BxDF<Color>::Transmission), R(R), T(T), refractionIndex(refractionIndex)
{
}
template<class Color>
Color IdealGlass<Color>::f(const vec3& wi, const vec3& wo, const vec3& normal) const {
	return Color(0.0);
}
template<class Color>
Color IdealGlass<Color>::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, bool isBackward) const {
	float cosTheta = glm::dot(wo, normal);
	vec3 tempNormal = normal;
	float eta = 1.0f / refractionIndex;
	if (cosTheta < 0.0f) {
		tempNormal *= -1.0f;
		eta = refractionIndex;
	}
	//fresnel
	float f0 = glm::pow2((eta - 1.0f) / (eta + 1.0f));
	// schlick approximation?
	float f = f0 + (1.0f - f0) * glm::pow(1.0f - std::abs(cosTheta), 5.0f);
	if (eta * eta * (1.0f - cosTheta * cosTheta) >= 1.0f) {
		f = 1.0f;
	}
	if (Random::random() < f) {
		sampledType = BxDF<Color>::Specular | BxDF<Color>::Reflection;
		pdf = f;
		wi = glm::reflect(-wo, tempNormal);
		return R * f / glm::abs(glm::dot(wi, tempNormal));
	}
	else {
		sampledType = BxDF<Color>::Specular | BxDF<Color>::Transmission;
		pdf = 1.0f - f;
		wi = glm::refract(-wo, tempNormal, eta);
		Color result = T * (1.0f - f) / glm::abs(glm::dot(wi, tempNormal));
		if (!isBackward)
			result *= eta * eta;
		/*else
			intens /= eta * eta;*/
		return result;
	}
}
template<class Color>
float IdealGlass<Color>::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return 0.0f;
}


namespace Spectral {
	// todo: cook-torrance
	// lambertian transmission
	// ideal glass
	// lambertian glass?
	// add Beer–Lambert law
	using BxDF = ::BxDF<Color>;
	using spBxDF = std::shared_ptr<BxDF>;
	using LambertianReflection = ::LambertianReflection<Color>;
	using SpecularReflection = ::SpecularReflection<Color>;
	using IdealGlass = ::IdealGlass<Color>;
}

namespace RGB {
	using BxDF = ::BxDF<Color>;
	using spBxDF = std::shared_ptr<BxDF>;
	using LambertianReflection = ::LambertianReflection<Color>;
	using SpecularReflection = ::SpecularReflection<Color>;
	using IdealGlass = ::IdealGlass<Color>;
}