#pragma once

#include "vulkan/vulkan.h"
#include "Common.h"
#include <vector>

class SwapChainManager;
class GfxDeviceManager;
class LogicalDeviceManager;
class RenderPassModule;
class PipelineModule;
class CommandBufferModule;
struct GLFWwindow;

class GraphicsEngine {
public:
	GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> logicalDeviceManager, VkSurfaceKHR surface,
		GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
		VkCommandPool commandPool, VkImageView textureImageView, VkSampler textureSampler,
		const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
		VkBuffer vertexBuffer, VkBuffer indexBuffer);

	~GraphicsEngine();

	void createGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
		VkDescriptorSetLayout descriptorSetLayout);

private:
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;
	SwapChainManager* swapChainManager;
	RenderPassModule* renderPassModule;
	PipelineModule* graphicsPipelineModule;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	CommandBufferModule* commandBufferModule;

	void cleanUpSwapChain();
	void createSwapChain(GfxDeviceManager* gfxDeviceManager,
		VkSurfaceKHR surface, GLFWwindow* window);
	void createSwapChainImageViews();
	void createRenderPassModule(GfxDeviceManager* gfxDeviceManager);

	void createColorResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void createDepthResources(GfxDeviceManager* gfxDeviceManager,
		VkCommandPool commandPool);
	void createFramebuffers();
	void createUniformBuffers(GfxDeviceManager* gfxDeviceManager);

	void createDescriptorPool();
	void createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
		VkImageView textureImageView, VkSampler textureSampler);
	void createCommandBuffers(VkCommandPool commandPool, const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices, VkBuffer vertexBuffer, VkBuffer indexBuffer);
};
