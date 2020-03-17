#pragma once

// NOTE: these two lines are necessary for hashing!!!!
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// TODO: need different versions of vertex:
// one with pos, color, normal and tex

// We could probably have a one-size-fits-all struct that accomodates
// different vertex data. but we won't over-engineer for now

struct VertexPos {
	glm::vec3 pos;
	
	VertexPos(const glm::vec3& pos) : pos(pos) { }
	
	bool operator==(const VertexPos& other) const {
		return pos == other.pos;
	}
	
	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPos);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	
	static std::array<VkVertexInputAttributeDescription, 1> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPos, pos);
		return attributeDescriptions;
	}
};

struct VertexPosColor {
	glm::vec3 pos;
	glm::vec3 color;
	
	VertexPosColor(const glm::vec3& pos, const glm::vec3& color) :
		pos(pos), color(color) { }
	
	bool operator==(const VertexPosColor& other) const {
		return pos == other.pos && color == other.color;
	}
	
	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosColor);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	
	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosColor, pos);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosColor, color);
		return attributeDescriptions;
	}
};

struct VertexPosTex {
	glm::vec3 pos;
	glm::vec2 texCoord;
	
	VertexPosTex(const glm::vec3& pos, const glm::vec2& texCoord) :
		pos(pos), texCoord(texCoord) { }
	
	bool operator==(const VertexPosTex& other) const {
		return pos == other.pos && texCoord == other.texCoord;
	}
	
	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosTex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	
	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosTex, pos);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosTex, texCoord);
		return attributeDescriptions;
	}
};

struct VertexPosNormal {
	glm::vec3 pos;
	glm::vec3 normal;
	
	VertexPosNormal(const glm::vec3& pos, const glm::vec3& normal) :
		pos(pos), normal(normal) { }
	
	bool operator==(const VertexPosNormal& other) const {
		return pos == other.pos && normal == other.normal;
	}
	
	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosNormal);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	
	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosNormal, pos);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosNormal, normal);
		return attributeDescriptions;
	}
};

struct VertexPosNormalTexCoord {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;
	
	VertexPosNormalTexCoord(const glm::vec3& pos, const glm::vec3& normal,
							const glm::vec2& texCoord) :
		pos(pos), normal(normal), texCoord(texCoord) { }
	
	bool operator==(const VertexPosNormalTexCoord& other) const {
		return pos == other.pos && normal == other.normal
			&& texCoord == other.texCoord;
	}
	
	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosNormalTexCoord);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	
	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosNormalTexCoord, pos);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosNormalTexCoord, normal);
		
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexPosNormalTexCoord, texCoord);
		return attributeDescriptions;
	}
};

struct VertexPosColorTexCoord {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	
	VertexPosColorTexCoord(const glm::vec3& pos, const glm::vec3& color,
						const glm::vec2& texCoord) :
		pos(pos), color(color), texCoord(texCoord) { }

	bool operator==(const VertexPosColorTexCoord& other) const {
		return pos == other.pos && color == other.color
			&& texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosColorTexCoord);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosColorTexCoord, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosColorTexCoord, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexPosColorTexCoord, texCoord);

		return attributeDescriptions;
	}
};

struct VertexPosNormalColorTexCoord {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoord;
	
	VertexPosNormalColorTexCoord(const glm::vec3& pos,
								 const glm::vec3& normal,
								 const glm::vec3& color,
								 const glm::vec2& texCoord) :
		pos(pos), normal(normal), color(color), texCoord(texCoord) { }

	bool operator==(const VertexPosNormalColorTexCoord& other) const {
		return pos == other.pos && normal == other.normal
			&& color == other.color
			&& texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(VertexPosNormalColorTexCoord);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(VertexPosNormalColorTexCoord, pos);
		
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(VertexPosNormalColorTexCoord, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(VertexPosNormalColorTexCoord, color);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(VertexPosNormalColorTexCoord, texCoord);

		return attributeDescriptions;
	}
};

namespace std {
	template<> struct hash<VertexPos> {
		size_t operator()(VertexPos const& vertex) const {
			return hash<glm::vec3>()(vertex.pos);
		}
	};

	template<> struct hash<VertexPosColor> {
		size_t operator()(VertexPosColor const& vertex) const {
			return hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1);
		}
	};

	template<> struct hash<VertexPosTex> {
		size_t operator()(VertexPosTex const& vertex) const {
			return hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);;
		}
	};

	template<> struct hash<VertexPosNormalTexCoord> {
		size_t operator()(VertexPosNormalTexCoord const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};

	template<> struct hash<VertexPosColorTexCoord> {
		size_t operator()(VertexPosColorTexCoord const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};

	template<> struct hash<VertexPosNormalColorTexCoord> {
		size_t operator()(VertexPosNormalColorTexCoord const& vertex) const {
			return (((hash<glm::vec3>()(vertex.pos) ^
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
					(hash<glm::vec3>()(vertex.color) << 1) >> 1) ^
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

