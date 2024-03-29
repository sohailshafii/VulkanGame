
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"
#include "CommonBufferModule.h"
#include "Resources/TextureCreator.h"
#include "Resources/ResourceLoader.h"
#include <thread>
#include <iostream>

GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	ResourceLoader *resourceLoader, VkSurfaceKHR surface,
	GLFWwindow* window, VkCommandPool mainCommandPool,
	VkCommandPoolCreateInfo poolCreateInfo,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	this->logicalDeviceManager = logicalDeviceManager;
	CreateSwapChain(gfxDeviceManager, surface, window);
	CreateSwapChainImageViews();
	CreateRenderPassModule(gfxDeviceManager);

	CreateColorResources(gfxDeviceManager, mainCommandPool);
	CreateDepthResources(gfxDeviceManager, mainCommandPool);
	CreateFramebuffers();
	
	size_t numSwapchainImages = swapChainFramebuffers.size();
	for (size_t i = 0; i < numSwapchainImages; i++) {
		commandBufferModules.push_back(new CommandBufferModule(1,
			logicalDeviceManager->GetDevice(), poolCreateInfo));
	}
	for (size_t i = 0; i < numSwapchainImages; i++) {
		commandBufferModulesPending.push_back(new CommandBufferModule(1,
			logicalDeviceManager->GetDevice(), poolCreateInfo));
	}
	pendingCommandModules = false;

	AddAndInitializeNewGameObjects(gfxDeviceManager, resourceLoader,
								   gameObjects);
}

void GraphicsEngine::AddAndInitializeNewGameObjects(
	GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, gameObjects);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, gameObjects);
	CreateDescriptorPoolAndSetsForGameObjects(gameObjects);

	CreateCommandBuffersForGameObjects(gameObjects, commandBufferModules);
	SetGameObjectsInitalizedRecursively(gameObjects);
}

void GraphicsEngine::SetGameObjectsInitalizedRecursively(
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	for (auto& gameObject : gameObjects) {
		gameObject->SetInitializedInEngine(true);
		if (gameObject->GetNumChildGameObjects() > 0) {
			SetGameObjectsInitalizedRecursively(gameObject->GetChildren());
		}
	}
}

void GraphicsEngine::ReRecordCommandsForGameObjects(
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const & inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjects) {
	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, allGameObjects);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, allGameObjects);
	CreateDescriptorPoolAndSetsForGameObjects(allGameObjects);
	
	CreateCommandBuffersForGameObjects(allGameObjects, commandBufferModulesPending);
	SetGameObjectsInitalizedRecursively(allGameObjects);
	pendingCommandModules = true;
	std::cout << "record command buffers\n";
}

void GraphicsEngine::RemoveGameObjectsAndRecordCommands(
	std::vector<VkFence> const& inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals) {
	for (auto & gameObject : gameObjectsToRemove) {
		gameObjectsPipelinesPendingRemoval.push(gameObject);
	}

	CreateCommandBuffersForGameObjects(allGameObjectsSansRemovals,
		commandBufferModulesPending);
	pendingCommandModules = true;
}

void GraphicsEngine::RemoveGameObjectsAndReRecordCommandsForAddedGameObjects(
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const& inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals) {
	for (auto& gameObject : gameObjectsToRemove) {
		gameObjectsPipelinesPendingRemoval.push(gameObject);
	}

	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, allGameObjectsSansRemovals);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, allGameObjectsSansRemovals);
	CreateDescriptorPoolAndSetsForGameObjects(allGameObjectsSansRemovals);

	CreateCommandBuffersForGameObjects(allGameObjectsSansRemovals,
		commandBufferModulesPending);
	SetGameObjectsInitalizedRecursively(allGameObjectsSansRemovals);
	pendingCommandModules = true;
	std::cout << "remove game objects, add a few, record command buffers\n";
}

GraphicsEngine::~GraphicsEngine() {
	CleanUpSwapChain();
}

void GraphicsEngine::CleanUpSwapChain() {
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(logicalDeviceManager->GetDevice(), swapChainFramebuffers[i],
			nullptr);
	}

	vkDestroyImageView(logicalDeviceManager->GetDevice(), colorImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->GetDevice(), colorImage, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), colorImageMemory, nullptr);

	vkDestroyImageView(logicalDeviceManager->GetDevice(), depthImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->GetDevice(), depthImage, nullptr);
	vkFreeMemory(logicalDeviceManager->GetDevice(), depthImageMemory, nullptr);

	if (commandBufferModules.size() > 0) {
		for (auto commandBufferModule : commandBufferModules) {
			delete commandBufferModule;
		}
	}
	if (commandBufferModulesPending.size() > 0) {
		for (auto commandBufferModule : commandBufferModulesPending) {
			delete commandBufferModule;
		}
	}

	gameObjectToPipelineModule.clear();
	
	if (renderPassModule != nullptr) {
		delete renderPassModule;
	}
	if (swapChainManager != nullptr) {
		delete swapChainManager;
	}
}

