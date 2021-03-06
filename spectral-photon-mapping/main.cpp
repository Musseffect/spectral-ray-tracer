
#include <windows.h>
#undef min
#undef max
#include "common.h"
#include "Tracer.h"
#include "Camera.h"
#include "Timer.h"
#include "Sphere.h"
#include "Disk.h"
#include "Rect.h"
#include "Triangle.h"
#include "Box.h"
#include "Mesh.h"
#include "Function1D.h"
#include <stdio.h>
#include <fstream>
#include <time.h>
#pragma comment(lib, "winmm.lib")


std::shared_ptr<Scene> createPrismScene() {
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	std::shared_ptr<Shape> lightRect = std::make_shared<Rect>();
	float z = std::sqrt(3.0f) / 4.0f;
	std::shared_ptr<Shape> prismShape = std::make_shared<Mesh>(std::initializer_list<Triangle>{
		Triangle(vec3(-0.5f, 0.5f, -z),
				 vec3(0.0f, 0.5f, z),
				 vec3(0.5f, 0.5f, -z)), //top

		Triangle(vec3(-0.5f, -0.5f, -z),
				 vec3(0.5f, -0.5f, -z),
				 vec3(0.0f, -0.5f, z)), // bottom

		Triangle(vec3(-0.5f, -0.5f, -z),
			     vec3(0.5f, 0.5f, -z),
				 vec3(0.5f, -0.5f, -z)),
		Triangle(vec3(-0.5f, -0.5f, -z),
				 vec3(-0.5f, 0.5f, -z),
				 vec3(0.5f, 0.5f, -z)),  // 1st side

		Triangle(vec3(0.0f, -0.5f, z),
				 vec3(-0.5f, 0.5f, -z),
				 vec3(-0.5f, -0.5f, -z)),
		Triangle(vec3(0.0f, -0.5f, z),
				 vec3(0.0f, 0.5f, z),
				 vec3(-0.5f, 0.5f, -z)),  // 2nd side

		Triangle(vec3(0.5f, -0.5f, -z),
				 vec3(0.0f, 0.5f, z),
				 vec3(0.0f, -0.5f, z)),
		Triangle(vec3(0.5f, -0.5f, -z),
				 vec3(0.5f, 0.5f, -z),
				 vec3(0.0f, 0.5f, z)) // 3rd side
		});
	spRGBColorSampler white(new RGBColorSampler(vec3(1.0f)));
	spRGBColorSampler gray(new RGBColorSampler(vec3(0.75f)));
	std::shared_ptr<Texture<spColorSampler>> whiteTexture = std::make_shared<ConstantTexture<spColorSampler>>(white);

	std::shared_ptr<DiffuseMaterial> whiteDiffuse = std::make_shared<DiffuseMaterial>(whiteTexture);
	std::shared_ptr<Shape> floorShape = std::make_shared<Rect>();
	std::shared_ptr<SceneObject> floor = std::make_shared<SceneObject>(
		floorShape,
		whiteDiffuse,
		Affine(Transform(vec3(0.0f),
			quat(),
			vec3(100.0f)
		))
	);
	std::shared_ptr<AnalyticalSampler<CauchyEquation>> glassRefraction = std::make_shared<AnalyticalSampler<CauchyEquation>>(fucedSilica);
	spTexture<spColorSampler> refractionTexture = std::make_shared<ConstantTexture<spColorSampler>>(glassRefraction);
	spTexture<spColorSampler> grayTexture = std::make_shared<ConstantTexture<spColorSampler>>(gray);
	std::shared_ptr<IdealGlassMaterial> glass = std::make_shared<IdealGlassMaterial>(whiteTexture, grayTexture, refractionTexture);

	spColorSampler lightColorSpectrum = std::make_shared<BlackBodyColorSampler>(6000.0f, 10.0f);
	std::shared_ptr<Light> light = std::make_shared<RectDirectionalLight>(
		Affine(Transform(vec3(-8.0f, 10.0f, 0.0f),
			glm::angleAxis(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColorSpectrum,
		vec2(2.0f, 20.0f)
		);
	std::shared_ptr<SceneObject> prism = std::make_shared<SceneObject>(
		prismShape,
		glass,
		Affine(Transform(vec3(0.0f, 15.0f, 0.0f),
			quat(),
			vec3(10.0f, 30.0f, 10.0f)
		))
	);
	scene->addObject(floor);
	scene->addObject(prism);
	scene->addLight(light);
	return scene;
}

std::shared_ptr<Scene> testMISScene() {
	throw std::runtime_error("Not impelmented");
}

std::shared_ptr<Scene> furnaceTestScene() {
	throw std::runtime_error("Not impelmented");
	/*spScene scene = std::make_shared<Scene>();
	spShape sphere = std::make_shared<Sphere>();
	spShape sphereLight = std::make_shared<Sphere>(vec3(1.0f), 10.0f);
	spSceneObject sphereObject = std::make_shared<SceneObject>();
	scene->addObject();
	scene->addLight();
	*/
}

// Done
std::shared_ptr<Scene> createCornwellBox() {
#define FULL_SPECTRAL_MATERIAL
//#define FULL_SPECTRAL_RGB
#ifdef FULL_SPECTRAL_MATERIAL
	std::shared_ptr<GridFunction1D> whiteSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.343, 0.445, 0.551, 0.624, 0.665, 0.687, 0.708, 0.723, 0.715, 0.71, 0.745, 0.758, 0.739, 0.767, 0.777, 0.765, 0.751, 0.745, 0.748, 0.729, 0.745, 0.757, 0.753, 0.75, 0.746, 0.747, 0.735, 0.732, 0.739, 0.734, 0.725, 0.721, 0.733, 0.725, 0.732, 0.743, 0.744, 0.748, 0.728, 0.716, 0.733, 0.726, 0.713, 0.74, 0.754, 0.764, 0.752, 0.736, 0.734, 0.741, 0.74, 0.732, 0.745, 0.755, 0.751, 0.744, 0.731, 0.733, 0.744, 0.731, 0.712, 0.708, 0.729, 0.73, 0.727, 0.707, 0.703, 0.729, 0.75, 0.76, 0.751, 0.739, 0.724, 0.73, 0.74, 0.737 }
	));
	std::shared_ptr<GridFunction1D> redSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.092, 0.096, 0.098, 0.097, 0.098, 0.095, 0.095, 0.097, 0.095, 0.094, 0.097, 0.098, 0.096, 0.101, 0.103, 0.104, 0.107, 0.109, 0.112, 0.115, 0.125, 0.14, 0.16, 0.187, 0.229, 0.285, 0.343, 0.39, 0.435, 0.464, 0.472, 0.476, 0.481, 0.462, 0.447, 0.441, 0.426, 0.406, 0.373, 0.347, 0.337, 0.314, 0.285, 0.277, 0.266, 0.25, 0.23, 0.207, 0.186, 0.171, 0.16, 0.148, 0.141, 0.136, 0.13, 0.126, 0.123, 0.121, 0.122, 0.119, 0.114, 0.115, 0.117, 0.117, 0.118, 0.12, 0.122, 0.128, 0.132, 0.139, 0.144, 0.146, 0.15, 0.152, 0.157, 0.159 }
	));
	std::shared_ptr<GridFunction1D> greenSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.04, 0.046, 0.048, 0.053, 0.049, 0.05, 0.053, 0.055, 0.057, 0.056, 0.059, 0.057, 0.061, 0.061, 0.06, 0.062, 0.062, 0.062, 0.061, 0.062, 0.06, 0.059, 0.057, 0.058, 0.058, 0.058, 0.056, 0.055, 0.056, 0.059, 0.057, 0.055, 0.059, 0.059, 0.058, 0.059, 0.061, 0.061, 0.063, 0.063, 0.067, 0.068, 0.072, 0.08, 0.09, 0.099, 0.124, 0.154, 0.192, 0.255, 0.287, 0.349, 0.402, 0.443, 0.487, 0.513, 0.558, 0.584, 0.62, 0.606, 0.609, 0.651, 0.612, 0.61, 0.65, 0.638, 0.627, 0.62, 0.63, 0.628, 0.642, 0.639, 0.657, 0.639, 0.635, 0.642 }
		));
	std::shared_ptr<GridFunction1D> lightSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.0f, 8.0f, 15.6f, 18.4f }
	));
	lightSpectrum->scale(1.0f);
	spSpectrumSampler whiteColorSpectrum(new SpectrumSampler(whiteSpectrum));
	spSpectrumSampler greenColorSpectrum(new SpectrumSampler(greenSpectrum));
	spSpectrumSampler redColorSpectrum(new SpectrumSampler(redSpectrum));
	spSpectrumSampler lightColorSpectrum(new SpectrumSampler(lightSpectrum));
	std::shared_ptr<ColorSampler> black(new ConstantSampler(0.0f));
