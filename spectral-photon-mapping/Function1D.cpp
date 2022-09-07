#include "Function1D.h"


GridFunction1D resample(const Function1D* function,int sampleCount, float min, float max, BorderType borderType, float border) {
	std::vector<float> values;
	values.reserve(sampleCount);
	for (int i = 0; i < sampleCount; ++i)
	{
		values.push_back(function->sample(glm::mix(min, max, i / static_cast<float>(sampleCount - 1))));
	}
	return std::move(GridFunction1D(min, max, values.begin(), values.end(), borderType, border));
}


GridFunction1D::GridFunction1D(float min, float max, floatList values, BorderType borderType, float border)
	: FiniteFunction1D(borderType, border), values(values)
{
	assert(min <= max);
	this->min = min;
	this->max = max;
	this->border = border;
	this->borderType = borderType;
}

void GridFunction1D::scale(float scaleFactor) {
	for (auto& value : values)
		value *= scaleFactor;
}

float GridFunction1D::sample(float x) const {
	assert(values.size() > 1);
	const float dx = (max - min) / (values.size() - 1);
	if (x < min) {
		switch (borderType) {
		case BorderType::CLAMP_TO_BORDER:
			return border;
		case BorderType::CLAMP_TO_EDGE:
			return values.front();
		case BorderType::INTERPOLATE:
			float x1 = min;
			float x2 = min + dx;
			if (x2 == x1) return values.front();
			float y1 = values[0];
			float y2 = values[1];
			return y1 - (y2 - y1) * (x1 - x) / dx;
		}
	}
	else if (x > max) {
		switch (borderType) {
		case BorderType::CLAMP_TO_BORDER:
			return border;
		case BorderType::CLAMP_TO_EDGE:
			return values.back();
		case BorderType::INTERPOLATE:
			float x1 = max;
			float x2 = max - dx;
			if (x2 == x1) return values.back();
			float y1 = values.back();
			float y2 = values[values.size() - 2];
			return y1 - (y1 - y2) * (x1 - x) / dx;
		}
	}
	float fx = (x - min) * (values.size() - 1) / (max - min);
	int index = static_cast<int>(glm::floor(fx));
	if (index < 0)
		return values.front();
	if (index >= values.size() - 1)
		return values.back();
	return glm::mix(values[index], values[index + 1], glm::fract(fx));
}

float IrregularFunction1D::sample(float x) const {
	assert(values.size() > 1);
	if (x < values.front().x) {
		switch (borderType) {
		case BorderType::CLAMP_TO_BORDER:
			return border;
		case BorderType::CLAMP_TO_EDGE:
			return values.front().y;
		case BorderType::INTERPOLATE:
			const auto& first = values.front();
			const auto& second = values[1];
			float x1 = first.x;
			float x2 = second.x;
			if (x2 == x1) return std::numeric_limits<float>::infinity() * glm::sign(first.y - second.y);
			float y1 = first.y;
			float y2 = second.y;
			return y1 - (y2 - y1) * (x1 - x) / (x2 - x1);
		}
	}
	else if (x > values.back().x) {
		switch (borderType) {
		case BorderType::CLAMP_TO_BORDER:
			return border;
		case BorderType::CLAMP_TO_EDGE:
			return values.back().y;
		case BorderType::INTERPOLATE:
			const auto& last = values.back();
			const auto& secondLast = values[values.size() - 2];
			float x1 = last.x;
			float x2 = secondLast.x;
			if (x2 == x1) return std::numeric_limits<float>::infinity() * glm::sign(last.y - secondLast.y);
			float y1 = last.y;
			float y2 = secondLast.y;
			return y1 - (y1 - y2) * (x1 - x) / (x1 - x2);
		}
	}

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
	return glm::mix<float>(left.y, right.y, glm::clamp((x - left.x) / dx, 0.0f, 1.0f));
}

void IrregularFunction1D::scale(float scaleFactor) {
	for (auto& value : values)
		value.y *= scaleFactor;
}

GridFunction1D IrregularFunction1D::resample(int sampleCount) const {
	assert(!values.empty());
	std::vector<float> resValues;
	resValues.reserve(sampleCount);
	int currentId = 0;
	for (int i = 0; i < sampleCount; ++i)
	{
		float x = glm::mix(values.front().x, values.back().x, i / static_cast<float>(sampleCount - 1));
		while (values[currentId].x < x && currentId < values.size()) {
			++currentId;
		}
		int prevId = glm::max(currentId - 1, 0);
		float value = 0.0f;
		if (prevId == currentId)
			value = values[prevId].y;
		else
			value = glm::mix(values[prevId].y, values[currentId].y, (x - values[prevId].x) / (values[currentId].x - values[prevId].x));
		resValues.push_back(value);
	}
	return std::move(GridFunction1D(values.front().x, values.back().x, resValues.begin(), resValues.end(), borderType, border));
}