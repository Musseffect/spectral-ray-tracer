#pragma once
#include "Base.h"
#include <functional>

namespace PrimitiveLocators {

	template<class Primitive>
	class BruteForce : public Base<Primitive> {
		AABB bounds;
		using Base<Primitive>::primitives;
	public:
		template <class Iterator>
		BruteForce(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get);
		template <class Iterator>
		BruteForce(Iterator begin, Iterator end, Primitive*(*get)(Iterator&));
		template <class Iterator>
		BruteForce(Iterator begin, Iterator end);
		template<class Object>
		std::vector<int> intersectedIndicies(const Object& object) const;
		virtual bool intersect(const Ray& ray) const override;
		virtual bool intersect(Ray& ray, HitInfo& hitInfo) const override;
		virtual AABB bbox() const override;
	};

	template<class Primitive>
	template <class Iterator>
	BruteForce<Primitive>::BruteForce(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get) {
		int primitivesCount = std::distance(begin, end);
		primitives.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back(get(it));
			bounds.append(primitives.back()->bbox());
		}
	}

	template<class Primitive>
	template <class Iterator>
	BruteForce<Primitive>::BruteForce(Iterator begin, Iterator end, Primitive*(*get)(Iterator&)){
		int primitivesCount = std::distance(begin, end);
		primitives.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back((*get)(it));
			bounds.append(primitives.back()->bbox());
		}
	}
	template<class Primitive>
	template <class Iterator>
	BruteForce<Primitive>::BruteForce(Iterator begin, Iterator end) {
		int primitivesCount = std::distance(begin, end);
		primitives.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back(&(*it));
			bounds.append(primitives.back()->bbox());
		}
	}
	template<class Primitive>
	AABB BruteForce<Primitive>::bbox() const {
		return bounds;
	}
	template<class Primitive>
	template<class Object>
	std::vector<int> BruteForce<Primitive>::intersectedIndicies(const Object& object) const {
		std::vector<int> result;
		if (!bounds.intersect(object))
			return result;
		for (int i = 0; i < primitives.size(); ++i) {
			if (primitives[i]->intersect(object))
				result.push_back(i);
		}
		return result;
	}
	template<class Primitive>
	bool BruteForce<Primitive>::intersect(const Ray& ray) const {
		if (!bounds.intersect(ray))
			return false;
		for (const auto& primitive : primitives) {
			if (primitive->intersect(ray))
				return true;
		}
		return false;
	}
	template<class Primitive>
	bool BruteForce<Primitive>::intersect(Ray& ray, HitInfo& hitInfo) const {
		if (!bounds.intersect(ray))
			return false;
		bool result = false;
		for (const auto& primitive : primitives) {
			result = primitive->intersect(ray, hitInfo) || result;
		}
		return result;
	}
}

