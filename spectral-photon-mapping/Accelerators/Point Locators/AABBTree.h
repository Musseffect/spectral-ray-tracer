#pragma once


namespace PointLocators {
	template<class Point>
	class AABBTree;

	class AABBTreePointPrimitiveSearch {
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
	class AABBTree: public Base<Point, AABBTreePointPrimitiveSearch> {
		struct Node {
			AABB aabb;
			enum Type {
				Leaf = 0,
				Inter = 1
			};
			unsigned int type : 1, firstChild : 31;
			int secondChild;
		};
		const int maxPointsInLeaf;
		using Base<Point, AABBTreePointPrimitiveSearch>::points;
		using Base<Point, AABBTreePointPrimitiveSearch>::isEmpty;
		std::vector<Node> nodes;
		void appendPoints(const Node& currentNode, std::vector<Point>& result) const;
		void appendIndices(const Node& currentNode, std::vector<int>& result) const;
		void pointsWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<Point*>& result) const;
		void indicesWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<int>& result) const;
		template<class Iterator>
		void build(Iterator begin, Iterator end, const AABB& aabb, int depth, int nodeIndex);
		template<class Primitive>
		void pointsWithinBounds(const Node& currentNode, const Primitive& primitive, std::vector<Point*> result) const;
	public:
		template<class Iterator>
		AABBTree(Iterator begin, Iterator end, int maxPointsInLeaf = 1, int maxDepth = 8);
		std::vector<Point*> pointsWithinRadius(const vec3& center, float radius) const override;
		std::vector<int> indicesWithinRadius(const vec3& center, float radius) const override;
	};

	template<class Point>
	void AABBTree<Point>::appendPoints(const Node& currentNode, std::vector<Point>& result) const {
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i)
				result.push_back(points[i]);
			return;
		}
		appendPoints(nodes[currentNode.firstChild], result);
		appendPoints(nodes[currentNode.secondChild], result);
	}
	template<class Point>
	void AABBTree<Point>::appendIndices(const Node& currentNode, std::vector<int>& result) const {
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i)
				result.push_back(i);
			return;
		}
		appendIndices(nodes[currentNode.firstChild], result);
		appendIndices(nodes[currentNode.secondChild], result);
	}
	template<class Point>
	void AABBTree<Point>::pointsWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<Point*>& result) const {
		const float Eps = 0.0001f;
		if (currentNode.aabb.outerSqrDistance(center) > radiusSqr + Eps)
			return;
		if (currentNode.aabb.isInBall(center, radiusSqr)) {
			appendPoints(currentNode, result);
			return;
		}
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i) {
				const auto* point = points[i];
				if (glm::distance2(point->position(), center) <= radiusSqr)
					result.push_back(point);
			}
			return;
		}
		pointsWithinRadius(nodes[currentNode.firstChild], center, radiusSqr, result);
		pointsWithinRadius(nodes[currentNode.secondChild], center, radiusSqr, result);
	}
	template<class Point>
	void AABBTree<Point>::indicesWithinRadius(const Node& currentNode, const vec3& center, float radiusSqr, std::vector<int>& result) const {
		const float Eps = 0.0001f;
		if (currentNode.aabb.outerSqrDistance(center) > radiusSqr + Eps)
			return;
		if (currentNode.aabb.isInBall(center, radiusSqr)) {
			appendIndices(currentNode, result);
			return;
		}
		if (currentNode.type == Node::Type::Leaf) {
			for (int i = currentNode.firstChild; i < currentNode.secondChild; ++i) {
				const auto* point = points[i];
				if (glm::length2(point->position() - center) <= radiusSqr)
					result.push_back(i);
			}
			return;
		}
		indicesWithinRadius(nodes[currentNode.firstChild], center, radiusSqr, result);
		indicesWithinRadius(nodes[currentNode.secondChild], center, radiusSqr, result);
	}
	template<class Point>
	template<class Iterator>
	void AABBTree<Point>::build(Iterator begin, Iterator end, const AABB& aabb, int depth, int nodeIndex) {
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
		auto splitIt = std::next(begin, median);
		std::nth_element(begin, splitIt, end,
			[splitCoord](const Point* a, const Point* b) {
			return a->position()[splitCoord] < b->position()[splitCoord];
		});
		AABB firstAABB;
		AABB secondAABB;
		for (auto it = begin; it != splitIt; ++it) {
			firstAABB.append(it->position());
		}
		for (auto it = splitIt; it != end; ++it) {
			secondAABB.append(it->position());
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
	template<class Point>
	template<class Primitive>
	void AABBTree<Point>::pointsWithinBounds(const Node& currentNode, const Primitive& primitive, std::vector<Point*> result) const {
		if (!primitive.intersect(currentNode.bbox))
			return;
		const int splitType = currentNode.splitType;
		if (splitType == Node::Type::LEAF) {
			for (int index = currentNode.leftId; index < currentNode.rightId; ++index) {
				const auto& point = points[index];
				if (primitive.intersect(point))
					result.push_back(point);
			}
			return;
		}
		pointsWithinBounds(nodes[currentNode.leftId], primitive, result);
		pointsWithinBounds(nodes[currentNode.rightId], primitive, result);
	}

	template<class Point>
	template<class Iterator>
	AABBTree<Point>::AABBTree(Iterator begin, Iterator end, int maxPointsInLeaf, int maxDepth) :maxPointsInLeaf(std::max(maxPointsInLeaf, 1)) {
		int size = std::distance(begin, end);
		if (size == 0)
			return;
		points.resize(size);
		int index = 0;
		AABB aabb;
		for (Iterator it = begin; it != end; ++it) {
			points[index] = *it;
			aabb.append(it->position());
			++index;
		}
		aabb.append(glm::max(aabb.center() + 0.0001f, aabb.max()));
		aabb.append(glm::min(aabb.center() - 0.0001f, aabb.min()));
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(this->points.size()));
		nodes.push_back(Node());
		build(points.begin(), points.end(), aabb, maxDepth, 0);
	}
	template<class Point>
	std::vector<Point*> AABBTree<Point>::pointsWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return {};
		std::vector<Point> result;
		pointsWithinRadius(nodes[0], center, radius * radius, result);
		return std::move(result);
	}
	template<class Point>
	std::vector<int> AABBTree<Point>::indicesWithinRadius(const vec3& center, float radius) const {
		if (isEmpty())
			return {};
		std::vector<int> result;
		indicesWithinRadius(nodes[0], center, radius * radius, result);
		return std::move(result);
	}
}