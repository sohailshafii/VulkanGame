#pragma once

#include "vulkan/vulkan.h"
#include <optional>
#include <vector>

class GfxDeviceManager {
public:
	struct QueueFamilyIndices {
/*#if __APPLE__
		optional<uint32_t> graphicsFamily;
		optional<uint32_t> presentFamily;
#else*/
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
//#endif
		

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

	QueueFamilyIndices findQueueFamilies(VkSurfaceKHR surface) const;
	SwapChainSupportDetails querySwapChainSupport(VkSurfaceKHR surface) const;

	VkPhysicalDevice getPhysicalDevice() const {
		return physicalDevice;
	}

	VkSampleCountFlagBits getMSAASamples() const {
		return msaaSamples;
	}
private:
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device,
		VkSurfaceKHR surface) const;
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
		VkSurfaceKHR surface) const;

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
