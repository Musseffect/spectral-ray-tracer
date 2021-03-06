#include "BxDF.h"
#include "Sampling.h"

bool sameHemisphere(const vec3& a, const vec3& b, const vec3 normal) {
	return ((glm::dot(a, normal) >= 0.0f) == (glm::dot(b, normal) >= 0.0f));
}

BxDF::BxDF(int type) :type(type) {}

int BxDF::getType() const { return type; }

bool BxDF::hasType(int type) const {
	return (this->type & type) != 0;
}

bool BxDF::hasExactType(int type) const {
	return this->type == type;
}

float BxDF::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return sameHemisphere(wo, wi, normal) > 0.0f ? glm::abs(glm::dot(wi, normal)) * glm::one_over_pi<float>() : 0.0f;
}

float BxDF::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const {
	sampledType = this->type;
	wi = Sampling::sampleCosWeighted(normal);
	if (glm::dot(wo, normal) < 0.0f)
		wi *= -1.0f;
	pdf = BxDF::pdf(wo, wi, normal);
	return f(wi, wo, normal, wavelength);
}


LambertianReflection::LambertianReflection(const spColorSampler& color)
	: BxDF(Diffuse | Reflection)
	, color(color)
{
}
float LambertianReflection::LambertianReflection::f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const {
	return color->sample(wavelength) / glm::pi<float>();
}


SpecularReflection::SpecularReflection(const spColorSampler& color)
	: BxDF(Specular | Reflection)
	, color(color)
{
}
float SpecularReflection::f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const {
	return 0.0;
}
float SpecularReflection::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const {
	sampledType = type;
	wi = glm::reflect(-wo, normal);
	if (glm::dot(wo, normal) < 0.0f)
		wi *= -1.0f;
	pdf = 1.0f;
	return color->sample(wavelength);
}
float SpecularReflection::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return 0.0f;
}


IdealGlass::IdealGlass(const spColorSampler& kr, const spColorSampler& kt, const spColorSampler& refractionIndex)
	: BxDF(Specular | Reflection | Transmission), kr(kr), kt(kt), refractionIndex(refractionIndex)
{
}
float IdealGlass::f(const vec3& wi, const vec3& wo, const vec3& normal, float wavelength) const {
	return 0.0;
}
float IdealGlass::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int& sampledType, float wavelength) const {
	float refl = kr->sample(wavelength);
	float transm = kt->sample(wavelength);
	float sum = refl + transm;
	pdf = 1.0f;
	if (Random::random() * sum < refl) {
		sampledType = Specular | Reflection;
		wi = glm::reflect(-wo, normal);
		if (glm::dot(wo, normal) < 0.0f)
			wi *= -1.0f;
	}
	else {
		sampledType = Specular | Transmission;
		float index = refractionIndex->sample(wavelength);
		float eta = index;
		vec3 n = normal;
		float cosTheta = glm::dot(wo, normal);
		if (cosTheta < 0.0f) {
			n *= -1.0f;
			eta = 1.0f / index;
		}
		if (eta * eta * (1.0f - cosTheta * cosTheta) >= 1.0) {
			sampledType = Specular | Reflection;
			wi = glm::reflect(-wo, n);
		} else
			wi = glm::refract(-wo, n, eta);
	}
	return sum != 0.0 ? 1.0f : 0.0f;
}
float IdealGlass::pdf(const vec3& wo, const vec3& wi, const vec3& normal) const {
	return 0.0f;
}


BSDF::BSDF() {
}
BSDF::BSDF(const std::vector<std::shared_ptr<BxDF>>& bxdfs) :bxdfs(bxdfs)
{
}
void BSDF::add(const std::shared_ptr<BxDF>& bxdf) {
	bxdfs.push_back(bxdf);
}
int BSDF::numComponents(int type) const {
	int result = 0;
	for (const auto& bxdf : bxdfs)
		result += bxdf->hasType(type);
	return result;
}
bool BSDF::hasType(int type) const {
	bool result = false;
	for (const auto& bxdf : bxdfs)
		result |= bxdf->hasType(type);
	return result;
}
float BSDF::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int matchTypes, int& sampledType, float wavelength) const {
	int matchingComponents = numComponents(matchTypes);
	pdf = 0.0f;
	if (matchingComponents == 0) {
		return 0.0f;
	}
	int choosen = Random::random(matchingComponents - 1);
	BxDF* bxdf = nullptr;
	for (int i = 0; i < bxdfs.size(); ++i) {
		if (bxdfs[i]->hasType(matchTypes) && choosen-- == 0) {
			bxdf = bxdfs[i].get();
			break;
		}
	}
	float f = bxdf->sampleF(wo, wi, normal, pdf, sampledType, wavelength);
	if (pdf == 0.0f)
		return 0.0f;
	if (!(bxdf->getType() & BxDF::Specular) && matchingComponents > 1)
	{
		for (int i = 0; i < bxdfs.size(); ++i)
			if (bxdfs[i].get() != bxdf && bxdfs[i]->hasType(matchTypes))
				pdf += bxdfs[i]->pdf(wo, wi, normal);
	}
	pdf /= matchingComponents;
	if (!(bxdf->getType() & BxDF::Specular) && matchingComponents > 1) {
		bool reflect = glm::dot(wi, normal) * glm::dot(wo, normal) > 0;
		for (int i = 0; i < bxdfs.size(); ++i) {
			bool isReflection = reflect && bxdfs[i]->hasType(BxDF::Reflection);
			bool isTransmission = !reflect && bxdfs[i]->hasType(BxDF::Transmission);
			if (bxdfs[i]->hasType(matchTypes) && (isReflection || isTransmission) && bxdfs[i].get() != bxdf)
				f += bxdfs[i]->f(wo, wi, normal, wavelength);
		}
	}
	return f;
}
float BSDF::f(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes, float wavelength) const {
	bool reflect = glm::dot(wi, normal) * glm::dot(wo, normal) > 0;
	float result = 0.0f;
	for (auto& bxdf : bxdfs)
	{
		if (bxdf->hasType(matchTypes) &&
			((reflect && (bxdf->hasType(BxDF::Type::Reflection))) ||
			(!reflect && (bxdf->hasType(BxDF::Type::Transmission)))))
			result += bxdf->f(wo, wi, normal, wavelength);
	}
	return result;
}
float BSDF::pdf(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const {
	float pdf = 0.0f;
	int matchingComponents = 0;
	for (int i = 0; i < bxdfs.size(); ++i)
		if (bxdfs[i]->hasType(matchTypes)) {
			matchingComponents++;
			pdf += bxdfs[i]->pdf(wo, wi, normal);
		}
	if (matchingComponents)
		return pdf / matchingComponents;
	return 0.0f;
}