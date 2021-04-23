#pragma once



class Config {
	float _spectrumMin = 400.0f;
	float _spectrumMax = 700.0f;
private:
	Config() = default;
public:
	static Config& get();
	float spectrumMin() const;
	void spectrumMin(float value);
	float spectrumMax() const;
	void spectrumMax(float value);
};