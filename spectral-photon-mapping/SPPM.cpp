#include "SPPM.h"


namespace Spectral {
	namespace SPPM {
		float powerHeuristic(int nf, float fPdf, int ng, float gPdf) {
			float f = nf * fPdf, g = ng * gPdf;
			return (f * f) / (f * f + g * g);
		}
		vec3 toGrid(const vec3& p, const AABB& bounds, const glm::ivec3& gridRes) {
			return glm::ivec3(glm::floor((p - bounds.min()) / bounds.size() * vec3(gridRes)));
		}
		vec3 toGridClamped(const vec3& p, const AABB& bounds, const glm::ivec3& gridRes) {
			return glm::clamp(glm::ivec3(glm::floor((p - bounds.min()) / bounds.size() * vec3(gridRes))), glm::ivec3(0), gridRes - 1);
		}
	}
}