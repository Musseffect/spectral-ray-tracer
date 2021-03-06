#pragma once
#include <memory>

template<class type>
class Image {
	std::unique_ptr<type[]> data;
	int _width;
	int _height;
public:
	Image(int width, int height, const type& defaultValue):_width(width), _height(height) {
		data = std::make_unique<type[]>(_width * _height);
	}
	type& operator()(int i, int j) {
		return data[i + j * _width];
	}
	const type& operator()(int i, int j) const {
		return data[i + j * _width];
	}
	void multiply(float scalar) {
		for (int i = 0; i < _width * _height; i++) {
			data[i] *= scalar;
		}
	}
	int width() const {
		return _width;
	}
	int height() const {
		return _height;
	}
};
