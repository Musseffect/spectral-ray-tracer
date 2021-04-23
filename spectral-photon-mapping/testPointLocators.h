#pragma once
#include "Random.h"
#include "PointLocators.h"
#include "Timer.h"
#include "common.h"
#include <vector>

void testPointLocators()
{
	struct Point { vec3 position; vec3 coords() const { return position; } };
	const int NumberOfPoints = 500;
	std::vector<Point> points;
	points.reserve(NumberOfPoints);
	int count = 100;
	for (int i = 0; i < NumberOfPoints; ++i)
	{
		vec3 point = vec3(Random::random(count), Random::random(count), Random::random(count));
		point /= count;
		points.push_back(Point{ vec3(Random::random(), Random::random(), Random::random()) });
	}
	/*
	points.push_back(Point{vec3(4.2f, 1.2f, 0.0f) / 5.0f});
	points.push_back(Point{ vec3(3.2f, 2.3f, 0.0f) / 5.0f });
	points.push_back(Point{ vec3(4.61f, 2.3f, 0.0f) / 5.0f });
	points.push_back(Point{ vec3(0.2f, 4.3f, 0.0f) / 5.0f });
	points.push_back(Point{ vec3(2.4f, 3.15f, 0.0f) / 5.0f });
	points.push_back(Point{ vec3(4.2f, 4.3f, 0.0f) / 5.0f });
	points.push_back(Point{ vec3(297.0f / 1000.0f, 405.0f / 1000.0f, 0.0f) / 5.0f });
	*/
	PointLocators::KdTree<Point> kdTree(points.begin(), points.end(), 8);
	PointLocators::BruteForce<Point> bruteForce(points.begin(), points.end());
	PointLocators::HashGrid<Point> grid(points.begin(), points.end(), 8);
	PointLocators::AABBTree<Point> aabbTree(points.begin(), points.end(), 8);
	const int NumberOfComparisons = 1545;
	for (int j = 0; j < NumberOfComparisons; ++j)
	{
		vec3 center = vec3(Random::random(count), Random::random(count), 0.0f) / count;
		float radius = glm::mix(0.1f, 0.5f, Random::random());
		float sqrRadius = radius * radius;
		std::vector<Point> trueResult;
		for (int i = 0; i < NumberOfPoints; ++i)
		{
			if (sqrRadius >= glm::dot(center - points[i].coords(), center - points[i].coords()))
			{
				trueResult.push_back(points[i]);
			}
		}
		std::vector<Point> kdTreeResult = kdTree.pointsWithinRadius(center, radius);
		std::vector<Point> bruteForceResult = bruteForce.pointsWithinRadius(center, radius);
		std::vector<Point> gridResult = grid.pointsWithinRadius(center, radius);
		std::vector<Point> aabbResult = aabbTree.pointsWithinRadius(center, radius);
		bool kdPasses = true;
		//printf("center (%f, %f, %f), radius %f\n", center.x, center.y, center.z, radius);
		for (auto trueResultPoint : trueResult)
		{
			bool found = false;
			for (auto kdPoint : kdTreeResult)
			{
				if (glm::distance2(trueResultPoint.coords(), kdPoint.coords()) < 0.001f)
				{
					found = true;
					break;
				}
			}
			if (!found) {
				printf("distance %f, %f\n", glm::distance2(trueResultPoint.coords(), center), radius * radius);
				printf("Kd tree FAILED\n");
				break;
			}
			found = false;
			for (auto bfPoint : bruteForceResult)
			{
				if (glm::distance2(trueResultPoint.coords(), bfPoint.coords()) < 0.001f)
				{
					found = true;
					break;
				}
			}
			if (!found) {
				printf("distance %f, %f\n", glm::distance2(trueResultPoint.coords(), center), radius * radius);
				printf("Brute force FAILED\n");
				break;
			}
			found = false;
			for (auto gridPoint : gridResult)
			{
				if (glm::distance2(trueResultPoint.coords(), gridPoint.coords()) < 0.001f)
				{
					found = true;
					break;
				}
			}
			if (!found) {
				printf("distance %f, %f\n", glm::distance2(trueResultPoint.coords(), center), radius * radius);
				printf("Grid FAILED\n");
				break;
			}
			found = false;
			for (auto aabbPoint : aabbResult)
			{
				if (glm::distance2(trueResultPoint.coords(), aabbPoint.coords()) < 0.001f)
				{
					found = true;
					break;
				}
			}
			if (!found) {
				printf("distance %f, %f\n", glm::distance2(trueResultPoint.coords(), center), radius * radius);
				printf("AABB FAILED\n");
				break;
			}
		}
	}
	printf("Done\n");
}

