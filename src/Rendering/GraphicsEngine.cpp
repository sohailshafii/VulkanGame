
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

GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	ResourceLoader *resourceLoader, VkSurfaceKHR surface,
	GLFWwindow* window, VkCommandPool commandPool,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	this->logicalDeviceManager = logicalDeviceManager;
	CreateSwapChain(gfxDeviceManager, surface,
		window);
	CreateSwapChainImageViews();
	CreateRenderPassModule(gfxDeviceManager);

	CreateColorResources(gfxDeviceManager, commandPool);
	CreateDepthResources(gfxDeviceManager, commandPool);
	CreateFramebuffers();
	
	commandBufferModule = new CommandBufferModule(swapChainFramebuffers.size(),
		logicalDeviceManager->GetDevice(), commandPool);

	AddAndInitializeNewGameObjects(gfxDeviceManager, resourceLoader,
								   gameObjects);
	
	CreateCommandBuffersForGameObjects(gameObjects);
	for (auto& gameObject : gameObjects) {
		gameObject->SetInitializedInEngine(true);
	}
}

void GraphicsEngine::AddAndInitializeNewGameObjects(
	GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, gameObjects);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, gameObjects);
	CreateDescriptorPoolAndSetsForGameObjects(gameObjects);
}

void GraphicsEngine::ReRecordCommandsForGameObjects(
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const & inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjects) {
	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, allGameObjects);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, allGameObjects);
	CreateDescriptorPoolAndSetsForGameObjects(allGameObjects);
	
	// TODO: should be done on separate thread
	vkWaitForFences(logicalDeviceManager->GetDevice(), (uint32_t)inFlightFences.size(),
					inFlightFences.data(), VK_TRUE,
					std::numeric_limits<uint64_t>::max());
	
	CreateCommandBuffersForGameObjects(allGameObjects);
	for (auto& gameObject : allGameObjects) {
		gameObject->SetInitializedInEngine(true);
	}
}

