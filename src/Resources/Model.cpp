#include "Resources/Model.h"

#include <string>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>
#include <vector>
#include <set>
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
std::shared_ptr<Model> Model::CreateIcosahedron(float radius,
												uint32_t numSubdivisions) {
	// 360 degrees divided by 5 is 72.0 degrees
	const float circumDivAngle = 72.0f * (float)M_PI / 180.0f;
	// elevation angle, assuming vertex of icosahedron is
	// 1 unit in y, 2 in x
	const float verticalAngle = (90.0f * (float)M_PI / 180.0f) - atanf(1.0f / 2.0f);
	const float verticalAngle2 = (90.0f * (float)M_PI / 180.0f) + atanf(1.0f / 2.0f);
	
	// 22 vertices to start with. five at each pole,
	// six across at verticalAngle, and then another
	// six across at 90 degrees + verticalAngle
	std::vector<ModelVert> vertices(22);
	std::vector<uint32_t> indices;
	// start at -126 at 1st row, -90 on second row
	float hAngle1 = -(float)M_PI * 0.5f - circumDivAngle * 0.5f;
	float hAngle2 = -(float)M_PI * 0.5f;
	// 11 U divisions for texture coordinates
	// see http://www.songho.ca/opengl/gl_sphere.html
	// for original icosahedron code
	float uDiv = 1.0f / 11.0f;
	float vDiv = 1.0f / 3.0f;
	glm::vec3 minColor = glm::vec3(0.1f, 0.1f, 0.1f),
		maxColor = glm::vec3(1.0f, 1.0f, 1.0f);
	
	std::unordered_map<uint32_t, std::set<TriangleEdgeSet>> vertexNeighbors;

	// top-most pole has several verts, each with its own
	// texture coordinate
	for (uint32_t i = 0; i < 5; i++) {
		vertices[i].position = glm::vec3(0.0f, radius, 0.0f);
		vertices[i].color = maxColor;
		vertices[i].texCoord = glm::vec2((1.0f + 2.0f * (float)i) * uDiv,
			0.0f);
	}
	
	// ten verts on first and second rows
	// compute elevation and length on plane
	float y = radius * cosf(verticalAngle);
	float xz = radius * sinf(verticalAngle);
	float y2 = radius * cosf(verticalAngle2);
	float xz2 = radius * sinf(verticalAngle2);
	for (uint32_t row1Index = 5; row1Index <= 10; ++row1Index) {
		// second row
		uint32_t row2Index = (row1Index + 6);
		uint32_t offsetIntoRow = (row1Index - 5);

		vertices[row1Index].position =
			glm::vec3(xz * sinf(hAngle1),
					y,
					xz * cosf(hAngle1));
		vertices[row1Index].color = minColor*0.333f + maxColor*0.6667f;
		vertices[row1Index].texCoord = glm::vec2(
			uDiv * (float)offsetIntoRow * 2.0f, vDiv);
		
		vertices[row2Index].position = glm::vec3(xz2 * sinf(hAngle2),
												y2,
												 xz2 * cosf(hAngle2));
		vertices[row2Index].color = minColor * 0.6667f + maxColor * 0.333f;
		vertices[row2Index].texCoord = glm::vec2(
			uDiv * (1.0f + (float)offsetIntoRow * 2.0f), 2.0f * vDiv);
		
		hAngle1 += circumDivAngle;
		hAngle2 += circumDivAngle;
	}
	
	for (uint32_t i = 17; i < 22; i++) {
		vertices[i].position = glm::vec3(0.0f,-radius, 0.0f);
		vertices[i].color = minColor;
		vertices[i].texCoord = glm::vec2((2.0f * (float)(i - 17)) * uDiv,
			3.0f * vDiv);
	}
	
	// top pole triangles
	uint32_t row1StartIndex = 5;
	for (uint32_t i = 0; i < 5; i++) {
		AddIcosahedronIndices(indices, i,
			row1StartIndex + i, row1StartIndex + i + 1, vertexNeighbors);
	}
	
	uint32_t row2StartIndex = 11;
	// first row above pole
	for (uint32_t i = 0; i < 5; i++) {
		uint32_t firstRow1Index = row1StartIndex + i;
		uint32_t secondRow1Index = firstRow1Index + 1;
		uint32_t firstRow2Index = row2StartIndex + i;
		uint32_t secondRow2Index = firstRow2Index + 1;
		AddIcosahedronIndices(indices, firstRow2Index,
			secondRow1Index, firstRow1Index, vertexNeighbors);
		AddIcosahedronIndices(indices, secondRow1Index,
			firstRow2Index, secondRow2Index, vertexNeighbors);
	}
	
	// bottom pole
	uint32_t poleStartIndex = 17;
	for (uint32_t i = 0; i < 5; i++) {
		AddIcosahedronIndices(indices, poleStartIndex + i,
			row2StartIndex + i + 1, row2StartIndex + i,
			vertexNeighbors);
	}
	
	SubdivideIcosahedron(vertices, indices, numSubdivisions, radius,
		vertexNeighbors);

	CalculateNormalVectors(vertices, vertexNeighbors);
	
	return std::make_shared<Model>(vertices, indices,
								   TopologyType::TriangleList);
}

