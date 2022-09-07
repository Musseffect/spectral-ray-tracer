#pragma once
#include "common.h"

using rgb = glm::vec3;
using vec3 = glm::vec3;
class ColorSampler;

float blackBodyRadiance(float wavelength, float temperature);

// integral of product of piecewise linear curves based on regularly sampled values from two distributions
float computeProductIntegral(const ColorSampler* const a, const ColorSampler* const b, float min, float max, int samples = 60);

vec3 wavelengthToRGB(float wavelength, float intensity = 1.0f);
vec3 spectrumToRGB(const ColorSampler* const sampler, float min, float max, int samples = 60);

vec3 CIEXYZ_to_CIERGB(vec3 xyz);
vec3 CIEXYZ1931_to_LinearSRGB(vec3 xyz);


class BlackBodySPD {
protected:
	float temperature = 2856; // standart illuminant A
public:
	float sample(float wavelength) const;
	BlackBodySPD(float temperature) : temperature(temperature) {};
};

class BlackBodySPDNormalized : protected BlackBodySPD {
	float temperature = 2856; // standart illuminant A
public:
	float sample(float wavelength) const;
	BlackBodySPDNormalized(float temperature) : BlackBodySPD(temperature) {};
};