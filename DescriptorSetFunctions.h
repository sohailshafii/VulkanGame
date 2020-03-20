#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.h>

// we have to make sure everything is aligned properly
// so make sure their offsets are properly divisible
struct UniformBufferObjectVert {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObjectLighting {
	alignas(16) glm::vec3 lightPosition;
};

class LogicalDeviceManager;

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
									VkDescriptorBufferInfo* bufferInfoVert,
									VkDescriptorBufferInfo* bufferInfoFrag);
	
	static VkDescriptorPool CreateDescriptorPool(VkDevice device, MaterialType materialType,
									 size_t numSwapChainImages);
	
	static VkDescriptorSetLayout CreateUnlitTintedTexturedDescriptorSetLayout(VkDevice device);
	static void UpdateDescriptorSetUnlitTintedTextured(VkDevice device,
													   VkDescriptorSet descriptorSet,
													   VkImageView textureImageView,
													   VkSampler textureSampler,
													   VkDescriptorBufferInfo* bufferInfoVert);
	static VkDescriptorPool CreateDescriptorPoolUnlitTintedTextured(VkDevice device,
																   size_t numSwapChainImages);
	
	static VkDescriptorSetLayout CreateSimpleLambertianDescriptorSetLayout(VkDevice
																		   device);
	static void UpdateDescriptorSetSimpleLambertian(VkDevice device,
													VkDescriptorSet descriptorSet,
													VkImageView textureImageView,
													VkSampler textureSampler,
													VkDescriptorBufferInfo *bufferInfoVert,
													VkDescriptorBufferInfo *bufferInfoFrag);
	static VkDescriptorPool CreateDescriptorPoolSimpleLambertian(VkDevice device,
																 size_t numSwapChainImages);
};

