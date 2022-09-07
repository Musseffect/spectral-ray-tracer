#pragma once
#include "BxDF.h"

template<class Color>
class BSDF {
	std::vector<spBxDF<Color>> bxdfs;
public:
	BSDF();
	BSDF(const std::vector<spBxDF<Color>>& bxdfs);
	void add(const spBxDF<Color>& bxdf);
	int numComponents(int type) const;
	bool hasType(int type) const;
	Color sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int matchTypes, int& sampledType, bool isBackward = true) const;
	Color f(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const;
	float pdf(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const;
};

namespace Spectral {
	using BSDF = ::BSDF<Color>;
	using spBSDF = std::shared_ptr<BSDF>;
	const auto makeBSDF = std::make_shared<BSDF>;
}

namespace Tristimulus {
	using BSDF = ::BSDF<Color>;
	using spBSDF = std::shared_ptr<BSDF>;
	const auto makeBSDF = std::make_shared<BSDF>;
}


template<class Color>
BSDF<Color>::BSDF() {
}
template<class Color>
BSDF<Color>::BSDF(const std::vector<spBxDF<Color>>& bxdfs) :bxdfs(bxdfs)
{
}
template<class Color>
void BSDF<Color>::add(const spBxDF<Color>& bxdf) {
	bxdfs.push_back(bxdf);
}
template<class Color>
int BSDF<Color>::numComponents(int type) const {
	int result = 0;
	for (const auto& bxdf : bxdfs)
		result += bxdf->hasType(type);
	return result;
}
template<class Color>
bool BSDF<Color>::hasType(int type) const {
	bool result = false;
	for (const auto& bxdf : bxdfs)
		result |= bxdf->hasType(type);
	return result;
}
template<class Color>
Color BSDF<Color>::sampleF(const vec3& wo, vec3& wi, const vec3& normal, float& pdf, int matchTypes, int& sampledType, bool isBackward) const {
	int matchingComponents = numComponents(matchTypes);
	pdf = 0.0f;
	if (matchingComponents == 0) {
		return 0.0f;
	}
	int choosen = Random::random(matchingComponents - 1);
	BxDF<Color>* bxdf = nullptr;
	for (int i = 0; i < bxdfs.size(); ++i) {
		if (bxdfs[i]->hasType(matchTypes) && choosen-- == 0) {
			bxdf = bxdfs[i].get();
			break;
		}
	}
	Color f = bxdf->sampleF(wo, wi, normal, pdf, sampledType, isBackward);
	if (pdf == 0.0f)
		return 0.0f;
	if (!(bxdf->getType() & BxDF<Color>::Specular) && matchingComponents > 1)
	{
		for (int i = 0; i < bxdfs.size(); ++i)
			if (bxdfs[i].get() != bxdf && bxdfs[i]->hasType(matchTypes))
				pdf += bxdfs[i]->pdf(wo, wi, normal);
	}
	pdf /= matchingComponents;
	if (!(bxdf->getType() & BxDF<Color>::Specular) && matchingComponents > 1) {
		bool reflect = glm::dot(wi, normal) * glm::dot(wo, normal) > 0;
		for (int i = 0; i < bxdfs.size(); ++i) {
			bool isReflection = reflect && bxdfs[i]->hasType(BxDF<Color>::Reflection);
			bool isTransmission = !reflect && bxdfs[i]->hasType(BxDF<Color>::Transmission);
			if (bxdfs[i]->hasType(matchTypes) && (isReflection || isTransmission) && bxdfs[i].get() != bxdf)
				f += bxdfs[i]->f(wo, wi, normal);
		}
	}
	return f;
}
template<class Color>
Color BSDF<Color>::f(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const {
	bool reflect = glm::dot(wi, normal) * glm::dot(wo, normal) > 0;
	Color result = Color(0.0f);
	for (auto& bxdf : bxdfs)
	{
		if (bxdf->hasType(matchTypes) &&
			((reflect && (bxdf->hasType(BxDF<Color>::Reflection))) ||
			(!reflect && (bxdf->hasType(BxDF<Color>::Transmission)))))
			result += bxdf->f(wo, wi, normal);
	}
	return result;
}
template<class Color>
float BSDF<Color>::pdf(const vec3& wo, const vec3& wi, const vec3& normal, int matchTypes) const {
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