void GraphicsEngine::CreateSwapChain(GfxDeviceManager* gfxDeviceManager,
	VkSurfaceKHR surface, GLFWwindow* window) {
	swapChainManager = new SwapChainManager(gfxDeviceManager,
		logicalDeviceManager.get());
	swapChainManager->Create(surface, window);
}

void GraphicsEngine::CreateSwapChainImageViews() {
	swapChainManager->CreateImageViews();
}

void GraphicsEngine::CreateRenderPassModule(GfxDeviceManager* gfxDeviceManager) {
	renderPassModule = new RenderPassModule(logicalDeviceManager->GetDevice(),
		gfxDeviceManager->GetPhysicalDevice(), swapChainManager->GetSwapChainImageFormat(),
		gfxDeviceManager->GetMSAASamples());
}

void GraphicsEngine::CreateColorResources(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool) {
	auto swapChainImageFormat = swapChainManager->GetSwapChainImageFormat();
	auto swapChainExtent = swapChainManager->GetSwapChainExtent();
	VkFormat colorFormat = swapChainImageFormat;

	Common::CreateImage(swapChainExtent.width, swapChainExtent.height, 1,
		gfxDeviceManager->GetMSAASamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory,
		logicalDeviceManager.get(), gfxDeviceManager);
	colorImageView = Common::CreateImageView(colorImage, colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT, 1, logicalDeviceManager.get());

	Common::TransitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager.get());
}

void GraphicsEngine::CreateDepthResources(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool) {
	VkFormat depthFormat = Common::FindDepthFormat(gfxDeviceManager->GetPhysicalDevice());
	auto swapChainExtent = swapChainManager->GetSwapChainExtent();
	Common::CreateImage(swapChainExtent.width, swapChainExtent.height,
		1, gfxDeviceManager->GetMSAASamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage, depthImageMemory, logicalDeviceManager.get(), gfxDeviceManager);
	depthImageView = Common::CreateImageView(depthImage, depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT, 1, logicalDeviceManager.get());

	Common::TransitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager.get());
}

