#include "Resources/Model.h"

#include <string>
#include <unordered_map>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include "Math/NoiseGenerator.h"
#include "Math/PerlinNoise.h"
#include "Math/CommonMath.h"

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

std::shared_ptr<Model> Model::CreatePlane(const glm::vec3& lowerLeft,
	const glm::vec3& side1Vec, const glm::vec3& side2Vec,
	uint32_t numSide1Points, uint32_t numSide2Points,
	NoiseGeneratorType noiseGeneratorType,
	uint32_t numNoiseLayers)
{
	std::vector<ModelVert> vertices;
	std::vector<uint32_t> indices;

	// make sure side1 and side2 are perpendicular
	if (fabs(glm::dot(side1Vec, side2Vec)) > 0.0f)
	{
		std::cerr << "Side vectors are not perpendicular; "
			<< "cannot create plane.\n";
		return nullptr;
	}

	glm::vec3 quadPoint;
	// if there are n points, there are (n-1) divisions
	glm::vec3 side1Div = side1Vec / (float)(numSide1Points - 1);
	glm::vec3 side2Div = side2Vec / (float)(numSide2Points - 1);

	// generate noise, if applicable
	float* noiseValues;
	glm::vec3* normValues;
	GeneratePlaneNoiseAndDerivatives(&noiseValues, &normValues,
								lowerLeft, side1Vec, side2Vec,
								numSide1Points, numSide2Points,
								noiseGeneratorType, numNoiseLayers);

	// for each piece in side 1 (row)
	glm::vec2 texCoord(0.0f, 0.0f);
	float uDiv = 1.0f / (float)(numSide1Points - 1);
	float vDiv = 1.0f / (float)(numSide2Points - 1);
	for (uint32_t side1Index = 0, oneDimIndex = 0; side1Index < numSide1Points;
		side1Index++)
	{
		quadPoint = (float)side1Index * side1Div + lowerLeft;
		texCoord = glm::vec2((float)side1Index * uDiv, 0.0f);
		// for each piece in side 2 (column)
		for (uint32_t side2Index = 0; side2Index < numSide2Points;
			side2Index++, oneDimIndex++)
		{
			quadPoint += side2Div;
			auto displacedPnt = quadPoint;
			auto noiseValue = noiseValues[oneDimIndex];
			//displacedPnt.y += noiseValue;
			texCoord.y = 1.0f - (float)side2Index * vDiv;
			vertices.push_back(Model::ModelVert(displacedPnt,
												normValues[oneDimIndex],
												glm::vec3(1.0f, 1.0f, 1.0f),
												texCoord));
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

	delete[] noiseValues;
	delete[] normValues;
	return std::make_shared<Model>(vertices, indices,
		TopologyType::TriangleStrip);
}

void Model::GeneratePlaneNoiseAndDerivatives(float** noiseValues,
								 glm::vec3** normals,
								 const glm::vec3& lowerLeft,
								 const glm::vec3& side1Vec,
								 const glm::vec3& side2Vec,
								 uint32_t numSide1Points,
								 uint32_t numSide2Points,
								 NoiseGeneratorType noiseGeneratorType,
								 uint32_t numNoiseLayers) {
	uint32_t numTotalPoints = numSide1Points * numSide1Points;
	*noiseValues = new float[numTotalPoints];
	*normals = new glm::vec3[numTotalPoints];
	glm::vec3* derivValues = new glm::vec3[numTotalPoints];
	
	auto noiseValuesPtr = *noiseValues;
	auto normalsPtr = *normals;
	for (uint32_t i = 0; i < numTotalPoints; i++) {
		noiseValuesPtr[i] = 0.0f;
		derivValues[i] = glm::vec3(0.0f, 0.0f, 0.0f);
		derivValues[i] = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (noiseGeneratorType != NoiseGeneratorType::None) {
		NoiseGenerator* noiseGenerator = new PerlinNoise();
		
		glm::vec3 quadPoint;
		// if there are n points, there are (n-1) divisions
		glm::vec3 side1Div = side1Vec / (float)(numSide1Points - 1);
		glm::vec3 side2Div = side2Vec / (float)(numSide2Points - 1);
		// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/perlin-noise-terrain-mesh
		// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/perlin-noise-part-2/perlin-noise-computing-derivatives
		float maxVal = 0;
		for (uint32_t side1Index = 0, oneDimIndex = 0; side1Index < numSide1Points;
			side1Index++)
		{
			quadPoint = (float)side1Index * side1Div + lowerLeft;
			for (uint32_t side2Index = 0; side2Index < numSide2Points;
				side2Index++, oneDimIndex++)
			{
				quadPoint += side2Div;
				float fractal = 0.0f;
				float amplitude = 1.0f;
				for (uint32_t layerIndex = 0; layerIndex < numNoiseLayers; layerIndex++) {
					glm::vec3 deriv;
					// TODO: this perlin code is crap. replace because it has bugs
					fractal += (1.0f + noiseGenerator->Eval(quadPoint, deriv)) * 0.5f * amplitude;
					quadPoint *= 2.0f;
					amplitude *= 0.5f;
					derivValues[oneDimIndex] = deriv;
				}

				if (fractal > maxVal) {
					maxVal = fractal;
				}
				noiseValuesPtr[oneDimIndex] = fractal;
				//std::cout << "fractal " << fractal << std::endl;
				derivValues[oneDimIndex] = glm::normalize(derivValues[oneDimIndex]);
				
				auto derivativeVal = derivValues[oneDimIndex];
				glm::vec3 tangent(1.0f, derivativeVal[0], 0.0f);
				glm::vec3 bitangent(0.0f, derivativeVal[2], 1.0f);
				normalsPtr[oneDimIndex] = glm::normalize(glm::vec3(-derivativeVal[0],
														 1.0f,
														 -derivativeVal[2]));
			}
		}
		for (uint32_t i = 0; i < numTotalPoints; i++) {
			noiseValuesPtr[i] /= maxVal;
		}
		
		delete noiseGenerator;
		delete [] derivValues;
	}
}

// based mostly on http://www.songho.ca/opengl/gl_sphere.html
std::shared_ptr<Model> Model::CreateIcosahedron(glm::vec3 const & origin,
												float radius,
												uint32_t numSubdivisions) {
	const float circumDivAngle = 72.0f * M_PI / 180.0f;
	const float verticalAngle = atanf(1.0f/2.0f);
	
	// 12 vertices to start with. one at each pole,
	// six across at verticalAngle, and then another
	// six across at -verticalAngle
	std::vector<ModelVert> vertices(12);
	std::vector<uint32_t> indices;
	// start at -126 at 1st row, -90 on second row
	float hAngle1 = -M_PI * 0.5f - circumDivAngle * 0.5f;
	float hAngle2 = -M_PI * 0.5f;
	
	// top-most pole
	vertices[0].position = glm::vec3(0.0f, 0.0f, radius);
	
	// ten verts on first and second rows
	for (unsigned int row1 = 1; row1 <= 5; ++row1) {
		// second row
		unsigned int row2 = (row1 + 5);
		
		// compute elevation and length on plane
		float z = radius * sinf(verticalAngle);
		float xy = radius * cosf(verticalAngle);
		
		vertices[row1].position =
			glm::vec3(xy * cosf(hAngle1), xy * sinf(hAngle1),
					  z);
		vertices[row2].position = glm::vec3(xy * cosf(hAngle2),
											xy * sinf(hAngle2),
											-z);
		
		hAngle1 += circumDivAngle;
		hAngle2 += circumDivAngle;
	}
	
	vertices[11].position = glm::vec3(0.0f, 0.0f,-radius);
	
	// top pole triangles
	uint32_t row1StartIndex = 1;
	for (uint32_t i = 0; i < 5; i++) {
		uint32_t secondIndex = row1StartIndex + i + 1;
		if (secondIndex > 5) {
			secondIndex = row1StartIndex;
		}
		AddIcosahedronIndices(indices, 0,
			row1StartIndex + i, secondIndex);
	}
	
	uint32_t row2StartIndex = 6;
	// first row above pole
	for (uint32_t i = 0; i < 5; i++) {
		uint32_t secondRow1Index = row1StartIndex + i + 1;
		uint32_t secondRow2Index = row2StartIndex + i + 1;
		if (secondRow1Index > 5) {
			secondRow1Index = row1StartIndex;
		}
		if (secondRow2Index > 10) {
			secondRow2Index = row2StartIndex;
		}
		AddIcosahedronIndices(indices, row2StartIndex + i,
			secondRow1Index, row1StartIndex + i);
		AddIcosahedronIndices(indices, secondRow1Index,
			row2StartIndex + i, secondRow2Index);
	}
	
	// bottom pole
	for (int i = 0; i < 5; i++) {
		uint32_t secondIndex = row2StartIndex + i + 1;
		if (secondIndex > 10) {
			secondIndex = row2StartIndex;
		}
		AddIcosahedronIndices(indices, 11, secondIndex,
			row2StartIndex + i);
	}
	
	SubdivideIcosahedron(vertices, indices, numSubdivisions);
	
	return std::make_shared<Model>(vertices, indices,
								   TopologyType::TriangleList);
}

void Model::AddIcosahedronIndices(std::vector<uint32_t>& indices,
								  uint32_t index1, uint32_t index2,
									uint32_t index3) {
	indices.push_back(index1);
	indices.push_back(index2);
	indices.push_back(index3);
}

void Model::SubdivideIcosahedron(std::vector<ModelVert>& vertices,
								 std::vector<uint32_t>& indices,
								 uint32_t numSubdivisions) {
	
}
