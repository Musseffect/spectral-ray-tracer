#pragma once
#include "common.h"
#include <vector>
#include <list>
#include <functional>

#include "BBox3D.h"


struct SpectralPhoton {
	float wavelength;
	vec3 position;
	vec3 wi;
	vec3 normal;
	float power;
	SceneObject* object;
	vec3 coords() const {
		return position;
	}
};


template<class Value>
using PushToOut = std::function<void(const Value&)>;

// KD tree for points
template<class Point>
class PointLocator {
protected:
	std::vector<Point> points;
public:
	const Point& pointAt(int index) const {
		assert(index >= 0 && index < points.size());
		return points[index];
	}
	bool isEmpty() const {
		return points.empty();
	}
};


template<class Point>
class BruteForceLocator : public PointLocator<Point> {
	BBox3D bounds;
public:
	template<class Iterator>
	BruteForceLocator(Iterator begin, Iterator end) {
		this->points.resize(std::distance(begin, end));
		int index = 0;
		for (Iterator it = begin; it != end; ++it) {
			this->points[index] = *it;
			bounds.append(it->coords());
			++index;
		}
		bounds.append(glm::max(bounds.center() + 0.0001f, bounds.max()));
		bounds.append(glm::min(bounds.center() - 0.0001f, bounds.min()));
	}
	std::vector<Point> pointsWithinRadius(const vec3& center, float radius) const {
		if (this->isEmpty())
			return {};
		float dist = bounds.outerDistance(center);
		float Epsilon = 0.001f;
		if (dist > radius + Epsilon)
			return {};
		radius *= radius;
		std::vector<Point> result;
		for (const auto& point : this->points) {
			vec3 dif = point.coords() - center;
			if (glm::dot(dif, dif) <= radius)
				result.push_back(point);
		}
		return std::move(result);
	}
	void indicesWithinRadius(const vec3& center, float radius, std::vector<int>& result) const {
		if (this->isEmpty())
			return;
		float dist = bounds.outerDistance(center);
		float Epsilon = 0.001f;
		if (dist > radius + Epsilon)
			return;
		radius *= radius;
		int index = 0;
		for (const auto& point : this->points) {
			vec3 dif = point.coords() - center;
			if (glm::dot(dif, dif) <= radius)
				result.push_back(index);
			++index;
		}
	}
};




