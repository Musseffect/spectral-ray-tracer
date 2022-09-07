#include "Distribution1D.h"


Distribution1D::Distribution1D(std::initializer_list<float> initializer) {
	integral = 0.0f;
	m_cdf.push_back(0.0f);
	for (auto value : initializer) {
		m_values.push_back(value);
		m_cdf.push_back(integral = integral + value);
	}
	m_cdf.back() = 1.0f;
	for (auto& value : m_cdf)
		value /= integral;
}
float Distribution1D::sampleContinuous(float value, float& pdf) const {
	assert(m_values.size() > 1);
	//todo: rewrite
	size_t l = 0;
	size_t r = m_values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (m_cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	float delta = value - m_cdf[l - 1];
	delta = delta / (m_cdf[l] - m_cdf[l - 1]);
	pdf = m_values[l] / integral;
	return (l + delta) / m_values.size();
}
int Distribution1D::sampleDiscrete(float value, float& pdf) const {
	assert(m_values.size() > 1);
	size_t l = 0;
	size_t r = m_values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (m_cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	pdf = discretePDF(l);
	return l;
}
int Distribution1D::sampleDiscrete(float value) const {
	int l = 0;
	int r = m_values.size() - 1;
	while (l != r) {
		int i = (r + l) / 2;
		if (m_cdf[i + 1] > value)
			r = i;
		else
			l = i + 1;
	}
	return l;
}
float Distribution1D::discretePDF(int index) const {
	return m_values[index] / (integral * m_values.size());
}
int Distribution1D::sampleDiscrete() const {
	float value = Random::random();
	return sampleDiscrete(value);
}