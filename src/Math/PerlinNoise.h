// https://github.com/sol-prog/Perlin_Noise/blob/master/PerlinNoise.h
// https://www.scratchapixel.com/code.php?id=57&origin=/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2
// will re-do later; using github version for now
#pragma once

#include <glm/glm.hpp>

class PerlinNoise {
public:
	PerlinNoise(const unsigned int seed = 2016);

	float Eval(const glm::vec3& p, glm::vec3& derivs) const;

private:
	uint8_t Hash(const int x, const int y, const int z) const;
	float GradientDotV(uint8_t perm, // a value between 0 and 255 
		float x, float y, float z) const;

	float Quintic(const float t) const;
	float QuinticDeriv(const float t) const;

	static const unsigned tableSize = 256;
	static const unsigned tableSizeMask = tableSize - 1;
	glm::vec3 gradients[tableSize];
	unsigned permutationTable[tableSize * 2];
};