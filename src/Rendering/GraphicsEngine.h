#pragma once

#include "vulkan/vulkan.h"
#include "Common.h"
#include "Resources/Model.h"
#include "GameObjects/GameObject.h"
#include "CommonBufferModule.h"
#include <vector>
#include <stack>
#include <map>

class SwapChainManager;
class GfxDeviceManager;
class LogicalDeviceManager;
class RenderPassModule;
class PipelineModule;
struct GLFWwindow;
class ImageTextureLoader;
class ResourceLoader;

class GraphicsEngine {
public:
	GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
				   std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
				   ResourceLoader *resourceLoader, VkSurfaceKHR surface,
				   GLFWwindow* window, VkCommandPool mainCommandPool,
					VkCommandPoolCreateInfo poolCreateInfo,
				   std::vector<std::shared_ptr<GameObject>>& gameObjects);

	~GraphicsEngine();

	SwapChainManager* GetSwapChainManager() { return swapChainManager; }
	RenderPassModule* GetRenderPassModule() { return renderPassModule; }
	// Right now image has one command buffer
	VkCommandBuffer* GetCommandBufferData(int imageIndex) {
		return commandBufferModules[imageIndex]->GetCommandBuffers().data(); }

	void ReRecordCommandsForGameObjects(GfxDeviceManager* gfxDeviceManager,
		ResourceLoader* resourceLoader, std::vector<VkFence> const & inFlightFences,
		std::vector<std::shared_ptr<GameObject>> const & allGameObjects);
	
	void RemoveGameObjectsAndRecordCommands(
		std::vector<VkFence> const& inFlightFences,
		std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
		std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals);

	void RemoveGameObjectsAndReRecordCommandsForAddedGameObjects(
		GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
		std::vector<VkFence> const& inFlightFences,
		std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
		std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals);

	void RecordCommandBuffersForCommandBufferModule(
		std::vector<std::shared_ptr<GameObject>> const& gameObjects,
		CommandBufferModule* commandBufferModule,
		VkFramebuffer swapChainFramebuffer,
		int swapChainIndex);

	void Update(std::vector<VkFence> const& inFlightFences);

private:
	// not owned by us
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	std::map<std::shared_ptr<GameObject>, std::shared_ptr<PipelineModule>>
		gameObjectToPipelineModule;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	// so that we can parallelize command buffer recording
	// create a vector of them
	std::vector<CommandBufferModule*> commandBufferModules;
	std::vector<CommandBufferModule*> commandBufferModulesPending;
	std::stack<std::shared_ptr<GameObject>> gameObjectsPipelinesPendingRemoval;
	bool pendingCommandModules;
	
	void AddAndInitializeNewGameObjects(GfxDeviceManager* gfxDeviceManager,
										ResourceLoader* resourceLoader,
										std::vector<std::shared_ptr<GameObject>>& gameObjects);
	void InitializeGameObjectsRecursively(std::vector<std::shared_ptr<GameObject>>& gameObjects);

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
		ResourceLoader* resourceLoader,
		std::vector<std::shared_ptr<GameObject>> const & gameObjects);
	void RecursivelyCollectGameObjectsToCreatePipelinesFor(std::vector<std::shared_ptr<GameObject>> const& gameObjects,
		std::vector<std::shared_ptr<GameObject>>& gameObjectsToCreatePipelinesFor);
	void AddNewPipeline(std::shared_ptr<GameObject> gameObject,
		std::shared_ptr<PipelineModule>* pipelineModulePtr,
		GfxDeviceManager* gfxDeviceManager,
		ResourceLoader* resourceLoader);
	std::shared_ptr<PipelineModule>
		FindExistingPipeline(std::shared_ptr<GameObject> const& gameObject);
	void RemoveGraphicsPipelinesFromPendingGameObjects();
	void RemoveGameObjectToPipelineRecursive(std::shared_ptr<GameObject> const& gameObject);

	void CreateUniformBuffersForGameObjects(GfxDeviceManager* gfxDeviceManager,
		std::vector<std::shared_ptr<GameObject>> const & gameObjects);
	void CreateDescriptorPoolAndSetsForGameObjects(
		std::vector<std::shared_ptr<GameObject>> const & gameObjects);
	void CreateCommandBuffersForGameObjects(
		std::vector<std::shared_ptr<GameObject>> const & gameObjects,
		std::vector<CommandBufferModule*> commandBufferModulesToUse);

	void RecordCommandForGameObjects(VkCommandBuffer& commandBuffer,
		std::vector<std::shared_ptr<GameObject>> const & gameObjects,
		bool renderOnlyTransparent,
		int swapChainIndex);
	void RecordCommandForGameObject(VkCommandBuffer& commandBuffer,
		std::shared_ptr<GameObject> const & gameObject, bool renderOnlyTransparent,
		int swapChainIndex);
};
