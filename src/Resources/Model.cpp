#include "Resources/Model.h"

#include <string>
#include <unordered_map>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>

Model::Model(const std::string& modelPath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn,
		&err, modelPath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<ModelVert, uint32_t> uniqueVertices = {};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			ModelVert vertex = {};
			vertex.position = {
				attrib.vertices[3 * (size_t)index.vertex_index],
				attrib.vertices[3 * (size_t)index.vertex_index + 1],
				attrib.vertices[3 * (size_t)index.vertex_index + 2]
			};

			if (attrib.texcoords.size() > 0) {
				vertex.texCoord = {
					attrib.texcoords[2 * (size_t)index.texcoord_index],
					// vulkan is top to bottom for texture
					1.0f - attrib.texcoords[2 * (size_t)index.texcoord_index + 1]
				};
			}

			if (attrib.colors.size() > 0) {
				vertex.color = { attrib.colors[3 * (size_t)index.vertex_index],
					attrib.colors[3 * (size_t)index.vertex_index + 1],
					attrib.colors[3 * (size_t)index.vertex_index + 2]
				};
			}
			
			if (attrib.normals.size() > 0) {
				vertex.normal = { attrib.normals[3 * (size_t)index.normal_index],
					attrib.normals[3 * (size_t)index.normal_index + 1],
					attrib.normals[3 * (size_t)index.normal_index + 2]
				};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>
					(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}

	modelTopology = TopologyType::TriangleList;
}

Model::Model(const std::vector<ModelVert>& vertices,
	  const std::vector<uint32_t>& indices,
		TopologyType modelTopology) : vertices(vertices),
	indices(indices), modelTopology(modelTopology) {
}

Model::~Model() {

}

std::shared_ptr<Model> Model::CreateQuad(const glm::vec3& lowerLeft,
	const glm::vec3& side1Vec, const glm::vec3& side2Vec,
	uint32_t numSide1Points, uint32_t numSide2Points)
{
	std::vector<ModelVert> vertices;
	std::vector<uint32_t> indices;

	// make sure side1 and side2 are perpendicular
	if (fabs(glm::dot(side1Vec, side2Vec)) > 0.0f)
	{
		std::cerr << "Side vectors are not perpendicular; "
			<< "cannot create quad.\n";
		return nullptr;
	}

	glm::vec3 quadPoint;
	// if there are n points, there are (n-1) divisions
	glm::vec3 side1Div = side1Vec / (float)(numSide1Points - 1);
	glm::vec3 side2Div = side2Vec / (float)(numSide2Points - 1);

	// for each piece in side 1 (row)
	glm::vec3 normal(0.0f, 1.0f, 0.0f);
	glm::vec2 texCoord(0.0f, 0.0f);
	float uDiv = 1.0f / (float)(numSide1Points - 1);
	float vDiv = 1.0f / (float)(numSide2Points - 1);
	for (uint32_t side1Index = 0; side1Index < numSide1Points;
		side1Index++)
	{
		quadPoint = (float)side1Index * side1Div;
		texCoord = glm::vec2((float)side1Index * uDiv, 0.0f);
		// for each piece in side 2 (column)
		for (uint32_t side2Index = 0; side2Index < numSide2Points;
			side2Index++)
		{
			quadPoint += side2Div;
			texCoord.y = 1.0f - (float)side2Index * vDiv;
			vertices.push_back(Model::ModelVert(quadPoint, normal,
				glm::vec3(1.0f, 1.0f, 1.0f), texCoord));
		}
	}

	for (uint32_t side1Index = 0; side1Index < numSide1Points - 1;
		side1Index++)
	{
		uint32_t nextSide1Index = side1Index + 1;
		for (uint32_t side2Index = 0; side2Index < numSide2Points;
			side2Index++)
		{
			// add bottom and top index of triangle strip each time
			uint32_t oneDimIndexBottom = side1Index * numSide2Points
				+ side2Index;
			uint32_t oneDimIndexTop = nextSide1Index * numSide2Points
				+ side2Index;
			indices.push_back(oneDimIndexTop);
			indices.push_back(oneDimIndexBottom);
		}
	}

	return std::make_shared<Model>(vertices, indices,
		TopologyType::TriangleStrip);
}

