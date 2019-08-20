#pragma once

#include "vulkan/vulkan.h"
#include <iostream>
#include <vector>

class VulkanInstance {
public:
	VulkanInstance(bool enableValidationLayers);
	~VulkanInstance();

	bool createdSuccesfully() const {
		return creationResult == VK_SUCCESS;
	}

	VkResult getCreationResult() const {
		return creationResult;
	}

	// TODO: return ref?
	VkInstance& getVkInstance() {
		return vkInstance;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

private:
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

	VkInstance vkInstance;
	VkResult creationResult;
};