
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
}

GraphicsEngine::~GraphicsEngine() {
	cleanUpSwapChain();
}

void GraphicsEngine::cleanUpSwapChain() {
	vkDestroyImageView(logicalDeviceManager->getDevice(), colorImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->getDevice(), colorImage, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), colorImageMemory, nullptr);

	if (graphicsPipelineModule != nullptr) {
		delete graphicsPipelineModule;
	}
	if (renderPassModule != nullptr) {
		delete renderPassModule;
	}
	if (swapChainManager != nullptr) {
		delete swapChainManager;
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
 