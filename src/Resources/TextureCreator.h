#pragma once

#include "vulkan/vulkan.h"
#include <string>
#include <memory>

class GfxDeviceManager;
class LogicalDeviceManager;

class TextureCreator {
public:
	TextureCreator(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);
	TextureCreator(unsigned char* pixels,
		int texWidth, int texHeight, int bytesPerPixel,
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		VkCommandPool commandPool);
	~TextureCreator();

	VkDeviceMemory GetTextureImageMemory() {
		return textureImageMemory;
	}

	VkImageView GetTextureImageView() {
		return textureImageView;
	}

	VkSampler GetTextureImageSampler() {
		return textureSampler;
	}

private:
	void CreateTextureImageFromFile(const std::string& path,
		GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateTextureImage(unsigned char* pixels,
		int texWidth, int texHeight, int bytesPerPixel,
		GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void GenerateMipmaps(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool, VkImage image, VkFormat imageFormat,
		uint32_t texWidth, uint32_t texHeight, uint32_t mipLevel);
	void CopyBufferToImage(VkCommandPool commandPool, VkBuffer buffer,
		VkImage image, uint32_t width, uint32_t height);

	void CreateTextureImageView(int bytesPerPixel);
	void CreateTextureSampler();

	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	uint32_t mipLevels;
	VkImage textureImage;

	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;
};