#ifdef FULL_SPECTRAL_RGB
	std::shared_ptr<RGBColorSampler> white(new RGBColorSampler(spectrumToRGB(whiteColorSpectrum.get(), 400.0f, 700.0f, 60) * 3.0f / 300.0f));
	std::shared_ptr<RGBColorSampler> green(new RGBColorSampler(spectrumToRGB(greenColorSpectrum.get(), 400.0f, 700.0f, 60) * 3.0f / 300.0f));
	std::shared_ptr<RGBColorSampler> red(new RGBColorSampler(spectrumToRGB(redColorSpectrum.get(), 400.0f, 700.0f, 60) * 3.0f / 300.0f));
	std::shared_ptr<RGBColorSampler> lightColor(new RGBColorSampler(spectrumToRGB(lightColorSpectrum.get(), 400.0f, 700.0f, 60) * 3.0f / 300.0f));

	printf("Red: %f, %f, %f \n", red->color().r, red->color().g, red->color().b);
	printf("Green: %f, %f, %f \n", green->color().r, green->color().g, green->color().b);
	printf("White: %f, %f, %f \n", white->color().r, white->color().g, white->color().b);
	printf("Light: %f, %f, %f \n", lightColor->color().r, lightColor->color().g, lightColor->color().b);
#else
	spSpectrumSampler& white = whiteColorSpectrum;
	spSpectrumSampler& green = greenColorSpectrum;
	spSpectrumSampler& red = redColorSpectrum;
	spSpectrumSampler& lightColor = lightColorSpectrum;