void GraphicsEngine::RemoveGameObjectsAndRecordCommands(
	std::vector<VkFence> const& inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals) {
	RemoveGraphicsPipelinesFromGameObjects(gameObjectsToRemove);
	// TODO: should be done on separate thread
	vkWaitForFences(logicalDeviceManager->GetDevice(), (uint32_t)inFlightFences.size(),
		inFlightFences.data(), VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	CreateCommandBuffersForGameObjects(allGameObjectsSansRemovals);
}

void GraphicsEngine::RemoveGameObjectsAndReRecordCommandsForAddedGameObjects(
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const& inFlightFences,
	std::vector<std::shared_ptr<GameObject>> const & gameObjectsToRemove,
	std::vector<std::shared_ptr<GameObject>> const & allGameObjectsSansRemovals) {
	RemoveGraphicsPipelinesFromGameObjects(gameObjectsToRemove);

	AddGraphicsPipelinesFromGameObjects(gfxDeviceManager, resourceLoader, allGameObjectsSansRemovals);
	CreateUniformBuffersForGameObjects(gfxDeviceManager, allGameObjectsSansRemovals);
	CreateDescriptorPoolAndSetsForGameObjects(allGameObjectsSansRemovals);

	// TODO: should be done on separate thread
	vkWaitForFences(logicalDeviceManager->GetDevice(), (uint32_t)inFlightFences.size(),
		inFlightFences.data(), VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	CreateCommandBuffersForGameObjects(allGameObjectsSansRemovals);
	for (auto& gameObject : allGameObjectsSansRemovals) {
		gameObject->SetInitializedInEngine(true);
	}
}

GraphicsEngine::~GraphicsEngine() {
	CleanUpSwapChain();
}

void GraphicsEngine::CleanUpSwapChain() {
	// TODO: verify not null throughout
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

	if (commandBufferModule != nullptr) {
		delete commandBufferModule;
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

// TODO: group similar pipelines, if possible (i.e. game objects with similar pipelines
// get drawn together)
void GraphicsEngine::AddGraphicsPipelinesFromGameObjects(
	GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader,
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	for (auto& gameObject : gameObjects) {
		// avoid invisible objects
		if (gameObject->IsInvisible()) {
			continue;
		}

		// avoid adding on that already exists
		if (gameObjectToPipelineModule.find(gameObject) !=
			gameObjectToPipelineModule.end())
		{
			continue;
		}
		gameObjectToPipelineModule[gameObject] = std::make_shared<PipelineModule>(
			gameObject->GetVertexShaderName(), gameObject->GetFragmentShaderName(),
			logicalDeviceManager->GetDevice(), swapChainManager->GetSwapChainExtent(),
			gfxDeviceManager, resourceLoader, gameObject->GetDescriptorSetLayout(),
			renderPassModule->GetRenderPass(), gameObject->GetMaterialType(),
			gameObject->GetPrimitiveTopology());
	}
}

void GraphicsEngine::RemoveGraphicsPipelinesFromGameObjects(
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	for (auto& gameObject : gameObjects) {
		if (gameObjectToPipelineModule.find(gameObject) !=
			gameObjectToPipelineModule.end())
		{
			continue;
		}
		gameObjectToPipelineModule.erase(gameObject);
	}
}

void GraphicsEngine::CreateUniformBuffersForGameObjects(GfxDeviceManager* gfxDeviceManager,
										  std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		if (gameObject->GetInitializedInEngine()) {
			continue;
		}
		gameObject->InitAndCreateUniformBuffers(gfxDeviceManager, numSwapChainImages);
	}
}

void GraphicsEngine::CreateDescriptorPoolAndSetsForGameObjects(
	std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		// TODO: make descriptor set more configurable
		if (gameObject->GetInitializedInEngine()) {
			continue;
		}
		gameObject->CreateDescriptorPoolAndSets(numSwapChainImages);
	}
}

// per object. have a ubo per object, then update that ubo based on the matrices associated
// move render logic to this class!
// TODO: record on separate thread
void GraphicsEngine::CreateCommandBuffersForGameObjects(
					std::vector<std::shared_ptr<GameObject>> const & gameObjects) {
	auto& commandBuffers = commandBufferModule->GetCommandBuffers();
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPassModule->GetRenderPass();
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		auto swapChainExtent = swapChainManager->GetSwapChainExtent();
		renderPassInfo.renderArea.extent = swapChainExtent;

		// order of clear values = order of attachments
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		clearValues[1].depthStencil = { 1.0f, 0 };

		//VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		// render opaque objects first, then transparent
		RecordCommandForGameObjects(commandBuffers[i],
			gameObjects, false, i);
		RecordCommandForGameObjects(commandBuffers[i],
			gameObjects, true, i);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void GraphicsEngine::RecordCommandForGameObjects(VkCommandBuffer &commandBuffer,
	std::vector<std::shared_ptr<GameObject>> const & gameObjects,
	bool renderOnlyTransparent,
	int commandBufferIndex) {
	size_t numGameObjects = gameObjects.size();
	for (size_t objectIndex = 0; objectIndex < numGameObjects;
		objectIndex++) {
		auto& gameObject = gameObjects[objectIndex];
		auto materialType = gameObject->GetMaterialType();
		bool isTransparentMat = materialType ==
			DescriptorSetFunctions::MaterialType::Text;
		// skip object is it does not pass our render-type test
		if (isTransparentMat && !renderOnlyTransparent ||
			!isTransparentMat && renderOnlyTransparent) {
			continue;
		}

		// skip invisible objects
		if (gameObject->IsInvisible()) {
			continue;
		}

		PipelineModule* pipelineModule = gameObjectToPipelineModule[gameObject].get();

		// skip if pipeline was not found
		if (pipelineModule == nullptr) {
			continue;
		}

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineModule->GetPipeline());
		// bind our vertex buffers
		VkBuffer vertexBuffers[] = { gameObject->GetVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, gameObject->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineModule->GetLayout(), 0, 1, gameObject->GetDescriptorSetPtr(commandBufferIndex),
			0, nullptr);
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(gameObject->GetModel()->GetIndices().size()),
			1, 0, 0, 0);
	}
}

 
