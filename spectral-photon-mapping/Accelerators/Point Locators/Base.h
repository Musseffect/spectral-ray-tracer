#pragma once
#include "../../BBox3D.h"
#include "../../Intersectable.h"



namespace PointLocators {
	static const float Epsilon = 0.000001f;
	class IPoint {
	public:
		virtual vec3 position() const = 0;
	};
	template<class Point, class PointPrimitiveSearch>
	class Base {
	protected:
		std::vector<Point*> points;
	public:
		/// Search points within sphere
		virtual std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const = 0;
		/// Search indices of points within sphere
		virtual std::vector<int> indicesWithinRadius(const vec3& center, float radius) const = 0;
		template<class Primitive>
		std::vector<Point*> pointsWithinBounds(const Primitive& primitive) const {
			return typename PointPrimitiveSearch::template pointsWithinBounds<Point, Primitive>(this, primitive);
		}
		/// Access point by index
		const Point& pointAt(int index) const {
			assert(index >= 0 && index < points.size());
			return *points[index];
		}
		bool isEmpty() const {
			return points.empty();
		}
	};
}