
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"
#include "CommonBufferModule.h"
#include "ImageTextureLoader.h"
#include "ResourceLoader.h"

// TODO renaming this to something else..it's not really a graphics engine, but 
// something that gets recreated when something like the window or parts of the pipeline
// change
GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	ResourceLoader *resourceLoader, VkSurfaceKHR surface,
	GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
	VkCommandPool commandPool, std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	this->logicalDeviceManager = logicalDeviceManager;
	CreateSwapChain(gfxDeviceManager, surface,
		window);
	CreateSwapChainImageViews();
	CreateRenderPassModule(gfxDeviceManager);
	CreateGraphicsPipeline(gfxDeviceManager, resourceLoader, descriptorSetLayout,
						   gameObjects);

	CreateColorResources(gfxDeviceManager, commandPool); // 5
	CreateDepthResources(gfxDeviceManager, commandPool); // 6
	CreateFramebuffers(); // 7
	CreateUniformBuffers(gfxDeviceManager, gameObjects); // 8
	
	CreateDescriptorPoolAndSets(descriptorSetLayout, gameObjects);
	CreateCommandBuffers(commandPool, gameObjects);
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

	for(auto graphicsPipelineModule : graphicsPipelineModules) {
		if (graphicsPipelineModule != nullptr) {
			delete graphicsPipelineModule;
		}
	}
	graphicsPipelineModules.clear();
	
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

void GraphicsEngine::CreateGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader, VkDescriptorSetLayout descriptorSetLayout,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	for(auto& gameObject : gameObjects) {
		graphicsPipelineModules.push_back(new PipelineModule(gameObject->GetVertexShaderName(),
															 gameObject->GetFragmentShaderName(), logicalDeviceManager->GetDevice(),
															 swapChainManager->GetSwapChainExtent(), gfxDeviceManager,
															 resourceLoader, descriptorSetLayout, renderPassModule->GetRenderPass()));
	}
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

void GraphicsEngine::CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
										  std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		gameObject->CreateCommandBuffers(gfxDeviceManager, numSwapChainImages);
	}
}

void GraphicsEngine::CreateDescriptorPoolAndSets(VkDescriptorSetLayout descriptorSetLayout,
								 std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->GetSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto& gameObject : gameObjects) {
		gameObject->CreateDescriptorPoolAndSets(numSwapChainImages, descriptorSetLayout);
	}
}

// TODO: create command buffer module that encapsulates the allocate info, etc
// TODO: it should be possible to have multiple pipelines/command buffers
// per object. have a ubo per object, then update that ubo based on the matrices associated
// move render logic to this class!
void GraphicsEngine::CreateCommandBuffers(VkCommandPool commandPool,
										  std::vector<std::shared_ptr<GameObject>>& gameObjects) {	
	commandBufferModule = new CommandBufferModule(swapChainFramebuffers.size(),
		logicalDeviceManager->GetDevice(), commandPool);
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
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = { 1.0f, 0 };

		//VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		// TODO: bind graphics pipeline to specific game objects
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineModules[0]->GetPipeline());

		for (const auto& gameObject : gameObjects) {
			// bind our vertex buffers
			VkBuffer vertexBuffers[] = { gameObject->GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], gameObject->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
									graphicsPipelineModules[0]->GetLayout(), 0, 1, gameObject->GetDescriptorSetPtr(i), 0, nullptr);
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(gameObject->GetModel()->GetIndices().size()),
							 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

 