#include "GfxDeviceManager.h"
#include <stdexcept>
#include <vector>
#include <set>
#include <string>
#include <algorithm>

GfxDeviceManager::GfxDeviceManager(const VkInstance& vkInstance,
	const VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
	physicalDevice = VK_NULL_HANDLE;
	msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	PickPhysicalDevice(vkInstance, surface, deviceExtensions);
}

GfxDeviceManager::~GfxDeviceManager() {

}

void GfxDeviceManager::PickPhysicalDevice(const VkInstance& vkInstance,
	VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (IsDeviceSuitable(device, surface, deviceExtensions)) {
			physicalDevice = device;
			msaaSamples = GetMaxUsableSampleCount(device);
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

bool GfxDeviceManager::IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
	const std::vector<const char*>& deviceExtensions) {
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	QueueFamilyIndices indices = FindQueueFamilies(device, surface);

	bool extensionsSupported = CheckDeviceExtensionSupport(device, deviceExtensions);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() &&
			!swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && extensionsSupported
		&& swapChainAdequate && ((bool)supportedFeatures.samplerAnisotropy
			== true
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
								 &&
			// need wide lines for testing wireframes
			(bool)supportedFeatures.wideLines
#endif
	);
}

VkSampleCountFlagBits GfxDeviceManager::GetMaxUsableSampleCount(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

	VkSampleCountFlags counts = std::min(
		physicalDeviceProperties.limits.framebufferColorSampleCounts,
		physicalDeviceProperties.limits.framebufferDepthSampleCounts);
	if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
	if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
	if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
	if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
	if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
	if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
	return VK_SAMPLE_COUNT_1_BIT;
}

GfxDeviceManager::QueueFamilyIndices GfxDeviceManager::FindQueueFamilies(
	VkSurfaceKHR surface) const {
	return FindQueueFamilies(physicalDevice, surface);
}

GfxDeviceManager::QueueFamilyIndices GfxDeviceManager::FindQueueFamilies(
	VkPhysicalDevice device,
	VkSurfaceKHR surface) const {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	// note that the queue family index for graphics and presentation can
	// differ
	// TODO: consider finding transfer queue
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device,
			i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.IsComplete()) {
			break;
		}
		i++;
	}

	return indices;
}

bool GfxDeviceManager::CheckDeviceExtensionSupport(VkPhysicalDevice device,
	const std::vector<const char*>& deviceExtensions) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
		nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
		availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(),
		deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

GfxDeviceManager::SwapChainSupportDetails GfxDeviceManager::QuerySwapChainSupport(
	VkSurfaceKHR surface) const {
	return QuerySwapChainSupport(physicalDevice, surface);
}

GfxDeviceManager::SwapChainSupportDetails GfxDeviceManager::QuerySwapChainSupport(
	VkPhysicalDevice device, VkSurfaceKHR surface) const {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
		&details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
			details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
			&presentModeCount, details.presentModes.data());
	}

	return details;
}
