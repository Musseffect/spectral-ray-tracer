#pragma once
#include "Base.h"

namespace PrimitiveLocators {
	static const float Epsilon = 0.000001f;
	template<class Primitive, class TreeBuilder>
	class AABBTree;

	template<class Primitive>
	class SplitMiddleTreeBuilder {};
	template<class Primitive>
	class SplitEqualCountsTreeBuilder {
		using Tree = AABBTree<Primitive, SplitEqualCountsTreeBuilder<Primitive>>;
		friend class Tree;
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree) {
			int size = end - begin;
			if (size == 1 || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBounds;
			for (int i = begin; i < end; ++i)
				centroidBounds.append(tree.boundsArray[tempIndices[i]]);
			int dimension = centroidBounds.maxExtentDirection();
			if (centroidBounds.max()[dimension] == centroidBounds.min()[dimension]) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			int mid = (begin + end) / 2;
			std::nth_element(&tempIndices[begin], &tempIndices[mid],
				&tempIndices[end - 1] + 1,
				[dimension, bounds = tree.boundsArray](int a, int b) {
				return bounds[a].center()[dimension] < bounds[b].center()[dimension];
			});
			AABB left;
			tree.nodes.emplace_back(Tree::Node::Type::Inter, 0, 0, bounds);
			std::vector<int> leftIndices(mid - begin);
			for (int i = begin; i < mid; ++i) {
				left.append(tree.boundsArray[tempIndices[i]]);
				leftIndices[i - begin] = tempIndices[i];
			}
			tree.nodes[nodeId].firstChild = nodeId + 1;
			build(0, mid - begin, depth - 1, left, nodeId + 1, leftIndices, tree);
			AABB right;
			for (int i = mid; i < end; ++i) {
				right.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(mid, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);
		}
	};
	template<class Primitive>
	class SplitBucketSAHTreeBuilder {};
	template<class Primitive>
	class SplitSAHMaxDirTreeBuilder {};

	template<class Primitive>
	class SAHTreeBuilder {
		using Tree = AABBTree<Primitive, SAHTreeBuilder<Primitive>>;
		friend class Tree;
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree) {
			int size = end - begin;
			if (size == 1 || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBounds;
			for (int i = begin; i < end; ++i)
				centroidBounds.append(tree.boundsArray[tempIndices[i]]);
			float bboxArea = bounds.area();
			int minDimension = 0;
			float minCostPlane = -1;
			float minCost = std::numeric_limits <float>::max();
			for (int dimension = 0; dimension < 3; dimension++) {
				if (centroidBounds.max()[dimension] == centroidBounds.min()[dimension])
					continue;
				const float TTraverse = 0.125f;
				const float TInters = 1.0f;
				for (int i = begin; i < end; ++i) {
					int primitiveIDFirst = tempIndices[i];
					float min = tree.boundsArray[primitiveIDFirst].min()[dimension];
					float max = tree.boundsArray[primitiveIDFirst].max()[dimension];
					for (float plane : { min, max}) {
						int leftItems = 0;
						int rightItems = 0;
						float costLeft = 0.0f;
						AABB left;
						AABB right;
						float costRight = 0.0f;
						for (int j = begin; j < end; ++j) {
							int primitiveID = tempIndices[j];
							if (tree.boundsArray[primitiveID].min()[dimension] <= plane) {
								costLeft += TInters;
								left.append(tree.boundsArray[primitiveID]);
								leftItems++;
							}
							if (tree.boundsArray[primitiveID].max()[dimension] >= plane) {
								costRight += TInters;
								right.append(tree.boundsArray[primitiveID]);
								rightItems++;
							}
						}
						if (leftItems == 0 || leftItems == size || rightItems == 0 || rightItems == size)
							continue;
						float leftArea = left.area();
						float rightArea = right.area();
						if ((bboxArea - leftArea) < Epsilon || (bboxArea - rightArea) < Epsilon)
							continue;
						float cost = TTraverse + (costLeft * left.area() + costRight * right.area()) / bboxArea;
						if (cost <= minCost) {
							minCost = cost;
							minCostPlane = plane;
							minDimension = dimension;
						}
					}
				}
				if (minCostPlane == -1 || minCost > TTraverse + size * TInters) {
					tree.addLeaf(begin, end, depth, bounds, tempIndices);
					return;
				}
			}
			AABB left;
			AABB right;
			int rightStart = end;
			int leftEnd = begin;

			for (int i = begin; i < rightStart && leftEnd != end && rightStart != begin; ++i) {
				int primitiveID = tempIndices[i];
				bool isLeft = false;
				if (tree.boundsArray[primitiveID].min()[minDimension] <= minCostPlane) {
					left.append(tree.boundsArray[primitiveID]);
					isLeft = true;
				}
				bool isRight = false;
				if (tree.boundsArray[primitiveID].max()[minDimension] >= minCostPlane) {
					right.append(tree.boundsArray[primitiveID]);
					isRight = true;
				}
				if (isLeft && !isRight) {
					if (leftEnd != i) {
						std::swap(tempIndices[leftEnd++], tempIndices[i]);
					}
					else
						leftEnd++;
				}
				else if (isRight && !isLeft) {
					if (rightStart - 1 != i) {
						std::swap(tempIndices[--rightStart], tempIndices[i]);
						--i;
					}
					else
						rightStart--;
				}
			}
			if (rightStart == end || rightStart == begin || leftEnd == begin || leftEnd == end) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			tree.nodes.emplace_back(Tree::Node::Type::Inter, 0, 0, bounds);
			std::vector<int> leftIndices(rightStart - begin);
			for (int i = 0; i < rightStart - begin; ++i) {
				leftIndices[i] = tempIndices[begin + i];
			}
			// compiler keeps optimizing int nodeId = tree.nodes.size() away so put it explicitly
			tree.nodes[nodeId].firstChild = nodeId + 1;
			build(0, rightStart - begin, depth - 1, left, nodeId + 1, leftIndices, tree);
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(leftEnd, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);
		}
	};

	template<class Primitive, class TreeBuilder>
	class AABBTree : public Intersectable {
		friend class SAHTreeBuilder<Primitive>;
		friend class SplitEqualCountsTreeBuilder<Primitive>;
	private:
		struct Node {
			enum Type : unsigned int {
				Leaf = 0,
				Inter = 1
			};
			unsigned int type : 1, firstChild : 31;
			unsigned int secondChild;
			AABB bbox;
			Node(unsigned int type, unsigned int firstChild, unsigned int secondChild, const BBox3D& bbox)
				:type(type), firstChild(firstChild), secondChild(secondChild), bbox(bbox)
			{}
		};
		std::vector<int> indices;
		std::vector<AABB> boundsArray;
		std::vector<Primitive*> primitives;
		std::vector<Node> nodes;
		AABB rootBounds;
		int maxPrimitiveInNode;
		void addLeaf(int begin, int end, int depth, const AABB& bounds, std::vector<int>& tempIndices) {
			int start = indices.size();
			for (int i = begin; i < end; ++i)
				indices.push_back(tempIndices[i]);
			nodes.emplace_back(Node::Type::Leaf, start, indices.size(), bounds);
		}
		//int build(int begin, int end, int depth, const AABB& bounds, std::vector<int>& tempIndices);
	public:
		template <class Iterator>
		AABBTree(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get, int maxDepth = -1, int maxPrimitiveInNode = 1);
		template <class Iterator>
		AABBTree(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int maxDepth = -1, int maxPrimitiveInNode = 1);
		template <class Iterator>
		AABBTree(Iterator begin, Iterator end, int maxDepth = -1, int maxPrimitiveInNode = 1);
		template<class Object>
		std::vector<int> intersectedIndicies(const Object& object) const;
		virtual bool intersect(const Ray& ray) const override;
		virtual bool intersect(Ray& ray, HitInfo& hitInfo) const override;
		virtual AABB bbox() const override;
		void print() const;
		bool test() const;
	};

#if 0
	template<class Primitive, class TreeBuilder>
	int AABBTree<Primitive, TreeBuilder>::build(int begin, int end, int depth, const AABB& bounds, std::vector<int>& tempIndices) {
		int nodeId = nodes.size();
		int size = end - begin;
		if (size == 1 || depth == 0) {
			// add leaf
			int start = indices.size();
			for (int i = begin; i < end; ++i)
				indices.push_back(tempIndices[i]);
			nodes.emplace_back(Node::Type::Leaf, start, indices.size(), bounds);
			return nodeId;
		}
		AABB centroidBounds;
		for (int i = begin; i < end; ++i)
			centroidBounds.append(boundsArray[tempIndices[i]]);
		/*if (splitType == SAH) {
			int minDimension = 0;
			float minPlane = 0;
			float minCost = std::numeric_limits<double>;
			for (int dimension = 0; dimension < 3; dimension++)
			{
			}
		}
		else if (splitType == SplitType::SimpleSAH) {

		}
		else if (splitType == SplitType::BucketSAH) {


		}
		else if (splitType == SplitType::EqualCount) {

		}
		else if (splitType == SplitType::Middle) {

		}*/
		int dimension = bounds.maxExtentDirection();
		if (centroidBounds.max()[dimension] == centroidBounds.min()[dimension]) {
			// add leaf
			int start = indices.size();
			for (int i = begin; i < end; ++i)
				indices.push_back(tempIndices[i]);
			nodes.emplace_back(Node::Type::Leaf, start, indices.size(), bounds);
			return;
		}
		// could assign individual values for different primitives
		const float TTraverse = 0.125f;
		const float TInters = 1.0f;
		float minCost = prevLevelCost;
		float minCostPlane = 0.0f;
		bool foundPlane = false;
		float bboxArea = bounds.area();
		for (int i = begin; i < end; ++i) {
			int primitiveIDFirst = tempIndices[i];
			float min = boundsArray[primitiveIDFirst].min()[dimension];
			float max = boundsArray[primitiveIDFirst].max()[dimension];
			for (float plane : { min, max}) {
				int leftItems = 0;
				int rightItems = 0;
				float costLeft = 0.0f;
				AABB left;
				AABB right;
				float costRight = 0.0f;
				for (int j = begin; j < end; ++j) {
					int primitiveID = tempIndices[j];
					if (boundsArray[primitiveID].min()[dimension] <= plane) {
						costLeft += TInters;
						left.append(boundsArray[primitiveID]);
						leftItems++;
					}
					if (boundsArray[primitiveID].max()[dimension] >= plane) {
						costRight += TInters;
						right.append(boundsArray[primitiveID]);
						rightItems++;
					}
				}
				if (leftItems == 0 || leftItems == size || rightItems == 0 || rightItems == size)
					continue;
				float leftArea = left.area();
				float rightArea = right.area();
				if ((bboxArea - leftArea) < Epsilon || (bboxArea - rightArea) < Epsilon)
					continue;
				float cost = TTraverse + (costLeft * left.area() + costRight * right.area()) / bboxArea;
				if (cost <= minCost) {
					minCost = cost;
					minCostPlane = plane;
					foundPlane = true;
				}
			}
		}
		if (!foundPlane) {
			int start = indices.size();
			for (int i = begin; i < end; ++i)
				indices.push_back(tempIndices[i]);
			nodes.emplace_back(Node::Type::Leaf, start, indices.size(), bounds);
			return;
		}
		AABB left;
		AABB right;
		int rightStart = end;
		int leftEnd = begin;
#ifdef DEBUG
		std::vector<int> classified(size, 0);
#endif
		//TODO: check and write this part
		// i think this should work fine, need to test it
		for (int i = begin; i < rightStart && leftEnd != end && rightStart != begin; ++i) {
			int primitiveID = tempIndices[i];
			bool isLeft = false;
			if (boundsArray[primitiveID].min()[dimension] <= minCostPlane) {
				left.append(boundsArray[primitiveID]);
				isLeft = true;
#ifdef DEBUG
				classified[i - begin] |= 1;
#endif
			}
			bool isRight = false;
			if (boundsArray[primitiveID].max()[dimension] >= minCostPlane) {
				right.append(boundsArray[primitiveID]);
				isRight = true;
#ifdef DEBUG
				classified[i - begin] |= 2;
#endif
			}
			if (isLeft && !isRight) {
				if (leftEnd != i) {
#ifdef DEBUG
					std::swap(classified[leftEnd - begin], classified[i - begin]);
#endif
					std::swap(tempIndices[leftEnd++], tempIndices[i]);
				}
				else
					leftEnd++;
			}
			else if (isRight && !isLeft) {
				if (rightStart - 1 != i) {
#ifdef DEBUG
					std::swap(classified[rightStart - 1 - begin], classified[i - begin]);
#endif
					std::swap(tempIndices[--rightStart], tempIndices[i]);
					--i;
				}
				else
					rightStart--;
			}
		}
#ifdef DEBUG
		for (int k = 0; k < size - 1; k++) {
			assert(!(classified[k] == 2 && classified[k + 1] == 3));
			assert(!(classified[k] == 2 && classified[k + 1] == 1));
			assert(!(classified[k] == 3 && classified[k + 1] == 1));
		}
#endif
		if (rightStart == end || rightStart == begin || leftEnd == begin || leftEnd == end) {
			int start = indices.size();
			for (int i = begin; i < end; ++i)
				indices.push_back(tempIndices[i]);
			nodes.emplace_back(Node::Type::Leaf, start, indices.size(), bounds);
			return;
		}
		nodes.emplace_back(Node::Type::Inter, 0, 0, bounds);
		nodes[nodeId].firstChild = build(begin, rightStart, depth - 1, left, tempIndices, splitType);
		nodes[nodeId].secondChild = build(leftEnd, end, depth - 1, right, tempIndices, splitType);
		return nodeId;
	}
#endif
	template<class Primitive, class Builder>
	template <class Iterator>
	AABBTree<Primitive, Builder>::AABBTree(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get, int maxDepth, int maxPrimitiveInNode) {
		int primitivesCount = std::distance(begin, end);
		std::vector<int> tempIndices;
		tempIndices.reserve(primitivesCount);
		primitives.reserve(primitivesCount);
		boundsArray.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back(get(it));
			boundsArray.push_back(primitives.back()->bbox());
			rootBounds.append(boundsArray.back());
			tempIndices.push_back(tempIndices.size());
		}
		// from pbrt book
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(primitives.size()));
		Builder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
		//build(0, primitivesCount, maxDepth - 1, rootBounds, std::numeric_limits<float>::max(), 0, tempIndices);
	}

	template<class Primitive, class Builder>
	template <class Iterator>
	AABBTree<Primitive, Builder>::AABBTree(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int maxDepth, int maxPrimitiveInNode) {
		int primitivesCount = std::distance(begin, end);
		primitives.reserve(primitivesCount);
		boundsArray.reserve(primitivesCount);
		std::vector<int> tempIndices;
		tempIndices.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push(get(it));
			boundsArray.push_back(primitives.back()->bbox());
			rootBounds.append(boundsArray.back());
			tempIndices.push_back(tempIndices.size());
		}
		// from pbrt book
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(primitives.size()));
		//build(0, primitivesCount, maxDepth - 1, rootBounds, std::numeric_limits<float>::max(), 0, tempIndices);
		Builder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
	}

	template<class Primitive, class Builder>
	template <class Iterator>
	AABBTree<Primitive, Builder>::AABBTree(Iterator begin, Iterator end, int maxDepth, int maxPrimitiveInNode) {
		int primitivesCount = std::distance(begin, end);
		std::vector<int> tempIndices;
		tempIndices.reserve(primitivesCount);
		primitives.reserve(primitivesCount);
		boundsArray.reserve(primitivesCount);
		for (auto it = begin; it != end; ++it) {
			primitives.push_back(&(*it));
			boundsArray.push_back(it->bbox());
			rootBounds.append(boundsArray.back());
			tempIndices.push_back(tempIndices.size());
		}
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(primitives.size()));
		//build(0, primitivesCount, maxDepth - 1, rootBounds, std::numeric_limits<float>::max(), 0, tempIndices);
		Builder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
	}

	template<class Primitive, class Builder>
	AABB AABBTree<Primitive, Builder>::bbox() const {
		return rootBounds;
	}

	template<class Primitive, class Builder>
	template<class Object>
	std::vector<int> AABBTree<Primitive, Builder>::intersectedIndicies(const Object& object) const {
		std::vector<int> result;
		if (nodes.empty())
			return result;
		std::stack<int> nodeStack;
		nodeStack.push(0);
		while (!nodeStack.empty()) {
			const Node& current = nodes[nodeStack.top()];
			nodeStack.pop();
			if (!current.bbox.intersect(object))
				continue;
			if (current.type != Node::Leaf) {
				nodeStack.push(current.secondChild);
				nodeStack.push(current.firstChild);
				continue;
			}
			for (int i = current.firstChild; i < current.secondChild; ++i) {
				if (primitives[indices[i]]->intersect(object))
					result.push_back(indices[i]);
			}
		}
		return result;
	}

	template<class Primitive, class Builder>
	bool AABBTree<Primitive, Builder>::intersect(const Ray& ray) const {
		if (nodes.empty())
			return false;
		std::stack<int> nodeStack;
		nodeStack.push(0);
		while (!nodeStack.empty()) {
			const Node& current = nodes[nodeStack.top()];
			nodeStack.pop();
			if (!current.bbox.intersect(ray))
				continue;
			if (current.type != Node::Leaf) {
				nodeStack.push(current.secondChild);
				nodeStack.push(current.firstChild);
				continue;
			}
			for (int i = current.firstChild; i < current.secondChild; ++i) {
				if (primitives[indices[i]]->intersect(ray))
					return true;
			}
		}
		return false;
	}
	template<class Primitive, class Builder>
	bool AABBTree<Primitive, Builder>::intersect(Ray& ray, HitInfo& hitInfo) const {
		if (nodes.empty())
			return false;
		bool result = false;
		std::stack<int> nodeStack;
		nodeStack.push(0);
		while (!nodeStack.empty()) {
			const Node& current = nodes[nodeStack.top()];
			nodeStack.pop();
			if (!current.bbox.intersect(ray))
				continue;
			if (current.type != Node::Leaf) {
				nodeStack.push(current.secondChild);
				nodeStack.push(current.firstChild);
				continue;
			}
			for (int i = current.firstChild; i < current.secondChild; ++i)
				result = primitives[indices[i]]->intersect(ray, hitInfo) || result;
		}
		return result;
	}
	template<class Primitive, class Builder>
	void AABBTree<Primitive, Builder>::print() const {
		int index = 0;
		for (auto primitive : primitives) {
			AABB aabb = primitive->bbox();
			printf("primitive %d: (%f, %f, %f), (%f, %f, %f) \n", index++, aabb.min().x, aabb.min().y, aabb.min().z,
				aabb.max().x, aabb.max().y, aabb.max().z);
		}
		std::stack<int> nodeStack;
		nodeStack.push(0);
		while (!nodeStack.empty()) {
			int nodeId = nodeStack.top();
			const Node& current = nodes[nodeStack.top()];
			nodeStack.pop();
			const AABB& aabb = current.bbox;
			if (current.type != Node::Leaf) {
				nodeStack.push(current.secondChild);
				nodeStack.push(current.firstChild);
				printf("Node %d\n\tChild nodes: %d, %d\n", nodeId, current.firstChild, current.secondChild);
			}
			else
				printf("Leaf %d\n\tChild primitives: [%d : %d)\n", nodeId, current.firstChild, current.secondChild);
			printf("\t AABB: (%f, %f, %f), (%f, %f, %f) \n", aabb.min().x, aabb.min().y, aabb.min().z,
				aabb.max().x, aabb.max().y, aabb.max().z);
		}
	}
	template<class Primitive, class Builder>
	bool AABBTree<Primitive, Builder>::test() const {
		if (nodes.empty())
			return true;
		bool result = true;
		std::stack<int> nodeStack;
		nodeStack.push(0);
		while (!nodeStack.empty()) {
			const Node& current = nodes[nodeStack.top()];
			nodeStack.pop();
			if (current.type != Node::Leaf) {
				nodeStack.push(current.secondChild);
				nodeStack.push(current.firstChild);
				if (!current.bbox.contains(nodes[current.secondChild].bbox) || !current.bbox.contains(nodes[current.firstChild].bbox))
					return false;
				continue;
			}
			for (int i = current.firstChild; i < current.secondChild; ++i) {
				if (!current.bbox.contains(primitives[indices[i]]->bbox()))
					return false;
			}
		}
		return result;
	}
}