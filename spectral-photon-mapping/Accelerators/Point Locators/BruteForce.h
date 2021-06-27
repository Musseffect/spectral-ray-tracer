#pragma once
#include "Base.h"

namespace PointLocators {
	template<class Point>
	class BruteForce;

	class BruteForcePointPrimitiveSearch {
		template<class Point, class Primitive>
		static std::vector<Point*> pointsWithinBounds(BruteForce<Point>* accelerator, const Primitive& primitive) {
			std::vector<Point*> result;
			if (accelerator->isEmpty())
				return result;
			if (!primitive.intersect(accelerator->bounds))
				return result;
			for (const auto* point : accelerator->points) {
				if (primitive.intersect(*point))
					result.push_back(point);
			}
			return result;
		}
	};

	template<class Point>
	class BruteForce : public Base<Point, BruteForcePointPrimitiveSearch> {
		AABB bounds;
		using Base<Point, BruteForcePointPrimitiveSearch>::points;
	public:
		friend class BruteForcePointPrimitiveSearch;
		template<class Iterator>
		BruteForce(Iterator begin, Iterator end);
		template<class Iterator>
		BruteForce(Iterator begin, Iterator end, Primitive*(*get)(Iterator&));
		virtual std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const override;
		virtual std::vector<int> indicesWithinRadius(const vec3& center, float radius) const override;
		virtual const Point& pointAt(int index) const override;
		bool isEmpty() const;
	};

	template<class Point>
	template<class Iterator>
	BruteForce<Point>::BruteForce(Iterator begin, Iterator end) {
		points.resize(std::distance(begin, end));
		int index = 0;
		for (Iterator it = begin; it != end; ++it) {
			points[index] = &(*it);
			bounds.append(it->position());
			++index;
		}
	}
	template<class Point>
	template<class Iterator>
	BruteForce<Point>::BruteForce(Iterator begin, Iterator end, Primitive*(*get)(Iterator&)) {
		points.resize(std::distance(begin, end));
		int index = 0;
		for (Iterator it = begin; it != end; ++it) {
			points[index] = get(it);
			bounds.append(it->position());
			++index;
		}
	}
	template<class Point>
	std::vector<Point*> BruteForce<Point>::pointsWithinRadius(const vec3& center, float radius) const {
		if (this->isEmpty())
			return {};
		float dist = bounds.outerDistance(center);
		if (dist > radius + Epsilon)
			return {};
		radius *= radius;
		std::vector<Point> result;
		for (const auto* point : this->points) {
			vec3 delta = point->position() - center;
			if (glm::dot(delta, delta) <= radius)
				result.push_back(point);
		}
		return std::move(result);
	}
	template<class Point>
	std::vector<int> BruteForce<Point>::indicesWithinRadius(const vec3& center, float radius) const {
		if (this->isEmpty())
			return;
		float dist = bounds.outerDistance(center);
		if (dist > radius + Epsilon)
			return;
		radius *= radius;
		std::vector<int> result;
		int index = 0;
		for (const auto* point : this->points) {
			vec3 delta = point->position() - center;
			if (glm::dot(delta, delta) <= radius)
				result.push_back(index);
			++index;
		}
	}
	template<class Point>
	const Point& BruteForce<Point>::pointAt(int index) const {
		return *points[index];
	}

	template<class Point>
	bool BruteForce<Point>::isEmpty() const {
		return points.empty();
	}
}