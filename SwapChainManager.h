#pragma once

#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "vulkan/vulkan.h"

class SwapChainManager {
public:
	SwapChainManager(GfxDeviceManager* gfxDeviceManager,
		LogicalDeviceManager* logicalDeviceManager);
	~SwapChainManager();

	void create(VkSurfaceKHR surface, GLFWwindow *window);
	void createImageViews();

	VkSwapchainKHR getSwapChain() {
		return swapChain;
	}

	const std::vector<VkImage>& getSwapChainImages() const {
		return swapChainImages;
	}

	const std::vector<VkImageView>& getSwapChainImageViews() const {
		return swapChainImageViews;
	}

	VkFormat getSwapChainImageFormat() {
		return swapChainImageFormat;
	}

	VkExtent2D getSwapChainExtent() {
		return swapChainExtent;
	}

private:
	VkSwapchainKHR swapChain;
	// TODO: shared ptrs
	LogicalDeviceManager *logicalDeviceManager;
	GfxDeviceManager *gfxDeviceManager;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>
		availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
		GLFWwindow *window);
};
