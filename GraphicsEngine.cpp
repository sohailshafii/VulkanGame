
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"
#include "CommonBufferModule.h"

// TODO renaming this to something else..it's not really a graphics engine, but 
// something that gets recreated when something like the window or parts of the pipeline
// change
GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager, VkSurfaceKHR surface,
	GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
	VkCommandPool commandPool, VkImageView textureImageView, VkSampler textureSampler,
	const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
	VkBuffer vertexBuffer, VkBuffer indexBuffer) {
	this->logicalDeviceManager = logicalDeviceManager;
	createSwapChain(gfxDeviceManager, surface,
		window);
	createSwapChainImageViews();
	createRenderPassModule(gfxDeviceManager);
	createGraphicsPipeline(gfxDeviceManager,descriptorSetLayout);

	createColorResources(gfxDeviceManager, commandPool); // 5
	createDepthResources(gfxDeviceManager, commandPool); // 6
	createFramebuffers(); // 7
	createUniformBuffers(gfxDeviceManager); // 8

	createDescriptorPool();
	createDescriptorSets(descriptorSetLayout, textureImageView, textureSampler);
	createCommandBuffers(commandPool, vertices, indices, vertexBuffer, indexBuffer);
}

GraphicsEngine::~GraphicsEngine() {
	cleanUpSwapChain();
}

void GraphicsEngine::cleanUpSwapChain() {
	// TODO: verify notnull throughout
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(logicalDeviceManager->getDevice(), swapChainFramebuffers[i],
			nullptr);
	}

	vkDestroyImageView(logicalDeviceManager->getDevice(), colorImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->getDevice(), colorImage, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), colorImageMemory, nullptr);

	vkDestroyImageView(logicalDeviceManager->getDevice(), depthImageView, nullptr);
	vkDestroyImage(logicalDeviceManager->getDevice(), depthImage, nullptr);
	vkFreeMemory(logicalDeviceManager->getDevice(), depthImageMemory, nullptr);

	if (commandBufferModule != nullptr) {
		delete commandBufferModule;
	}

	if (graphicsPipelineModule != nullptr) {
		delete graphicsPipelineModule;
	}
	if (renderPassModule != nullptr) {
		delete renderPassModule;
	}
	size_t numSwapChainImages = swapChainManager->getSwapChainImages().size();
	if (swapChainManager != nullptr) {
		delete swapChainManager;
	}

	// TODO: each swap chain image should have an associated meta data with it
	// so create SwapChainImage class for that
	for (size_t i = 0; i < numSwapChainImages; i++) {
		vkDestroyBuffer(logicalDeviceManager->getDevice(), uniformBuffers[i], nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(logicalDeviceManager->getDevice(), descriptorPool, nullptr);
}

void GraphicsEngine::createSwapChain(GfxDeviceManager* gfxDeviceManager,
	VkSurfaceKHR surface, GLFWwindow* window) {
	swapChainManager = new SwapChainManager(gfxDeviceManager,
		logicalDeviceManager.get());
	swapChainManager->create(surface, window);
}

void GraphicsEngine::createSwapChainImageViews() {
	swapChainManager->createImageViews();
}

void GraphicsEngine::createRenderPassModule(GfxDeviceManager* gfxDeviceManager) {
	renderPassModule = new RenderPassModule(logicalDeviceManager->getDevice(),
		gfxDeviceManager->getPhysicalDevice(), swapChainManager->getSwapChainImageFormat(),
		gfxDeviceManager->getMSAASamples());
}

void GraphicsEngine::createGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
	VkDescriptorSetLayout descriptorSetLayout) {
	graphicsPipelineModule = new PipelineModule("shaders/vert.spv",
		"shaders/frag.spv", logicalDeviceManager->getDevice(),
		swapChainManager->getSwapChainExtent(), gfxDeviceManager,
		descriptorSetLayout, renderPassModule->GetRenderPass());
}

void GraphicsEngine::createColorResources(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool) {
	auto swapChainImageFormat = swapChainManager->getSwapChainImageFormat();
	auto swapChainExtent = swapChainManager->getSwapChainExtent();
	VkFormat colorFormat = swapChainImageFormat;

	Common::createImage(swapChainExtent.width, swapChainExtent.height, 1,
		gfxDeviceManager->getMSAASamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory,
		logicalDeviceManager.get(), gfxDeviceManager);
	colorImageView = Common::createImageView(colorImage, colorFormat,
		VK_IMAGE_ASPECT_COLOR_BIT, 1, logicalDeviceManager.get());

	Common::transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager.get());
}

void GraphicsEngine::createDepthResources(GfxDeviceManager* gfxDeviceManager,
	VkCommandPool commandPool) {
	VkFormat depthFormat = Common::findDepthFormat(gfxDeviceManager->getPhysicalDevice());
	auto swapChainExtent = swapChainManager->getSwapChainExtent();
	Common::createImage(swapChainExtent.width, swapChainExtent.height,
		1, gfxDeviceManager->getMSAASamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage, depthImageMemory, logicalDeviceManager.get(), gfxDeviceManager);
	depthImageView = Common::createImageView(depthImage, depthFormat,
		VK_IMAGE_ASPECT_DEPTH_BIT, 1, logicalDeviceManager.get());

	Common::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager.get());
}

void GraphicsEngine::createFramebuffers() {
	auto& swapChainImageViews = swapChainManager->getSwapChainImageViews();
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 3> attachments = {
			colorImageView,
			depthImageView,
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPassModule->GetRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		auto swapChainExtent = swapChainManager->getSwapChainExtent();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDeviceManager->getDevice(), &framebufferInfo, nullptr,
			&swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create framebuffer!");
		}
	}
}

void GraphicsEngine::createUniformBuffers(GfxDeviceManager* gfxDeviceManager) {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	}
}

void GraphicsEngine::createDescriptorPool() {
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	if (vkCreateDescriptorPool(logicalDeviceManager->getDevice(), &poolInfo,
		nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void GraphicsEngine::createDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
	VkImageView textureImageView, VkSampler textureSampler) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(),
		descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	if (vkAllocateDescriptorSets(logicalDeviceManager->getDevice(), &allocInfo,
		descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(logicalDeviceManager->getDevice(), static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(), 0,
			nullptr);
	}
}

void GraphicsEngine::createCommandBuffers(VkCommandPool commandPool, const std::vector<Vertex>& vertices,
	const std::vector<uint32_t>& indices, VkBuffer vertexBuffer, VkBuffer indexBuffer) {
	commandBufferModule = new CommandBufferModule(swapChainFramebuffers.size(),
		logicalDeviceManager->getDevice(), commandPool);
	auto& commandBuffers = commandBufferModule->getCommandBuffers();
	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPassModule->GetRenderPass();
		renderPassInfo.framebuffer = swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		auto swapChainExtent = swapChainManager->getSwapChainExtent();
		renderPassInfo.renderArea.extent = swapChainExtent;

		// order of clear values = order of attachments
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0 };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineModule->GetPipeline());

		// bind our vertex buffers
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineModule->GetLayout(), 0, 1, &descriptorSets[i], 0, nullptr);
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()),
			1, 0, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

 