void testPointLocatorsPerformance(int pointCount) {

	Timer<double> timer;
	struct Point { vec3 position; double payload[8]; inline const vec3& coords() const { return position; } };
	std::vector<Point> points;
	for (int i = 0; i < pointCount; i++)
		points.push_back(Point{ vec3(Random::random(), Random::random(), Random::random()) });
	timer.restart();
	for (int i = 0; i < 20; ++i) {
		PointLocators::KdTree<Point> kdTree(points.begin(), points.end(), 10, 12);
	}
	double kdTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		PointLocators::BruteForce<Point> bruteForce(points.begin(), points.end());
	}
	double bfTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		PointLocators::HashGrid<Point> hashGrid(points.begin(), points.end(), 20);
	}
	double gridTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		PointLocators::AABBTree<Point> aabb(points.begin(), points.end(), 1, 12);
	}
	double aabbTime = timer.elapsedAndRestart();

	PointLocators::KdTree<Point> kdTree(points.begin(), points.end(), 1, 12);
	double kdSearchTime = 0.0;
	timer.restart();
	for (int i = 0; i < 1000; ++i) {
		vec3 center(Random::random(), Random::random(), Random::random());
		double radius = Random::random() * 0.5 + 0.1;
		std::vector<int> indices;
		kdTree.indicesWithinRadius(center, radius, indices);
	}
	kdSearchTime += timer.elapsedAndRestart();
	timer.restart();
	PointLocators::BruteForce<Point> bruteForce(points.begin(), points.end());
	double bfSearchTime = 0.0;
	timer.restart();
	for (int i = 0; i < 1000; ++i) {
		vec3 center(Random::random(), Random::random(), Random::random());
		double radius = Random::random() * 0.5 + 0.1;
		std::vector<int> indices;
		bruteForce.indicesWithinRadius(center, radius, indices);
	}
	bfSearchTime += timer.elapsedAndRestart();
	timer.restart();
	PointLocators::HashGrid<Point> grid(points.begin(), points.end(), 20);
	double gridSearchTime = 0.0;
	timer.restart();
	for (int i = 0; i < 1000; ++i) {
		vec3 center(Random::random(), Random::random(), Random::random());
		double radius = Random::random() * 0.5 + 0.1;
		std::vector<int> indices;
		grid.indicesWithinRadius(center, radius, indices);
	}
	gridSearchTime += timer.elapsedAndRestart();
	timer.restart();
	PointLocators::AABBTree<Point> aabb(points.begin(), points.end(), 1, 12);
	double aabbSearchTime = 0.0;
	timer.restart();
	for (int i = 0; i < 1000; ++i) {
		vec3 center(Random::random(), Random::random(), Random::random());
		double radius = Random::random() * 0.5 + 0.1;
		std::vector<int> indices;
		aabb.indicesWithinRadius(center, radius, indices);
	}
	aabbSearchTime += timer.elapsedAndRestart();
	printf("Point count: %d\n", pointCount);
	printf("Kd tree creation %f\n", float(kdTime));
	printf("Brute force creation %f\n", float(bfTime));
	printf("Hash grid creation %f\n", float(gridTime));
	printf("AABB tree creation %f\n\n", float(aabbTime));
	printf("Kd tree search time %f\n", float(kdSearchTime));
	printf("Brute force search time %f\n", float(bfSearchTime));
	printf("Hash grid search time %f\n", float(gridSearchTime));
	printf("AABB tree search time %f\n", float(aabbSearchTime));
}
