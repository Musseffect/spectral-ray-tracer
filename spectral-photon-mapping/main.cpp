#include <windows.h>
#undef min
#undef max
#include "common.h"
#include "SPPM.h"
#include "Debug.h"
#include "./Accelerators/Point Locators/KdTree.h"
#include "Camera.h"
#include "Timer.h"
#include "Scenes.h"
#include "./Accelerators/Primitive Locators/KdTree.h"
#include "./Accelerators/Primitive Locators/AABBTree.h"
#include "./Accelerators/Primitive Locators/Grid.h"
#include "./Accelerators/Primitive Locators/BruteForce.h"

#include <stdio.h>
#include <fstream>
#include <algorithm>
#include <time.h>
#pragma comment(lib, "winmm.lib")


void saveImagePPM(const char* filename, const Image<rgb>& image) {
	std::ofstream file;
	file.open(filename, std::ofstream::out | std::ofstream::trunc);
	if (!file.is_open()) {
		printf("Error. Cannot open file \"%s\"", filename);
	}
	file << "P3\n" << image.width() << " " << image.height() << "\n255\n";
	for (int j = 0; j < image.height(); j++) {
		for (int i = 0; i < image.width(); i++) {
			// flip y direction
			vec3 color = image(i, image.height() - j - 1);
			color = glm::pow(color, vec3(1.0f / 2.2f));
			int r = glm::clamp<int>(int(255.0f * color.r + 0.5f), 0, 255);
			int g = glm::clamp<int>(int(255.0f * color.g + 0.5f), 0, 255);
			int b = glm::clamp<int>(int(255.0f * color.b + 0.5f), 0, 255);
			file << r << " " << g << " " << b << "\n";
		}
	}
	file.close();
}

std::string formatFilename() {
	char name[] = "output_00:00:00_00-00-0000";
	time_t rawtime;
	struct tm  timeinfo;
	time(&rawtime);
	if (localtime_s(&timeinfo, &rawtime) == 0)
	{
		sprintf_s(name + sizeof(name) - 20, 20, "%.2d-%.2d-%.2d_%.2d-%.2d-%.4d",
			timeinfo.tm_hour,
			timeinfo.tm_min,
			timeinfo.tm_sec,
			timeinfo.tm_mday,
			timeinfo.tm_mon,
			1900 + timeinfo.tm_year);
	}
	return "./Out/" + std::string(name);
}

void genStats(int samples, std::function<float()> random) {
	std::vector<float> values;
	values.reserve(samples);
	for (int i = 0; i < samples; ++i) {
		float value = random();
		values.push_back(value);
	}
	std::sort(values.begin(), values.end(), std::less<float>());
	float largestGap = 0.0f;
	float smallestGap = std::numeric_limits<float>::max();
	float averageGap = 0.0f;
	for (int i = 0; i < samples - 1; ++i){
		float gap = std::abs(values[i + 1] - values[i]);
		largestGap = std::max(largestGap, gap);
		smallestGap = std::min(smallestGap, gap);
		averageGap += gap;
	}
	averageGap /= samples - 1;
	float disrepancy = 1.0f / samples;
	float deviation = 0.0f;
	for (int i = 0; i < samples; ++i) {
		if (i < samples - 1)
			deviation += std::pow(std::abs(values[i + 1] - values[i]) - averageGap, 2.0f);
		for (int j = i + 1; j < samples; ++j)
			disrepancy = std::max(disrepancy, std::abs(float(j + 1 - i) / samples - std::abs(values[j] - values[i])));
	}
	deviation *= 1000000.0f;
	deviation /= samples - 2;
	printf("average gap: %f\n", averageGap);
	printf("1 over n: %f\n\n", 1.0f / samples);
	printf("deviation: %f\n", deviation);
	printf("disrepancy: %f\n\n", disrepancy);
	printf("max gap: %f\n", largestGap);
	printf("min gap: %f\n\n", smallestGap);
}
#define USE_AABBTREE 0
#define USE_GRID 1
#define USE_KDTREE 2
#define USE_BRUTEFORCE 3
#define USE_SCENE_ACCEL USE_BRUTEFORCE
#if USE_SCENE_ACCEL == USE_AABBTREE
#define BACKWARD_TYPE "AABBTree"
#define PARAMETERS -1
using PrimitiveAccelerator = PrimitiveLocators::AABBTree<Intersectable>;
#elif USE_SCENE_ACCEL == USE_GRID
#define BACKWARD_TYPE "Grid"
using PrimitiveAccelerator = PrimitiveLocators::Grid<Intersectable>;
#elif USE_SCENE_ACCEL == USE_KDTREE
#define BACKWARD_TYPE "KdTree"
using PrimitiveAccelerator = PrimitiveLocators::KdTree<Intersectable>;
#elif USE_SCENE_ACCEL == USE_BRUTEFORCE
#define BACKWARD_TYPE "BruteForce"
#define PARAMETERS
using PrimitiveAccelerator = PrimitiveLocators::BruteForce<Intersectable>;
#else
static_asset("are u ohuely tam?", false);
#endif

