#pragma once

#include "vulkan/vulkan.h"

// Represents common Image and the view onto it, etc.
class CommonImage {
public:
	CommonImage();
	~CommonImage();

private:
	VkImage image;
	VkImageView imageView;
};
