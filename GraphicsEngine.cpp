
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"

GraphicsEngine::GraphicsEngine() {

}

void GraphicsEngine::CreateEngine(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
	GLFWwindow* window) {
	swapChainManager = new SwapChainManager(gfxDeviceManager,
		logicalDeviceManager);
	swapChainManager->create(surface, window);
}

GraphicsEngine::~GraphicsEngine() {
	CleanUpSwapChain();
}

void GraphicsEngine::CleanUpSwapChain() {
	if (swapChainManager != nullptr) {
		delete swapChainManager;
	}
}
 