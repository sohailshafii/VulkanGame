#pragma once

#include "vulkan/vulkan.h"

class SwapChainManager;
class GfxDeviceManager;
class LogicalDeviceManager;
struct GLFWwindow;

class GraphicsEngine {
public:
	GraphicsEngine();

	void CreateEngine(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager, VkSurfaceKHR surface,
		GLFWwindow* window);

	~GraphicsEngine();

private:
	SwapChainManager* swapChainManager;

	void CleanUpSwapChain();
};
