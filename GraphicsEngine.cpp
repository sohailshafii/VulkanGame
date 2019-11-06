
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"

GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
	GLFWwindow* window) {
	createSwapChain(gfxDeviceManager, logicalDeviceManager, surface,
		window);
	createSwapChainImageViews();
	createRenderPassModule(gfxDeviceManager, logicalDeviceManager);
}

GraphicsEngine::~GraphicsEngine() {
	cleanUpSwapChain();
}

void GraphicsEngine::cleanUpSwapChain() {
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
 