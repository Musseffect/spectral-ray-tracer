#pragma once
#include <memory>
#include "Scene.h"
#include "Mesh.h"
#include "Box.h"
#include "Sphere.h"
#include "Disk.h"
#include "Rect.h"
#include "Box.h"
#include "Function1D.h"
#include "Color.h"
#include "Material.h"
#include "Refraction.h"
#include "./Accelerators/Primitive Locators/BruteForce.h"

template<class RayTracerAccel>
class Scene;

template<class RayTracerAccel>
spScene<RayTracerAccel> createPrismScene();

template<class RayTracerAccel>
spScene<RayTracerAccel> testMISScene();

template<class RayTracerAccel>
spScene<RayTracerAccel> furnaceTestScene();

template<class RayTracerAccel>
spScene<RayTracerAccel> createCornwellBox();


template<class RayTracerAccel>
spScene<RayTracerAccel> createPrismScene() {
	spScene<RayTracerAccel> scene = std::make_shared<Scene<RayTracerAccel>>();
	float z = std::sqrt(3.0f) / 4.0f;
	vec3 a(-0.5f, 0.5f, -z), b(0.0f, 0.5f, z), c(0.5f, 0.5f, -z);
	vec3 d(-0.5f, -0.5f, -z), e(0.5f, -0.5f, -z), f(0.0f, -0.5f, z);
	std::shared_ptr<Shape> prismShape = std::make_shared<Mesh<PrimitiveLocators::BruteForce<Triangle>>>(std::initializer_list<Triangle>{
		Triangle(vec3(-0.5f, 0.5f, -z),
			vec3(0.0f, 0.5f, z),
			vec3(0.5f, 0.5f, -z)), //top

			Triangle(vec3(-0.5f, -0.5f, -z),
				vec3(0.5f, -0.5f, -z),
				vec3(0.0f, -0.5f, z)), // bottom

			Triangle(vec3(-0.5f, -0.5f, -z),// d
				vec3(0.5f, 0.5f, -z), // c
				vec3(0.5f, -0.5f, -z)), //e
			Triangle(vec3(-0.5f, -0.5f, -z),//d
				vec3(-0.5f, 0.5f, -z),//a
				vec3(0.5f, 0.5f, -z)),//c  // 1st side

			Triangle(vec3(0.0f, -0.5f, z),//f
				vec3(-0.5f, 0.5f, -z),//a
				vec3(-0.5f, -0.5f, -z)),//d
			Triangle(vec3(0.0f, -0.5f, z),//f
				vec3(0.0f, 0.5f, z),//b
				vec3(-0.5f, 0.5f, -z)),//a  // 2nd side

			Triangle(vec3(0.5f, -0.5f, -z),//e
				vec3(0.0f, 0.5f, z),//b
				vec3(0.0f, -0.5f, z)),//f
			Triangle(vec3(0.5f, -0.5f, -z),//e
				vec3(0.5f, 0.5f, -z),//c
				vec3(0.0f, 0.5f, z)) //b // 3rd side
	});
	spColorSampler black(new ConstantSampler(0.0f));
	spColorSampler white(new ConstantSampler(0.9f));
	spRGBColorSampler gray(new RGBColorSampler(vec3(0.75f)));
	spTex<spColorSampler> whiteTexture = std::make_shared<ConstantTexture<spColorSampler>>(white);
	spTex<spColorSampler> blackTexture = std::make_shared<ConstantTexture<spColorSampler>>(black);
	spTex<spColorSampler> grayTexture = std::make_shared<ConstantTexture<spColorSampler>>(gray);

	std::shared_ptr<Spectral::DiffuseMaterial> whiteDiffuse = std::make_shared<Spectral::DiffuseMaterial>(whiteTexture);
	std::shared_ptr<Spectral::DiffuseMaterial> grayDiffuse = std::make_shared<Spectral::DiffuseMaterial>(grayTexture);
	std::shared_ptr<Shape> floorShape = std::make_shared<Rect>();
	std::shared_ptr<Primitive> floor = std::make_shared<Primitive>(
		floorShape,
		whiteDiffuse,
		Affine(Transform(vec3(0.0f),
			quat(),
			vec3(1000.0f)
		))
		);
	std::shared_ptr<Function1D> customRefractionCurve(new GridFunction1D(400.0f, 700.0f, { 1.5f, 1.35f, 1.3f }));
	std::shared_ptr<ColorSampler> customRefraction = std::make_shared<SpectrumSampler>(customRefractionCurve);
	//use flint glass
	std::shared_ptr<ColorSampler> glassRefraction = std::make_shared<AnalyticalSampler<CauchyEquation>>(SF10);

	std::shared_ptr<ColorSampler> constantGlassRefraction = std::make_shared<ConstantSampler>(1.5);

	spTex<spColorSampler> refractionTexture = std::make_shared<ConstantTexture<spColorSampler>>(customRefraction);

	std::shared_ptr<Spectral::IdealGlassMaterial> glass = std::make_shared<Spectral::IdealGlassMaterial>(grayTexture, whiteTexture, refractionTexture);

	spColorSampler lightColorSpectrum = std::make_shared<ConstantSampler>(15.5f);
	std::shared_ptr<Light> light = std::make_shared<RectDirectionalLight>(
		Affine(Transform(vec3(-30.0f, 10.0f, -1.0f),
			glm::angleAxis(glm::radians(-90.1f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColorSpectrum,
		vec2(20.0f, 1.0f)
		);
	spColorSampler lightColorSpectrum2 = std::make_shared<ConstantSampler>(0.5f);
	std::shared_ptr<Rect> lightRect = std::make_shared<Rect>(vec2(50.0f, 50.0f));
	std::shared_ptr<Light> secondLight = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(0.0f, 50.0f, 0.0f),
			glm::angleAxis(glm::radians(180.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColorSpectrum2,
		lightRect
		);
	/*std::shared_ptr<Rect> lightRect = std::make_shared<Rect>(vec2(10.0f, 0.1f));
	std::shared_ptr<Light> light = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(-20.0f, 5.0f, 0.0f),
			glm::angleAxis(glm::radians(-99.8f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(1.0f, 1.0f, 1.0f))),
		lightColorSpectrum,
		lightRect
		);*/
	spPrimitive prism = std::make_shared<Primitive>(
		prismShape,
		glass,
		Affine(Transform(vec3(0.0f, 10.002f, 0.0f),
			quat(),
			vec3(15.0f, 20.0f, 15.0f)
		))
		);
	spPrimitive reflector = std::make_shared<Primitive>(
		floorShape,
		grayDiffuse,
		Affine(Transform(vec3(30.0f, 10.0f, -10.0f),
			glm::angleAxis(glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f)) * glm::angleAxis(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(20.0f, 1.0f, 40.0f)
		))
		);
	scene->addPrimitive(floor);
	scene->addPrimitive(reflector);
	scene->addPrimitive(prism);
	scene->addLight(light);
	scene->addLight(secondLight);
	return scene;
}

template<class RayTracerAccel>
spScene<RayTracerAccel> testMISScene() {
	std::shared_ptr<Scene<RayTracerAccel>> scene = std::make_shared<Scene<RayTracerAccel>>();
	std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>();
	std::shared_ptr<Rect> rect = std::make_shared<Rect>();
	/*
	std::shared_ptr<Light> firstSphere = ;
	std::shared_ptr<Light> secondSphere = ;
	std::shared_ptr<Light> thirdSphere = ;
	std::shared_ptr<Light> fourthSphere = ;

	std::shared_ptr<Shape> firstRect = ;
	std::shared_ptr<Shape> secondRect = ;
	std::shared_ptr<Shape> thirdRect = ;
	std::shared_ptr<Shape> fourthRect = ;

	std::shared_ptr<Shape> backWall = ;
	std::shared_ptr<Shape> bottomWall = ;

	std::shared_ptr<Light> ambientLight = std::make_shared<Light>(
		sphere,
		grayDiffuse,
		Affine(Transform(vec3(0.0f),
			quat(),
			vec3(100.0f)
		))
		);
	*/

	throw std::runtime_error("Not impelmented");
}

template<class RayTracerAccel>
spScene<RayTracerAccel> furnaceTestScene() {
	spScene<RayTracerAccel> scene = std::make_shared<Scene>();
	spShape sphereShape = std::make_shared<Sphere>(vec3(0.0f), 0.5);
	spRGBColorSampler gray(new RGBColorSampler(vec3(0.18f)));
	spTex<spColorSampler> grayTexture = std::make_shared<ConstantTexture<spColorSampler>>(gray);
	std::shared_ptr<Spectral::DiffuseMaterial> grayDiffuse = std::make_shared<Spectral::DiffuseMaterial>(grayTexture);
	spPrimitive spherePrimitive = std::make_shared<Primitive>(
		sphereShape,
		grayDiffuse,
		Affine(Transform(vec3(0.0f, 0.0f, 0.0f), glm::quat(), vec3(1.0f, 1.0f, 1.0f)))
		);
	scene->addPrimitive(spherePrimitive);
	std::shared_ptr<Light> rectLight = std::make_shared<DiffuseAreaLight>(
		Affine(Transform(vec3(0.0f, 0.0f 0.0f), glm::quat(), vec3(1000.0f, 1000.0f, 1000.0f))),
		lightColor,
		sphereShape
		);
	scene->addLight();
	throw std::runtime_error("Not impelmented");
	/*spScene scene = std::make_shared<Scene>();
	spShape sphere = std::make_shared<Sphere>();
	spShape sphereLight = std::make_shared<Sphere>(vec3(1.0f), 10.0f);
	spSceneObject sphereObject = std::make_shared<SceneObject>();
	scene->addPrimitive();
	scene->addLight();
	*/
}

template<class RayTracerAccel>
std::shared_ptr<Scene<RayTracerAccel>> createCornwellBox() {
#define FULL_SPECTRAL_MATERIAL
	//#define FULL_SPECTRAL_RGB
#ifdef FULL_SPECTRAL_MATERIAL
	std::shared_ptr<GridFunction1D> whiteSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.343f, 0.445, 0.551, 0.624, 0.665, 0.687, 0.708, 0.723, 0.715, 0.71, 0.745, 0.758, 0.739, 0.767, 0.777, 0.765, 0.751, 0.745, 0.748, 0.729, 0.745, 0.757, 0.753, 0.75, 0.746, 0.747, 0.735, 0.732, 0.739, 0.734, 0.725, 0.721, 0.733, 0.725, 0.732, 0.743, 0.744, 0.748, 0.728, 0.716, 0.733, 0.726, 0.713, 0.74, 0.754, 0.764, 0.752, 0.736, 0.734, 0.741, 0.74, 0.732, 0.745, 0.755, 0.751, 0.744, 0.731, 0.733, 0.744, 0.731, 0.712, 0.708, 0.729, 0.73, 0.727, 0.707, 0.703, 0.729, 0.75, 0.76, 0.751, 0.739, 0.724, 0.73, 0.74, 0.737 }
	));
	std::shared_ptr<GridFunction1D> redSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.092, 0.096, 0.098, 0.097, 0.098, 0.095, 0.095, 0.097, 0.095, 0.094, 0.097, 0.098, 0.096, 0.101, 0.103, 0.104, 0.107, 0.109, 0.112, 0.115, 0.125, 0.14, 0.16, 0.187, 0.229, 0.285, 0.343, 0.39, 0.435, 0.464, 0.472, 0.476, 0.481, 0.462, 0.447, 0.441, 0.426, 0.406, 0.373, 0.347, 0.337, 0.314, 0.285, 0.277, 0.266, 0.25, 0.23, 0.207, 0.186, 0.171, 0.16, 0.148, 0.141, 0.136, 0.13, 0.126, 0.123, 0.121, 0.122, 0.119, 0.114, 0.115, 0.117, 0.117, 0.118, 0.12, 0.122, 0.128, 0.132, 0.139, 0.144, 0.146, 0.15, 0.152, 0.157, 0.159 }
	));
	std::shared_ptr<GridFunction1D> greenSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.04, 0.046, 0.048, 0.053, 0.049, 0.05, 0.053, 0.055, 0.057, 0.056, 0.059, 0.057, 0.061, 0.061, 0.06, 0.062, 0.062, 0.062, 0.061, 0.062, 0.06, 0.059, 0.057, 0.058, 0.058, 0.058, 0.056, 0.055, 0.056, 0.059, 0.057, 0.055, 0.059, 0.059, 0.058, 0.059, 0.061, 0.061, 0.063, 0.063, 0.067, 0.068, 0.072, 0.08, 0.09, 0.099, 0.124, 0.154, 0.192, 0.255, 0.287, 0.349, 0.402, 0.443, 0.487, 0.513, 0.558, 0.584, 0.62, 0.606, 0.609, 0.651, 0.612, 0.61, 0.65, 0.638, 0.627, 0.62, 0.63, 0.628, 0.642, 0.639, 0.657, 0.639, 0.635, 0.642 }
	));
	std::shared_ptr<GridFunction1D> lightSpectrum(new GridFunction1D(400.0f, 700.0f,
		{ 0.0f, 8.0f, 15.6, 18.4f }
	));
	lightSpectrum->scale(1.0f);
	spColorSampler whiteColorSpectrum(new SpectrumSampler(whiteSpectrum));
	spColorSampler greenColorSpectrum(new SpectrumSampler(greenSpectrum));
	spColorSampler redColorSpectrum(new SpectrumSampler(redSpectrum));
	spColorSampler lightColorSpectrum(new SpectrumSampler(lightSpectrum));
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
	spColorSampler& white = whiteColorSpectrum;
	spColorSampler& green = greenColorSpectrum;
	spColorSampler& red = redColorSpectrum;
	spColorSampler& lightColor = lightColorSpectrum;
