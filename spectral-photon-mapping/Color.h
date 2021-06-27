#pragma once
#include <glm/common.hpp>
#include <functional>
#include <array>

#include "Distribution1D.h"
#include "Function1D.h"

using rgb = glm::vec3;

namespace Spectral {
	using Color = float;
}
namespace RGB {
	using Color = rgb;
}


class ColorSampler;

vec3 XYZToRGB(vec3 xyz);

// integral of product of piecewise linear curves based on regularly sampled values from two distributions
float integratProduct(const ColorSampler* const a, const ColorSampler* const b, float min, float max, int samples = 60);

template<class T>
T xFit_1931(T wave) {
	T t1 = (wave - T(442.0))*((wave < T(442.0)) ? T(0.0624) : T(0.0374));
	T t2 = (wave - T(599.8))*((wave < T(599.8)) ? T(0.0264) : T(0.0323));
	T t3 = (wave - T(501.1))*((wave < T(501.1)) ? T(0.0490) : T(0.0382));
	return T(0.362) * glm::exp(-T(0.5) * t1 * t1) +
		T(1.056) * glm::exp(-T(0.5) * t2 * t2) -
		T(0.065) * glm::exp(-T(0.5) * t3 * t3);
}

template<class T>
T yFit_1931(T wave) {
	T t1 = (wave - T(568.8))*((wave < T(568.8)) ? T(0.0213) : T(0.0247));
	T t2 = (wave - T(530.9))*((wave < T(530.9)) ? T(0.0613) : T(0.0322));
	return T(0.821) * glm::exp(-T(0.5) * t1 * t1) +
		T(0.286) * glm::exp(-T(0.5) * t2 * t2);
}

template<class T>
T zFit_1931(T wave) {
	T t1 = (wave - T(437.0))*((wave < T(437.0)) ? T(0.0845) : T(0.0278));
	T t2 = (wave - T(459.0))*((wave < T(459.0)) ? T(0.0385) : T(0.0725));
	return T(1.217) * glm::exp(-T(0.5) * t1 * t1) +
		T(0.681) * glm::exp(-T(0.5) * t2 * t2);
}

vec3 wavelengthToRGB(float wavelength, float intensity = 1.0f);
vec3 spectrumToRGB(const ColorSampler* const sampler, float min, float max, int samples = 60);

static const float D65 = 6504;

class BlackBodySPD {
protected:
	float temperature = 2856; // standart illuminant A
public:
	float sample(float wavelength) const;
	BlackBodySPD(float temperature) : temperature(temperature) {};
};

class BlackBodySPDNormalized: protected BlackBodySPD {
	float temperature = 2856; // standart illuminant A
public:
	float sample(float wavelength) const;
	BlackBodySPDNormalized(float temperature) : BlackBodySPD(temperature) {};
};

class ColorSampler {
public:
	float integral(float min, float max, int samples = 60) const;
	// wavelength in nanometers
	virtual float sample(float wavelength) const = 0;
};

class ConstantSampler : public ColorSampler {
	float value;
public:
	ConstantSampler(float value) : value(value) {}
	virtual float sample(float wavelength) const override {
		return value;
	}
};

template<typename Functor>
class AnalyticalSampler : public ColorSampler {
private:
	Functor function;
public:
	AnalyticalSampler(const Functor& function):function(function)
	{
	}
	virtual float sample(float wavelength) const override {
		return function(wavelength);
	}
};

class BlackBodyColorSampler : public ColorSampler {
	BlackBodySPDNormalized spd;
	float intensity;
public:
	BlackBodyColorSampler(float temperature, float intensity) :spd(temperature), intensity(intensity){
	}
	virtual float sample(float wavelength) const override;
};

class RGBColorSampler : public ColorSampler {
	vec3 rgb;
	// reconstruction curves
	static GridFunction1D redCurve;
	static GridFunction1D greenCurve;
	static GridFunction1D blueCurve;
public:
	RGBColorSampler(const vec3& rgb) :rgb(rgb) {}
	vec3 color() const { return rgb; }
	virtual float sample(float wavelength) const override;
};

class SpectrumSampler : public ColorSampler {
	std::shared_ptr<Function1D> function;
public:
	SpectrumSampler(const std::shared_ptr<Function1D>& function) : function(function) {}
	virtual float sample(float wavelength) const override;
};



using spColorSampler = std::shared_ptr<ColorSampler>;
using spSpectrumSampler = std::shared_ptr<SpectrumSampler>;
using spRGBColorSampler = std::shared_ptr<RGBColorSampler>;
using spBlackBodyColorSampler = std::shared_ptr<BlackBodyColorSampler>;

template<class T>
using spAnalyticalSampler = std::shared_ptr<AnalyticalSampler<T>>;