#pragma once
#include <vector>
#include <array>

class CauchyEquation {
	std::vector<float> coeffs;
public:
	CauchyEquation(const std::vector<float>& coeffs) :coeffs(coeffs)
	{
	}
	float operator()(float wavelength) const {
		assert(coeffs.size() > 0.0);
		float result = coeffs.front();
		wavelength *= wavelength;
		for (int i = 1; i < coeffs.size(); ++i) {
			result += coeffs[i] / wavelength;
		}
		return result;
	}
};

class SellmeierEquation {
	std::vector<std::array<float, 2>> coeffs;
public:
	SellmeierEquation(const std::vector<std::array<float, 2>>& coeffs)
		:coeffs(coeffs)
	{
	}
	float operator()(float wavelength) const {
		float result = 1.0f;
		wavelength *= wavelength;
		for (auto coeff : coeffs) {
			result += coeff[0] * wavelength / (wavelength - coeff[1]);
		}
		return std::sqrt(result);
	}
};

// from wikipedia
static CauchyEquation fucedSilica({ 1.458f, 0.00354f });
static CauchyEquation BK7({ 1.5046f, 0.0042f });
static CauchyEquation K5({ 1.522f, 0.00459f });
static CauchyEquation BaK4({ 1.569f, 0.00531f });
static CauchyEquation BaF10({ 1.67f, 0.00743f });
static CauchyEquation SF10({ 1.728f, 0.01342f });