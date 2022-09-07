#include <windows.h>
#include "scenes.h"
#undef min
#undef max
#include <spectral-photon-mapping\common.h>
#include <spectral-photon-mapping\Image.h>
#include <spectral-photon-mapping\Color.h>
#include <spectral-photon-mapping\Timer.h>
#include <spectral-photon-mapping\Progress.h>
#include <spectral-photon-mapping\SPPM.h>
#include <spectral-photon-mapping\Debug.h>
#include <spectral-photon-mapping\Camera.h>
#include <spectral-photon-mapping\Accelerators\Primitive Locators\KdTree.h>
#include <spectral-photon-mapping\Accelerators\Primitive Locators\AABBTree.h>
#include <spectral-photon-mapping\Accelerators\Primitive Locators\Grid.h>
#include <spectral-photon-mapping\Accelerators\Primitive Locators\BruteForce.h>
#include <spectral-photon-mapping\Accelerators\Point Locators\KdTree.h>
#include <spectral-photon-mapping\Accelerators\Point Locators\AABBTree.h>
#include <spectral-photon-mapping\Accelerators\Point Locators\Grid.h>
#include <spectral-photon-mapping\Accelerators\Point Locators\BruteForce.h>


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
			color = sRGB::fromLinear(color);
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

#define USE_AABBTREE 0
#define USE_GRID 1
#define USE_KDTREE 2
#define USE_BRUTEFORCE 3
#define USE_SCENE_ACCEL USE_BRUTEFORCE
#if USE_SCENE_ACCEL == USE_AABBTREE
#define SCENE_ACCEL_TYPE "AABBTree"
#define PARAMETERS -1
using PrimitiveAccelerator = PrimitiveLocators::AABBTree<Intersectable>;
#elif USE_SCENE_ACCEL == USE_GRID
#define SCENE_ACCEL_TYPE "Grid"
using PrimitiveAccelerator = PrimitiveLocators::Grid<Intersectable>;
#elif USE_SCENE_ACCEL == USE_KDTREE
#define SCENE_ACCEL_TYPE "KdTree"
using PrimitiveAccelerator = PrimitiveLocators::KdTree<Intersectable>;
#elif USE_SCENE_ACCEL == USE_BRUTEFORCE
#define SCENE_ACCEL_TYPE "BruteForce"
#define PARAMETERS
using PrimitiveAccelerator = PrimitiveLocators::BruteForce<Intersectable>;
#else
static_asset("Unknown primitive accelerator", false);
#endif


using VisPoint = Spectral::SPPM::VisibilityPoint;
#define VIS_ACCEL USE_BRUTEFORCE
#if VIS_ACCEL == USE_AABBTREE
using Builder = PrimitiveLocators::SplitEqualCountsTreeBuilder<VisPoint>;
using VisPointAccel = PrimitiveLocators::AABBTree<VisPoint>;
#define VIS_ACCEL_TYPE "AABBTree"
#define VIS_ACCEL_PARAMS , 10
#elif VIS_ACCEL == USE_GRID
using VisPointAccel = PrimitiveLocators::Grid<VisPoint>;
#define FORWARD_TYPE "Grid"
#define VIS_ACCEL_PARAMS glm::ivec3(100)
#elif VIS_ACCEL == USE_KDTREE
using VisPointAccel = PrimitiveLocators::KdTree<VisPoint>;
#define VIS_ACCEL_TYPE "KdTree"
#elif VIS_ACCEL == USE_BRUTEFORCE
using VisPointAccel = PrimitiveLocators::BruteForce<VisPoint>;
#define FORWARD_TYPE "BruteForce"
#define VIS_ACCEL_PARAMS
#else
static_assert("Unknown macro", false);
#endif



void renderCornellBox();
void renderPrismScene();



int main()
{
	renderCornellBox();
	//renderPrismScene();
	PlaySound(TEXT("SystemStart"), NULL, SND_ALIAS);
	getchar();
	return 0;
}

void renderCornellBox() {
	auto scene = createCornwellBox<PrimitiveAccelerator>();
	scene->buildAccelerator(PARAMETERS);

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
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(39.3076f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(278.0f, 273.0f, -800.0f), vec3(278.0f, 273.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / static_cast<float>(width)
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
	printf("\nTime for backward SPPM tracing (scene accel: %s): %f\n", SCENE_ACCEL_TYPE, timer.elapsed());
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
		height / static_cast<float>(width)
		);
	/*std::shared_ptr<Camera> camera = std::make_unique<OrthoProjectionCamera>(20.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(0.0f, 10.0f, -20.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / static_cast<float>(width)
		);*/
#else
	/*std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(35.0f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(28.0f, 43.0f, -100.0f), vec3(0.0f, 7.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / static_cast<float>(width)
		);*/
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(55.0f) / 2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(0.0f, 43.0f, 80.0f), vec3(0.0f, 7.5f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / static_cast<float>(width)
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




