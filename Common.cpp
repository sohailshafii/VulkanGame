#include "Common.h"
#include "LogicalDeviceManager.h"

VkImageView Common::createImageView(VkImage image, VkFormat format,
	VkImageAspectFlags aspectFlags, uint32_t mipLevels,
	LogicalDeviceManager* logicalDeviceManager) {
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(logicalDeviceManager->getDevice(), &viewInfo, nullptr,
		&imageView) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture image view!");
	}

	return imageView;
}
