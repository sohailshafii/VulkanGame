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

		bool IsComplete() {
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

	QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface) const;
	SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface) const;

	VkPhysicalDevice GetPhysicalDevice() const {
		return physicalDevice;
	}

	VkSampleCountFlagBits GetMSAASamples() const {
		return msaaSamples;
	}
private:
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device,
		VkSurfaceKHR surface) const;
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device,
		VkSurfaceKHR surface) const;

	void PickPhysicalDevice(const VkInstance& vkInstance,
		VkSurfaceKHR surface, const std::vector<const char*>& deviceExtensions);
	bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions);

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device,
		const std::vector<const char*>& deviceExtensions);
	VkSampleCountFlagBits GetMaxUsableSampleCount(VkPhysicalDevice device);

	VkPhysicalDevice physicalDevice;
	VkSampleCountFlagBits msaaSamples;
};