#endif
#else
	std::shared_ptr<RGBColorSampler> white(new RGBColorSampler(vec3(1.0f)));
	std::shared_ptr<RGBColorSampler> green(new RGBColorSampler(vec3(0.0f, 1.0f, 0.0f)));
	std::shared_ptr<RGBColorSampler> red(new RGBColorSampler(vec3(1.0f, 0.0f, 0.0f)));
	std::shared_ptr<RGBColorSampler> lightColor(new RGBColorSampler(vec3(10.075f)));
	spConstTex<ColorSampler> greyTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.15f)));
	spConstTex<ColorSampler> whiteTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(1.0f)));
	spConstTex<ColorSampler> redTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(1.0f, 0.0f, 0.0f)));
	spConstTex<ColorSampler> greenTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.0f, 1.0f, 0.0f)));
#endif
	spScene<RayTracerAccel> scene = std::make_shared<Scene<RayTracerAccel>>();
	spShape sphere = std::make_shared<Sphere>();
	spShape rect = std::make_shared<Rect>();//Done
	spShape box = std::make_shared<Box>();//Done
	spShape disc = std::make_shared<Disc>();//Done
	//std::shared_ptr<ConstantTexture<ColorSampler>> greyTexture = std::make_shared<ConstantTexture<ColorSampler>>(std::make_shared<RGBColorSampler>(vec3(0.15f)));
	spConstTex<spColorSampler> whiteTexture = makeConstTex<spColorSampler>(white);
	spConstTex<spColorSampler> redTexture = makeConstTex<spColorSampler>(red);
	spConstTex<spColorSampler> greenTexture = makeConstTex<spColorSampler>(green);
	spConstTex<spColorSampler> blackTexture = makeConstTex<spColorSampler>(black);
	spColorSampler refraction = std::make_shared<AnalyticalSampler<CauchyEquation>>(BK7);
	spConstTex<spColorSampler> refractionTexture = makeConstTex<spColorSampler>(refraction);
	Spectral::spMaterial whiteDiffuse = Spectral::makeDiffuseMat(whiteTexture);
	Spectral::spMaterial redDiffuse = Spectral::makeDiffuseMat(redTexture);
	Spectral::spMaterial greenDiffuse = Spectral::makeDiffuseMat(greenTexture);
	Spectral::spMaterial glassMaterial = std::make_shared<Spectral::IdealGlassMaterial>(whiteTexture, blackTexture, refractionTexture);
	//Spectral::spMaterial specularMirror = std::make_shared<Spectral::SpecularMirrorMaterial>(greyTexture);
	std::shared_ptr<Primitive> floor = std::make_shared<Primitive>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 0.0f, 279.6f),
			quat(),
			vec3(550.0f, 1.0f, 559.2f)))
		);
	std::shared_ptr<Primitive> ceiling = std::make_shared<Primitive>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 550.0f, 279.6f),
			quat(),
			vec3(550.0f, -1.0f, 559.2f)))
		);
	std::shared_ptr<Primitive> backWall = std::make_shared<Primitive>(
		rect,
		whiteDiffuse,
		Affine(Transform(vec3(275.0f, 275.0f, 559.2f),
			glm::angleAxis(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f)),
			vec3(550.0f, -1.0f, 550.0f)))
		);
	std::shared_ptr<Primitive> leftWall = std::make_shared<Primitive>(
		rect,
		greenDiffuse,
		Affine(Transform(vec3(550.0f, 275.0f, 279.6),
			glm::angleAxis(-glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(550.0f, -1.0f, 559.2f)))
		);
	std::shared_ptr<Primitive> rightWall = std::make_shared<Primitive>(
		rect,
		redDiffuse,
		Affine(Transform(vec3(0.0f, 275.0f, 279.6),
			glm::angleAxis(glm::radians(90.0f), vec3(0.0f, 0.0f, 1.0f)),
			vec3(550.0f, -1.0f, 559.2f)))
		);
	std::shared_ptr<Primitive> shortBox = std::make_shared<Primitive>(
		box,
		whiteDiffuse,
		Affine(Transform(vec3(185.0f, 82.5f, 169.0f),
			glm::angleAxis(-glm::radians(17.0f), vec3(0.0f, 1.0f, 0.0f)),
			vec3(165.0f, 165.0f, 165.0f)))
		);
	std::shared_ptr<Primitive> tallBox = std::make_shared<Primitive>(
		box,
		whiteDiffuse,
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
			/*
			//left
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
	scene->addPrimitive(floor);
	scene->addPrimitive(ceiling);
	scene->addPrimitive(backWall);
	scene->addPrimitive(leftWall);
	scene->addPrimitive(rightWall);
	scene->addPrimitive(rightWall);
	scene->addPrimitive(shortBox);
	scene->addPrimitive(tallBox);
	scene->addLight(rectLight);
	/*scene->addLight(rectLight1);
	scene->addLight(rectLight2);*/
	/*scene->addLight(rectLight3);
	scene->addLight(rectLight4);*/
	return scene;
}