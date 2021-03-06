#include "Distribution1D.h"


Distribution1D::Distribution1D(std::initializer_list<float> initializer) {
	integral = 0.0f;
	cdf.push_back(0.0f);
	for (auto value : initializer) {
		values.push_back(value);
		cdf.push_back(integral = integral + value);
	}
	cdf.back() = 1.0f;
	for (auto& value : cdf)
		value /= integral;
}
float Distribution1D::sampleContinuous(float value, float& pdf) const {
	//todo: rewrite
	int l = 0;
	int r = values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	float delta = value - cdf[l - 1];
	delta = delta / (cdf[l] - cdf[l - 1]);
	pdf = values[l] / integral;
	return (l + delta) / values.size();
}
int Distribution1D::sampleDiscrete(float value, float& pdf) const {
	int l = 0;
	int r = values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	pdf = discretePDF(l);
	return l;
}
int Distribution1D::sampleDiscrete(float value) const {
	int l = 0;
	int r = values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	return l;
}
float Distribution1D::discretePDF(int index) const {
	return values[index] / (integral * values.size());
}
int Distribution1D::sampleDiscrete() const {
	float value = Random::random();
	return sampleDiscrete(value);
}