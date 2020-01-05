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
#include "PipelineModule.h"
#include "RenderPassModule.h"
#include "Common.h"
#include "CommonBufferModule.h"
#include "GraphicsEngine.h"
#include "ImageTextureLoader.h"

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
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	VkSurfaceKHR surface;

	VkDescriptorSetLayout descriptorSetLayout;

	GraphicsEngine* graphicsEngine;

	VkCommandPool commandPool;

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

	uint32_t mipLevels;
	ImageTextureLoader* imageTexture;

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

		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createTextureImage();

		graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
			surface, window, descriptorSetLayout, commandPool, imageTexture,
			vertices, indices, vertexBuffer, indexBuffer);

		createSyncObjects();
	}

	void createSurface() {
		if (glfwCreateWindowSurface(instance->getVkInstance(), window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void createLogicalDevice() {
		logicalDeviceManager = std::make_shared<LogicalDeviceManager>(gfxDeviceManager,
			instance, surface, deviceExtensions, enableValidationLayers);
	}

	void recreateSwapChain() {
		int width = 0, height = 0;
		// in case window is minimized; wait for it
		// to come back up again
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logicalDeviceManager->getDevice());

		delete graphicsEngine;
		graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
			surface, window, descriptorSetLayout, commandPool, imageTexture,
			vertices, indices, vertexBuffer, indexBuffer);
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

	void createTextureImage() {
		imageTexture = new ImageTextureLoader(TEXTURE_PATH,
			gfxDeviceManager, logicalDeviceManager, commandPool);
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
		Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void*data;
		vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

		Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
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
		Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(logicalDeviceManager->getDevice(), stagingBufferMemory);

		Common::createBuffer(logicalDeviceManager.get(), gfxDeviceManager, bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(logicalDeviceManager->getDevice(), stagingBuffer, nullptr);
		vkFreeMemory(logicalDeviceManager->getDevice(), stagingBufferMemory, nullptr);
	}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = Common::beginSingleTimeCommands(commandPool,
			logicalDeviceManager.get());

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // optional
		copyRegion.dstOffset = 0; // optional
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		Common::endSingleTimeCommands(commandBuffer, commandPool, logicalDeviceManager.get());
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
			graphicsEngine->GetSwapChainManager()->getSwapChain(), std::numeric_limits<uint64_t>::max(),
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
		submitInfo.pCommandBuffers = &(graphicsEngine->GetCommandBufferModule()->getCommandBuffers()[imageIndex]);

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

		VkSwapchainKHR swapChains[] = { graphicsEngine->GetSwapChainManager()->getSwapChain() };
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

	// TODO: use push constants, that's more efficient
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
		auto swapChainExtent = graphicsEngine->GetSwapChainManager()->getSwapChainExtent();
		ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width /
			(float)swapChainExtent.height, 0.1f, 10.0f);
		ubo.proj[1][1] *= -1; // flip Y -- opposite of opengl

		void* data;
		auto& uniformBuffersMemory = graphicsEngine->GetUniformBuffersMemory();
		vkMapMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[currentImage], 0,
			sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(logicalDeviceManager->getDevice(), uniformBuffersMemory[currentImage]);
	}

	void cleanUp() {
		delete graphicsEngine;

		delete imageTexture;

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

		logicalDeviceManager.reset();

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
