#pragma once

#include "vulkan/vulkan.h"

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
		GLFWwindow* window);

	~GraphicsEngine();

	void createGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkDescriptorSetLayout descriptorSetLayout);

private:
	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	PipelineModule* graphicsPipelineModule;

	void cleanUpSwapChain();
	void createSwapChain(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
		GLFWwindow* window);
	void createSwapChainImageViews();
	void createRenderPassModule(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager);
};
