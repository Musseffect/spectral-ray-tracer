#pragma once
#include <vector>
#include <memory>
#include <assert.h>

#include "common.h"
#include "Random.h"
using floatList = std::initializer_list<float>;
using doubleList = std::initializer_list<double>;

class GridFunction1D;
class Function1D {
public:
	virtual float sample(float x) const = 0;
};

using spFunction1D = std::shared_ptr<Function1D>;

template<class Functor>
class AnalyticalFunction1D : public Function1D {
	Functor func;
public:
	AnalyticalFunction1D(Functor func) : func(func) {
	}
	virtual float sample(float x) const override {
		return func(x);
	}
};

enum class BorderType {
	CLAMP_TO_BORDER = 0,
	CLAMP_TO_EDGE = 1,
	INTERPOLATE = 2
};

class FiniteFunction1D : public Function1D {
protected:
	BorderType borderType = BorderType::CLAMP_TO_BORDER;
	float border = 0.0f;
public:
	FiniteFunction1D(BorderType borderType, float border) : borderType(borderType), border(border) {
	}
};

class GridFunction1D : public FiniteFunction1D {
	float min;
	float max;
	std::vector<float> values;
	friend class IrregularFunction1D;
public:
	GridFunction1D(float min, float max, floatList values, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f);
	void scale(float scaleFactor);
	template<class Iterator, class Getter>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, Getter get, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f);
	template<class Iterator>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f);
	virtual float sample(float x) const override;
};

template<class Iterator, class Getter>
GridFunction1D::GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, Getter get, BorderType borderType, float border)
	: FiniteFunction1D(borderType, border)
	, min(min)
	, max(max)
{
	assert(min <= max);
	for (auto it = begin; it != end; it++)
		values.push_back(get(it));
}

template<class Iterator>
GridFunction1D::GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, BorderType type, float border)
	: FiniteFunction1D(borderType, border)
	, min(min)
	, max(max)
{
	assert(min <= max);
	for (auto it = begin; it != end; it++)
		values.push_back(*it);
}

//todo: add constructor, test this
class IrregularFunction1D : public FiniteFunction1D {
	struct Sample { float y; float x; };
	std::vector<Sample> values;
public:
	IrregularFunction1D(std::initializer_list<std::pair<float, float>> values, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f)
		:FiniteFunction1D(borderType, border)
	{
		assert(values.size() > 1);
		for (const auto& pair : values) {
			this->values.push_back(Sample{ pair.first, pair.second });
		}
		std::sort(this->values.begin(), this->values.end(), [](const Sample& a, const Sample& b) { return a.x < b.x; });
	}
	void scale(float scaleFactor);
	// Getter returns std::pair
	template<class Iterator, class Getter>
	IrregularFunction1D(const Iterator& begin, const Iterator& end, Getter get, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f)
		:FiniteFunction1D(borderType, border)
	{
		for (auto it = begin; it != end; it++) {
			const auto value = get(it);
			values.push_back(Sample{ value.first, value.second });
		}
		std::sort(values.begin(), values.end(), [](const Sample& a, const Sample& b) { return a.x < b.x; });
	}
	// Iterators from container of std::pairs
	template<class Iterator>
	IrregularFunction1D(const Iterator& begin, const Iterator& end, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0.0f)
		:FiniteFunction1D(borderType, border)
	{
		for (auto it = begin; it != end; it++)
			values.push_back(Sample{ it->first, it->second });
		std::sort(values.begin(), values.end(), [](const Sample& a, const Sample& b) { return a.x < b.x; });
	}
	virtual float sample(float x) const override;
	GridFunction1D resample(int sampleCount) const;
};

class PiecewiseFunction : public Function1D {
	std::vector<float> edges;
	std::vector<spFunction1D> functions;
public:
	virtual float sample(float x) const override {
		assert(!functions.empty());
		assert(edges.size() > 1);
		size_t l = 0;
		size_t r = edges.size() - 1;
		// binary search
		while (r - l > 1) {
			size_t edgeId = (l + r) / 2;
			if (edges[edgeId] < x)
				l = edgeId;
			else
				r = edgeId;
		}
		return functions[l]->sample(x);
	}
	// todo: add constructor
};

GridFunction1D resample(const Function1D* function, int sampleCount, float min, float max, BorderType borderType = BorderType::CLAMP_TO_BORDER, float border = 0);