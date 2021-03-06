#include "Material.h"

spDiffuseMat makeDiffuseMat(const spTexture<spColorSampler>& kr) {
	return std::make_shared<DiffuseMaterial>(kr);
}