using VisPoint = Spectral::SPPM::VisibilityPoint;
#define VISIBILITY_ACCEL USE_BRUTEFORCE
#if VISIBILITY_ACCEL == USE_AABBTREE
using Builder = PrimitiveLocators::SplitEqualCountsTreeBuilder<VisPoint>;
using VisPointSearch = PrimitiveLocators::AABBTree<VisPoint, Builder>;
#define FORWARD_TYPE "AABBTree"
#define VISIBILITY_PARAMS , 10
#elif VISIBILITY_ACCEL == USE_GRID
using VisPointSearch = PrimitiveLocators::Grid<VisPoint>;
#define FORWARD_TYPE "Grid"
#define VISIBILITY_PARAMS glm::ivec3(100)
#elif VISIBILITY_ACCEL == USE_KDTREE
using VisPointSearch = PrimitiveLocators::KdTree<VisPoint>;
#define FORWARD_TYPE "KdTree"
#elif VISIBILITY_ACCEL == USE_BRUTEFORCE
using VisPointSearch = PrimitiveLocators::BruteForce<VisPoint>;
#define FORWARD_TYPE "BruteForce"
#define VISIBILITY_PARAMS
#else
static_assert("Unknown macro", false);
#endif

//#define RENDER_FORWARD
void renderCornellBox() {
	// create scene
	spScene<PrimitiveAccelerator> scene = createCornwellBox<PrimitiveAccelerator>();
	scene->buildAccelerator(PARAMETERS);
	//scene->print();
	Spectral::SPPM::Tracer<PrimitiveAccelerator> sppmTracer;
	Debug::Tracer<PrimitiveAccelerator> debugTracer;
	// setup settings
	Spectral::SPPM::Settings settings;
	settings.threads = 8;
	settings.tileSize = 16;
	settings.photonsPerIteration = 100000;
	settings.iterations = 300;
	settings.maxDepth = 8;
	settings.initialRadius = 10.5f;
	sppmTracer.setSettings(settings);
	const int width = 256;
	const int height = 256;
	std::unique_ptr<Progress> progress = std::make_unique<ConsoleProgress>();
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(39.3076f)/2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(278.0f, 273.0f, -800.0f), vec3(278.0f, 273.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / float(width)
	);
	// Focal length  0.035
	// Width, height 0.025x0.025
	//2.0*atan(sensor_width / (2.0*focal_length))
	sppmTracer.setScene(scene);
	sppmTracer.setCamera(camera);
	debugTracer.setScene(scene);
	debugTracer.setCamera(camera);
	// render image
	Timer<float> timer;
#ifdef RENDER_FORWARD
	Image<rgb> image = sppmTracer.renderForward<VisPointSearch>(width, height, progress VISIBILITY_PARAMS);
	printf("\nTime for forward SPPM tracing (scene accel: %s) (vis. point accel: %s): %f\n", BACKWARD_TYPE, FORWARD_TYPE, timer.elapsed());
#else
	//Image<rgb> image = debugTracer.renderAmbientOcclusion(width, height, 240, 12800.1);
	//Image<rgb> image = debugTracer.renderDiffuse(width, height, 20);
	//Image<rgb> image = debugTracer.renderObjectColor(width, height, 20);
	Image<rgb> image = sppmTracer.renderBackward<PointLocators::KdTree<Spectral::SPPM::SpectralPhoton>>(width, height, progress);
	printf("\nTime for backward SPPM tracing (scene accel: %s): %f\n", BACKWARD_TYPE, timer.elapsed());
#endif
	std::string filename = formatFilename() + ".ppm";
	saveImagePPM(filename.c_str(), image);
}

void renderPrismScene() {
	std::shared_ptr<Scene<PrimitiveAccelerator>> scene = createPrismScene<PrimitiveAccelerator>();
	scene->buildAccelerator(PARAMETERS);
	Spectral::SPPM::Tracer<PrimitiveAccelerator> sppmTracer;
	Debug::Tracer<PrimitiveAccelerator> debugTracer;
	// setup settings
	Spectral::SPPM::Settings settings;
	settings.threads = 8;
	settings.tileSize = 16;
	settings.photonsPerIteration = 10000;
	settings.iterations = 400;
	settings.maxDepth = 8;
	settings.initialRadius = 2.5f;
	sppmTracer.setSettings(settings);
	const int width = 256;
	const int height = 256;
	std::unique_ptr<Progress> progress = std::make_unique<ConsoleProgress>();
//#define TOP_CAMERA
#ifdef TOP_CAMERA
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(79.3076f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(0.0f, 43.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)).inverse(),
		height / float(width)
		);
	/*std::shared_ptr<Camera> camera = std::make_unique<OrthoProjectionCamera>(20.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(0.0f, 10.0f, -20.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / float(width)
		);*/
