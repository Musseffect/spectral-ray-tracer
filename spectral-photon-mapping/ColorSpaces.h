#pragma once
#include "common.h"
#include "ColorSampler.h"

// spectral data from https://www.chromaxion.com/spectral-library.php
//2° Standard Observer
namespace CIE1931 {
	/// CIEXYZ observer matching functions for E illuminant
	extern const spSpectrumSampler X;
	extern const spSpectrumSampler Y;
	extern const spSpectrumSampler Z;

	// analytical x matching function
	template<class T>
	T xFit(T wavelength) {
		T t1 = (wavelength - T(442.0))*((wavelength < T(442.0)) ? T(0.0624) : T(0.0374));
		T t2 = (wavelength - T(599.8))*((wavelength < T(599.8)) ? T(0.0264) : T(0.0323));
		T t3 = (wavelength - T(501.1))*((wavelength < T(501.1)) ? T(0.0490) : T(0.0382));
		return T(0.362) * glm::exp(-T(0.5) * t1 * t1) +
			T(1.056) * glm::exp(-T(0.5) * t2 * t2) -
			T(0.065) * glm::exp(-T(0.5) * t3 * t3);
	}
	// analytical y matching function
	template<class T>
	T yFit(T wavelength) {
		T t1 = (wavelength - T(568.8))*((wavelength < T(568.8)) ? T(0.0213) : T(0.0247));
		T t2 = (wavelength - T(530.9))*((wavelength < T(530.9)) ? T(0.0613) : T(0.0322));
		return T(0.821) * glm::exp(-T(0.5) * t1 * t1) +
			T(0.286) * glm::exp(-T(0.5) * t2 * t2);
	}
	// analytical z matching function
	template<class T>
	T zFit(T wavelength) {
		T t1 = (wavelength - T(437.0))*((wavelength < T(437.0)) ? T(0.0845) : T(0.0278));
		T t2 = (wavelength - T(459.0))*((wavelength < T(459.0)) ? T(0.0385) : T(0.0725));
		return T(1.217) * glm::exp(-T(0.5) * t1 * t1) +
			T(0.681) * glm::exp(-T(0.5) * t2 * t2);
	}

	/// CIERGB observer matching functions
	extern const spSpectrumSampler R;
	extern const spSpectrumSampler G;
	extern const spSpectrumSampler B;
}

//10° Standard Observer
namespace CIE1964 {
	extern const spSpectrumSampler X;
	extern const spSpectrumSampler Y;
	extern const spSpectrumSampler Z;
}
// physiologically-relevant
// from http://cvrl.ioo.ucl.ac.uk/cmfs.htm
namespace CIE2012 {
	extern const spSpectrumSampler X;
	extern const spSpectrumSampler Y;
	extern const spSpectrumSampler Z;
}

vec3 gamma(const vec3& color, float power);

namespace sRGB {
	extern const vec3 XYZPrimaryR;
	extern const vec3 XYZPrimaryG;
	extern const vec3 XYZPrimaryB;
	extern const vec3 XYZWhitePoint;
	/// CIE XYZ with D65 white point to linear sRGB
	extern const mat3 fromXYZ1931;
	extern const mat3 toXYZ1931;
	/// apply gamma to linear sRGB
	vec3 fromLinear(const vec3& color);
	/// convert sRGB with gamma to linear sRGB (gamma expansion)
	vec3 toLinear(const vec3& color);
}

namespace CIERGB {
	extern const vec3 XYZPrimaryR;
	extern const vec3 XYZPrimaryG;
	extern const vec3 XYZPrimaryB;
	extern const vec3 XYZWhitePoint;
	extern const mat3 fromXYZ1931;
	extern const mat3 toXYZ1931;
	vec3 fromLinear(const vec3& color);
	vec3 toLinear(const vec3& color);
}

namespace CIELAB {
	float f(float x);
	float fInv(float x);
	extern const vec3 fromXYZ(const vec3& color, const vec3& whitePoint);
	const vec3 toXYZ(const vec3& color, const vec3& whitePoint);
}

// todo:
/// rgb usualy is gamma corrected for HSL, HSV
vec3 HSL_to_rgb(const vec3& hsl);
vec3 HSV_to_rgb(const vec3& hsv);