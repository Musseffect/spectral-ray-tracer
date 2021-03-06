#pragma once
#include <vector>
#include <assert.h>

#include "common.h"
#include "Random.h"


class Distribution1D {
	std::vector<float> cdf;
	std::vector<float> values;
	float integral;
public:
	template<class Iterator>
	Distribution1D(const Iterator& begin, const Iterator& end, float(*get)(const Iterator&));
	template<class Iterator>
	Distribution1D(const Iterator& begin, const Iterator& end);
	Distribution1D(std::initializer_list<float> initializer);
	float sampleContinuous(float value, float& pdf) const;
	int sampleDiscrete(float value, float& pdf) const;
	int sampleDiscrete(float value) const;
	float discretePDF(int index) const;
	int sampleDiscrete() const;
};

template<class Iterator>
Distribution1D::Distribution1D(const Iterator& begin, const Iterator& end, float(*get)(const Iterator&)) {
	integral = 0.0f;
	cdf.push_back(0.0f);
	for (auto it = begin; it != end; it++) {
		values.push_back(get(it));
		cdf.push_back(integral = integral + get(it));
	}
	cdf.back() = 1.0f;
	for (auto& value : cdf)
		value /= integral;
}

template<class Iterator>
Distribution1D::Distribution1D(const Iterator& begin, const Iterator& end) {
	integral = 0.0f;
	cdf.push_back(0.0f);
	for (auto it = begin; it != end; it++) {
		values.push_back(*it);
		cdf.push_back(integral = integral + *it);
	}
	cdf.back() = 1.0f;
	for (auto& value : cdf)
		value /= integral;
}