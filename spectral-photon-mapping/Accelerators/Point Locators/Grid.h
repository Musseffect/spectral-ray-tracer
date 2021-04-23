#pragma once
#include "Base.h"



namespace PointLocators {
	template<class Point>
	class HashGrid;

	class HashGridPointPrimitiveSearch {
		template<class Point, class Primitive>
		static std::vector<Point*> pointsWithinBounds(HashGrid<Point>* accelerator, const Primitive& primitive) {
			std::vector<Point*> result;
			if (accelerator->isEmpty())
				return result;
			if (!primitive.intersect(accelerator->_bounds))
				return result;
			vec3 cellSize = (accelerator->_bounds.max() - accelerator->_bounds.min()) / accelerator->size;
			AABB primitiveBounds = primitive.bounds();
			glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((primitiveBounds.min() - accelerator->_bounds._min) / cellSize)));
			glm::ivec3 max = glm::min(glm::ivec3(size - 1), glm::ivec3(glm::floor((primitiveBounds.max() - accelerator->_bounds._min) / cellSize)));
			for (int x = min.x; x <= max.x; x++)
			{
				for (int y = min.y; y <= max.y; y++)
				{
					for (int z = min.z; z <= max.z; z++)
					{
						if (!primitive.intersect(BBox3D(min + accelerator->_bounds.min(), max + accelerator->_bounds.max())))
							continue;
						for (auto& pointPtr : current) {
							if (primitive.intersect(*pointPtr))
								result.push_back(index);
						}
					}
				}
			}
			return result;
		}
	};
	template<class Point>
	class HashGrid : public Base<Point, HashGridPointPrimitiveSearch> {
		std::vector<Point*> points;
		std::vector<std::vector<int>> indices;
		BBox3D _bounds;
		int size;
		int index(int i, int j, int k) const;
	public:
		template<class Iterator>
		HashGrid(Iterator begin, Iterator end, int gridSize);
		template<class Iterator>
		HashGrid(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int gridSize);
		virtual std::vector<Point> pointsWithinRadius(const vec3& center, float radius) const override;
		virtual std::vector<int> indicesWithinRadius(const vec3& center, float radius) const override;
		virtual const Point& pointAt(int index) const override;
		bool isEmpty() const;
	};
	template<class Point>
	int HashGrid<Point>::index(int i, int j, int k) const {
		assert(i >= 0 && j >= 0 && k >= 0 && i < size && j < size && k < size);
		return i + (j + k * size) * size;
	}
	template<class Point>
	template<class Iterator>
	HashGrid<Point>::HashGrid(Iterator begin, Iterator end, int gridSize) : points(gridSize * gridSize * gridSize) {
		int id = 0;
		for (Iterator it = begin; it != end; ++it) {
			_bounds.append(it->coords());
			++id;
		}
		size = gridSize;
		id = 0;
		vec3 cellSize = _bounds.size() / size;
		for (Iterator it = begin; it != end; ++it) {
			glm::ivec3 coords = glm::min(glm::ivec3(glm::floor((it->position() - _bounds._min) / cellSize)), glm::ivec3(size - 1));
			points[index(coords.x, coords.y, coords.z)].push_back(&(*it));
		}
	}
	template<class Point>
	template<class Iterator>
	HashGrid<Point>::HashGrid(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int gridSize) : points(gridSize * gridSize * gridSize) {
		int id = 0;
		for (Iterator it = begin; it != end; ++it) {
			_bounds.append(get(it)->coords());
			++id;
		}
		size = gridSize;
		id = 0;
		vec3 cellSize = _bounds.size() / size;
		for (Iterator it = begin; it != end; ++it) {
			glm::ivec3 coords = glm::min(glm::ivec3(glm::floor((get(it)->position() - _bounds._min) / cellSize)), glm::ivec3(size - 1));
			points[index(coords.x, coords.y, coords.z)].push_back(get(it));
		}
	}
	template<class Point>
	std::vector<Point> HashGrid<Point>::pointsWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return {};
		float sqrDist = bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return {};
		vec3 cellSize = (_bounds._max - _bounds._min) / size;
		glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((center - vec3(radius + Epsilon) - bounds._min) / cellSize)));
		glm::ivec3 max = glm::min(glm::ivec3(size - 1), glm::ivec3(glm::floor((center + vec3(radius + Epsilon) - bounds._min) / cellSize)));
		std::vector<Point> result;
		for (int x = min.x; x <= max.x; x++)
		{
			for (int y = min.y; y <= max.y; y++)
			{
				for (int z = min.z; z <= max.z; z++)
				{
					const vec3 boxCenter = vec3(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f) * cellSize + bounds._min - center;
					if (BBox3D::outerSqrDistToBox(boxCenter, 0.5f * cellSize) > radiusSqr + Epsilon)
						continue;
					const auto& current = indices[index(x, y, z)];
					if (glm::length2(glm::max(glm::abs(boxCenter - vec3(cellSize)), glm::abs(boxCenter + vec3(cellSize)))) <= radiusSqr) {

						for (auto& index : current) {
							const Point& point = this->points[index];
							result.push_back(point);
						}
						continue;
					}

					for (auto& index : current) {
						const Point& point = this->points[index];
						if (glm::length2(center - point.coords()) <= radiusSqr)
							result.push_back(point);
					}
				}
			}
		}
		return std::move(result);
	}
	template<class Point>
	std::vector<int> HashGrid<Point>::indicesWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return;
		float sqrDist = bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return;
		vec3 cellSize = (_bounds._max - _bounds._min) / size;
		glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((center - vec3(radius + Epsilon) - bounds._min) / cellSize)));
		glm::ivec3 max = glm::min(glm::ivec3(size - 1), glm::ivec3(glm::floor((center + vec3(radius + Epsilon) - bounds._min) / cellSize)));
		for (int x = min.x; x <= max.x; x++)
		{
			for (int y = min.y; y <= max.y; y++)
			{
				for (int z = min.z; z <= max.z; z++)
				{
					const vec3 boxCenter = vec3(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f) * cellSize + bounds._min - center;
					if (BBox3D::outerSqrDistToBox(boxCenter, 0.5f * cellSize) > radiusSqr + Epsilon)
						continue;
					const auto& current = indices[index(x, y, z)];
					if (glm::length2(glm::max(glm::abs(boxCenter - vec3(cellSize)), glm::abs(boxCenter + vec3(cellSize)))) <= radiusSqr) {

						for (auto& index : current)
							result.push_back(index);
						continue;
					}

					for (auto& index : current) {
						const Point& point = this->points[index];
						if (glm::length2(center - point.coords()) <= radiusSqr)
							result.push_back(index);
					}
				}
			}
		}
	}
	template<class Point>
	const Point& HashGrid<Point>::pointAt(int index) const {
		return *points[index];
	}
	template<class Point>
	bool HashGrid<Point>::isEmpty() const {
		return points.size() == 0;
	}
}