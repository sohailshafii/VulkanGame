#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

class GfxDeviceManager {
public:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() &&
				presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	GfxDeviceManager(const VkInstance& vkInstance, const
		VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
	~GfxDeviceManager();

	QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface);
	SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface);

	VkPhysicalDevice getPhysicalDevice() {
		return physicalDevice;
	}

	VkSampleCountFlagBits getMSAASamples() const {
		return msaaSamples;
	}
private:
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
		VkSurfaceKHR surface);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
		VkSurfaceKHR surface);

	void pickPhysicalDevice(const VkInstance& vkInstance,
		VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
	bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device,
		const std::vector<const char*>& deviceExtensions);
	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice device);

	VkPhysicalDevice physicalDevice;
	VkSampleCountFlagBits msaaSamples;
};
