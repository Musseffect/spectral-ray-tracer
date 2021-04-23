#include "Config.h"


Config& Config::get() {
	static Config config;
	return config;
}
float Config::spectrumMin() const {
	return _spectrumMin;
}
void Config::spectrumMin(float value) {
	_spectrumMin = value;
}
float Config::spectrumMax() const {
	return _spectrumMax;
}
void Config::spectrumMax(float value) {
	_spectrumMax = value;
}