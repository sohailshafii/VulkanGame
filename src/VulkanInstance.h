#pragma once

#include "vulkan/vulkan.h"
#include <iostream>
#include <vector>

class VulkanInstance {
public:
	VulkanInstance(bool enableValidationLayers);
	~VulkanInstance();

	bool CreatedSuccesfully() const {
		return creationResult == VK_SUCCESS;
	}

	VkResult GetCreationResult() const {
		return creationResult;
	}

	// TODO: return ref?
	VkInstance& GetVkInstance() {
		return vkInstance;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

private:
	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions(bool enableValidationLayers);
	void SetupDebugMessenger(bool enableValidationLayers);

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&
		createInfo);
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance,
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator);

	VkInstance vkInstance;
	VkResult creationResult;
	bool enableValidationLayers;

	VkDebugUtilsMessengerEXT debugMessenger;
};