void GraphicsEngine::CreateFramebuffers() {
	auto& swapChainImageViews = swapChainManager->GetSwapChainImageViews();
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPassModule->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		auto swapChainExtent = swapChainManager->GetSwapChainExtent();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDeviceManager->GetDevice(), &framebufferInfo, nullptr,
			&swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void GraphicsEngine::AddGraphicsPipelinesFromGameObjects(
	GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader,
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	std::vector<std::shared_ptr<GameObject>> gameObjectsToCreatePipelinesFor;
	
	RecursivelyCollectGameObjectsToCreatePipelinesFor(gameObjects, gameObjectsToCreatePipelinesFor);

	int numPipelinesToCreates = (int)gameObjectsToCreatePipelinesFor.size();
	std::vector<std::thread> threads(numPipelinesToCreates);
	std::vector<std::shared_ptr<PipelineModule>> pipelineModulePerGameObject;
	pipelineModulePerGameObject.resize(numPipelinesToCreates);

	// this should be ok, because each pipeline module in the array is allocated separately
	// and exists in a separate location
	for (int i = 0; i < numPipelinesToCreates; i++) {
		threads[i] = std::thread(
			&GraphicsEngine::AddNewPipeline, this, gameObjectsToCreatePipelinesFor[i],
			pipelineModulePerGameObject.data() + i, gfxDeviceManager, resourceLoader);
	}

	for (size_t i = 0; i < numPipelinesToCreates; i++) {
		threads[i].join();
	}

	for (int i = 0; i < numPipelinesToCreates; i++) {
		auto gameObject = gameObjectsToCreatePipelinesFor[i];
		// TODO: it's possible for new game objects to have duplicate pipelines, consider
		// filtering those out too
		gameObjectToPipelineModule[gameObject] =
			pipelineModulePerGameObject[i];
	}
}

void GraphicsEngine::RecursivelyCollectGameObjectsToCreatePipelinesFor(
	std::vector<std::shared_ptr<GameObject>> const& gameObjects,
	std::vector<std::shared_ptr<GameObject>>& gameObjectsToCreatePipelinesFor) {
	for (auto& gameObject : gameObjects) {
		// avoid invisible objects, or ones that already have pipelines
		if (!gameObject->IsInvisible() && gameObjectToPipelineModule.find(gameObject) ==
			gameObjectToPipelineModule.end()) {
			auto possibleExistingPipeline = FindMatchingPipelineFromAnotherGameObject(gameObject);
			if (possibleExistingPipeline == nullptr) {
				gameObjectsToCreatePipelinesFor.push_back(gameObject);
			}
			else {
				gameObjectToPipelineModule[gameObject] = possibleExistingPipeline;
			}
		}
		
		auto& children = gameObject->GetChildren();
		RecursivelyCollectGameObjectsToCreatePipelinesFor(children, gameObjectsToCreatePipelinesFor);
	}
}

void GraphicsEngine::AddNewPipeline(std::shared_ptr<GameObject> gameObject,
	std::shared_ptr<PipelineModule>* pipelineModulePtr,
	GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader) {
	*pipelineModulePtr =
		std::make_shared<PipelineModule>(
			gameObject->GetVertexShaderName(), gameObject->GetFragmentShaderName(),
			logicalDeviceManager->GetDevice(), swapChainManager->GetSwapChainExtent(),
			gfxDeviceManager, resourceLoader, gameObject->GetDescriptorSetLayout(),
			renderPassModule->GetRenderPass(), gameObject->GetMaterialType(),
			gameObject->GetPrimitiveTopology());
}

std::shared_ptr<PipelineModule> GraphicsEngine::FindMatchingPipelineFromAnotherGameObject(
		std::shared_ptr<GameObject> const& gameObject) {
	std::map<std::shared_ptr<GameObject>, std::shared_ptr<PipelineModule>>::iterator it;
	for (it = gameObjectToPipelineModule.begin(); it != gameObjectToPipelineModule.end(); it++)
	{
		std::shared_ptr<PipelineModule> const & pipeline = it->second;
		if (pipeline->MatchesMaterialAndTopologyTypes(gameObject->GetMaterialType(),
			gameObject->GetPrimitiveTopology())) {
			return pipeline;
		}
	}
	return nullptr;
}

void GraphicsEngine::RemoveGraphicsPipelinesFromPendingGameObjects() {
	while (gameObjectsPipelinesPendingRemoval.size() > 0) {
		std::shared_ptr<GameObject> gameObject = gameObjectsPipelinesPendingRemoval.top();
		gameObjectsPipelinesPendingRemoval.pop();
		if (gameObjectToPipelineModule.find(gameObject) != gameObjectToPipelineModule.end())
		{
			gameObjectToPipelineModule.erase(gameObject);
		}

		RemoveGameObjectToPipelineRecursive(gameObject);
	}
}

void GraphicsEngine::RemoveGameObjectToPipelineRecursive(std::shared_ptr<GameObject> const& gameObject) {
	size_t numChildGameObjects = gameObject->GetNumChildGameObjects();
	for (size_t childIndex = 0; childIndex < numChildGameObjects; childIndex++) {
		auto& childGameObject = gameObject->GetChildGameObject(childIndex);
		if (gameObjectToPipelineModule.find(childGameObject) !=
			gameObjectToPipelineModule.end()) {
			gameObjectToPipelineModule.erase(childGameObject);
		}
		RemoveGameObjectToPipelineRecursive(childGameObject);
	}
}

void GraphicsEngine::CreateUniformBuffersForGameObjects(GfxDeviceManager* gfxDeviceManager,
										  std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		// if parent is initialized, then children are too
		if (gameObject->GetInitializedInEngine()) {
			continue;
		}
		gameObject->InitAndCreateUniformBuffers(gfxDeviceManager, numSwapChainImages);
		auto& children = gameObject->GetChildren();
		CreateUniformBuffersForGameObjects(gfxDeviceManager, children);
	}
}

void GraphicsEngine::CreateDescriptorPoolAndSetsForGameObjects(
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		if (gameObject->GetInitializedInEngine()) {
			continue;
		}
		gameObject->CreateDescriptorPoolAndSets(numSwapChainImages);
		auto& children = gameObject->GetChildren();
		CreateDescriptorPoolAndSetsForGameObjects(children);
	}
}