#endif
#else
	std::shared_ptr<RGBColorSampler> white(new RGBColorSampler(vec3(1.0f)));
	std::shared_ptr<RGBColorSampler> green(new RGBColorSampler(vec3(0.0f, 1.0f, 0.0f)));
	std::shared_ptr<RGBColorSampler> red(new RGBColorSampler(vec3(1.0f, 0.0f, 0.0f)));
	std::shared_ptr<RGBColorSampler> lightColor(new RGBColorSampler(vec3(10.075f)));
	std::shared_ptr<ConstantTexture<ColorSampler>> greyTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.15f)));
	std::shared_ptr<ConstantTexture<ColorSampler>> whiteTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(1.0f)));
	std::shared_ptr<ConstantTexture<ColorSampler>> redTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(1.0f, 0.0f, 0.0f)));
	std::shared_ptr<ConstantTexture<ColorSampler>> greenTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.0f, 1.0f, 0.0f)));
#endif
	spScene scene = std::make_shared<Scene>();
	spShape sphere = std::make_shared<Sphere>();
	spShape rect = std::make_shared<Rect>();//Done
	spShape box = std::make_shared<Box>();//Done
	spShape disc = std::make_shared<Disc>();//Done
	//std::shared_ptr<ConstantTexture<ColorSampler>> greyTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.15f)));
	spConstTexture<spColorSampler> whiteTexture = makeConstTexture<spColorSampler>(white);
	spConstTexture<spColorSampler> redTexture = makeConstTexture<spColorSampler>(red);
	spConstTexture<spColorSampler> greenTexture = makeConstTexture<spColorSampler>(green);
	spConstTexture<spColorSampler> blackTexture = makeConstTexture<spColorSampler>(black);
	spColorSampler refraction = std::make_shared<AnalyticalSampler<CauchyEquation>>(BK7);
	spConstTexture<spColorSampler> refractionTexture = makeConstTexture<spColorSampler>(refraction);
	spMaterial whiteDiffuse = makeDiffuseMat(whiteTexture);
	spMaterial redDiffuse = makeDiffuseMat(redTexture);
	spMaterial greenDiffuse = makeDiffuseMat(greenTexture);
	spMaterial glassMaterial = std::make_shared<IdealGlassMaterial>(blackTexture, whiteTexture, refractionTexture);
	//std::shared_ptr<Material> specularMirror = std::make_shared<SpecularMirrorMaterial>(greyTexture);
	std::shared_ptr<SceneObject> floor = std::make_shared<SceneObject>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 0.0f, 279.6f),
			quat(),
			vec3(550.0f, 1.0f, 559.2f)))
		);
	std::shared_ptr<SceneObject> ceiling = std::make_shared<SceneObject>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 550.0f, 279.6f),
			quat(),
			vec3(550.0f, -1.0f, 559.2f)))
	);
	std::shared_ptr<SceneObject> backWall = std::make_shared<SceneObject>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 275.0f, 559.2f),
			glm::angleAxis(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f)),
			vec3(550.0f, -1.0f, 550.0f)))
	);
	std::shared_ptr<SceneObject> leftWall = std::make_shared<SceneObject>(
		rect,
		greenDiffuse,
		Affine(Transform(vec3(550.0f, 275.0f, 279.6),
			glm::angleAxis(-glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(550.0f, -1.0f, 559.2f)))
	);
	std::shared_ptr<SceneObject> rightWall = std::make_shared<SceneObject>(
		rect,
		redDiffuse,
		Affine(Transform(vec3(0.0f, 275.0f, 279.6),
			glm::angleAxis(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(550.0f, -1.0f, 559.2f)))
	);
	std::shared_ptr<SceneObject> shortBox = std::make_shared<SceneObject>(
		box,
		whiteDiffuse,
		Affine(Transform(vec3(185.0f, 82.5f, 169.0f),
			glm::angleAxis(-glm::radians(17.0f), vec3(0.0f, 1.0f, 0.0f)),
			vec3(165.0f, 165.0f, 165.0f)))
		);
	std::shared_ptr<SceneObject> tallBox = std::make_shared<SceneObject>(
		box,
		glassMaterial,
		Affine(Transform(vec3(368.5f, 165.0f, 351.25f),
			glm::angleAxis(glm::radians(17.0f), vec3(0.0f, 1.0f, 0.0f)),
			vec3(165.0f, 330.0f, 165.0f)))
	);
	std::shared_ptr<Box> lightBox = std::make_shared<Box>(vec3(0.0f), vec3(130.0f, 0.01f, 105.0f));
	std::shared_ptr<Rect> lightRect = std::make_shared<Rect>(vec2(130.0f, 105.0f));

	/*std::shared_ptr<Light> rectLight = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(278.0f, 548.8f - 0.1f, 279.6f),
			quat(),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColor,
		lightBox
	);*/
	//back
	std::shared_ptr<Light> rectLight = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(278.0f, 548.8f - 0.1f, 279.6f),
			glm::angleAxis(glm::radians(180.0f), vec3(1.0f, 0.0f, 0.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColor,
		lightRect
		);

	/*std::shared_ptr<Light> rectLight = std::make_shared<RectDirectionalLight>(
		Affine(Transform(vec3(278.0f, 548.8f - 0.1f, 279.6f),
			glm::angleAxis(glm::radians(180.0f), vec3(1.0f, 0.0f, 0.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColor,
		vec2(130.0f, 105.0f)
		);*/
	/*
	//front
	std::shared_ptr<Light> rectLight2 = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(278.0f, 548.8f - 5.01f, 279.6f - 52.5f),
			glm::angleAxis(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)),
			vec3(130.0f, 10.0f, 10.0f))),
		lightColor,
		rect
		);*/
	/*//left
	std::shared_ptr<Light> rectLight3 = std::make_shared<DiffuseAreaLight>(
		Affine(Rigid(vec3(278.0f, 548.8f - 5.01f, 279.6f),
			quat(),
			vec3(130.0f, 10.01f, 105.0f))),
		lightColor,
		rect
		);
	//right
	std::shared_ptr<Light> rectLight4 = std::make_shared<DiffuseAreaLight>(
		Affine(Rigid(vec3(278.0f, 548.8f - 5.01f, 279.6f),
			quat(),
			vec3(130.0f, 10.01f, 105.0f))),
		lightColor,
		rect
		);*/
	scene->addObject(floor);
	scene->addObject(ceiling);
	scene->addObject(backWall);
	scene->addObject(leftWall);
	scene->addObject(rightWall);
	scene->addObject(rightWall);
	scene->addObject(shortBox);
	scene->addObject(tallBox);
	scene->addLight(rectLight);
	/*scene->addLight(rectLight1);
	scene->addLight(rectLight2);*/
	/*scene->addLight(rectLight3);
	scene->addLight(rectLight4);*/
	return scene;
}

// Done
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
			vec3 color = image(i, image.height() - j - 1) * 2.0f;
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
	KdTree<Point> kdTree(points.begin(), points.end(), 8);
	BruteForceLocator<Point> bruteForce(points.begin(), points.end());
	HashGrid<Point> grid(points.begin(), points.end(), 8);
	AABBTree<Point> aabbTree(points.begin(), points.end(), 8);
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
		KdTree<Point> kdTree(points.begin(), points.end(), 10, 12);
	}
	double kdTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		BruteForceLocator<Point> bfpointLocator(points.begin(), points.end());
	}
	double bfTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		HashGrid<Point> bfpointLocator(points.begin(), points.end(), 20);
	}
	double gridTime = timer.elapsedAndRestart();
	for (int i = 0; i < 20; ++i) {
		AABBTree<Point> aabb(points.begin(), points.end(), 1, 12);
	}
	double aabbTime = timer.elapsedAndRestart();

	KdTree<Point> kdTree(points.begin(), points.end(), 1, 12);
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
	BruteForceLocator<Point> bfpointLocator(points.begin(), points.end());
	double bfSearchTime = 0.0;
	timer.restart();
	for (int i = 0; i < 1000; ++i) {
		vec3 center(Random::random(), Random::random(), Random::random());
		double radius = Random::random() * 0.5 + 0.1;
		std::vector<int> indices;
		bfpointLocator.indicesWithinRadius(center, radius, indices);
	}
	bfSearchTime += timer.elapsedAndRestart();
	timer.restart();
	HashGrid<Point> grid(points.begin(), points.end(), 20);
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
	AABBTree<Point> aabb(points.begin(), points.end(), 1, 12);
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

