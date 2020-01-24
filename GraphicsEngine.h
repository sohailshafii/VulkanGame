#pragma once

#include "vulkan/vulkan.h"
#include "Common.h"
#include "Model.h"
#include "GameObject.h"
#include <vector>

class SwapChainManager;
class GfxDeviceManager;
class LogicalDeviceManager;
class RenderPassModule;
class PipelineModule;
class CommandBufferModule;
struct GLFWwindow;
class ImageTextureLoader;
class ResourceLoader;

class GraphicsEngine {
public:
	GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface,
		GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
		VkCommandPool commandPool, std::shared_ptr<ImageTextureLoader> imageTexture, std::vector<std::shared_ptr<GameObject>>& gameObjects);

	~GraphicsEngine();

	SwapChainManager* GetSwapChainManager() { return swapChainManager; }
	RenderPassModule* GetRenderPassModule() { return renderPassModule; }
	PipelineModule* GetPipelineModule() { return graphicsPipelineModule; }
	CommandBufferModule* GetCommandBufferModule() { return commandBufferModule; }
	std::vector<VkDeviceMemory>& GetUniformBuffersMemory() { return uniformBuffersMemory; }

private:
	// not owned by us
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	PipelineModule* graphicsPipelineModule;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	CommandBufferModule* commandBufferModule;

	void CleanUpSwapChain();
	void CreateGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
		ResourceLoader* resourceLoader, VkDescriptorSetLayout descriptorSetLayout);
	void CreateSwapChain(GfxDeviceManager* gfxDeviceManager,
		VkSurfaceKHR surface, GLFWwindow* window);
	void CreateSwapChainImageViews();
	void CreateRenderPassModule(GfxDeviceManager* gfxDeviceManager);

	void CreateColorResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateDepthResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateFramebuffers();
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager);

	void CreateDescriptorPool();
	void CreateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
		VkImageView textureImageView, VkSampler textureSampler);
	void CreateCommandBuffers(VkCommandPool commandPool, std::vector<std::shared_ptr<GameObject>>& gameObjects);
};
