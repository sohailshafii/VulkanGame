#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vulkan/vulkan.h>

// we have to make sure everything is aligned properly
// so make sure their offsets are properly divisible
struct UniformBufferObjectModelViewProj {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObjectModelViewProjTime {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) float time;
};

#define MAX_RIPPLE_COUNT 10

struct UniformBufferObjectModelViewProjRipple {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	//alignas(16) glm::vec3 ripplePointsLocal[MAX_RIPPLE_COUNT];
	alignas(4) float rippleStartTime;// [MAX_RIPPLE_COUNT] ;
	alignas(4) float time;
	alignas(4) float maxRippleDuration;
};

struct UniformBufferFragUnlitColor {
	alignas(16) glm::vec4 objectColor;
};

struct UniformBufferObjectLighting {
	alignas(16) glm::vec3 lightPosition;
};

class LogicalDeviceManager;
class ImageTextureLoader;

// Defines descriptor sets for different, pre-defined materials
class DescriptorSetFunctions
{
public:
	enum MaterialType { UnlitColor = 0, UnlitTintedTextured,
		WavySurface, BumpySurface, MotherShip };
	
	static VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device,
														   MaterialType materialType);
	static void UpdateDescriptorSet(VkDevice device,
									MaterialType materialType,
									VkDescriptorSet descriptorSet,
									ImageTextureLoader* imageTextureLoader,
									glm::vec4 const & tintColor,
									VkDescriptorBufferInfo* bufferInfoVert,
									VkDescriptorBufferInfo* bufferInfoFrag);
	
	static VkDescriptorPool CreateDescriptorPool(VkDevice device, MaterialType materialType,
									 size_t numSwapChainImages);

	static VkDescriptorSetLayout CreateUnlitColorDescriptorSetLayout(
		VkDevice device);
	static void UpdateDescriptorSetUnlitColor(VkDevice device,
										VkDescriptorSet descriptorSet,
										glm::vec4 const& tintColor,
										VkDescriptorBufferInfo* bufferInfoVert,
										VkDescriptorBufferInfo* bufferInfoFrag);
	static VkDescriptorPool CreateDescriptorPoolUnlitColor(VkDevice device,
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

	static VkDescriptorSetLayout CreateBumpySurfaceDescriptorSetLayout(
		VkDevice device);
	static void UpdateDescriptorSetBumpySurface(VkDevice device,
		VkDescriptorSet descriptorSet,
		VkImageView textureImageView,
		VkSampler textureSampler,
		VkDescriptorBufferInfo* bufferInfoVert);
	static VkDescriptorPool CreateDescriptorPoolBumpySurface(VkDevice device,
		size_t numSwapChainImages);
};

