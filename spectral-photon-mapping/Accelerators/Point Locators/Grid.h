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
			glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((primitiveBounds.min() - accelerator->_bounds.min()) / cellSize)));
			glm::ivec3 max = glm::min(glm::ivec3(accelerator->size - 1), glm::ivec3(glm::floor((primitiveBounds.max() - accelerator->_bounds.min()) / cellSize)));
			for (int x = min.x; x <= max.x; x++)
			{
				for (int y = min.y; y <= max.y; y++)
				{
					for (int z = min.z; z <= max.z; z++)
					{
						const vec3 boxMin = vec3(float(x), float(y), float(z)) * cellSize;
						const vec3 boxMax = vec3(float(x + 1), float(y + 1), float(z + 1)) * cellSize;
						if (!primitive.intersect(BBox3D(boxMin + accelerator->_bounds.min(), boxMax + accelerator->_bounds.min())))
							continue;
						const auto& current = accelerator->indices[accelerator->index(x, y, z)];
						for (auto& index : current) {
							const Point& point = accelerator->points[index];
							if (primitive.intersect(point))
								result.push_back(&point);
						}
					}
				}
			}
			return result;
		}
	};
	template<class Point>
	class HashGrid : public Base<Point, HashGridPointPrimitiveSearch> {
		struct listNode {
			int next = -1;
			int index = -1;
			listNode(int next, int index) : next(next), index(index) {}
		};
		using Base<Point, HashGridPointPrimitiveSearch>::points;
		std::vector<int> grid;
		std::vector<listNode> nodes;
		BBox3D _bounds;
		glm::ivec3 size;
		int index(int i, int j, int k) const;
	public:
		template<class Iterator>
		HashGrid(Iterator begin, Iterator end, const glm::ivec3& gridSize);
		template<class Iterator>
		HashGrid(Iterator begin, Iterator end, Point*(*get)(Iterator&), const glm::ivec3& gridSize);
		virtual std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const override;
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
	HashGrid<Point>::HashGrid(Iterator begin, Iterator end, const glm::ivec3& gridSize):size(gridSize), grid(gridSize.x * gridSize.y * gridSize.z, -1){
		points.reserve(std::distance(begin, end));
		for (Iterator it = begin; it != end; ++it) {
			_bounds.append(it->position());
			points.append(&(*it));
		}
		vec3 diag = _bounds.size();
		vec3 cellSize = _bounds.size() / size;
		nodes.reserve(points.size());
		for (int i = 0; i < points.size(); ++i) {
			glm::ivec3 coords = glm::min(glm::ivec3(glm::floor((points[i]->position() - _bounds.min()) / cellSize)), glm::ivec3(size - 1));
			int gridIndex = index(coords.x, coords.y, coords.z);
			nodes.push_back(listNode(grid[gridIndex], i));
			grid[gridIndex] = nodes.size() - 1;
		}
	}
	template<class Point>
	template<class Iterator>
	HashGrid<Point>::HashGrid(Iterator begin, Iterator end, Point*(*get)(Iterator&), const glm::ivec3& gridSize) :size(gridSize), grid(gridSize.x * gridSize.y * gridSize.z, -1) {
		points.reserve(std::distance(begin, end));
		for (Iterator it = begin; it != end; ++it) {
			_bounds.append(get(it)->position());
			points.append(get(it));
		}
		vec3 diag = _bounds.size();
		vec3 cellSize = _bounds.size() / size;
		nodes.reserve(points.size());
		for (int i = 0; i < points.size(); ++i) {
			glm::ivec3 coords = glm::min(glm::ivec3(glm::floor((points[i]->position() - _bounds.min()) / cellSize)), glm::ivec3(size - 1));
			int gridIndex = index(coords.x, coords.y, coords.z);
			nodes.push_back(listNode(grid[gridIndex], i));
			grid[gridIndex] = nodes.size() - 1;
		}
	}
	template<class Point>
	std::vector<Point*> HashGrid<Point>::pointsWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return {};
		float sqrDist = _bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return {};
		vec3 cellSize = _bounds.size() / size;
		glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((center - vec3(radius + Epsilon) - _bounds.min()) / cellSize)));
		glm::ivec3 max = glm::min(glm::ivec3(size - 1), glm::ivec3(glm::floor((center + vec3(radius + Epsilon) - _bounds.min()) / cellSize)));
		std::vector<Point> result;
		for (int x = min.x; x <= max.x; x++)
		{
			for (int y = min.y; y <= max.y; y++)
			{
				for (int z = min.z; z <= max.z; z++)
				{
					const vec3 boxCenter = vec3(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f) * cellSize + _bounds.min() - center;
					if (BBox3D::outerSqrDistToBox(boxCenter, 0.5f * cellSize) > radiusSqr + Epsilon)
						continue;
					auto current = grid[index(x, y, z)];
					if (glm::length2(glm::max(glm::abs(boxCenter - vec3(cellSize)), glm::abs(boxCenter + vec3(cellSize)))) <= radiusSqr) {

						while (current != -1) {
							const auto& node = nodes[current];
							const Point& point = this->points[node.index];
							result.push_back(point);
							current = node.next;
						}
						continue;
					}
					while (current != -1) {
						const auto& node = nodes[current];
						const Point* point = this->points[node.index];
						if (glm::length2(center - point->position()) <= radiusSqr)
							result.push_back(point);
						current = node.next;
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
		float sqrDist = _bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return;
		std::vector<int> result;
		vec3 cellSize = _bounds.size() / size;
		glm::ivec3 min = glm::max(glm::ivec3(0), glm::ivec3(glm::floor((center - vec3(radius + Epsilon) - _bounds.min()) / cellSize)));
		glm::ivec3 max = glm::min(glm::ivec3(size - 1), glm::ivec3(glm::floor((center + vec3(radius + Epsilon) - _bounds.min()) / cellSize)));
		for (int x = min.x; x <= max.x; x++)
		{
			for (int y = min.y; y <= max.y; y++)
			{
				for (int z = min.z; z <= max.z; z++)
				{
					const vec3 boxCenter = vec3(float(x) + 0.5f, float(y) + 0.5f, float(z) + 0.5f) * cellSize + _bounds.min() - center;
					if (BBox3D::outerSqrDistToBox(boxCenter, 0.5f * cellSize) > radiusSqr + Epsilon)
						continue;
					auto current = grid[index(x, y, z)];
					if (glm::length2(glm::max(glm::abs(boxCenter - vec3(cellSize)), glm::abs(boxCenter + vec3(cellSize)))) <= radiusSqr) {
						while (current != -1) {
							const auto& node = nodes[current];
							result.push_back(node.index);
							current = node.next;
						}
						continue;
					}

					for (auto& index : current) {
						const auto& node = nodes[current];
						const Point* point = this->points[node.index];
						if (glm::length2(center - point->position()) <= radiusSqr)
							result.push_back(node.index);
						current = node.next;
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