void Model::AddIcosahedronIndices(std::vector<uint32_t>& indices,
									uint32_t index1, uint32_t index2,
									uint32_t index3,
									std::unordered_map<uint32_t,std::set<TriangleEdgeSet>>
									& vertexNeighbors) {
	indices.push_back(index1);
	indices.push_back(index2);
	indices.push_back(index3);
	
	vertexNeighbors[index1].insert(TriangleEdgeSet(index2, index3));
	vertexNeighbors[index2].insert(TriangleEdgeSet(index3, index1));
	vertexNeighbors[index3].insert(TriangleEdgeSet(index1, index2));
}

void Model::SubdivideIcosahedron(std::vector<ModelVert>& vertices,
								 std::vector<uint32_t>& indices,
								 uint32_t numSubdivisions,
								 float radius,
								 std::unordered_map<uint32_t, std::set<TriangleEdgeSet>>
								 & vertexNeighbors) {
	ModelVert newV1, newV2, newV3;
	std::vector<uint32_t> tmpIndices;
	std::vector<ModelVert> tmpVerts;

	vertexNeighbors.clear();

	for (uint32_t subDiv = 0; subDiv < numSubdivisions; subDiv++) {
		// build new indices by storing old indices into temp array
		tmpIndices = indices;
		tmpVerts = vertices;
		size_t numCurrentIndices = tmpIndices.size();
		indices.clear();
		vertexNeighbors.clear();
		vertices.clear();
		for (size_t index = 0; index <= numCurrentIndices-3; index += 3) {
			size_t oldIndex1 = tmpIndices[index],
				oldIndex2 = tmpIndices[index + 1],
				oldIndex3 = tmpIndices[index + 2];
			ModelVert const & v1 = tmpVerts[oldIndex1];
			ModelVert const & v2 = tmpVerts[oldIndex2];
			ModelVert const & v3 = tmpVerts[oldIndex3];

			// split each half edge
			ComputeHalfVertex(v1, v2, newV1, radius);
			ComputeHalfVertex(v2, v3, newV2, radius);
			ComputeHalfVertex(v1, v3, newV3, radius);

			uint32_t newV1Index = vertices.size();
			vertices.push_back(newV1);
			uint32_t newV2Index = vertices.size();
			vertices.push_back(newV2);
			uint32_t newV3Index = vertices.size();
			vertices.push_back(newV3);
			oldIndex1 = vertices.size();
			vertices.push_back(v1);
			oldIndex2 = vertices.size();
			vertices.push_back(v2);
			oldIndex3 = vertices.size();
			vertices.push_back(v3);

			// topmost triangle in new subdiv
			AddIcosahedronIndices(indices, oldIndex1,
				newV1Index, newV3Index, vertexNeighbors);
			// center triangle
			AddIcosahedronIndices(indices, newV2Index,
				newV3Index, newV1Index, vertexNeighbors);
			// bottom left triangle
			AddIcosahedronIndices(indices, newV1Index,
				oldIndex2, newV2Index, vertexNeighbors);
			// bottom right triangle
			AddIcosahedronIndices(indices, newV3Index,
				newV2Index, oldIndex3, vertexNeighbors);
		}
	}
}

void Model::ComputeHalfVertex(ModelVert const& v1, ModelVert const& v2,
	ModelVert& halfVertex, float radius)
{
	// really half-vector -- make sure it matches radius
	glm::vec3 halfVertexPos = (v1.position + v2.position);
	// normalize vertex then scale it by radius
	float scale = radius / glm::length(halfVertexPos);
	halfVertexPos *= scale;

	halfVertex.position = halfVertexPos;
	halfVertex.color = 0.5f * (v1.color + v2.color);
	halfVertex.texCoord = 0.5f * (v1.texCoord + v2.texCoord);
}

void Model::CalculateNormalVectors(std::vector<ModelVert>& vertices,
	std::unordered_map<uint32_t, std::set<TriangleEdgeSet>>& vertexNeighbors) {
	size_t numVertices = vertices.size();
	for (size_t vertIndex = 0; vertIndex < numVertices; vertIndex++) {
		ComputeNormal(vertIndex, vertices, vertexNeighbors);
	}
}

glm::vec3 Model::ComputeNormal(uint32_t vertexIndex,
							   std::vector<ModelVert>& vertices,
							   std::unordered_map<uint32_t,std::set<TriangleEdgeSet>> & vertexNeighbors) {
	std::set<TriangleEdgeSet> const & neighbors = vertexNeighbors[vertexIndex];
	glm::vec3 normalVec(0.0f, 0.0f, 0.0f);
	
	glm::vec3 const & ourVertex = vertices[vertexIndex].position;
	size_t numNeighbors = neighbors.size();
	
	for(TriangleEdgeSet const & triangleEdgeSet : neighbors) {
		auto index1 = triangleEdgeSet.pointIndex1;
		auto index2 = triangleEdgeSet.pointIndex2;
		ModelVert const & point1 = vertices[index1];
		ModelVert const& point2 = vertices[index2];

		glm::vec3 vectorToPoint1 = (point1.position - ourVertex);
		glm::normalize(vectorToPoint1);
		glm::vec3 vectorToPoint2 = (point2.position - ourVertex);
		glm::normalize(vectorToPoint2);

		normalVec += glm::cross(vectorToPoint2, vectorToPoint1);
	}

	normalVec /= (float)numNeighbors;
	glm::normalize(normalVec);
	return normalVec;
}
