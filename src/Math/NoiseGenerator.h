#pragma once

#include <glm/glm.hpp>

class NoiseGenerator {
public:
	virtual float Eval(const glm::vec3& p, glm::vec3& derivs) const = 0;

private:
};
