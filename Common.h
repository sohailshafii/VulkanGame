#pragma once

#include "vulkan/vulkan.h"
#include <array>
#include <glm/glm.hpp>
// NOTE: these two lines are necessary for hashing!!!!
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class LogicalDeviceManager;
class GfxDeviceManager;

class Common {
public:
	static VkImageView createImageView(VkImage image, VkFormat format,
		VkImageAspectFlags aspectFlags, uint32_t mipLevels,
		LogicalDeviceManager* logicalDeviceManager);

	static VkFormat findSupportedFormat(const std::vector<VkFormat>&
		candidates, VkImageTiling tiling, VkFormatFeatureFlags
		features, VkPhysicalDevice physicalDevice);

	static VkFormat findDepthFormat(VkPhysicalDevice physicalDevice);

	static VkCommandBuffer beginSingleTimeCommands(VkCommandPool commandPool,
		LogicalDeviceManager* logicalDeviceManager);
	static void endSingleTimeCommands(VkCommandBuffer commandBuffer,
		VkCommandPool commandPool, LogicalDeviceManager* logicalDeviceManager);

	static void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout, uint32_t mipLevels, VkCommandPool commandPool,
		LogicalDeviceManager* logicalDeviceManager);

	static bool hasStencilComponent(VkFormat format);

	static void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,
		LogicalDeviceManager* logicalDeviceManager, GfxDeviceManager* gfxDeviceManager);

	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,
		GfxDeviceManager* gfxDeviceManager);
};

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color
			&& texCoord == other.texCoord;
	}

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

// we have to make sure everything is aligned properly
// so make sure their offsets are properly divisible
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

