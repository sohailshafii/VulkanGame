#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <memory>

class GfxDeviceManager;
class LogicalDeviceManager;

class ImageTextureLoader {
public:
	ImageTextureLoader(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);
	~ImageTextureLoader();

private:
	void createTextureImage(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void generateMipmaps(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool, VkImage image, VkFormat imageFormat,
		uint32_t texWidth, uint32_t texHeight, uint32_t mipLevel);
	void copyBufferToImage(VkCommandPool commandPool, VkBuffer buffer,
		VkImage image, uint32_t width, uint32_t height);

	void createTextureImageView();
	void createTextureSampler();

	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};
