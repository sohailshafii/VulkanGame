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
#define MAX_STALK_COUNT 4

struct RipplePointLocal {
	alignas(16) glm::vec4 ripplePosition;
	alignas(4) float rippleDuration;
	alignas(4) float rippleStartTime;
};

struct StalkPointLocal {
	alignas(16) glm::vec4 stalkPosition;
	alignas(4) float stalkSpawnTime;
};

struct UniformBufferObjectModelViewProjRipple {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) RipplePointLocal ripplePointsLocal[MAX_RIPPLE_COUNT];
	alignas(16) StalkPointLocal stalkPointsLocal[MAX_STALK_COUNT];
	alignas(4) float time;
	alignas(4) float shudderStartTime;
	alignas(4) float shudderDuration;
};

struct UniformBufferFragUnlitColor {
	alignas(16) glm::vec4 objectColor;
};

struct UniformBufferObjectLighting {
	alignas(16) glm::vec3 lightPosition;
};

class LogicalDeviceManager;
class TextureCreator;

// Defines descriptor sets for different, pre-defined materials
class DescriptorSetFunctions
{
public:
	enum class MaterialType : char { UnlitColor = 0, UnlitTintedTextured,
		WavySurface, BumpySurface, MotherShip, Text };
	
	static VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice device,
														   MaterialType materialType);
	static void UpdateDescriptorSet(VkDevice device,
									MaterialType materialType,
									VkDescriptorSet descriptorSet,
									TextureCreator* imageTextureLoader,
									glm::vec4 const & tintColor,
									VkDescriptorBufferInfo* bufferInfoVert,
									VkDescriptorBufferInfo* bufferInfoFrag);
	
	static VkDescriptorPool CreateDescriptorPool(VkDevice device, MaterialType materialType,
									 size_t numSwapChainImages);

	static VkDescriptorSetLayout CreateTextDescriptorSetLayout(VkDevice device);
	static void UpdateDescriptorSetText(VkDevice device,
		VkDescriptorSet descriptorSet,
		glm::vec4 const& tintColor,
		VkImageView textureImageView,
		VkSampler textureSampler,
		VkDescriptorBufferInfo* bufferInfoVert,
		VkDescriptorBufferInfo* bufferInfoFrag);
	static VkDescriptorPool CreateDescriptorPoolText(VkDevice device,
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

