#pragma once


namespace PointLocators {
	template<class Point>
	class AABBTree {
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
		AABBTree(Iterator begin, Iterator end, int maxPointsInLeaf = 1, int maxDepth = 8) :maxPointsInLeaf(std::max(maxPointsInLeaf, 1)) {
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
}