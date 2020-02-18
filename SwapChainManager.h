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

	void Create(VkSurfaceKHR surface, GLFWwindow *window);
	void CreateImageViews();

	VkSwapchainKHR GetSwapChain() {
		return swapChain;
	}

	const std::vector<VkImage>& GetSwapChainImages() const {
		return swapChainImages;
	}

	const std::vector<VkImageView>& GetSwapChainImageViews() const {
		return swapChainImageViews;
	}

	VkFormat GetSwapChainImageFormat() {
		return swapChainImageFormat;
	}

	VkExtent2D GetSwapChainExtent() {
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

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>
		availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
		GLFWwindow *window);
};
