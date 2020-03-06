#pragma once

#include <vulkan/vulkan.h>

class DescriptorSetFunctions
{
public:
	static VkDescriptorSetLayout CreateUnlitTintedTexturedDescriptorSetLayout(VkDevice device);
};

