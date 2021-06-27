#pragma once
#include "Base.h"
#include <functional>

namespace PrimitiveLocators {
	template<class Primitive, class TreeBuilder>
	class AABBTree;

	/// Separate axis on two equal width halfs
	template<class Primitive, int minPrims = 1, bool useMaxDir = false>
	class MiddleTreeBuilder {
		using Tree = AABBTree<Primitive, MiddleTreeBuilder<Primitive, minPrims, useMaxDir>>;
		friend class Tree;
	protected:
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree) {
			int numPrims = end - begin;
			if (numPrims <= minPrims || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBBox;
			for (int i = begin; i < end; ++i)
				centroidBBox.append(tree.boundsArray[tempIndices[i]]);
			int dim = centroidBBox.maxExtentDirection();
			if (centroidBBox.max()[dim] == centroidBBox.min()[dim]) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			float middlePlane = 0.5 * (bounds.max() + bounds.min());

			int* midPtr = std::partition(&tempIndices[begin], &tempIndices[end - 1] + 1,
				[dim, middlePlane, &bounds = tree.boundsArray](int index) {
				return bounds[index].center()[dim] < middlePlane;
			});
			int middle = midPtr - &tempIndices.begin()[0];
			if (middle == begin && middle == end) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB left;
			tree.nodes.emplace_back(Tree::Node::Type::Inter, 0, 0, bounds);
			for (int i = begin; i < middle; ++i) {
				left.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].firstChild = nodeId + 1;
			build(begin, middle, depth - 1, left, nodeId + 1, tempIndices, tree);
			AABB right;
			for (int i = middle; i < end; ++i) {
				right.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(middle, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);
		}
	};
	// TODO test
	/// divide
	template<class Primitive, int minPrims = 1>
	class EqualCountsTreeBuilder {
		using Tree = AABBTree<Primitive, EqualCountsTreeBuilder<Primitive, minPrims>>;
		friend class Tree;
	protected:
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree) {
			int numPrims = end - begin;
			if (numPrims <= minPrims || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBBox;
			for (int i = begin; i < end; ++i)
				centroidBBox.append(tree.boundsArray[tempIndices[i]]);
			int dim = centroidBBox.maxExtentDirection();
			if (centroidBBox.max()[dim] == centroidBBox.min()[dim]) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			int mid = (begin + end) / 2;
			std::nth_element(&tempIndices[begin], &tempIndices[mid],
				&tempIndices[end - 1] + 1,
				[&dim, &bounds = tree.boundsArray](int a, int b) {
				return bounds[a].center()[dim] < bounds[b].center()[dim];
			});
			AABB left;
			tree.nodes.emplace_back(Tree::Node::Type::Inter, 0, 0, bounds);
			for (int i = begin; i < mid; ++i) {
				left.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].firstChild = nodeId + 1;
			build(begin, mid, depth - 1, left, nodeId + 1, tempIndices, tree);
			AABB right;
			for (int i = mid; i < end; ++i) {
				right.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(mid, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);
		}
	};
	// todo: TEST
	/// Bucketed Surface-Area-Heuristic builder
	template<class Primitive, int minPrims = 1, bool useMaxDir = false, int bucketSize = 12>
	class BucketSAHTreeBuilder {
		using Tree = AABBTree<Primitive, BucketSAHTreeBuilder<Primitive, minPrims, useMaxDir, bucketSize>>;
		friend class Tree;
	protected:
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree, int minPrims) {
			int numPrims = end - begin;
			if (numPrims <= minPrims || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBBox;
			for (int i = begin; i < end; ++i)
				centroidBBox.append(tree.boundsArray[tempIndices[i]]);

			struct Bucket {
				AABB bbox;
				int count = 0;
			};

			float bboxArea = bounds.area();

			float minCost = std::numeric_limits<float>::max();
			int minCostPlane = -1;
			int minDim = -1;
			for (int dim = 0; dim < 3; ++dim) {
				if (centroidBBox.max()[dim] == centroidBBox.min()[dim])
					continue;
				float bucketWidth = centroidBBox.size()[dim] / float(bucketSize);
				Bucket buckets[bucketSize];
				for (int i = begin; i != end; ++i) {
					int primitiveId = tempIndices[i];
					float centroid = tree.boundsArray[primitiveId].center()[dim] - centroidBBox.min()[dim];
					int bucketId = std::min(bucketSize - 1, centroid / bucketWidth);
					buckets[bucketId].count++;
					buckets[bucketId].bbox.append(tree.boundsArray[primitiveId]);
				}
				for (int planeId = 1; planeId < bucketSize; ++planeId) {
					int leftItems = 0;
					int rightItems = 0;
					AABB leftBBox;
					AABB rightBBox;
					for (int id = 0; id < planeId; ++id) {
						leftItems += buckets[id].count;
						leftBBox.append(buckets[id].bbox);
					}
					for (int id = planeId; id < bucketSize; ++id) {
						rightItems += buckets[id].count;
						rightBBox.append(buckets[id].bbox);
					}

					const float TTraverse = 0.125f;
					const float TInters = 1.0f;
					float cost = TTraverse + (leftItems * TInters * leftBBox.area() + rightItems * TTraverse * rightBBox.area()) / bboxArea;
					if (cost <= minCost) {
						minCost = cost;
						minCostPlane = planeId;
						minDim = dim;
					}
				}
			}
			if (minCostPlane == -1 || minCost > numPrims) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			float bucketWidth = centroidBBox.size()[minDim] / float(bucketSize);
			int* midPtr = std::partition(&tempIndices[begin],
				&tempIndices[end - 1] + 1,
				[minDim, bucketWidth, minCostPlane, &tempIndices](int id) {
				int primitiveId = tempIndices[id];
				float centroid = tree.boundsArray[primitiveId].center()[minDim] - centroidBBox.min()[minDim];
				int bucketId = std::min(bucketSize - 1, centroid / bucketWidth);
				return bucketId <= minCostPlane;
			});
			int middle = midPtr - &tempIndices.begin();
			//NOT_IMPLEMENTED();
			AABB left;
			tree.nodes.emplace_back(Tree::Node::Type::Inter, 0, 0, bounds);
			for (int i = begin; i < middle; ++i) {
				left.append(tree.boundsArray[tempIndices[i]]);
			}
			build(begin, middle, depth - 1, left, nodeId + 1, tempIndices, tree);
			AABB right;
			for (int i = middle; i < end; ++i) {
				right.append(tree.boundsArray[tempIndices[i]]);
			}
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(middle, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);

		}

	};

	// FULL SAH with overlapping boxes, slow as fuck
	template<class Primitive, int minPrims = 1, bool useMaxDir = false>
	class FullSAHTreeBuilder {
		using Tree = AABBTree<Primitive, FullSAHTreeBuilder<Primitive, minPrims, useMaxDir>>;
		friend class Tree;
	protected:
		static void build(int begin, int end, int depth, const AABB& bounds, int nodeId, std::vector<int>& tempIndices, Tree& tree) {
			int numPrims = end - begin;
			if (numPrims <= minPrims || depth == 0) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB centroidBBox;
			for (int i = begin; i < end; ++i)
				centroidBBox.append(tree.boundsArray[tempIndices[i]]);
			float bboxArea = bounds.area();
			int minCostDim = 0;
			float minCostPlane = -1;
			float minCost = std::numeric_limits<float>::max();
			int minDim = 0;
			int maxDim = 2;
			if (useMaxDir) {
				minDim = maxDim = bounds.maxExtentDirection();
			}
			for (int dim = minDim; dim <= maxDim; dim++) {
				if (centroidBBox.max()[dim] == centroidBBox.min()[dim])
					continue;
				for (int i = begin; i < end; ++i) {
					int primitiveIdFirst = tempIndices[i];
					float min = tree.boundsArray[primitiveIdFirst].min()[dim];
					float max = tree.boundsArray[primitiveIdFirst].max()[dim];
					for (float plane : { min, max}) {
						int leftItems = 0;
						int rightItems = 0;
						float costLeft = 0.0f;
						AABB left;
						AABB right;
						float costRight = 0.0f;
						const float TTraverse = 0.125f;
						const float TInters = 1.0f;
						for (int j = begin; j < end; ++j) {
							int primitiveId = tempIndices[j];
							if (tree.boundsArray[primitiveId].min()[dim] <= plane) {
								costLeft += TInters;
								left.append(tree.boundsArray[primitiveId]);
								leftItems++;
							}
							if (tree.boundsArray[primitiveId].max()[dim] >= plane) {
								costRight += TInters;
								right.append(tree.boundsArray[primitiveId]);
								rightItems++;
							}
						}
						if (leftItems == 0 || leftItems == numPrims || rightItems == 0 || rightItems == numPrims)
							continue;
						float leftArea = left.area();
						float rightArea = right.area();
						if ((bboxArea - leftArea) < Epsilon || (bboxArea - rightArea) < Epsilon)
							continue;
						const float TTraverse = 0.125f;
						const float TInters = 1.0f;
						float cost = TTraverse + (costLeft * left.area() + costRight * right.area()) / bboxArea;
						if (cost <= minCost) {
							minCost = cost;
							minCostPlane = plane;
							minCostDim = dim;
						}
					}
				}
			}
			if (minCostPlane == -1 || minCost > numPrims) {
				tree.addLeaf(begin, end, depth, bounds, tempIndices);
				return;
			}
			AABB left;
			AABB right;
			int rightStart = end;
			int leftEnd = begin;

			for (int i = begin; i < rightStart && leftEnd != end && rightStart != begin; ++i) {
				int primitiveId = tempIndices[i];
				bool isLeft = false;
				if (tree.boundsArray[primitiveId].min()[minCostDim] <= minCostPlane) {
					left.append(tree.boundsArray[primitiveId]);
					isLeft = true;
				}
				bool isRight = false;
				if (tree.boundsArray[primitiveId].max()[minCostDim] >= minCostPlane) {
					right.append(tree.boundsArray[primitiveId]);
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
			// compiler keeps optimizing int nodeId = tree.nodes.size() away and breaking this method so put it explicitly
			tree.nodes[nodeId].firstChild = nodeId + 1;
			build(0, rightStart - begin, depth - 1, left, nodeId + 1, leftIndices, tree);
			tree.nodes[nodeId].secondChild = tree.nodes.size();
			build(leftEnd, end, depth - 1, right, tree.nodes.size(), tempIndices, tree);
		}
	};

	/// AABB tree for primitives with "AABB bbox()" method
	template<class Primitive, class TreeBuilder>
	class AABBTree : public Base<Primitive> {
		template<class Primitive, int minPrims, bool useMaxDir>
		friend class MiddleTreeBuilder;
		template<class Primitive, int minPrims>
		friend class EqualCountsTreeBuilder;
		template<class Primitive, int minPrims, bool useMaxDir, int bucketSize>
		friend class BucketSAHTreeBuilder;
		template<class U, int minPrims, bool useMaxDir>
		friend class FullSAHTreeBuilder;
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
		AABBTree(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get, int maxDepth = -1);
		template <class Iterator>
		AABBTree(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int maxDepth = -1);
		template <class Iterator>
		AABBTree(Iterator begin, Iterator end, int maxDepth = -1);
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
		AABB centroidBBox;
		for (int i = begin; i < end; ++i)
			centroidBBox.append(boundsArray[tempIndices[i]]);
		int dimension = bounds.maxExtentDirection();
		if (centroidBBox.max()[dimension] == centroidBBox.min()[dimension]) {
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
			int primitiveIdFirst = tempIndices[i];
			float min = boundsArray[primitiveIdFirst].min()[dimension];
			float max = boundsArray[primitiveIdFirst].max()[dimension];
			for (float plane : { min, max}) {
				int leftItems = 0;
				int rightItems = 0;
				float costLeft = 0.0f;
				AABB left;
				AABB right;
				float costRight = 0.0f;
				for (int j = begin; j < end; ++j) {
					int primitiveId = tempIndices[j];
					if (boundsArray[primitiveId].min()[dimension] <= plane) {
						costLeft += TInters;
						left.append(boundsArray[primitiveId]);
						leftItems++;
					}
					if (boundsArray[primitiveId].max()[dimension] >= plane) {
						costRight += TInters;
						right.append(boundsArray[primitiveId]);
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
			int primitiveId = tempIndices[i];
			bool isLeft = false;
			if (boundsArray[primitiveId].min()[dimension] <= minCostPlane) {
				left.append(boundsArray[primitiveId]);
				isLeft = true;
#ifdef DEBUG
				classified[i - begin] |= 1;
#endif
			}
			bool isRight = false;
			if (boundsArray[primitiveId].max()[dimension] >= minCostPlane) {
				right.append(boundsArray[primitiveId]);
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
	template<class Primitive, class TreeBuilder>
	template <class Iterator>
	AABBTree<Primitive, TreeBuilder>::AABBTree(Iterator begin, Iterator end, std::function<Primitive*(Iterator&)> get, int maxDepth) {
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
		// heuristic from pbrt book
		if (maxDepth <= 0)
			maxDepth = std::round(8 + 1.3f * glm::log2(primitives.size()));
		TreeBuilder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
		//build(0, primitivesCount, maxDepth - 1, rootBounds, std::numeric_limits<float>::max(), 0, tempIndices);
	}

	template<class Primitive, class TreeBuilder>
	template <class Iterator>
	AABBTree<Primitive, TreeBuilder>::AABBTree(Iterator begin, Iterator end, Primitive*(*get)(Iterator&), int maxDepth) {
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
		TreeBuilder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
	}

	template<class Primitive, class TreeBuilder>
	template <class Iterator>
	AABBTree<Primitive, TreeBuilder>::AABBTree(Iterator begin, Iterator end, int maxDepth) {
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
		TreeBuilder::build(0, primitivesCount, maxDepth - 1, rootBounds, 0, tempIndices, *this);
	}

	template<class Primitive, class TreeBuilder>
	AABB AABBTree<Primitive, TreeBuilder>::bbox() const {
		return rootBounds;
	}

	template<class Primitive, class TreeBuilder>
	template<class Object>
	std::vector<int> AABBTree<Primitive, TreeBuilder>::intersectedIndicies(const Object& object) const {
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

	template<class Primitive, class TreeBuilder>
	bool AABBTree<Primitive, TreeBuilder>::intersect(const Ray& ray) const {
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
	template<class Primitive, class TreeBuilder>
	bool AABBTree<Primitive, TreeBuilder>::intersect(Ray& ray, HitInfo& hitInfo) const {
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
	template<class Primitive, class TreeBuilder>
	void AABBTree<Primitive, TreeBuilder>::print() const {
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
	template<class Primitive, class TreeBuilder>
	bool AABBTree<Primitive, TreeBuilder>::test() const {
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