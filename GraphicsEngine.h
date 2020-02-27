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
				   ResourceLoader *resourceLoader, VkSurfaceKHR surface,
				   GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
				   VkCommandPool commandPool, std::vector<std::shared_ptr<GameObject>>& gameObjects);

	~GraphicsEngine();

	SwapChainManager* GetSwapChainManager() { return swapChainManager; }
	RenderPassModule* GetRenderPassModule() { return renderPassModule; }
	//PipelineModule* GetPipelineModule() { return graphicsPipelineModule; }
	CommandBufferModule* GetCommandBufferModule() { return commandBufferModule; }

private:
	// not owned by us
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	std::vector<PipelineModule*> graphicsPipelineModules;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	CommandBufferModule* commandBufferModule;

	void CleanUpSwapChain();
	void CreateGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
		ResourceLoader* resourceLoader, VkDescriptorSetLayout descriptorSetLayout,
		std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void CreateSwapChain(GfxDeviceManager* gfxDeviceManager,
		VkSurfaceKHR surface, GLFWwindow* window);
	void CreateSwapChainImageViews();
	void CreateRenderPassModule(GfxDeviceManager* gfxDeviceManager);

	void CreateColorResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateDepthResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateFramebuffers();
	void CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
							  std::vector<std::shared_ptr<GameObject>>& gameObjects);

	void CreateDescriptorPoolAndSets(VkDescriptorSetLayout descriptorSetLayout,
									 std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void CreateCommandBuffers(VkCommandPool commandPool, std::vector<std::shared_ptr<GameObject>>& gameObjects);
};
