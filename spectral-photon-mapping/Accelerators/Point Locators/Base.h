#pragma once
#include "../../BBox3D.h"
#include "../../Intersectable.h"



namespace PointLocators {
	class IPoint {
	public:
		virtual vec3 position() const = 0;
	};
	template<class Point, class PointPrimitiveSearch>
	class Base {
	public:
		virtual std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const = 0;
		virtual std::vector<int> indicesWithinRadius(const vec3& center, float radius) const = 0;
		template<class Primitive>
		std::vector<Point*> pointsWithinBounds(const Primitive& primitive) const {
			return typename PointPrimitiveSearch::template pointsWithinBounds<Point, Primitive>(this, primitive);
		}
		virtual const Point& pointAt(int index) const = 0;
	};
}