void GraphicsEngine::RecordCommandBuffersForCommandBufferModule(
	std::vector<std::shared_ptr<GameObject>> const& gameObjects,
	CommandBufferModule* commandBufferModule,
	VkFramebuffer swapChainFramebuffer,
	int swapChainIndex) {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	auto& commandBuffer = commandBufferModule->GetCommandBuffers()[0];
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPassModule->GetRenderPass();
	renderPassInfo.framebuffer = swapChainFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	auto swapChainExtent = swapChainManager->GetSwapChainExtent();
	renderPassInfo.renderArea.extent = swapChainExtent;

	// order of clear values = order of attachments
	std::array<VkClearValue, 2> clearValues = {};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	//VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE);

	// render opaque objects first, then transparent
	RecordCommandForGameObjects(commandBuffer, gameObjects, false, swapChainIndex);
	RecordCommandForGameObjects(commandBuffer, gameObjects, true, swapChainIndex);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void GraphicsEngine::Update(std::vector<VkFence> const& inFlightFences) {
	if (pendingCommandModules) {
		// have to wait for device to become available
		vkWaitForFences(logicalDeviceManager->GetDevice(), (uint32_t)inFlightFences.size(),
			inFlightFences.data(), VK_TRUE,
			std::numeric_limits<uint64_t>::max());
		// pending becomes active while active becomes pending
		size_t numModules = commandBufferModules.size();
		for (size_t i = 0; i < numModules; i++) {
			auto* oldPtr = commandBufferModules[i];
			commandBufferModules[i] = commandBufferModulesPending[i];
			commandBufferModulesPending[i] = oldPtr;
		}
		RemoveGraphicsPipelinesFromPendingGameObjects();
		pendingCommandModules = false;
	}
}

// per object. have a ubo per object, then update that ubo based on the matrices associated
void GraphicsEngine::CreateCommandBuffersForGameObjects(
					std::vector<std::shared_ptr<GameObject>> const & gameObjects,
					std::vector<CommandBufferModule*> commandBufferModulesToUse) {
	// right now we have one thread per swap chain image, could expand further on that
	size_t numThreads = commandBufferModulesToUse.size();
	std::vector<std::thread> threads(numThreads);
	for (int i = 0; i < numThreads; i++) {
		// this should be ok, because each command buffer module in the array is allocated separately
		// and exists in a separate location
		threads[i] = std::thread(
			&GraphicsEngine::RecordCommandBuffersForCommandBufferModule, this,
			gameObjects, commandBufferModulesToUse[i], swapChainFramebuffers[i], i);
	}
	
	for (size_t i = 0; i < numThreads; i++) {
		threads[i].join();
	}
}

void GraphicsEngine::RecordCommandForGameObjects(VkCommandBuffer &commandBuffer,
	std::vector<std::shared_ptr<GameObject>> const & gameObjects,
	bool renderOnlyTransparent,
	int swapChainIndex) {
	size_t numGameObjects = gameObjects.size();
	for (size_t objectIndex = 0; objectIndex < numGameObjects;
		objectIndex++) {
		auto& gameObject = gameObjects[objectIndex];
		RecordCommandForGameObject(commandBuffer, gameObject, renderOnlyTransparent,
			swapChainIndex);
		auto& children = gameObject->GetChildren();
		if (!children.empty()) {
			RecordCommandForGameObjects(commandBuffer, children,
				renderOnlyTransparent, swapChainIndex);
		}
	}
}

void GraphicsEngine::RecordCommandForGameObject(VkCommandBuffer& commandBuffer,
	std::shared_ptr<GameObject> const & gameObject, bool renderOnlyTransparent,
	int swapChainIndex) {
	auto materialType = gameObject->GetMaterialType();
	bool isTransparentMat = materialType ==
		DescriptorSetFunctions::MaterialType::Text;
	// skip object is it does not pass our render-type test
	if (isTransparentMat && !renderOnlyTransparent ||
		!isTransparentMat && renderOnlyTransparent) {
		return;
	}

	// skip invisible objects
	if (gameObject->IsInvisible()) {
		return;
	}

	PipelineModule* pipelineModule = gameObjectToPipelineModule[gameObject].get();

	// skip if pipeline was not found
	if (pipelineModule == nullptr) {
		return;
	}

	// TODO: we can possibly organize game objects based on similar pipelines
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineModule->GetPipeline());
	// bind our vertex buffers
	VkBuffer vertexBuffers[] = { gameObject->GetVertexBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffer, gameObject->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineModule->GetLayout(), 0, 1, gameObject->GetDescriptorSetPtr(swapChainIndex),
		0, nullptr);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(gameObject->GetModel()->GetIndices().size()),
		1, 0, 0, 0);
}
 
