#pragma once

#include "vulkan/vulkan.h"
#include <array>
#include <vector>
#include <glm/glm.hpp>

class LogicalDeviceManager;
class GfxDeviceManager;

class Common {
public:
	static VkImageView CreateImageView(VkImage image, VkFormat format,
		VkImageAspectFlags aspectFlags, uint32_t mipLevels,
		LogicalDeviceManager* logicalDeviceManager);

	static VkFormat FindSupportedFormat(const std::vector<VkFormat>&
		candidates, VkImageTiling tiling, VkFormatFeatureFlags
		features, VkPhysicalDevice physicalDevice);

	static VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

	static VkCommandBuffer BeginSingleTimeCommands(VkCommandPool commandPool,
		LogicalDeviceManager* logicalDeviceManager);
	static void EndSingleTimeCommands(VkCommandBuffer commandBuffer,
		VkCommandPool commandPool, LogicalDeviceManager* logicalDeviceManager);

	static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
		VkImageLayout newLayout, uint32_t mipLevels, VkCommandPool commandPool,
		LogicalDeviceManager* logicalDeviceManager);

	static bool HasStencilComponent(VkFormat format);

	static void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
		VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory,
		LogicalDeviceManager* logicalDeviceManager, GfxDeviceManager* gfxDeviceManager);

	static uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties,
		GfxDeviceManager* gfxDeviceManager);

	static void CreateBuffer(LogicalDeviceManager* logicalDeviceManager,
		GfxDeviceManager* gfxDeviceManager, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	
	static void CopyBuffer(LogicalDeviceManager* logicalDeviceManager, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};

// we have to make sure everything is aligned properly
// so make sure their offsets are properly divisible
struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

