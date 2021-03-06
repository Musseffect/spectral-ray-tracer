#pragma once
#include <vector>
#include <assert.h>

#include "common.h"
#include "Random.h"

class GridFunction1D;

class Function1D {
public:
	virtual float sample(float x) const = 0;
	virtual GridFunction1D resample(int sampleCount) const = 0;
};

class GridFunction1D : public Function1D {
	std::vector<float> values;
	float min;
	float max;
	GridFunction1D(float min, float max);
	friend class IrregularFunction1D;
public:
	GridFunction1D(float min, float max, std::initializer_list<float> values);
	void scale(float scaleFactor);
	template<class Iterator>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, float(*get)(const Iterator&));
	template<class Iterator>
	GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end);
	virtual float sample(float x) const override;
	virtual GridFunction1D resample(int sampleCount) const override;
};

template<class Iterator>
GridFunction1D::GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end, float(*get)(const Iterator&)) :
	min(min), max(max)
{
	assert(min <= max);
	for (auto it = begin; it != end; it++)
		values.push_back(get(it));
}
template<class Iterator>
GridFunction1D::GridFunction1D(float min, float max, const Iterator& begin, const Iterator& end) :
	min(min), max(max)
{
	assert(min <= max);
	float sum = 0.0f;
	for (auto it = begin; it != end; it++)
		values.push_back(*it);
}

class IrregularFunction1D : public Function1D {
	struct Samples { float y; float x; };
	std::vector<Samples> values;
public:
	virtual float sample(float x) const override;
	virtual GridFunction1D resample(int sampleCount) const override;
};