//#define USE_PRESORT
template<class Point>
class KdTree: public PointLocator<Point> {
	struct KdNode {
		enum Type: unsigned int {
			SPLIT_X = 0,
			SPLIT_Y = 1,
			SPLIT_Z = 2,
			LEAF = 3
		};
		unsigned int splitType : 2, leftId : 30;
		unsigned int rightId;
		float splitCoord;
		BBox3D bbox;
		KdNode(unsigned int splitType, unsigned int leftId, unsigned int rightId, float splitCoord, const BBox3D& bbox)
			:splitType(splitType), leftId(leftId), rightId(rightId), splitCoord(splitCoord), bbox(bbox)
		{
		}
	};
	std::vector<KdNode> nodes;
	BBox3D bounds;
	int maxPointsInNode;
	// orthogonal search from
	// https://doc.cgal.org/latest/Spatial_searching/index.html
	void appendNodes(const KdNode& current, std::vector<Point>& result) const {
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				const auto& point = this->points[index];
				result.push_back(point);
			}
			return;
		}
		appendNodes(nodes[current.leftId], result);
		appendNodes(nodes[current.rightId], result);
	}
	void appendNodes(const KdNode& current, std::vector<int>& result) const {
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				result.push_back(index);
			}
			return;
		}
		appendNodes(nodes[current.leftId], result);
		appendNodes(nodes[current.rightId], result);
	}
	void pointsWithinRadius(const KdNode& current, const BBox3D& currentBBox, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<Point>& result) const {
		const float Eps = 0.0001f;
		if (parentDistanceSqr > radiusSqr + Eps)
			return;
		if (current.bbox.isInBall(center, radiusSqr)) {
			appendNodes(current, result);
			return;
		}
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				const auto& point = this->points[index];
				if (glm::length2(point.coords() - center) <= radiusSqr)
					result.push_back(point);
			}
			return;
		}
		const KdNode* const childs[2] = { &nodes[current.leftId], &nodes[current.rightId] };

		BBox3D childBBoxes[2] = {currentBBox, currentBBox};
		const float cuttingValue = currentBBox._min[splitType] + current.splitCoord * (currentBBox._max[splitType] - childBBoxes[0]._min[splitType]);
		childBBoxes[0]._max[splitType] = cuttingValue;
		childBBoxes[1]._min[splitType] = childBBoxes[0]._max[splitType];

		const int swap = center[splitType] < cuttingValue;

		const float firstNodeDistanceSqr = parentDistanceSqr;
		const float widthLo = childBBoxes[swap]._max[splitType] - childBBoxes[swap]._min[splitType];
		const float distLo = std::max(0.0f, std::abs(center[splitType] - (childBBoxes[swap]._max[splitType] + childBBoxes[swap]._min[splitType]) * 0.5f) - widthLo * 0.5f);
		const float secondNodeDistanceSqr = parentDistanceSqr - distLo * distLo + std::pow(cuttingValue - center[splitType], 2.0f);
		pointsWithinRadius(*childs[swap], childBBoxes[swap], center, radiusSqr, firstNodeDistanceSqr, result);
		pointsWithinRadius(*childs[1 - swap], childBBoxes[1 - swap], center, radiusSqr, secondNodeDistanceSqr, result);
	}
	void indicesWithinRadius(const KdNode& current, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<int>& result) const {
		const float Eps = 0.0001f;
		if (parentDistanceSqr > radiusSqr + Eps)
			return;
		if (current.bbox.isInBall(center, radiusSqr)) {
			appendNodes(current, result);
			return;
		}
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				const auto& point = this->points[index];
				if (glm::length2(point.coords() - center) <= radiusSqr)
					result.push_back(index);
			}
			return;
		}
		const float firstNodeDistanceSqr = nodes[current.leftId].bbox.outerSqrDistance(center);
		const float secondNodeDistanceSqr = nodes[current.rightId].bbox.outerSqrDistance(center);
		indicesWithinRadius(nodes[current.leftId], center, radiusSqr, firstNodeDistanceSqr, result);
		indicesWithinRadius(nodes[current.rightId], center, radiusSqr, secondNodeDistanceSqr, result);
	}
	void build(typename const std::vector<Point>::iterator& begin, typename const std::vector<Point>::iterator& end, int depth, const BBox3D& bounds, int nodeIndex) {
		int size = std::distance(begin, end);
		assert(size != 0);
		int splitCoord = bounds.maxExtentDirection();
		if (depth <= 1 || bounds.size()[splitCoord] < 0.0001f || size <= maxPointsInNode) {
			int startIndex = std::distance(this->points.begin(), begin);
			int endIndex = startIndex + size;
			BBox3D bbox;
			for (int i = startIndex; i < endIndex; ++i)
				bbox.append(this->points[i].coords());
			nodes[nodeIndex].splitType = 3;
			nodes[nodeIndex].leftId = startIndex;
			nodes[nodeIndex].rightId = endIndex;
			nodes[nodeIndex].bbox = bbox;
			return;
		}
		const int median = size / 2;
		typename std::vector<Point>::iterator medianIt = std::next(begin, median);
		std::nth_element(begin, medianIt, end,
			[splitCoord](const Point& a, const Point& b) {
				return a.coords()[splitCoord] < b.coords()[splitCoord];
		});
		BBox3D childBounds[2] = { bounds, bounds };
		childBounds[0]._max[splitCoord] = medianIt->coords()[splitCoord];
		childBounds[1]._min[splitCoord] = medianIt->coords()[splitCoord];
		const float splitValue = (medianIt->coords()[splitCoord] - bounds._min[splitCoord]) / bounds.size()[splitCoord];
		int leftId = nodes.size();
		nodes.emplace_back(0, 0, -1, 0.5f, BBox3D());
		int rightId = nodes.size();
		nodes.emplace_back(0, 0, -1, 0.5f, BBox3D());
		nodes[nodeIndex].leftId = leftId;
		nodes[nodeIndex].rightId = rightId;
		nodes[nodeIndex].splitType = splitCoord;
		nodes[nodeIndex].splitCoord = splitValue;
		nodes[nodeIndex].bbox = bounds;
		build(begin, medianIt, depth - 1, childBounds[0], leftId);
		build(medianIt, end, depth - 1, childBounds[1], rightId);
	}
