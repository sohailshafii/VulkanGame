#include "Model.h"

#include <string>
#include <unordered_map>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

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
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			if (attrib.texcoords.size() > 0) {
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					// vulkan is top to bottom for texture
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (attrib.colors.size() > 0) {
				vertex.color = { attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
				};
			}
			
			if (attrib.normals.size() > 0) {
				vertex.normal = { attrib.normals[3 * index.vertex_index + 0],
					attrib.normals[3 * index.vertex_index + 1],
					attrib.normals[3 * index.vertex_index + 2]
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
}

Model::Model(const std::vector<ModelVert>& vertices,
	  const std::vector<uint32_t>& indices) : vertices(vertices),
	indices(indices) {
}

Model::~Model() {

}