int main() {
	/*testPointLocatorsPerformance(1000);
	testPointLocatorsPerformance(10000);
	testPointLocatorsPerformance(100000);
	testPointLocatorsPerformance(1000000);
	//testPointLocators();
	getchar();
	return 0;*/
	// create scene
	std::shared_ptr<Scene> scene = createCornwellBox();
	photon_mapping::Tracer tracer;
	tracer.setScene(scene);
	// setup settings
	photon_mapping::Settings settings;
	settings.threads = 8;
	settings.tileSize = 16;
	settings.photonsPerIteration = 10000;
	settings.iterations = 300;
	settings.maxDepth = 8;
	settings.initialRadius = 25.0f;
	tracer.setSettings(settings);
	const int width = 128;
	const int height = 128;
	std::unique_ptr<Progress> progress = std::make_unique<ConsoleProgress>();
	//focal length 0.035
	//width: 0.025, height: 0.025
	std::shared_ptr<Camera> camera = std::make_unique<Pinhole>(glm::radians(39.3076f)/2.0f, 1.0f, 0.0f,
		Affine::lookAt(vec3(278.0f, 273.0f, -800.0f), vec3(278.0f, 273.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)).inverse(),
		height / float(width)
		);
	// Focal length  0.035
	// Width, height 0.025x0.025
	//2.0*atan(sensor_width / (2.0*focal_length))
	tracer.setCamera(camera);
	// render image
	Timer<float> timer;
#ifdef RENDER_FORWARD
	Image<rgb> image = tracer.renderForward(width, height);
	printf("\nTime for forward SPPM tracing: %f/n", timer.elapsed());
#else
	Image<rgb> image = tracer.renderBackward(width, height, progress);
	printf("\nTime for backward SPPM tracing: %f\n", timer.elapsed());
#endif
	std::string filename = formatFilename() + ".ppm";
	// save image to file
	saveImagePPM(filename.c_str(), image);
	// todo: play sound
	PlaySound(TEXT("SystemStart"), NULL, SND_ALIAS);
	getchar();
	return 0;
}