#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <set>
#include <cstring>
#include <optional>
#include <algorithm>
#include <fstream>

#include <array>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "VulkanInstance.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "SwapChainManager.h"
#include "ShaderModule.h"
#include "PipelineModule.h"
#include "RenderPassModule.h"
#include "Common.h"
#include "CommonBufferModule.h"
#include "GraphicsEngine.h"

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

private:
	GLFWwindow *window;
	const int WIDTH = 800;
	const int HEIGHT = 600;

	const std::string MODEL_PATH = "./models/chalet.obj";
	const std::string TEXTURE_PATH = "./textures/chalet.jpg";

	const int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VulkanInstance *instance;
	GfxDeviceManager *gfxDeviceManager;
	LogicalDeviceManager *logicalDeviceManager;

	VkSurfaceKHR surface;

	SwapChainManager *swapChainMgr;
	RenderPassModule* renderPassModule;
	VkDescriptorSetLayout descriptorSetLayout;

	PipelineModule* graphicsPipelineModule;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	GraphicsEngine* graphicsEngine;

	VkCommandPool commandPool;
	CommandBufferModule* commandBufferModule;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow* window, int width,
		int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>
			(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void createInstance() {
		instance = new VulkanInstance(enableValidationLayers);

		if (!instance->createdSuccesfully()) {
			std::cout << "Return value: " << instance->getCreationResult() << std::endl;
			throw std::runtime_error("failed to create instance! ");
		}
	}

	void pickPhysicalDevice() {
		gfxDeviceManager = new GfxDeviceManager(instance->getVkInstance(), surface,
			deviceExtensions);
	}

	void initVulkan() {
		createInstance();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createDescriptorSetLayout();
		createCommandPool();

		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();

		createColorResources();
		createDepthResources();
		createFramebuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance->getVkInstance(), window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void createLogicalDevice() {
		logicalDeviceManager = new LogicalDeviceManager(gfxDeviceManager,
			instance, surface, deviceExtensions, enableValidationLayers);
	}

	void cleanupSwapChain() {
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(logicalDeviceManager->getDevice(), swapChainFramebuffers[i], nullptr);
		}

		vkDestroyImageView(logicalDeviceManager->getDevice(), colorImageView, nullptr);
		vkDestroyImage(logicalDeviceManager->getDevice(), colorImage, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), colorImageMemory, nullptr);

		vkDestroyImageView(logicalDeviceManager->getDevice(), depthImageView, nullptr);
		vkDestroyImage(logicalDeviceManager->getDevice(), depthImage, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), depthImageMemory, nullptr);

		delete commandBufferModule;

		delete graphicsPipelineModule;
		delete renderPassModule;

		size_t numSwapChainImages = swapChainMgr->getSwapChainImages().size();
		delete swapChainMgr;

		//delete graphicsEngine;

		// TODO: each swap chain image should have an associated meta data with it
		// so create SwapChainImage class for that
		for (size_t i = 0; i < numSwapChainImages; i++) {
			vkDestroyBuffer(logicalDeviceManager->getDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(logicalDeviceManager->getDevice(), descriptorPool, nullptr);
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		// in case window is minimized
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logicalDeviceManager->getDevice());

		cleanupSwapChain();

		createSwapChain(); // 1
		createImageViews(); // 2
		createRenderPass(); // 3
		createGraphicsPipeline(); // 4
		createColorResources(); // 5
		createDepthResources(); // 6
		createFramebuffers(); // 7
		createUniformBuffers(); // 8
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffers();
	}

	void createSwapChain() {
		swapChainMgr = new SwapChainManager(gfxDeviceManager,
			logicalDeviceManager);
		swapChainMgr->create(surface, window);

	//	graphicsEngine = new GraphicsEngine(gfxDeviceManager,
		//	logicalDeviceManager, surface, window);
	}

	void createImageViews() {
		swapChainMgr->createImageViews();
	}

	void createRenderPass() {
		renderPassModule = new RenderPassModule(logicalDeviceManager->getDevice(),
			gfxDeviceManager->getPhysicalDevice(), 
			swapChainMgr->getSwapChainImageFormat(), gfxDeviceManager->getMSAASamples());
	}

	void createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		
		std::array<VkDescriptorSetLayoutBinding, 2> bindings
			= { uboLayoutBinding, samplerLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(logicalDeviceManager->getDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void createGraphicsPipeline() {
		graphicsPipelineModule = new PipelineModule("shaders/vert.spv",
			"shaders/frag.spv", logicalDeviceManager->getDevice(),
			swapChainMgr->getSwapChainExtent(), gfxDeviceManager,
			descriptorSetLayout, renderPassModule->GetRenderPass());
	}

	void createFramebuffers() {
		auto& swapChainImageViews = swapChainMgr->getSwapChainImageViews();
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
			auto swapChainExtent = swapChainMgr->getSwapChainExtent();
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(logicalDeviceManager->getDevice(), &framebufferInfo, nullptr,
				&swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}
	}

	void createCommandPool() {
		GfxDeviceManager::QueueFamilyIndices queueFamilyIndices = gfxDeviceManager->
			findQueueFamilies(surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0;

		if (vkCreateCommandPool(logicalDeviceManager->getDevice(), &poolInfo, nullptr,
			&commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void createColorResources() {
		auto swapChainImageFormat = swapChainMgr->getSwapChainImageFormat();
		auto swapChainExtent = swapChainMgr->getSwapChainExtent();
		VkFormat colorFormat = swapChainImageFormat;

		Common::createImage(swapChainExtent.width, swapChainExtent.height, 1,
			gfxDeviceManager->getMSAASamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, logicalDeviceManager,
			gfxDeviceManager);
		colorImageView = Common::createImageView(colorImage, colorFormat,
			VK_IMAGE_ASPECT_COLOR_BIT, 1, logicalDeviceManager);

		Common::transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager);
	}

	void createDepthResources() {
		VkFormat depthFormat = Common::findDepthFormat(gfxDeviceManager->getPhysicalDevice());
		auto swapChainExtent = swapChainMgr->getSwapChainExtent();
		Common::createImage(swapChainExtent.width, swapChainExtent.height,
			1, gfxDeviceManager->getMSAASamples(), depthFormat, VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			depthImage, depthImageMemory, logicalDeviceManager, gfxDeviceManager);
		depthImageView = Common::createImageView(depthImage, depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT, 1, logicalDeviceManager);

		Common::transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1, commandPool, logicalDeviceManager);
	}

	void createTextureImage() {
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(),
			&texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("Failed to load texture image!");
		}
		mipLevels = static_cast<uint32_t>(std::floor(
			std::log2(std::max(texWidth, texHeight)))) + 1;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, imageSize, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer,
			stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, imageSize,
			0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

		stbi_image_free(pixels);

		Common::createImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory,
			logicalDeviceManager, gfxDeviceManager);

		Common::transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			mipLevels, commandPool, logicalDeviceManager);
		// transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		// while generating mipmaps
		
		copyBufferToImage(stagingBuffer, textureImage,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

		/*transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels);*/

		vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
		generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth,
			texHeight, mipLevels);
	}

	void generateMipmaps(VkImage image, VkFormat imageFormat, uint32_t texWidth,
		uint32_t texHeight, uint32_t mipLevels) {
		// check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(gfxDeviceManager->getPhysicalDevice(), imageFormat,
			&formatProperties);

		if (!(formatProperties.optimalTilingFeatures &&
			VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("Texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = Common::beginSingleTimeCommands(commandPool,
			logicalDeviceManager);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;
		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit = {};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 :
				1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer, image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit, VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr, 0, nullptr, 1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr, 0, nullptr, 1, &barrier);

		Common::endSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager);
	}

	void createTextureImageView() {
		textureImageView = Common::createImageView(textureImage,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, logicalDeviceManager);
	}

	void createTextureSampler() {
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		if (vkCreateSampler(logicalDeviceManager->getDevice(), &samplerInfo, nullptr,
			&textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create texture sampler!");
		}
	}

	void loadModel() {
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn,
			&err, MODEL_PATH.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices = {};
		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex = {};
				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					// vulkan is top to bottom for texture
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>
						(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	void createVertexBuffer() {
		VkDeviceSize bufferSize = sizeof(vertices[0])*vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void*data;
		vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
	}

	void createIndexBuffer() {
		VkDeviceSize bufferSize = sizeof(indices[0])*indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

		Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
	}

	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		const std::vector<VkImage>& swapChainImages = swapChainMgr->getSwapChainImages();
		uniformBuffers.resize(swapChainImages.size());
		uniformBuffersMemory.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			Common::createBuffer(logicalDeviceManager, gfxDeviceManager, bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		}
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = Common::beginSingleTimeCommands(commandPool,
			logicalDeviceManager);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // optional
		copyRegion.dstOffset = 0; // optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		Common::endSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager);
	}

	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
		uint32_t height) {
		VkCommandBuffer commandBuffer = Common::beginSingleTimeCommands(commandPool,
			logicalDeviceManager);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		Common::endSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager);
	}

	// TODO: create command buffer module that encapsulates the allocate info, etc
	void createCommandBuffers() {
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
			auto swapChainExtent = swapChainMgr->getSwapChainExtent();
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

	void createDescriptorPool() {
		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		const std::vector<VkImage>& swapChainImages = swapChainMgr->getSwapChainImages();
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

	void createDescriptorSets() {
		const std::vector<VkImage>& swapChainImages = swapChainMgr->getSwapChainImages();
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

	void createSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(logicalDeviceManager->getDevice(), &semaphoreInfo, nullptr,
				&imageAvailableSemaphores[i])
				!= VK_SUCCESS ||
				vkCreateSemaphore(logicalDeviceManager->getDevice(), &semaphoreInfo, nullptr,
				&renderFinishedSemaphores[i])
				!= VK_SUCCESS ||
				vkCreateFence(logicalDeviceManager->getDevice(), &fenceInfo, nullptr,
					&inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create a semaphore for a frame!");
			}
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		// wait for all operations to finish before cleaning up
		vkDeviceWaitIdle(logicalDeviceManager->getDevice());
	}

	void drawFrame() {
		vkWaitForFences(logicalDeviceManager->getDevice(), 1, &inFlightFences[currentFrame], VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(logicalDeviceManager->getDevice(),
			swapChainMgr->getSwapChain(), std::numeric_limits<uint64_t>::max(),
			imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		updateUniformBuffer(imageIndex);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(commandBufferModule->getCommandBuffers()[imageIndex]);

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(logicalDeviceManager->getDevice(), 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(logicalDeviceManager->getGraphicsQueue(), 1, &submitInfo,
			inFlightFences[currentFrame]) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChainMgr->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr;

		result = vkQueuePresentKHR(logicalDeviceManager->getPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result ==
			VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void updateUniformBuffer(uint32_t currentImage) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>
			(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
			glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
			glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		auto swapChainExtent = swapChainMgr->getSwapChainExtent();
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
			(float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl

		void* data;
		vkMapMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[currentImage]);
	}

	void cleanUp() {
		cleanupSwapChain();

		vkDestroySampler(logicalDeviceManager->getDevice(), textureSampler, nullptr);
		vkDestroyImageView(logicalDeviceManager->getDevice(), textureImageView, nullptr);

		vkDestroyImage(logicalDeviceManager->getDevice(), textureImage, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), textureImageMemory, nullptr);
		
		vkDestroyDescriptorSetLayout(logicalDeviceManager->getDevice(), descriptorSetLayout, nullptr);

		vkDestroyBuffer(logicalDeviceManager->getDevice(), indexBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(logicalDeviceManager->getDevice(), vertexBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), vertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDeviceManager->getDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logicalDeviceManager->getDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logicalDeviceManager->getDevice(), inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(logicalDeviceManager->getDevice(), commandPool, nullptr);

		delete logicalDeviceManager;

		vkDestroySurfaceKHR(instance->getVkInstance(), surface, nullptr);

		delete gfxDeviceManager;

		delete instance;

		glfwDestroyWindow(window);

		glfwTerminate();
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
