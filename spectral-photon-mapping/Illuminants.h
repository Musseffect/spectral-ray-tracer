#pragma once
#include "common.h"
#include "ColorSampler.h"


namespace Illuminants {
	//from http://www.brucelindbloom.com/index.html?Eqn_XYZ_to_RGB.html

	mat3 bradfordAdaptationMatrix(const vec3& srcWhitePoint, const vec3& dstWhitePoint);
	// chromatic adaptation using XYZ scaling
	vec3 adaptXYZScaling(const vec3& color, const vec3& srcWhitePoint, const vec3& dstWhitePoint);
	// chromatic adaptation using Bradford method
	vec3 adaptBradford(const vec3& color, const vec3& srcWhitePoint, const vec3& dstWhitePoint);
	// incandescent / tungsten
	namespace A {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spAnalyticalSampler<std::function<float(float)>> analyticSpectrum;
		extern const spSpectrumSampler spectrum;
	}
	// obsolete, direct sunlight at noon
	namespace B {
		extern const float T;
		extern const spSpectrumSampler spectrum;
	}
	// obsolete, average / North sky daylight
	namespace C {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spSpectrumSampler spectrum;
	}
	// equal energy
	namespace E {
		extern const vec3 WhitePoint;
		extern const spColorSampler spectrum;
	}
	// horizon light
	namespace D50 {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spSpectrumSampler spectrum;
	}
	// mid-morning / mid-afternoon daylight
	namespace D55 {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spSpectrumSampler spectrum;

	}
	// noon daylight
	namespace D65 {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spSpectrumSampler spectrum;
	}
	// North sky daylight
	namespace D75 {
		//2° Standard Observer white point
		extern const vec3 WhitePoint;
		extern const float T;
		extern const spSpectrumSampler spectrum;
	}
	// 	high-efficiency blue phosphor monitors, BT.2035
	namespace D93 {
		extern const float T;
		extern const vec3 WhitePoint;
	}
	// todo: add other illuminants
}
