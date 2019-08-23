#include "GfxDeviceManager.h"
#include <stdexcept>
#include <vector>
#include <algorithm>

GfxDeviceManager::GfxDeviceManager(const VkInstance& vkInstance) {
	physicalDevice = VK_NULL_HANDLE;
	msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	pickPhysicalDevice(vkInstance);
}

GfxDeviceManager::~GfxDeviceManager() {

}

void GfxDeviceManager::pickPhysicalDevice(const VkInstance& vkInstance) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

bool GfxDeviceManager::isDeviceSuitable(VkPhysicalDevice device) {
	return true;
}

VkSampleCountFlagBits GfxDeviceManager::getMaxUsableSampleCount(VkPhysicalDevice device) {
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
