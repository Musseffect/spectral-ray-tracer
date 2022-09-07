#include "ColorUtils.h"
#include "Illuminants.h"
#include "ColorSpaces.h"

using namespace glm;

float blackBodyRadiance(float wavelength, float temperature) {
	const float c = 299792458.0f;
	const float h = 6.62606957e-34f;
	const float kb = 1.3806488e-23f;
	float l = wavelength * 1e-9f;
	float l5 = l * l * l * l * l;
	return (2.0f * h * c * c) / (l5 * (std::exp(h * c / (l * kb * temperature)) - 1.0f));
}

float computeProductIntegral(const ColorSampler* const a, const ColorSampler* const b, float min, float max, int samples) {
	assert(samples != 1);
	float value = 0.0;
	float aCur = a->sample(min);
	float bCur = b->sample(min);
	float dx = (max - min) / static_cast<float>(samples);
	for (int i = 1; i < samples; ++i) {
		float x = (max - min) * i / static_cast<float>(samples) + min;
		float aNext = a->sample(x);
		float bNext = b->sample(x);
		float da = aNext - aCur;
		float db = bNext - bCur;
		value += aCur * bCur + (aCur * db + bCur * da) * 0.5f + da * db / 3.0f;
		aCur = aNext;
		bCur = bNext;
	}
	return value * dx;
}


/// returns linear RGB values in CIERGB
vec3 CIEXYZ_to_CIERGB(vec3 xyz) {
	const mat3 M(2.3706743f, -0.9000405f, -0.4706338f,
		-0.5138850f, 1.4253036f, 0.0885814f,
		0.0052982, -0.0146949f, 1.0093968f);
	return glm::transpose(M) * xyz;
}

// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

/// returns linear RGB values in sRGB color space
vec3 CIEXYZ1931_to_LinearSRGB(vec3 xyz) {

	/*const mat3 M(3.2406255f, -1.537208f, -0.4986286f,
		-0.9689307f, 1.8757571, 0.0415175f,
		0.0557101, -0.2040211, 1.0569959f);
	const mat3 adaptation(0.9531874, -0.0382467, 0.0026068,
		-0.0265906, 1.0288406, -0.0030332,
		0.0238731, 0.0094060, 1.0892565);
	xyz = glm::transpose(adaptation) * xyz;
	return glm::transpose(M) * xyz;*/


	// Adaptation matrix from E to D65 white point
	namespace I = Illuminants;
	static const mat3 adaptation = I::bradfordAdaptationMatrix(I::E::WhitePoint, I::D65::WhitePoint);
	xyz = adaptation * xyz;
	return sRGB::fromXYZ1931 * xyz;
}

vec3 wavelengthToCIEXYZ1931(float wavelength, float intensity) {
	return intensity * vec3(CIE1931::xFit(wavelength), CIE1931::yFit(wavelength), CIE1931::zFit(wavelength));
}
/// wavelength to linear sRGB
vec3 wavelengthToRGB(float wavelength, float intensity) {
	return CIEXYZ1931_to_LinearSRGB(intensity * vec3(CIE1931::xFit(wavelength), CIE1931::yFit(wavelength), CIE1931::zFit(wavelength)));
}

/// convert spectrum to linear sRGB
vec3 spectrumToRGB(const ColorSampler* const sampler, float min, float max, int samples) {
	assert(samples > 1);
	vec3 xyz(0.0f);
	for (int i = 0; i < samples; ++i) {
		float wavelength = glm::mix(min, max, i / static_cast<float>(samples - 1));
		xyz += wavelengthToCIEXYZ1931(wavelength, sampler->sample(wavelength));
	}
	xyz *= (max - min) / static_cast<float>(samples);
	return glm::max(CIEXYZ1931_to_LinearSRGB(xyz), 0.0f);
}






float BlackBodySPD::sample(float wavelength) const {
	return blackBodyRadiance(wavelength, temperature);
}

float BlackBodySPDNormalized::sample(float wavelength) const {
	return BlackBodySPD::sample(wavelength) / BlackBodySPD::sample(2.8977721e6f / temperature);
}

