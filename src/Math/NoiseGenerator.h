#pragma once

#include <glm/glm.hpp>

enum class NoiseGeneratorType : char { None = 0, Perlin };

class NoiseGenerator {
public:
	
	virtual ~NoiseGenerator() {
		
	}
	
	virtual float Eval(const glm::vec3& p, glm::vec3& derivs) const = 0;

private:
};