#else
	/*std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(35.0f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(28.0f, 43.0f, -100.0f), vec3(0.0f, 7.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / float(width)
		);*/
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(55.0f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(0.0f, 43.0f, 80.0f), vec3(0.0f, 7.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / float(width)
		);
#endif
	sppmTracer.setScene(scene);
	sppmTracer.setCamera(camera);
	debugTracer.setScene(scene);
	debugTracer.setCamera(camera);
	Timer<float> timer;
	//#define RENDER_FORWARD
#ifdef RENDER_FORWARD
	Image<rgb> image = sppmTracer.renderForward<VisPointSearch>(width, height, progress VISIBILITY_PARAMS);
	printf("\nTime for forward SPPM tracing: %f\n", timer.elapsed());
#else
	Image<rgb> image = sppmTracer.renderBackward<PointLocators::KdTree<Spectral::SPPM::SpectralPhoton>>(width, height, progress);
	//Image<rgb> image = debugTracer.renderAmbientOcclusion(width, height, 60, 0.1);
	//Image<rgb> image = debugTracer.renderDiffuse(width, height, 20);
	//Image<rgb> image = debugTracer.renderObjectColor(width, height, 20);
	printf("\nTime for backward SPPM tracing: %f\n", timer.elapsed());
#endif
	/*image.transform([](const rgb& value) {
		float luminance = glm::max(glm::dot(vec3(0.2126, 0.7152, 0.0722), value), 0.0f);
		return value * luminance / (luminance + 1.0f);
	});*/
	std::string filename = formatFilename() + ".ppm";
	saveImagePPM(filename.c_str(), image);
}

#include "testAccelerators.h"

int main() {
	/*runTestPrimitiveAccelerators();
	return 0;*/
	/*const int statCount = 10;
	const int samples = 50000;
	int stats[statCount];
	memset(stats, 0, statCount * sizeof(int));
	std::vector<float> values;
	values.reserve(samples);
	for (int i = 0; i < samples; ++i) {
		//float value = glm::mix(400.f, 700.0f, Random::random());
		float value = glm::mix(400.f, 700.0f, Sampling::sampleStratified(10));
		value = (value - 400.0f) / (300.0f);
		values.push_back(value);
		stats[std::min(int(floor(value * statCount)), statCount - 1)]++;
	}
	//largest gap between closest points
	std::sort(values.begin(), values.end(), std::less<float>());
	float largestGap = 0.0f;
	for (int i = 0; i < samples - 1; ++i)
	{
		largestGap = std::max(largestGap, std::abs(values[i + 1] - values[i]));
	}
	float disrepancy = 1.0f / samples;
	for (int i = 0; i < samples; ++i)
	{
		for (int j = i + 1; j < samples; ++j)
		{
			disrepancy = std::max(disrepancy, std::abs(float(j + 1 - i) / samples - std::abs(values[j] - values[i])));
		}
	}
	printf("disrepancy: %f\n", disrepancy);
	printf("gap: %f, 1 over n: %f\n", largestGap, 1.0f / samples);
	for (int i = 0; i < statCount; ++i)
		printf("stats[%d] = %d\n", i, stats[i]);
	auto rand = [&]() { return Random::random(); };
	//auto strat = [&]() {return Sampling::sampleStratified(1000); };
	/*auto strat = [&]() {
		static float init = Random::random();
		init = glm::fract(init + (std::sqrt(5.0) - 1.0) / 2.0);
		return init; };
	auto strat = [&]() {
		static float init = Random::random() * 0.5;
		init = glm::fract(init + 0.5 + Random::random());
		return init; };
	printf("Rand 10000\n");
	genStats(10000, rand);
	printf("Strat 10000\n");
	genStats(10000, strat);
	printf("Rand 100000\n");
	genStats(100000, rand);
	printf("Strat 100000\n");
	genStats(100000, strat);
	getchar();
	return 0;*/
	renderCornellBox();
	//renderPrismScene();
	//runTestPrimitiveResults();
	//runTestPrimitiveAccelerators();
	PlaySound(TEXT("SystemStart"), NULL, SND_ALIAS);
	getchar();
	return 0;
}