#pragma once

// NOTE: these two lines are necessary for hashing!!!!
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <vector>
#include <array>
#include <glm/glm.hpp>
#include "vulkan/vulkan.h"
#include "Vertex.h"

class Model {
public:
	struct ModelVert {
		ModelVert() { }
		ModelVert(const glm::vec3& position) :
			position(position) { }
		ModelVert(const glm::vec3& position, const glm::vec3&
				  normal) : position(position),
			normal(normal) { }
		
		ModelVert(const glm::vec3& position, const glm::vec3&
				  normal, const glm::vec3& color) :
			position(position), normal(normal),
			color(color) { }
		ModelVert(const glm::vec3& position, const glm::vec3&
				  normal, const glm::vec3& color,
				  const glm::vec2& texCoord) : position(position),
			normal(normal), color(color), texCoord(texCoord) { }
		
		bool operator==(const ModelVert& other) const {
			return position == other.position && normal == other.normal
				&& color == other.color
				&& texCoord == other.texCoord;
		}
		
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;
	};
	
	Model() {}
	Model(const std::string& modelPath);
	Model(const std::vector<ModelVert>& vertices,
		  const std::vector<uint32_t>& indices);
	~Model();
	
	static std::shared_ptr<Model> CreateQuad(const glm::vec3& lowerLeft,
		const glm::vec3& side1Vec, const glm::vec3& side2Vec,
		float numSide1Points, float numSide2Points);
	
	const std::vector<VertexPos> BuildAndReturnVertsPos() {
		auto vertsToBuild = std::vector<VertexPos>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPos(modelVert.position));
		}
		return vertsToBuild;
	}
	
	const std::vector<VertexPosColor> BuildAndReturnVertsPosColor() {
		auto vertsToBuild = std::vector<VertexPosColor>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPosColor(modelVert.position,
												  modelVert.color));
		}
		return vertsToBuild;
	}
	
	const std::vector<VertexPosTex> BuildAndReturnVertsPosTex() {
		auto vertsToBuild = std::vector<VertexPosTex>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPosTex(modelVert.position,
												  modelVert.texCoord));
		}
		return vertsToBuild;
	}

	const std::vector<VertexPosNormalTexCoord> BuildAndReturnVertsPosNormalTexCoord() {
		auto vertsToBuild = std::vector<VertexPosNormalTexCoord>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPosNormalTexCoord(modelVert.position,
														   modelVert.normal,
														   modelVert.texCoord));
		}
		return vertsToBuild;
	}
	
	const std::vector<VertexPosColorTexCoord> BuildAndReturnVertsPosColorTexCoord() {
		auto vertsToBuild = std::vector<VertexPosColorTexCoord>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPosColorTexCoord(modelVert.position,
														   modelVert.color,
														   modelVert.texCoord));
		}
		return vertsToBuild;
	}
	
	const std::vector<VertexPosNormalColorTexCoord> BuildAndReturnVertsPosNormalColorTexCoord()
	{
		auto vertsToBuild = std::vector<VertexPosNormalColorTexCoord>();
		for (auto &modelVert : vertices) {
			vertsToBuild.push_back(VertexPosNormalColorTexCoord(modelVert.position,
																modelVert.normal,
																modelVert.color,
																modelVert.texCoord));
		}
		return vertsToBuild;
	}

	const std::vector<uint32_t>& GetIndices() {
		return indices;
	}

private:
	std::vector<ModelVert> vertices;
	std::vector<uint32_t> indices;
};

namespace std {
	template<> struct hash<Model::ModelVert> {
		size_t operator()(Model::ModelVert const& vertex) const {
			return (((hash<glm::vec3>()(vertex.position) ^
					  (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
					(hash<glm::vec3>()(vertex.color) << 1) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

