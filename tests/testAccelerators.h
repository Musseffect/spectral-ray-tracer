#pragma once
#include <spectral-photon-mapping/common.h>
#include <spectral-photon-mapping/BBox3D.h>
#include <spectral-photon-mapping/Timer.h>
#include <spectral-photon-mapping/Random.h>
#include <spectral-photon-mapping/Sampling.h>
#include <spectral-photon-mapping/Accelerators/Primitive Locators/KdTree.h>
#include <spectral-photon-mapping/Accelerators/Primitive Locators/AABBTree.h>
#include <spectral-photon-mapping/Accelerators/Primitive Locators/Grid.h>
#include <spectral-photon-mapping/Accelerators/Primitive Locators/BruteForce.h>

struct StatCounter {
	double min;
	double max;
	double sum;
	int count;
#ifdef KEEP_SAMPLES
	std::vector<double> samples;
#endif
	StatCounter() {
		reset();
	}
	void reset() {
		min = std::numeric_limits<double>::max();
		max = std::numeric_limits<double>::lowest();
		sum = 0.0f;
		count = 0;
#ifdef KEEP_SAMPLES
		samples.clear();
#endif
	}
	void advance(double value) {
		min = std::min(value, min);
		max = std::max(value, max);
		sum += value;
		count++;
#ifdef KEEP_SAMPLES
		samples.push_back(value);
#endif
	}
	void printStats() {
		printf("min: %f, max: %f, average: %f", min, max, sum / double(count));
	}
#ifdef KEEP_SAMPLES
	template<int binCount>
	void histogram() {
		int bins[binCount];
		memset(bins, 0, sizeof(double) * binCount);
		for (auto sample : samples) {
			int bin = std::min((int)std::floor((sample - min) / (max - min)), binCount - 1);
			bins[bin]++;
		}
		int id = 0;
		printf("histogram: \n");
		for (auto bin : bins) {
			printf("\tbin[%d] = %f\n", id, bin / double(count));
		}
	}
#endif
};

class BoxWrapper: public Intersectable {
	BBox3D box;
public:
	template<class...Params>
	BoxWrapper(Params...params) : box(params...)
	{}
	virtual BBox3D bbox() const override {
		return box;
	}
	virtual bool intersect(const Ray& ray) const override {
		Sleep(0);
		return box.intersectInclusive(ray);
	}
	virtual bool intersect(Ray& ray, HitInfo& hitInfo) const override {
		Sleep(0);
		bool result = box.intersectInclusive(ray, hitInfo.t);
		if (result) {
			ray.tMax = hitInfo.t;
			hitInfo.globalPosition = ray.ro + ray.rd * hitInfo.t;
		}
		return result;
	}
	template<class Point>
	bool intersect(const Point& point) const {
		Sleep(0);
		return box.intersect<Point>(point);
	}
};

struct Point {
	vec3 point;
	vec3 position() const {
		return point;
	}
};

