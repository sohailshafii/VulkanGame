#include "PerlinNoise.h"
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <functional>

PerlinNoise::PerlinNoise(const unsigned int seed) {
	std::mt19937 generator(seed);
	std::uniform_real_distribution<float> distribution;
	auto dice = std::bind(distribution, generator);
	for (unsigned i = 0; i < tableSize; ++i) {
#if 0 
		// bad
		float gradientLen2;
		do {
			gradients[i] = Vec3f(2 * dice() - 1, 2 * dice() - 1, 2 * dice() - 1);
			gradientLen2 = gradients[i].length2();
		} while (gradientLen2 > 1);
		gradients[i].normalize();
#else 
		// better
		float theta = acos(2 * dice() - 1);
		float phi = (float)(2.0 * dice() * 3.1415);

		float x = cos(phi) * sin(theta);
		float y = sin(phi) * sin(theta);
		float z = cos(theta);
		gradients[i] = glm::vec3(x, y, z);
#endif 
		permutationTable[i] = i;
	}
}

float PerlinNoise::Eval(const glm::vec3 & p, glm::vec3 & derivs) const {
	int xi0 = ((int)std::floor(p.x)) & tableSizeMask;
	int yi0 = ((int)std::floor(p.y)) & tableSizeMask;
	int zi0 = ((int)std::floor(p.z)) & tableSizeMask;

	int xi1 = (xi0 + 1) & tableSizeMask;
	int yi1 = (yi0 + 1) & tableSizeMask;
	int zi1 = (zi0 + 1) & tableSizeMask;

	float tx = p.x - ((int)std::floor(p.x));
	float ty = p.y - ((int)std::floor(p.y));
	float tz = p.z - ((int)std::floor(p.z));

	float u = Quintic(tx);
	float v = Quintic(ty);
	float w = Quintic(tz);

	// generate vectors going from the grid points to p
	float x0 = tx, x1 = tx - 1;
	float y0 = ty, y1 = ty - 1;
	float z0 = tz, z1 = tz - 1;

	float a = GradientDotV(Hash(xi0, yi0, zi0), x0, y0, z0);
	float b = GradientDotV(Hash(xi1, yi0, zi0), x1, y0, z0);
	float c = GradientDotV(Hash(xi0, yi1, zi0), x0, y1, z0);
	float d = GradientDotV(Hash(xi1, yi1, zi0), x1, y1, z0);
	float e = GradientDotV(Hash(xi0, yi0, zi1), x0, y0, z1);
	float f = GradientDotV(Hash(xi1, yi0, zi1), x1, y0, z1);
	float g = GradientDotV(Hash(xi0, yi1, zi1), x0, y1, z1);
	float h = GradientDotV(Hash(xi1, yi1, zi1), x1, y1, z1);

	float du = QuinticDeriv(tx);
	float dv = QuinticDeriv(ty);
	float dw = QuinticDeriv(tz);

	float k0 = a;
	float k1 = (b - a);
	float k2 = (c - a);
	float k3 = (e - a);
	float k4 = (a + d - b - c);
	float k5 = (a + f - b - e);
	float k6 = (a + g - c - e);
	float k7 = (b + c + e + h - a - d - f - g);

	derivs.x = du * (k1 + k4 * v + k5 * w + k7 * v * w);
	derivs.y = dv * (k2 + k4 * u + k6 * w + k7 * v * w);
	derivs.z = dw * (k3 + k5 * u + k6 * v + k7 * v * w);

	return k0 + k1 * u + k2 * v + k3 * w + k4 * u * v + k5 * u * w +
		k6 * v * w + k7 * u * v * w;
}

uint8_t PerlinNoise::Hash(const int x, const int y,
	const int z) const {
	auto indexOne = permutationTable[x] + y;
	// this is stupid; real perlin code would not have this bug
	if (indexOne > 255) {
		indexOne = 255;
	}
	auto indexTwo = permutationTable[indexOne] + z;
	if (indexTwo > 255) {
		indexTwo = 255;
	}
	return permutationTable[indexTwo];
}

float PerlinNoise::GradientDotV(uint8_t perm, // a value between 0 and 255 
	float x, float y, float z) const {
	switch (perm & 15) {
		case  0: return  x + y; // (1,1,0) 
		case  1: return -x + y; // (-1,1,0) 
		case  2: return  x - y; // (1,-1,0) 
		case  3: return -x - y; // (-1,-1,0) 
		case  4: return  x + z; // (1,0,1) 
		case  5: return -x + z; // (-1,0,1) 
		case  6: return  x - z; // (1,0,-1) 
		case  7: return -x - z; // (-1,0,-1) 
		case  8: return  y + z; // (0,1,1), 
		case  9: return -y + z; // (0,-1,1), 
		case 10: return  y - z; // (0,1,-1), 
		case 11: return -y - z; // (0,-1,-1) 
		case 12: return  y + x; // (1,1,0) 
		case 13: return -x + y; // (-1,1,0) 
		case 14: return -y + z; // (0,-1,1)
		case 15:
		default:
			return -y - z; // (0,-1,-1) 
	}
}

float PerlinNoise::Quintic(const float t) const {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float PerlinNoise::QuinticDeriv(const float t) const {
	return 30 * t * t * (t * (t - 2) + 1);
}
