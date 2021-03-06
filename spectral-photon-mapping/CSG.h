#pragma once
#include <set>
#include <stack>
#include <memory>
#include <vector>
#include "Transform.h"
#include "Shape.h"


namespace CSG
{
	class Node {
	protected:
		BBox3D cachedBBox;
	public:
		virtual std::vector<Node*> getChilds() const = 0;
		BBox3D bbox() const {
			return cachedBBox;
		};
		virtual std::vector<Interval> intersect(const Ray& ray) const = 0;
	};
	using spNode = std::shared_ptr<Node>;

	class OperatorNode : public Node {
	public:
		OperatorNode(const spNode& left, const spNode& right) : leftNode(left), rightNode(right) {
		}
	protected:
		spNode leftNode;
		spNode rightNode;
	};

	class PrimitiveNode : public Node {
		std::shared_ptr<Shape> shape;
		Affine transform;
	public:
		PrimitiveNode(const std::shared_ptr<Shape>& shape, const Affine& transform)
			:shape(shape), transform(transform) {
			cachedBBox = transform.transform(shape->bbox());
		}
		std::vector<Node*> getChilds() const override {
			return std::vector<Node*>();
		}
	};

	class SubtractionNode : protected OperatorNode {
		std::vector<Interval> op(const std::vector<Interval>& leftIntersections, const std::vector<Interval>& rightIntersections) const {
			throw std::runtime_error("Not implemented");
		}
	public:
		SubtractionNode(const spNode& left, const spNode& right):OperatorNode(left, right) {
			cachedBBox = subtractionOp(leftNode->bbox(), rightNode->bbox());
		}
		std::vector<Node*> getChilds() const override {
			return std::vector<Node*>({ leftNode.get(), rightNode.get() });
		}
		std::vector<Interval> intersect(const Ray& ray) const override {
			std::vector<Interval> leftIntersections = leftNode->intersect(ray);
			std::vector<Interval> rightIntersections = rightNode->intersect(ray);
			if (rightIntersections.empty())
				return leftIntersections;
			return op(leftIntersections, rightIntersections);
		}
	};

	class UnionNode : protected OperatorNode {
		std::vector<Interval> op(const std::vector<Interval>& leftIntersections, const std::vector<Interval>& rightIntersections) const {
			throw std::runtime_error("Not implemented");
		}
	public:
		UnionNode(const spNode& left, const spNode& right) :OperatorNode(left, right) {
			cachedBBox = unionOp(leftNode->bbox(), rightNode->bbox());
		}
		std::vector<Node*> getChilds() const override {
			return std::vector<Node*>({ leftNode.get(), rightNode.get() });
		}
		std::vector<Interval> intersect(const Ray& ray) const override {
			std::vector<Interval> leftIntersections = leftNode->intersect(ray);
			std::vector<Interval> rightIntersections = rightNode->intersect(ray);
			if (rightIntersections.empty())
				return leftIntersections;
			if (leftIntersections.empty())
				return rightIntersections;
			return op(leftIntersections, rightIntersections);
		}
	};

	class IntersectionNode : public OperatorNode {
		std::vector<Interval> op(const std::vector<Interval>& leftIntersections, const std::vector<Interval>& rightIntersections) const {
			throw std::runtime_error("Not implemented");
		}
	public:
		IntersectionNode(const spNode& left, const spNode& right) :OperatorNode(left, right) {
			cachedBBox = intersectionOp(leftNode->bbox(), rightNode->bbox());
		}
		std::vector<Node*> getChilds() const override {
			return std::vector<Node*>({ leftNode.get(), rightNode.get() });
		}
		std::vector<Interval> intersect(const Ray& ray) const override {
			std::vector<Interval> leftIntersections = leftNode->intersect(ray);
			std::vector<Interval> rightIntersections = rightNode->intersect(ray);
			if (rightIntersections.empty())
				return rightIntersections;
			if (leftIntersections.empty())
				return leftIntersections;
			return op(leftIntersections, rightIntersections);
		}
	};

	class Tree : public Shape {
		void computeBBox() {
			cachedBBox = root->bbox();
		}
	public:
		BBox3D bbox() const override {
			return cachedBBox;
		}
		bool intersect(const Ray& ray) const override {
			if (!bbox().intersect(ray))
				return false;
			auto intervals = root->intersect(ray);
			return !intervals.empty();
		}
		bool intersect(const Ray& ray, HitInfo& hitInfo) const override {
			if (!bbox().intersect(ray))
				return false;
			auto intervals = root->intersect(ray);
			for (auto interval : intervals) {
				if (interval.min > ray.tMax)
					return false;
				if (interval.min >= ray.tMin) {
					hitInfo.t = interval.min;
					hitInfo.normal = interval.minNormal;
					hitInfo.localPosition = ray.ro + ray.rd * hitInfo.t;
					return true;
				}
				if (interval.max > ray.tMax)
					return false;
				if (interval.max >= ray.tMin) {
					hitInfo.t = interval.max;
					hitInfo.normal = interval.maxNormal;
					hitInfo.localPosition = ray.ro + ray.rd * hitInfo.t;
					return true;
				}
			}
			return false;
		}
		Tree(std::shared_ptr<Node> root) :root(root) {
			computeBBox();
		}
		bool isValid() {
			if (root == nullptr)
				return false;
			std::set<Node*> markedNodes;
			std::stack<Node*> stack;
			stack.push(root.get());
			while (!stack.empty()) {
				Node* node = stack.top();
				stack.pop();
				if (markedNodes.count(node) > 0)
					return false;
				for (auto child : node->getChilds())
					stack.push(child);
				markedNodes.insert(node);
				return true;
			}
			return true;
		}
	private:
		BBox3D cachedBBox;
		std::shared_ptr<Node> root;
	};

}

