#pragma once

#include "vulkan/vulkan.h"
#include <vector>

class SwapChainManager;
class GfxDeviceManager;
class LogicalDeviceManager;
class RenderPassModule;
class PipelineModule;
struct GLFWwindow;

class GraphicsEngine {
public:
	GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
		GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
		VkCommandPool commandPool);

	~GraphicsEngine();

	void createGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkDescriptorSetLayout descriptorSetLayout);

private:
	LogicalDeviceManager* logicalDeviceManager; // TODO: shared_ptr
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

	void cleanUpSwapChain();
	void createSwapChain(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
		GLFWwindow* window);
	void createSwapChainImageViews();
	void createRenderPassModule(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager);

	void createColorResources(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkCommandPool commandPool);
	void createDepthResources(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkCommandPool commandPool);
	void createFramebuffers();
	void createUniformBuffers(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager);
};
