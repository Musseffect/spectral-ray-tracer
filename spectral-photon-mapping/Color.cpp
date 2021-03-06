#include "Color.h"

vec3 XYZToRGB(vec3 xyz) {
	const mat3 XYZ_to_RGB(2.3706743f, -0.9000405f, -0.4706338f,
		-0.5138850f, 1.4253036f, 0.0885814f,
		0.0052982f, -0.0146949f, 1.0093968f);
	return xyz * XYZ_to_RGB;
}

vec3 wavelengthToRGB(float wavelength, float intensity) {
	return glm::max(XYZToRGB(intensity * vec3(xFit_1931(wavelength),
		yFit_1931(wavelength), zFit_1931(wavelength))), 0.0f);
}

vec3 spectrumToRGB(const ColorSampler* const sampler, float min, float max, int samples) {
	vec3 xyz(0.0f);
	for (int i = 0; i < samples; ++i) {
		float wavelength = glm::mix(min, max, i / float(samples - 1));
		xyz += vec3(xFit_1931(wavelength), yFit_1931(wavelength), zFit_1931(wavelength)) * sampler->sample(wavelength);
	}
	xyz *= (max - min) / float(samples - 1);
	return glm::max(XYZToRGB(xyz), 0.0f);
}

float BlackBodySPD::sample(float wavelength) const {
	const float c = 299792458.0f;
	const float h = 6.62606957e-34;
	const float kb = 1.3806488e-23;
	float l = wavelength * 1e-9;
	float l5 = l * l * l * l * l;
	return (2.0f * h * c * c) / (l5 * (std::exp(h * c / (l * kb * temperature)) - 1.0f));
}

float ColorSampler::integral(float min, float max, int samples) const  {
	float result = 0.0;
	for (int i = 0; i < samples; i++)
		result += sample(glm::lerp(min, max, i / float(samples - 1)));
	//  |----w---|
	//  ----------
	// _|__|__|__|_
	// min      max
	// w = max - min
	// width of one bin = w / (bins - 1)
	return result * (max - min) / float(samples - 1);
}

float BlackBodyColorSampler::sample(float wavelength) const {
	return spd.sample(wavelength) * intensity;
}

GridFunction1D RGBColorSampler::redCurve(380.0f, 730.0f,
	{ 0.021592459f, 0.020293111f, 0.021807906f, 0.023803297f, 0.025208132f, 0.025414957f, 0.024621282f, 0.020973705f, 0.015752802f, 0.01116804f, 0.008578277f, 0.006581877f, 0.005171723f, 0.004545205f, 0.00414512f, 0.004343112f, 0.005238155f, 0.007251939f, 0.012543656f, 0.028067132f, 0.091342277f, 0.484081092f, 0.870378324f, 0.939513128f, 0.960926994f, 0.968623763f, 0.971263883f, 0.972285819f, 0.971898742f, 0.972691859f, 0.971734812f, 0.97234454f, 0.97150339f, 0.970857997f, 0.970553866f, 0.969671404f });
GridFunction1D RGBColorSampler::greenCurve(380.0f, 730.0f,
	{ 0.010542406f, 0.010878976f, 0.011063512f, 0.010736566f, 0.011681813f, 0.012434719f, 0.014986907f, 0.020100392f, 0.030356263f, 0.063388962f, 0.173423837f, 0.568321142f, 0.827791998f, 0.916560468f, 0.952002841f, 0.964096452f, 0.970590861f, 0.972502542f, 0.969148203f, 0.955344651f, 0.892637233f, 0.5003641f, 0.116236717f, 0.047951391f, 0.027873526f, 0.020057963f, 0.017382174f, 0.015429109f, 0.01543808f, 0.014546826f, 0.015197773f, 0.014285896f, 0.015069123f, 0.015506263f, 0.015545797f, 0.016302839f });
GridFunction1D RGBColorSampler::blueCurve(380.0f, 730.0f,
	{ 0.967865135f, 0.968827912f, 0.967128582f, 0.965460137f, 0.963110055f, 0.962150324f, 0.960391811f, 0.958925903f, 0.953890935f, 0.925442998f, 0.817997886f, 0.42509696f, 0.167036273f, 0.078894327f, 0.043852038f, 0.031560435f, 0.024170984f, 0.020245519f, 0.01830814f, 0.016588218f, 0.01602049f, 0.015554808f, 0.013384959f, 0.012535491f, 0.011199484f, 0.011318274f, 0.011353953f, 0.012285073f, 0.012663188f, 0.012761325f, 0.013067426f, 0.013369566f, 0.013427487f, 0.01363574f, 0.013893597f, 0.014025757f });

float RGBColorSampler::sample(float wavelength) const {
	vec3 curve(
		redCurve.sample(wavelength),
		greenCurve.sample(wavelength),
		blueCurve.sample(wavelength)
	);
	return glm::max(glm::dot(curve, rgb), 0.0f);
}

float SpectrumSampler::sample(float wavelength) const {
	return function->sample(wavelength);
}