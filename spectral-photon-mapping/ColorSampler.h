#pragma once
#include <functional>
#include <array>

#include "ColorUtils.h"
#include "Distribution1D.h"
#include "Function1D.h"

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
	AnalyticalSampler(const Functor& function) :function(function)
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
	BlackBodyColorSampler(float temperature, float intensity) :spd(temperature), intensity(intensity) {
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
	spFunction1D function;
public:
	SpectrumSampler(const spFunction1D& function) : function(function) {}
	virtual float sample(float wavelength) const override;
};


ALIAS_SMARTPOINTERS(ColorSampler);
ALIAS_SMARTPOINTERS(SpectrumSampler);
ALIAS_SMARTPOINTERS(RGBColorSampler);
ALIAS_SMARTPOINTERS(ConstantSampler);
ALIAS_SMARTPOINTERS(BlackBodyColorSampler);

/*using spSpectrumSampler = std::shared_ptr<ColorSampler>;
using spSpectrumSampler = std::shared_ptr<SpectrumSampler>;
using spRGBColorSampler = std::shared_ptr<RGBColorSampler>;
using spBlackBodyColorSampler = std::shared_ptr<BlackBodyColorSampler>;*/

template<class T>
using spAnalyticalSampler = std::shared_ptr<AnalyticalSampler<T>>;


spSpectrumSampler makeSpectrumSampler(const spFunction1D& value);