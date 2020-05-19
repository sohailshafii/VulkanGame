#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.h>

// we have to make sure everything is aligned properly
// so make sure their offsets are properly divisible
struct UniformBufferObjectUnlitTintedTexVert {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObjectGerstnerVert {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) float time;
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
		WavySurface };
	
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
	
	static VkDescriptorSetLayout CreateWavySurfaceDescriptorSetLayout(VkDevice
																		device);
	static void UpdateDescriptorSetWavySurface(VkDevice device,
												VkDescriptorSet descriptorSet,
												VkImageView textureImageView,
												VkSampler textureSampler,
												VkDescriptorBufferInfo *bufferInfoVert);
	static VkDescriptorPool CreateDescriptorPoolWavySurface(VkDevice device,
															size_t numSwapChainImages);
};

