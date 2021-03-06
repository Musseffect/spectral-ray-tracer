#pragma once
#include "Base.h"

namespace PointLocators {
	template<class Point>
	class KdTree;

	class KdTreePointPrimitiveSearch {
		template<class Point, class Primitive>
		static std::vector<Point*> pointsWithinBounds(KdTree<Point>* accelerator, const Primitive& primitive) {
			std::vector<Point*> result;
			if (accelerator->isEmpty())
				return result;
			if (!primitive.intersect(accelerator->nodes[0].bbox))
				return result;
			accelerator->pointsWithinBounds(accelerator->nodes[0], primitive, result);
			return result;
		}
	};

	template<class Point>
	class KdTree : Base<Point, KdTreePointPrimitiveSearch>{
		struct KdNode {
			enum Type : unsigned int {
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
			{}
		};
		AABB bounds;
		std::vector<KdNode> nodes;
		using Base<Point, KdTreePointPrimitiveSearch>::points;
		int maxPointsInNode;
		// orthogonal search from
		// https://doc.cgal.org/latest/Spatial_searching/index.html
		void appendNodes(const KdNode& current, std::vector<Point*>& result) const;
		void appendNodes(const KdNode& current, std::vector<int>& result) const;
		void pointsWithinRadius(const KdNode& current, const AABB& currentBounds, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<Point*>& result) const;
		template<class Primitive>
		void pointsWithinBounds(const KdNode& current, const Primitive& primitive, std::vector<Point*>& result) const;
		void indicesWithinRadius(const KdNode& current, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<int>& result) const;
		template<class Iterator>
		void build(Iterator begin, Iterator end, int depth, const BBox3D& bounds, int nodeIndex);
	public:
		template<class Iterator>
		KdTree(Iterator begin, Iterator end, int maxPointsInNode = 10, int maxDepth = -1);
		std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const;
		std::vector<int> indicesWithinRadius(const vec3& center, float radius) const;
		using Base<Point, KdTreePointPrimitiveSearch>::pointAt;
		using Base<Point, KdTreePointPrimitiveSearch>::isEmpty;
	};

	template<class Point>
	void KdTree<Point>::appendNodes(const KdNode& current, std::vector<Point*>& result) const {
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				auto* point = points[index];
				result.push_back(point);
			}
			return;
		}
		appendNodes(nodes[current.leftId], result);
		appendNodes(nodes[current.rightId], result);
	}

	template<class Point>
	void KdTree<Point>::appendNodes(const KdNode& current, std::vector<int>& result) const {
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

	template<class Point>
	void KdTree<Point>::pointsWithinRadius(const KdNode& current, const AABB& currentBounds, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<Point*>& result) const {
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
				auto* point = points[index];
				if (glm::length2(point->position() - center) <= radiusSqr)
					result.push_back(point);
			}
			return;
		}
		const KdNode* const childs[2] = { &nodes[current.leftId], &nodes[current.rightId] };

		BBox3D childBBoxes[2] = { currentBounds, currentBounds };
		const float cuttingValue = currentBounds.min()[splitType] + current.splitCoord * (currentBounds.max()[splitType] - childBBoxes[0].min()[splitType]);
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

	template<class Point>
	template<class Primitive>
	void KdTree<Point>::pointsWithinBounds(const KdNode& current, const Primitive& primitive, std::vector<Point*>& result) const {
		if (!primitive.intersect(current.bbox))
			return;
		const int splitType = current.splitType;
		if (splitType == KdNode::Type::LEAF) {
			for (int index = current.leftId; index < current.rightId; ++index) {
				auto* point = points[index];
				if (primitive.intersect(*point))
					result.push_back(point);
			}
			return;
		}
		pointsWithinBounds(nodes[current.leftId], primitive, result);
		pointsWithinBounds(nodes[current.rightId], primitive, result);
	}

	template<class Point>
	void KdTree<Point>::indicesWithinRadius(const KdNode& current, const vec3& center, float radiusSqr, float parentDistanceSqr, std::vector<int>& result) const {
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
				auto* point = points[index];
				if (glm::length2(point->position() - center) <= radiusSqr)
					result.push_back(index);
			}
			return;
		}
		const float firstNodeDistanceSqr = nodes[current.leftId].bbox.outerSqrDistance(center);
		const float secondNodeDistanceSqr = nodes[current.rightId].bbox.outerSqrDistance(center);
		indicesWithinRadius(nodes[current.leftId], center, radiusSqr, firstNodeDistanceSqr, result);
		indicesWithinRadius(nodes[current.rightId], center, radiusSqr, secondNodeDistanceSqr, result);
	}

	template<class Point>
	template<class Iterator>
	void KdTree<Point>::build(Iterator begin, Iterator end, int depth, const BBox3D& bounds, int nodeIndex) {
		int size = std::distance(begin, end);
		assert(size != 0);
		int splitCoord = bounds.maxExtentDirection();
		if (depth <= 1 || bounds.size()[splitCoord] < 0.0001f || size <= maxPointsInNode) {
			int startIndex = std::distance(points.begin(), begin);
			int endIndex = startIndex + size;
			BBox3D bbox;
			for (int i = startIndex; i < endIndex; ++i)
				bbox.append(points[i]->position());
			nodes[nodeIndex].splitType = 3;
			nodes[nodeIndex].leftId = startIndex;
			nodes[nodeIndex].rightId = endIndex;
			nodes[nodeIndex].bbox = bbox;
			return;
		}
		const int median = size / 2;
		Iterator medianIt = std::next(begin, median);
		std::nth_element(begin, medianIt, end,
			[splitCoord](const Point* a, const Point* b) {
			return a->position()[splitCoord] < b->position()[splitCoord];
		});
		BBox3D childBounds[2] = { bounds, bounds };
		childBounds[0]._max[splitCoord] = (*medianIt)->position()[splitCoord];
		childBounds[1]._min[splitCoord] = (*medianIt)->position()[splitCoord];
		const float splitValue = ((*medianIt)->position()[splitCoord] - bounds._min[splitCoord]) / bounds.size()[splitCoord];
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

	template<class Point>
	template<class Iterator>
	KdTree<Point>::KdTree(Iterator begin, Iterator end, int maxPointsInNode, int maxDepth) :maxPointsInNode(std::max(maxPointsInNode, 1))
	{
		int size = std::distance(begin, end);
		if (size == 0)
			return;
		points.reserve(size);
		for (Iterator it = begin; it != end; ++it) {
			points.push_back(&(*it));
			bounds.append(it->position());
		}
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(points.size()));
		nodes.emplace_back(0, 0, -1, 0.0f, bounds);
		build(points.begin(), points.end(), maxDepth, bounds, 0);
	}

	template<class Point>
	std::vector<Point*> KdTree<Point>::pointsWithinRadius(const vec3& center, float radius) const {
		std::vector<Point*> result;
		if (!isEmpty())
			pointsWithinRadius(nodes[0], bounds, center, radius, bounds.outerSqrDistance(center), result);
		return result;
	}

	template<class Point>
	std::vector<int> KdTree<Point>::indicesWithinRadius(const vec3& center, float radius) const {
		std::vector<int> result;
		if (!isEmpty())
			indicesWithinRadius(nodes[0], center, radius * radius, bounds.outerSqrDistance(center), result);
		return result;
	}
}