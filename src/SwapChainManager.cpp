#include "SwapChainManager.h"
#include <algorithm>
#include <stdexcept>
#include "Common.h"

SwapChainManager::SwapChainManager(GfxDeviceManager* gfxDeviceManager,
	LogicalDeviceManager* logicalDeviceManager) {
	this->logicalDeviceManager = logicalDeviceManager;
	this->gfxDeviceManager = gfxDeviceManager;
	this->swapChain = VK_NULL_HANDLE;
	this->swapChainExtent = VkExtent2D();
}

SwapChainManager::~SwapChainManager() {
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(logicalDeviceManager->GetDevice(), swapChainImageViews[i], nullptr);
	}
	vkDestroySwapchainKHR(logicalDeviceManager->GetDevice(), swapChain, nullptr);
}

void SwapChainManager::Create(VkSurfaceKHR surface, GLFWwindow *window) {
	GfxDeviceManager::SwapChainSupportDetails swapChainSupport =
		gfxDeviceManager->QuerySwapChainSupport(surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(
		swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(
		swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities,
		window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	// maxImageCount = 0 means no limit besides memory requirements
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	GfxDeviceManager::QueueFamilyIndices indices = gfxDeviceManager->FindQueueFamilies(
		surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(logicalDeviceManager->GetDevice(), &createInfo, nullptr,
		&swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(logicalDeviceManager->GetDevice(), swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDeviceManager->GetDevice(), swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void SwapChainManager::CreateImageViews() {
	auto swapChainImageFormat = GetSwapChainImageFormat();
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = Common::CreateImageView(
			swapChainImages[i], swapChainImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT, 1, logicalDeviceManager);
	}
}

VkSurfaceFormatKHR SwapChainManager::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
	// special case where vulkan tells us no preferred format exists
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto &availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR SwapChainManager::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>
	availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		// in case mailbox not available, default to immediate
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkExtent2D SwapChainManager::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
	GLFWwindow *window) {
	// if uint32_t max is not used, just return what capabilities gives us
	if (capabilities.currentExtent.width !=
		std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width),
			static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width,
			std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height,
			std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
