#pragma once

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "vulkan/vulkan.h"
#include "Vertex.h"

class Model {
public:
	Model(const std::string& modelPath);
	~Model();

	const std::vector<Vertex>& getVertices() {
		return vertices;
	}

	const std::vector<uint32_t>& getIndices() {
		return indices;
	}

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};



