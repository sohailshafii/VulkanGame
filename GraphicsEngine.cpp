
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"
#include "Common.h"

GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
	GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
	VkCommandPool commandPool) {
	this->logicalDeviceManager = logicalDeviceManager;
	createSwapChain(gfxDeviceManager, logicalDeviceManager, surface,
		window);
	createSwapChainImageViews();
	createRenderPassModule(gfxDeviceManager, logicalDeviceManager);
	createGraphicsPipeline(gfxDeviceManager, logicalDeviceManager,
		descriptorSetLayout);

	createColorResources(gfxDeviceManager, logicalDeviceManager, commandPool); // 5
	createDepthResources(gfxDeviceManager, logicalDeviceManager, commandPool); // 6
	createFramebuffers(); // 7
	createUniformBuffers(gfxDeviceManager, logicalDeviceManager); // 8
}

GraphicsEngine::~GraphicsEngine() {
	cleanUpSwapChain();
}

void GraphicsEngine::cleanUpSwapChain() {
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(logicalDeviceManager->getDevice(), swapChainFramebuffers[i],
			nullptr);
	}

	vkDestroyImageView(logicalDeviceManager->getDevice(), colorImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->getDevice(), colorImage, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), colorImageMemory, nullptr);

	vkDestroyImageView(logicalDeviceManager->getDevice(), depthImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->getDevice(), depthImage, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), depthImageMemory, nullptr);

	if (graphicsPipelineModule != nullptr) {
		delete graphicsPipelineModule;
	}
	if (renderPassModule != nullptr) {
		delete renderPassModule;
	}
	size_t numSwapChainImages = swapChainManager->getSwapChainImages().size();
	if (swapChainManager != nullptr) {
		delete swapChainManager;
	}

	// TODO: each swap chain image should have an associated meta data with it
		// so create SwapChainImage class for that
	for (size_t i = 0; i < numSwapChainImages; i++) {
		vkDestroyBuffer(logicalDeviceManager->getDevice(), uniformBuffers[i], nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[i], nullptr);
	}
}

void GraphicsEngine::createSwapChain(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
	GLFWwindow* window) {
	swapChainManager = new SwapChainManager(gfxDeviceManager,
		logicalDeviceManager);
	swapChainManager->create(surface, window);
}

void GraphicsEngine::createSwapChainImageViews() {
	swapChainManager->createImageViews();
}

void GraphicsEngine::createRenderPassModule(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager) {
	renderPassModule = new RenderPassModule(logicalDeviceManager->getDevice(),
		gfxDeviceManager->getPhysicalDevice(), swapChainManager->getSwapChainImageFormat(),
		gfxDeviceManager->getMSAASamples());
}

void GraphicsEngine::createGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkDescriptorSetLayout descriptorSetLayout) {
	graphicsPipelineModule = new PipelineModule("shaders/vert.spv",
		"shaders/frag.spv", logicalDeviceManager->getDevice(),
		swapChainManager->getSwapChainExtent(), gfxDeviceManager,
		descriptorSetLayout, renderPassModule->GetRenderPass());
}

void GraphicsEngine::createColorResources(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkCommandPool commandPool) {
	auto swapChainImageFormat = swapChainManager->getSwapChainImageFormat();
	auto swapChainExtent = swapChainManager->getSwapChainExtent();
	VkFormat colorFormat = swapChainImageFormat;

	Common::createImage(swapChainExtent.width, swapChainExtent.height, 1,
		gfxDeviceManager->getMSAASamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory,
		logicalDeviceManager, gfxDeviceManager);
	colorImageView = Common::createImageView(colorImage, colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT, 1, logicalDeviceManager);

	Common::transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager);
}

void GraphicsEngine::createDepthResources(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkCommandPool commandPool) {
	VkFormat depthFormat = Common::findDepthFormat(gfxDeviceManager->getPhysicalDevice());
	auto swapChainExtent = swapChainManager->getSwapChainExtent();
	Common::createImage(swapChainExtent.width, swapChainExtent.height,
		1, gfxDeviceManager->getMSAASamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage, depthImageMemory, logicalDeviceManager, gfxDeviceManager);
	depthImageView = Common::createImageView(depthImage, depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT, 1, logicalDeviceManager);

	Common::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager);
}

void GraphicsEngine::createFramebuffers() {
	auto& swapChainImageViews = swapChainManager->getSwapChainImageViews();
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
		auto swapChainExtent = swapChainManager->getSwapChainExtent();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDeviceManager->getDevice(), &framebufferInfo, nullptr,
			&swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void GraphicsEngine::createUniformBuffers(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

 