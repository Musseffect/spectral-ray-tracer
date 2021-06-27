#include "Function1D.h"


GridFunction1D::GridFunction1D(float min, float max) :min(min), max(max) {}
GridFunction1D::GridFunction1D(float min, float max, std::initializer_list<float> values) :
	min(min), max(max), values(values)
{
	assert(min <= max);
}
void GridFunction1D::scale(float scaleFactor) {
	for (auto& value : values)
		value *= scaleFactor;
}

float GridFunction1D::sample(float x) const {
	assert(values.size() > 1);
	float fx = (x - min) * (values.size() - 1) / (max - min);
	int index = static_cast<int>(glm::floor(fx));
	if (index < 0)
		return values.front();
	if (index >= values.size() - 1)
		return values.back();
	return glm::mix(values[index], values[index + 1], glm::fract(fx));
}

GridFunction1D GridFunction1D::resample(int sampleCount) const {
	GridFunction1D result(min, max);
	for (int i = 0; i < sampleCount; ++i)
	{
		result.values.push_back(sample(glm::mix(min, max, i / float(sampleCount - 1))));
	}
	return result;
}

float IrregularFunction1D::sample(float x) const {
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
GridFunction1D IrregularFunction1D::resample(int sampleCount) const {
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