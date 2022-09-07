#pragma once
#include <vector>
#include <assert.h>

#include "common.h"
#include "Random.h"


class ContinousDistribution1D {
	std::vector<float> m_cdf;
	std::vector<float> m_values;
public:
	template<class Iterator>
	ContinousDistribution1D(const Iterator& begin, const Iterator& end, float(*get)(const Iterator&));
	template<class Iterator>
	ContinousDistribution1D(const Iterator& begin, const Iterator& end);
	ContinousDistribution1D(std::initializer_list<float> initializer);
	float sample(float value) const;
	float sample(float value, float& pdf) const;
	float pdf(float value) const;
};

class DiscreteDistribution1D {
public:
	template<class Iterator>
	DiscreteDistribution1D(const Iterator& begin, const Iterator& end, float(*get)(const Iterator&));
	template<class Iterator>
	DiscreteDistribution1D(const Iterator& begin, const Iterator& end);
	DiscreteDistribution1D(std::initializer_list<float> initializer);

};

class Distribution1D {
	std::vector<float> m_cdf;
	std::vector<float> m_values;
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
	m_cdf.push_back(0.0f);
	for (auto it = begin; it != end; it++) {
		m_values.push_back(get(it));
		m_cdf.push_back(integral = integral + get(it));
	}
	m_cdf.back() = 1.0f;
	for (auto& value : m_cdf)
		value /= integral;
}

template<class Iterator>
Distribution1D::Distribution1D(const Iterator& begin, const Iterator& end) {
	integral = 0.0f;
	m_cdf.push_back(0.0f);
	for (auto it = begin; it != end; it++) {
		m_values.push_back(*it);
		m_cdf.push_back(integral = integral + *it);
	}
	m_cdf.back() = 1.0f;
	for (auto& value : m_cdf)
		value /= integral;
}