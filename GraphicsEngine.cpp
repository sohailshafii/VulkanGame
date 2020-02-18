
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "RenderPassModule.h"
#include "PipelineModule.h"
#include "CommonBufferModule.h"
#include "ImageTextureLoader.h"
#include "ResourceLoader.h"

// TODO renaming this to something else..it's not really a graphics engine, but 
// something that gets recreated when something like the window or parts of the pipeline
// change
GraphicsEngine::GraphicsEngine(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager,
	ResourceLoader *resourceLoader, VkSurfaceKHR surface,
	GLFWwindow* window, VkDescriptorSetLayout descriptorSetLayout,
	VkCommandPool commandPool, std::shared_ptr<ImageTextureLoader> imageTexture,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	this->logicalDeviceManager = logicalDeviceManager;
	CreateSwapChain(gfxDeviceManager, surface,
		window);
	CreateSwapChainImageViews();
	CreateRenderPassModule(gfxDeviceManager);
	CreateGraphicsPipeline(gfxDeviceManager, resourceLoader, descriptorSetLayout);

	CreateColorResources(gfxDeviceManager, commandPool); // 5
	CreateDepthResources(gfxDeviceManager, commandPool); // 6
	CreateFramebuffers(); // 7
	CreateUniformBuffers(gfxDeviceManager, gameObjects); // 8

	/*CreateDescriptorPool();
	CreateDescriptorSets(descriptorSetLayout, imageTexture->getTextureImageView(),
		imageTexture->getTextureImageSampler(), gameObjects);*/
	CreateDescriptorPoolAndSets(descriptorSetLayout, imageTexture->getTextureImageView(), imageTexture->getTextureImageSampler(), gameObjects);
	CreateCommandBuffers(commandPool, gameObjects);
}

GraphicsEngine::~GraphicsEngine() {
	CleanUpSwapChain();
}

void GraphicsEngine::CleanUpSwapChain() {
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
	if (swapChainManager != nullptr) {
		delete swapChainManager;
	}
}

void GraphicsEngine::CreateSwapChain(GfxDeviceManager* gfxDeviceManager,
	VkSurfaceKHR surface, GLFWwindow* window) {
	swapChainManager = new SwapChainManager(gfxDeviceManager,
		logicalDeviceManager.get());
	swapChainManager->create(surface, window);
}

void GraphicsEngine::CreateSwapChainImageViews() {
	swapChainManager->createImageViews();
}

void GraphicsEngine::CreateRenderPassModule(GfxDeviceManager* gfxDeviceManager) {
	renderPassModule = new RenderPassModule(logicalDeviceManager->getDevice(),
		gfxDeviceManager->getPhysicalDevice(), swapChainManager->getSwapChainImageFormat(),
		gfxDeviceManager->getMSAASamples());
}

void GraphicsEngine::CreateGraphicsPipeline(GfxDeviceManager* gfxDeviceManager,
	ResourceLoader* resourceLoader, VkDescriptorSetLayout descriptorSetLayout) {
	graphicsPipelineModule = new PipelineModule("shaders/vert.spv",
		"shaders/frag.spv", logicalDeviceManager->getDevice(),
		swapChainManager->getSwapChainExtent(), gfxDeviceManager,
		resourceLoader, descriptorSetLayout, renderPassModule->GetRenderPass());
}

void GraphicsEngine::CreateColorResources(GfxDeviceManager* gfxDeviceManager,
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

void GraphicsEngine::CreateDepthResources(GfxDeviceManager* gfxDeviceManager,
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

void GraphicsEngine::CreateFramebuffers() {
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

void GraphicsEngine::CreateUniformBuffers(GfxDeviceManager* gfxDeviceManager,
										  std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto gameObject : gameObjects) {
		gameObject->CreateCommandBuffers(gfxDeviceManager, numSwapChainImages);
	}
}

/*void GraphicsEngine::CreateDescriptorPool() {
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
}*/

// ubo -> descriptor set -> draw calls
// each ubo has its own descriptor set.
// for each object, we have N ubos, and each ubo has an associated descriptor set
// we need a way to allow multiple ubos, which means multiple descriptor sets, each of which is associated with a game object. so associate a descriptor set with a game object
/*void GraphicsEngine::CreateDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
	VkImageView textureImageView, VkSampler textureSampler,
	std::vector<std::shared_ptr<GameObject>>& gameObjects) {
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
		// TODO: do this per game object?
		bufferInfo.buffer = gameObjects[0]->GetUniformBuffer(i);
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
}*/

void GraphicsEngine::CreateDescriptorPoolAndSets(VkDescriptorSetLayout descriptorSetLayout,
								 VkImageView textureImageView, VkSampler textureSampler,
								 std::vector<std::shared_ptr<GameObject>>& gameObjects) {
	const std::vector<VkImage>& swapChainImages = swapChainManager->getSwapChainImages();
	size_t numSwapChainImages = swapChainImages.size();
	for(auto gameObject : gameObjects) {
		gameObject->CreateDescriptorPoolAndSets(numSwapChainImages, descriptorSetLayout, textureImageView, textureSampler);
	}
}

// TODO: create command buffer module that encapsulates the allocate info, etc
// TODO: it should be possible to have multiple pipelines/command buffers
// per object. have a ubo per object, then update that ubo based on the matrices associated
// move render logic to this class!
void GraphicsEngine::CreateCommandBuffers(VkCommandPool commandPool,
										  std::vector<std::shared_ptr<GameObject>>& gameObjects) {	
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
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = { 1.0f, 0 };

		//VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipelineModule->GetPipeline());

		for (const auto& gameObject : gameObjects) {
			// bind our vertex buffers
			VkBuffer vertexBuffers[] = { gameObject->GetVertexBuffer() };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdBindIndexBuffer(commandBuffers[i], gameObject->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS,
									graphicsPipelineModule->GetLayout(), 0, 1, gameObject->GetDescriptorSetPtr(i), 0, nullptr);
			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(gameObject->GetModel()->GetIndices().size()),
							 1, 0, 0, 0);
		}

		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

 
