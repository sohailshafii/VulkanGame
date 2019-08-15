#pragma once

#include "vulkan/vulkan.h"
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

	const std::vector<const char*> validationLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

private:
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

	VkInstance vkInstance;
	VkResult creationResult;
};