template<class Accel, class...Params>
void testPrimitiveAcceleratorPerformance(int numBoxes, int numCreations, int numRays, int numPoints, vec3 sceneSize, vec3 boxSizeMin, vec3 boxSizeMax, Params...params) {
	std::vector<BoxWrapper> boxes;
	Timer<double> timer;
	for (int i = 0; i < numBoxes; i++) {
		vec3 size = glm::mix(boxSizeMin, boxSizeMax, vec3(Random::random(), Random::random(), Random::random()));
		vec3 center = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		boxes.emplace_back(center - size * 0.5, center + size * 0.5);
	}
	const int SleepTimeMs = 100;
	Sleep(SleepTimeMs);

	StatCounter statCounter;
	timer.restart();
	for (int i = 0; i < numCreations; ++i) {
		Accel accel(boxes.begin(), boxes.end(), params...);
		statCounter.advance(timer.elapsedAndRestart());
	}
	printf("Creation time:\n");
	statCounter.printStats();
	printf("\n");
	Sleep(SleepTimeMs);
	Accel accel(boxes.begin(), boxes.end(), params...);
	statCounter.reset();
	for (int i = 0; i < numRays; ++i) {
		Ray ray;
		ray.ro = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		ray.rd = Sampling::uniformSphere();
		timer.restart();
		accel.intersect(ray);
		statCounter.advance(timer.elapsedAndRestart());
	}
	printf("Ray intersection simple time:\n");
	statCounter.printStats();
	printf("\n");
	Sleep(SleepTimeMs);
	statCounter.reset();
	for (int i = 0; i < numRays; ++i) {
		Ray ray;
		ray.ro = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		ray.rd = Sampling::uniformSphere();
		HitInfo hitInfo;
		hitInfo.t = -1.0;
		timer.restart();
		accel.intersect(ray, hitInfo);
		statCounter.advance(timer.elapsedAndRestart());
	}
	printf("Ray intersection advance time:\n");
	statCounter.printStats();
	printf("\n");
	Sleep(SleepTimeMs);
	statCounter.reset();
	// todo(me): test sphere-point intersection
	for (int i = 0; i < numPoints; ++i) {
		Point point = { (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize };
		timer.restart();
		std::vector<int> result = accel.intersectedIndicies<Point>(point);
		statCounter.advance(timer.elapsedAndRestart());
	}
	printf("Point primitive intersection time:\n");
	statCounter.printStats();
}

template<class Accel, class...Params>
void testPrimitiveAcceleratorResults(int numBoxes, int numRays, int numPoints, vec3 sceneSize, vec3 boxSizeMin, vec3 boxSizeMax, Params...params) {
	std::vector<BoxWrapper> boxes;
	Timer<double> timer;
	for (int i = 0; i < numBoxes; i++) {
		vec3 size = glm::mix(boxSizeMin, boxSizeMax, vec3(Random::random(), Random::random(), Random::random()));
		vec3 center = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		boxes.emplace_back(center - size * 0.5, center + size * 0.5);
	}
	Accel accel(boxes.begin(), boxes.end(), params...);
	printf("Test result %d\n", accel.test());
	double Epsilon = 0.001f;
	int errors = 0;
	for (int i = 0; i < numRays; ++i) {
		Ray ray;
		ray.ro = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		ray.rd = Sampling::uniformSphere();
		bool result = false;
		for (auto& object : boxes) {
			if (object.intersect(ray)) {
				result = true;
				break;
			}
		}
		if (accel.intersect(ray) != result)
			errors++;
	}
	printf("Visibility test result: %d\n", errors);
	errors = 0;
	for (int i = 0; i < numRays; ++i) {
		Ray ray;
		ray.ro = (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize;
		ray.rd = Sampling::uniformSphere();
		bool result = false;
		HitInfo info;
		for (auto& object : boxes) {
			result = object.intersect(ray, info) || result;
		}
		HitInfo info2;
		ray.tMax = std::numeric_limits<double>::max();
		if (accel.intersect(ray, info2) != result) {
			errors++;
			continue;
		}
		if (result && glm::distance2(info.globalPosition, info2.globalPosition) > Epsilon)
			errors++;
	}
	printf("Ray intersection errors: %d\n", errors);
	errors = 0;
	for (int i = 0; i < numPoints; ++i) {
		Point point = { (vec3(Random::random(), Random::random(), Random::random()) - vec3(0.5)) * sceneSize };
		timer.restart();
		std::vector<int> result = accel.intersectedIndicies<Point>(point);
		std::vector<int> result2;
		for (int j = 0; j < boxes.size(); ++j) {
			if (boxes[j].intersect(point)) {
				result2.push_back(j);
			}
		}
		for (auto id : result) {
			bool isCorrect = false;
			for (auto id2 : result2) {
				if (id == id2) {
					isCorrect = true;
					break;
				}
			}
			if (!isCorrect)
				errors++;

		}
	}
	printf("Primitive-primitive intersection: %d\n", errors);
}

using SAHTree = PrimitiveLocators::AABBTree < BoxWrapper, PrimitiveLocators::SAHTreeBuilder<BoxWrapper>>;
using EqualCounts = PrimitiveLocators::AABBTree <BoxWrapper, PrimitiveLocators::SplitEqualCountsTreeBuilder<BoxWrapper>>;
using BruteForce = PrimitiveLocators::BruteForce<BoxWrapper>;

void runTestPrimitiveAccelerators() {
	printf("BruteForce\n");
	testPrimitiveAcceleratorPerformance<BruteForce>(1000, 100, 100, 50, vec3(1.0), vec3(0.01), vec3(0.03));
	printf("\n");
	printf("AABBTree SAH\n");
	testPrimitiveAcceleratorPerformance<SAHTree>(1000, 100, 100, 50, vec3(1.0), vec3(0.01), vec3(0.03), -1);
	printf("\n");
	printf("AABBTree Equal counts\n");
	testPrimitiveAcceleratorPerformance<EqualCounts>(1000, 100, 100, 50, vec3(1.0), vec3(0.01), vec3(0.03), -1);
}

void runTestPrimitiveResults() {
	//printf("BruteForce\n");
	//testPrimitiveAcceleratorResults<BruteForce>(1000, 10, 10, vec3(1.0), vec3(0.01), vec3(0.03));
	//printf("\n");
	printf("AABBTree SAH\n");
	testPrimitiveAcceleratorResults<SAHTree>(1000, 1500, 1500, vec3(1.0), vec3(0.01), vec3(0.03), -1);
	printf("\n");
	printf("AABBTree Equal counts\n");
	testPrimitiveAcceleratorResults<EqualCounts>(1000, 1500, 1500, vec3(1.0), vec3(0.01), vec3(0.03), -1);
}