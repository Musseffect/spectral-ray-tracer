#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/optimum_pow.hpp>

#define NOT_IMPLEMENTED() static_assert(false, "Not implemented")

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using mat3 = glm::mat3;
using quat = glm::quat;


#define ALIAS_SMARTPOINTERS(classname)\
	class classname;\
	using sp##classname = std::shared_ptr<classname>;\
	using scp##classname = std::shared_ptr<const classname>;

