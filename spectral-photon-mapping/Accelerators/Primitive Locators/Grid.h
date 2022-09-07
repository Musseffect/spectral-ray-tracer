#pragma once
#include "Base.h"
#include <list>
#include <stdexcept>

namespace PrimitiveLocators {
	template<class Primitive>
	class Grid : public Intersectable {
		std::vector<std::list<int>> indices;
		std::vector<Primitive*> primitives;
		AABB bounds;
		glm::ivec3 size;
		private:
			glm::ivec3 index(const glm::vec3& point) const {
				return glm::ivec3(glm::floor((point - bounds.min()) / bounds.size() * vec3(size)));
			}
		public:
			template <class Iterator>
			Grid(Iterator begin, Iterator end, glm::ivec3 size);
			AABB bbox() const;
			template<class Object>
			std::vector<int> intersectedIndicies(const Object& object) const;
			std::vector<int> intersectedIndicies(const vec3& point) const;
			bool intersect(const Ray& ray) const;
			bool intersect(Ray& ray, HitInfo& hitInfo) const;
	};
	template<class Primitive>
	template <class Iterator>
	Grid<Primitive>::Grid(Iterator begin, Iterator end, glm::ivec3 size) :size(size) {
		int primitivesCount = std::distance(begin, end);
		indices.resize(size.x * size.y * size.z);
		primitives.reserve(primitivesCount);
		std::vector<AABB> boundsArray;
		boundsArray.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back(&(*it));
			boundsArray.push_back(it->bbox());
			bounds.append(boundsArray.back());
		}
		for (int id = 0; id < primitives.size(); ++id) {
			AABB bbox = boundsArray[id];
			glm::ivec3 min = index(bbox.min());
			glm::ivec3 max = index(bbox.max());
			for (int k = min.z; k <= max.z; ++k) {
				int indexZ = k;
				for (int j = min.y; j <= max.y; ++j) {
					int indexY = indexZ * size.y + j;
					for (int i = min.x; i <= max.x; ++i) {
						int index = indexY * size.x + i;
						indices[index].push_back(id);
					}
				}
			}
		}
	}
	template<class Primitive>
	AABB Grid<Primitive>::bbox() const {
		return bounds;
	}

	template<class Primitive>
	template<class Object>
	std::vector<int> Grid<Primitive>::intersectedIndicies(const Object& object) const {
		std::vector<int> result;
		if (primitives.empty())
			return result;
		AABB bbox = object->bbox();
		glm::ivec3 min = index(bbox.min());
		glm::ivec3 max = index(bbox.max());
		for (int k = min.z; k <= max.z; ++k) {
			int indexZ = k;
			for (int j = min.y; j <= max.y; ++j) {
				int indexY = indexZ * size.y + j;
				for (int i = min.x; i <= max.x; ++i) {
					int index = indexY * size.x + i;
					for (auto id : indices[index]) {
						if (primitives[id]->intersect(object))
							result.push_back(id);
					}
				}
			}
		}
		return result;
	}
	template<class Primitive>
	std::vector<int> Grid<Primitive>::intersectedIndicies(const vec3& point) const {
		std::vector<int> result;
		if (primitives.empty())
			return result;
		glm::ivec3 cell = index(point);
		int index = cell.x + size.x * (cell.y + size.y * cell.z);
		for (auto id : indices[index]) {
			if (primitives[id]->intersect(point))
				result.push_back(id);
		}
		return result;
	}
	template<class Primitive>
	bool Grid<Primitive>::intersect(const Ray& ray) const {
		if (primitives.empty())
			return false;
		throw std::runtime_error("");
	}
	template<class Primitive>
	bool Grid<Primitive>::intersect(Ray& ray, HitInfo& hitInfo) const {
		if (primitives.empty())
			return false;
		throw std::runtime_error("");
	}
}