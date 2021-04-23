#pragma once
#include <memory>

template<class PixelType>
class Image {
	std::unique_ptr<PixelType[]> data;
	int _width;
	int _height;
public:
	Image(int width, int height, const PixelType& defaultValue);
	PixelType& operator()(int i, int j);
	const PixelType& operator()(int i, int j) const;
	void multiply(float scalar);
	int width() const;
	int height() const;
	void transform(std::function<PixelType(const PixelType&)> function);
};

template<class PixelType>
Image<PixelType>::Image(int width, int height, const PixelType& defaultValue) :_width(width), _height(height) {
	data = std::make_unique<PixelType[]>(_width * _height);
}
template<class PixelType>
PixelType& Image<PixelType>::operator()(int i, int j) {
	return data[i + j * _width];
}
template<class PixelType>
const PixelType& Image<PixelType>::operator()(int i, int j) const {
	return data[i + j * _width];
}
template<class PixelType>
void Image<PixelType>::multiply(float scalar) {
	for (int i = 0; i < _width * _height; i++)
		data[i] *= scalar;
}
template<class PixelType>
int Image<PixelType>::width() const {
	return _width;
}
template<class PixelType>
int Image<PixelType>::height() const {
	return _height;
}

template<class PixelType>
void Image<PixelType>::transform(std::function<PixelType(const PixelType&)> function) {
	for (int i = 0; i < _width * _height; i++)
		data[i] = function(data[i]);
}
