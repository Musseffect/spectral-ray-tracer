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
	Distribution1D(const Iterator& begin, const Iterator& end, float (*get)(const Iterator&)) {
		integral = 0.0f;
		cdf.push_back(0);
		for (auto it = begin; it != end; it++) {
			values.push_back(get(it));
			cdf.push_back(integral = integral + get(it));
		}
		for (auto& value : cdf)
			value /= integral;
	}
	template<class Iterator>
	Distribution1D(const Iterator& begin, const Iterator& end) {
		integral = 0.0f;
		cdf.push_back(0);
		for (auto it = begin; it != end; it++) {
			values.push_back(*it);
			cdf.push_back(integral = integral + *it);
		}
		for (auto& value : cdf)
			value /= integral;
	}
	Distribution1D(std::initializer_list<float> initializer) {
		integral = 0.0f;
		cdf.push_back(0);
		for (auto value : initializer) {
			values.push_back(value);
			cdf.push_back(integral = integral + value);
		}
		for (auto& value : cdf)
			value /= integral;
	}
	float sampleContinuous(float value, float& pdf) const {
		//todo: rewrite
		int l = 0;
		int r = values.size() - 1;
		while (l != r) {
			int i = (r + l) / 2;
			if (cdf[i] <= value)
				r = i;
			else
				l = i + 1;
		}
		float delta = value - cdf[l];
		delta = delta / (cdf[l + 1] - cdf[l]);
		pdf = values[l] / integral;
		return (l + delta) / values.size();
	}
	int sampleDiscrete(float value, float& pdf) const {
		int l = 0;
		int r = values.size() - 1;
		while (l != r) {
			int i = (r + l) / 2;
			if (cdf[i] <= value)
				r = i;
			else
				l = i + 1;
		}
		pdf = discretePDF(l);
		return l;
	}
	int sampleDiscrete(float value) const {
		int l = 0;
		int r = values.size() - 1;
		while (l != r) {
			int i = (r + l) / 2;
			if (cdf[i] <= value)
				r = i;
			else
				l = i + 1;
		}
		return l;
	}
	float discretePDF(int index) const {
		return values[index] / (integral * values.size());
	}
	int sampleDiscrete() const {
		float value = Random::random();
		return sampleDiscrete(value);
	}
};

class Function1D {
public:
	virtual float sample(float x) const = 0;
};

class GridFunction1D: public Function1D {
	std::vector<float> values;
	float min;
	float max;
	GridFunction1D(float min, float max):min(min), max(max) {}
	friend class IrregularFunction1D;
public:
	GridFunction1D(float min, float max, std::initializer_list<float> values) :
		min(min), max(max), values(values)
	{
		assert(min <= max);
	}
	template<class Iterator>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, float(*get)(const Iterator&)) :
		min(min), max(max)
	{
		assert(min <= max);
		for (auto it = begin; it != end; it++)
			values.push_back(get(it));
	}
	template<class Iterator>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end) :
		min(min), max(max)
	{
		assert(min <= max);
		float sum = 0.0f;
		for (auto it = begin; it != end; it++)
			values.push_back(*it);
	}
	virtual float sample(float x) const override {
		assert(values.size() > 1);
		float fx = (x - min) * (values.size() - 1) / (max - min);
		int index = glm::clamp<int>(static_cast<int>(glm::floor(fx)), 0, values.size() - 1);
		return glm::mix(values[index], values[glm::min<int>(index + 1, values.size() - 1)], glm::fract(fx));
	}
	GridFunction1D resample(int sampleCount) {
		GridFunction1D result(min, max);
		for (int i = 0; i < sampleCount; ++i)
		{
			result.values.push_back(sample(glm::mix(min, max, i / float(sampleCount - 1))));
		}
		return result;
	}
};

class IrregularFunction1D: public Function1D {
	struct Samples { float y; float x;};
	std::vector<Samples> values;
public:
	virtual float sample(float x) const override {
		assert(!values.empty());
		if (values.front().x >= x)
			return values.front().y;
		if (values.back().x <= x)
			return values.back().y;
		int l = 0;
		int r = values.size() - 1;
		while (r - l > 1) {
			int i = (l + r) / 2;
			if (values[i].x < x)
				l = i;
			else
				r = i;
		}
		const auto& left = values[l];
		const auto& right = values[r];
		float dx = (right.x - left.x);
		return glm::lerp(left.y, right.y, glm::clamp((x - left.x) / dx, 0.0f, 1.0f));
	}
	GridFunction1D resample(int sampleCount) {
		assert(!values.empty());
		GridFunction1D result(values.front().x, values.back().x);
		int currentId = 0;
		for (int i = 0; i < sampleCount; ++i)
		{
			float x = glm::mix(result.min, result.max, i / float(sampleCount - 1));
			while (values[currentId].x < x && currentId < values.size()) {
				++currentId;
			}
			int prevId = glm::max(currentId - 1, 0);
			float value = 0.0f;
			if (prevId == currentId)
				value = values[prevId].y;
			else
				value = glm::mix(values[prevId].y, values[currentId].y, (x - values[prevId].x) / (values[currentId].x - values[prevId].x));
			result.values.push_back(value);
		}
		return result;
	}
};