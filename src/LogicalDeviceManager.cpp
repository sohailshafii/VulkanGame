#include "LogicalDeviceManager.h"
#include "GfxDeviceManager.h"
#include "VulkanInstance.h"
#include <set>

LogicalDeviceManager::LogicalDeviceManager(const GfxDeviceManager *gfxDeviceManager,
	const VulkanInstance *instance, const VkSurfaceKHR surface,
	const std::vector<const char*>& deviceExtensions,
	bool enableValidationLayers) {
	GfxDeviceManager::QueueFamilyIndices indices =
		gfxDeviceManager->FindQueueFamilies(surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE; // try to smooth
	deviceFeatures.fillModeNonSolid = true; // TODO: remove
	// out interior of object at pref cost

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// necessary for stuff like swap chains!
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		auto& validationLayers = instance->validationLayers;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(gfxDeviceManager->GetPhysicalDevice(), &createInfo, nullptr, &device)
		!= VK_SUCCESS) {
		throw std::runtime_error("Failed to create logic device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(),
		0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(),
		0, &presentQueue);
}

LogicalDeviceManager::~LogicalDeviceManager() {
	vkDestroyDevice(device, nullptr);
}
