#include "ImageTextureLoader.h"

#include "stb_image.h"
#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "Common.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"

ImageTextureLoader::ImageTextureLoader(const std::string& path,
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	VkCommandPool commandPool) {
	this->logicalDeviceManager = logicalDeviceManager;
	CreateTextureImage(path, gfxDeviceManager, commandPool);
	CreateTextureImageView();
	CreateTextureSampler();
}

ImageTextureLoader::~ImageTextureLoader() {
	vkDestroySampler(logicalDeviceManager->GetDevice(), textureSampler, nullptr);
	vkDestroyImageView(logicalDeviceManager->GetDevice(), textureImageView, nullptr);

	vkDestroyImage(logicalDeviceManager->GetDevice(), textureImage, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), textureImageMemory, nullptr);
}

void ImageTextureLoader::CreateTextureImage(const std::string& path,
	GfxDeviceManager* gfxDeviceManager, VkCommandPool commandPool) {
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(path.c_str(),
		&texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = (VkDeviceSize)texWidth *
		(VkDeviceSize)texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}
	mipLevels = static_cast<uint32_t>(std::floor(
		std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	Common::CreateBuffer(logicalDeviceManager.get(), gfxDeviceManager, imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, 0, imageSize,
		0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory);

	stbi_image_free(pixels);

	Common::CreateImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory,
		logicalDeviceManager.get(), gfxDeviceManager);

	Common::TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels, commandPool, logicalDeviceManager.get());
	// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	// while generating mipmaps

	CopyBufferToImage(commandPool, stagingBuffer, textureImage,
		static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	/*transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		mipLevels);*/

	vkDestroyBuffer(logicalDeviceManager->GetDevice(), stagingBuffer, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), stagingBufferMemory, nullptr);
	GenerateMipmaps(gfxDeviceManager, commandPool, textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
}

void ImageTextureLoader::GenerateMipmaps(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool, VkImage image, VkFormat imageFormat,
	uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels) {
	// check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(gfxDeviceManager->GetPhysicalDevice(),
		imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures &&
		VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("Texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = Common::BeginSingleTimeCommands(commandPool,
		logicalDeviceManager.get());

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;
	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 :
			1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer, image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr, 0, nullptr, 1, &barrier);

	Common::EndSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager.get());
}

void ImageTextureLoader::CopyBufferToImage(VkCommandPool commandPool,
	VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	VkCommandBuffer commandBuffer = Common::BeginSingleTimeCommands(commandPool,
		logicalDeviceManager.get());

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	Common::EndSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager.get());
}

void ImageTextureLoader::CreateTextureImageView() {
	textureImageView = Common::CreateImageView(textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, logicalDeviceManager.get());
}

void ImageTextureLoader::CreateTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;

	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);

	if (vkCreateSampler(logicalDeviceManager->GetDevice(), &samplerInfo, nullptr,
		&textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler!");
	}
}
