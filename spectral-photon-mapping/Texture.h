#pragma once
#include "Color.h"

template<class T>
class Texture {
public:
	virtual const T sample(const HitInfo& hitInfo) const = 0;
	virtual ~Texture() {}
};

template<class T>
class ConstantTexture : public Texture<T> {
	T value;
public:
	ConstantTexture(const T& value) :value(value)
	{
	}
	virtual const T sample(const HitInfo& hitInfo) const override {
		return value;
	}
};

template<class T>
class CheckerboardTexture2D : public Texture<T> {
	T white;
	T black;
public:
	CheckerboardTexture2D(const T& black, const T& white) :black(black), white(white)
	{
	}
	virtual const T sample(const HitInfo& hitInfo) const override {
		glm::ivec2 iuv = glm::floor(hitInfo.uv * 2.0f);
		if ((iuv.x + iuv.y) % 2 == 0) {
			return black;
		}
		return white;
	}
};

template<class T>
class CheckerboardTexture3D : public Texture<T> {
	T white;
	T black;
public:
	CheckerboardTexture3D(const T& black, const T& white) :black(black), white(white)
	{
	}
	virtual const T sample(const HitInfo& hitInfo) const override {
		glm::ivec3 iuvs = glm::floor(hitInfo.localPosition * 2.0f);
		if ((iuvs.x + iuvs.y + iuvs.z) % 2 == 0) {
			return black;
		}
		return white;
	}
};

template<class T>
using spTex = std::shared_ptr<Texture<T>>;
template<class T>
using spConstTex = std::shared_ptr<ConstantTexture<T>>;

template<class T>
spConstTex<T> makeConstTex(const T& value) {
	return std::make_shared<ConstantTexture<T>>(value);
}

