
#pragma once

#include "NoiseGenerator.h"

class NoNoise : public NoiseGenerator {
public:
	NoNoise() {

	}

	float Eval(const glm::vec3& p, glm::vec3& derivs) const {
		derivs = glm::vec3(0.0f, 0.0f, 0.0f);
		return 0.0f;
	}

};