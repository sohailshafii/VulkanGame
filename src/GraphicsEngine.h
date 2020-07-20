#pragma once

#include "vulkan/vulkan.h"
#include "Common.h"
#include "Resources/Model.h"
#include "GameObjects/GameObject.h"
#include <vector>
#include <map>

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
				   GLFWwindow* window, VkCommandPool commandPool,
				   std::vector<std::shared_ptr<GameObject>>& gameObjects);

	~GraphicsEngine();

	SwapChainManager* GetSwapChainManager() { return swapChainManager; }
	RenderPassModule* GetRenderPassModule() { return renderPassModule; }
	//PipelineModule* GetPipelineModule() { return graphicsPipelineModule; }
	CommandBufferModule* GetCommandBufferModule() { return commandBufferModule; }

	void AddNewGameObjects(GfxDeviceManager* gfxDeviceManager,
						   ResourceLoader* resourceLoader,
						   std::vector<VkFence> const & inFlightFences,
						   std::vector<std::shared_ptr<GameObject>>& newGameObjects,
						   std::vector<std::shared_ptr<GameObject>>& allGameObjects);
	
private:
	// not owned by us
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	std::map<std::shared_ptr<GameObject>, PipelineModule*> gameObjectToPipelineModule;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	CommandBufferModule* commandBufferModule;
	
	void AddAndInitializeNewGameObjects(GfxDeviceManager* gfxDeviceManager,
										ResourceLoader* resourceLoader,
										std::vector<std::shared_ptr<GameObject>>& gameObjects);

	void CleanUpSwapChain();
	void CreateSwapChain(GfxDeviceManager* gfxDeviceManager,
		VkSurfaceKHR surface, GLFWwindow* window);
	void CreateSwapChainImageViews();
	void CreateRenderPassModule(GfxDeviceManager* gfxDeviceManager);

	void CreateColorResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateDepthResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void CreateFramebuffers();

	void AddGraphicsPipelinesFromGameObjects(GfxDeviceManager* gfxDeviceManager,
		ResourceLoader* resourceLoader, std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void CreateUniformBuffersForGameObjects(GfxDeviceManager* gfxDeviceManager,
		std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void CreateDescriptorPoolAndSetsForGameObjects(
		std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void CreateCommandBuffersForGameObjects(
		std::vector<std::shared_ptr<GameObject>>& gameObjects);
};
