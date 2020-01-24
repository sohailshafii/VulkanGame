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

	const std::vector<Vertex>& GetVertices() {
		return vertices;
	}

	const std::vector<uint32_t>& GetIndices() {
		return indices;
	}

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};



