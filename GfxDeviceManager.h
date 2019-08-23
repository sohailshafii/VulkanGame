#pragma once

#include "vulkan/vulkan.h"
#include <optional>

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

	GfxDeviceManager(const VkInstance& vkInstance);
	~GfxDeviceManager();

	VkPhysicalDevice getPhysicalDevice() {
		return physicalDevice;
	}

	VkSampleCountFlagBits getMSAASamples() const {
		return msaaSamples;
	}
private:
	void pickPhysicalDevice(const VkInstance& vkInstance);
	bool isDeviceSuitable(VkPhysicalDevice device);

	VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	VkPhysicalDevice physicalDevice;
	VkSampleCountFlagBits msaaSamples;
};