public:
	template<class Iterator>
	KdTree(Iterator begin, Iterator end, int maxPointsInNode = 10, int maxDepth = -1):maxPointsInNode(std::max(maxPointsInNode, 1))
	{
		int size = std::distance(begin, end);
		if (size == 0)
			return;
		this->points.resize(size);
		int index = 0;
		for (Iterator it = begin; it != end; ++it) {
			this->points[index] = *it;
			bounds.append(it->coords());
			++index;
		}
		bounds.append(glm::max(bounds.center() + 0.0001f, bounds.max()));
		bounds.append(glm::min(bounds.center() - 0.0001f, bounds.min()));
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(this->points.size()));
		nodes.emplace_back(0, 0, -1, 0.0f, bounds);
		build(this->points.begin(), this->points.end(), maxDepth, bounds, 0);
	}
	std::vector<Point> pointsWithinRadius(const vec3& center, float radius) const {
		if (this->isEmpty())
			return {};
		const float distSqr = bounds.outerSqrDistance(center);
		std::vector<Point> result;
		pointsWithinRadius(nodes[0], bounds, center, radius, distSqr, result);
		return std::move(result);
	}
	void indicesWithinRadius(const vec3& center, float radius, std::vector<int>& result) const {
		if (this->isEmpty())
			return;
		const float distSqr = bounds.outerSqrDistance(center);
		indicesWithinRadius(nodes[0], center, radius * radius, distSqr, result);
	}
};

template<class Point>
class HashGrid : public PointLocator<Point> {
	std::vector<std::vector<int>> indices;
	BBox3D bounds;
	int size;
	int index(int i, int j, int k) const {
		assert(i >= 0 && j >= 0 && k >= 0 && i < size && j < size && k < size);
		return i + (j + k * size) * size;
	}
public:
	template<class Iterator>
	HashGrid(Iterator begin, Iterator end, int size): indices(size * size * size) {
		this->points.resize(std::distance(begin, end));
		int id = 0;
		for (Iterator it = begin; it != end; ++it) {
			this->points[id] = *it;
			bounds.append(it->coords());
			++id;
		}
		// prevent extremely small bounds
		bounds.append(glm::max(bounds.center() + 0.0001f, bounds.max()));
		bounds.append(glm::min(bounds.center() - 0.0001f, bounds.min()));
		this->size = size;
		vec3 cellSize = bounds.size() / size;
		for (int i = 0; i < this->points.size(); i++) {
			glm::ivec3 coords = glm::min(glm::ivec3(glm::floor((this->points[i].coords() - bounds._min) / cellSize)), glm::ivec3(size - 1));
			indices[index(coords.x, coords.y, coords.z)].push_back(i);
		}
	}
	std::vector<Point> pointsWithinRadius(const vec3& center, float radius) const {
		if (this->isEmpty())
			return {};
		float sqrDist = bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return {};
		vec3 cellSize = (bounds._max - bounds._min) / size;
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
	void indicesWithinRadius(const vec3& center, float radius, std::vector<int>& result) const {
		if (this->isEmpty())
			return;
		float sqrDist = bounds.outerSqrDistance(center);
		float Epsilon = 0.01f;
		float radiusSqr = radius * radius;
		if (sqrDist > radiusSqr + Epsilon)
			return;
		vec3 cellSize = (bounds._max - bounds._min) / size;
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

					for(auto& index: current) {
						const Point& point = this->points[index];
						if (glm::length2(center - point.coords()) <= radiusSqr)
							result.push_back(index);
					}
				}
			}
		}
	}
};

