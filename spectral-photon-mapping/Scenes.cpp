#include "Scenes.h"


std::shared_ptr<Mesh<PrimitiveLocators::BruteForce<Triangle>>> createPrism(float scale = 0.5f){
	float x = scale;
	float y = scale;
	float z = scale * std::sqrt(3.0f) / 4.0f;
	return std::make_shared<Mesh<PrimitiveLocators::BruteForce<Triangle>>>(std::initializer_list<Triangle>{
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
}

