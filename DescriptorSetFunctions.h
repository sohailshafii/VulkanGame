#pragma once

#include <vulkan/vulkan.h>

// Defines descriptor sets for different, pre-defined materials
class DescriptorSetFunctions
{
public:
	enum MaterialType { UnlitTintedTextured = 0,
		SimpleLambertian };
	
	static VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device,
														   MaterialType materialType);
	static void UpdateDescriptorSet(VkDevice device,
									MaterialType materialType,
									VkDescriptorSet descriptorSet,
									VkImageView textureImageView,
									VkSampler textureSampler,
									VkDescriptorBufferInfo* bufferInfo);
	
	static VkDescriptorSetLayout CreateUnlitTintedTexturedDescriptorSetLayout(VkDevice device);
	static void UpdateDescriptorSetUnlitTintedTextured(VkDevice device,
													   VkDescriptorSet descriptorSet,
													   VkImageView textureImageView,
													   VkSampler textureSampler,
													   VkDescriptorBufferInfo* bufferInfo);
};