template<class Point>
class AABBTree {
	struct Node {
		AABB aabb;
		enum Type {
			Leaf = 0,
			Inter = 1
		};
		unsigned int type:1, firstChild:31;
		int secondChild;
	};
	const int maxPointsInLeaf;
	std::vector<Node> nodes;
	std::vector<Point> points;
	void appendPoints(const Node& currentNode, std::vector<Point>& result) const {
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i)
				result.push_back(points[i]);
			return;
		}
		appendPoints(nodes[currentNode.firstChild], result);
		appendPoints(nodes[currentNode.secondChild], result);
	}
	void appendIndices(const Node& currentNode, std::vector<int>& result) const {
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i)
				result.push_back(i);
			return;
		}
		appendIndices(nodes[currentNode.firstChild], result);
		appendIndices(nodes[currentNode.secondChild], result);
	}
	void pointsWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<Point>& result) const {
		const float Eps = 0.0001f;
		if (currentNode.aabb.outerSqrDistance(center) > radiusSqr + Eps)
			return;
		if (currentNode.aabb.isInBall(center, radiusSqr)) {
			appendPoints(currentNode, result);
			return;
		}
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i) {
				const auto& point = points[i];
				if (glm::distance2(point.coords(), center) <= radiusSqr)
					result.push_back(point);
			}
			return;
		}
		pointsWithinRadius(nodes[currentNode.firstChild], center, radiusSqr, result);
		pointsWithinRadius(nodes[currentNode.secondChild], center, radiusSqr, result);
	}
	void indicesWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<int>& result) const {
		const float Eps = 0.0001f;
		if (currentNode.aabb.outerSqrDistance(center) > radiusSqr + Eps)
			return;
		if (currentNode.aabb.isInBall(center, radiusSqr)) {
			appendIndices(currentNode, result);
			return;
		}
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i) {
				const auto& point = points[i];
				if (glm::length2(point.coords() - center) <= radiusSqr)
					result.push_back(i);
			}
			return;
		}
		indicesWithinRadius(nodes[currentNode.firstChild], center, radiusSqr, result);
		indicesWithinRadius(nodes[currentNode.secondChild], center, radiusSqr, result);
	}
	void build(typename const std::vector<Point>::iterator& begin, typename const std::vector<Point>::iterator& end, const AABB& aabb, int depth, int nodeIndex) {
		int size = std::distance(begin, end);
		int splitCoord = aabb.maxExtentDirection();
		if (size <= maxPointsInLeaf || depth <= 1 || aabb.size()[splitCoord] < 0.0001f) {
			int start = std::distance(points.begin(), begin);
			int end = start + size;
			nodes[nodeIndex].type = Node::Type::Leaf;
			nodes[nodeIndex].firstChild = start;
			nodes[nodeIndex].secondChild = end;
			nodes[nodeIndex].aabb = aabb;
			return;
		}
		const int median = size / 2;
		typename std::vector<Point>::iterator splitIt = std::next(begin, median);
		std::nth_element(begin, splitIt, end,
			[splitCoord](const Point& a, const Point& b) {
			return a.coords()[splitCoord] < b.coords()[splitCoord];
		});
		AABB firstAABB;
		AABB secondAABB;
		for (auto it = begin; it != splitIt; ++it) {
			firstAABB.append(it->coords());
		}
		for (auto it = splitIt; it != end; ++it) {
			secondAABB.append(it->coords());
		}
		int firstId = nodes.size();
		nodes.push_back(Node());
		int secondId = nodes.size();
		nodes.push_back(Node());
		nodes[nodeIndex].firstChild = firstId;
		nodes[nodeIndex].secondChild = secondId;
		nodes[nodeIndex].type = Node::Type::Inter;
		nodes[nodeIndex].aabb = aabb;
		build(begin, splitIt, firstAABB, depth - 1, firstId);
		build(splitIt, end, secondAABB, depth - 1, secondId);
	}
public:
	template<class Iterator>
	AABBTree(Iterator begin, Iterator end, int maxPointsInLeaf = 1, int maxDepth = 8):maxPointsInLeaf(std::max(maxPointsInLeaf, 1)){
		int size = std::distance(begin, end);
		if (size == 0)
			return;
		points.resize(size);
		int index = 0;
		AABB aabb;
		for (Iterator it = begin; it != end; ++it) {
			points[index] = *it;
			aabb.append(it->coords());
			++index;
		}
		aabb.append(glm::max(aabb.center() + 0.0001f, aabb.max()));
		aabb.append(glm::min(aabb.center() - 0.0001f, aabb.min()));
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(this->points.size()));
		nodes.push_back(Node());
		build(points.begin(), points.end(), aabb, maxDepth, 0);
	}
	std::vector<Point> pointsWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return {};
		std::vector<Point> result;
		pointsWithinRadius(nodes[0], center, radius * radius, result);
		return std::move(result);
	}
	void indicesWithinRadius(const vec3& center, float radius, std::vector<int>& result) const {
		if (!isEmpty())
			indicesWithinRadius(nodes[0], center, radius * radius, result);
	}
	bool isEmpty() const {
		return points.empty();
	}
};