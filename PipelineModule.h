#pragma once

#include "vulkan/vulkan.h"
#include "GfxDeviceManager.h"
#include "DescriptorSetFunctions.h"
#include <string>

class ResourceLoader;

class PipelineModule {
public:
	PipelineModule(const std::string& vertShaderName,
		const std::string& fragShaderName, VkDevice device,
		VkExtent2D swapChainExtent, GfxDeviceManager *gfxDeviceManager,
		ResourceLoader *resourceLoader,
		VkDescriptorSetLayout descriptorSetLayout,
		VkRenderPass renderPass,
		DescriptorSetFunctions::MaterialType materialType);
	~PipelineModule();

	VkPipelineLayout GetLayout() {
		return pipelineLayout;
	}

	VkPipeline GetPipeline() {
		return graphicsPipeline;
	}

private:
	VkPipelineLayout pipelineLayout;
	VkDevice device;
	VkPipeline graphicsPipeline;
};
