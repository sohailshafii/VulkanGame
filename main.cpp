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
#include "ResourceLoader.h"
#include "Model.h"
#include "Camera.h"

#include "GameObject.h"

class HelloTriangleApplication {
public:
	void run() {
		InitWindow();
		InitVulkan();
		MainLoop();
		CleanUp();
	}

	static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void ProcessInput(GLFWwindow* window, float frameTime);

private:
	GLFWwindow *window;
	const int WIDTH = 800;
	const int HEIGHT = 600;

#if __APPLE__
	const std::string MODEL_PATH = "../../models/chalet.obj";
	const std::string CUBE_MODEL_PATH = "../../models/cube.obj";
	const std::string TEXTURE_PATH = "../../textures/chalet.jpg";
#else
	const std::string MODEL_PATH = "../models/chalet.obj";
	const std::string CUBE_MODEL_PATH = "../models/cube.obj";
	const std::string TEXTURE_PATH = "../textures/chalet.jpg";
#endif

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

	/*VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;*/

	ResourceLoader* resourceLoader;
	std::vector<std::shared_ptr<GameObject>> gameObjects;

	// need to be static for cursor callback function to work
	static Camera mainCamera;
	static bool firstMouse;
	static float lastX, lastY;
	static float lastFrameTime;

	void InitWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetCursorPosCallback(window, MouseCallback);
		// for fps mode we want to capture the mouse
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
	}

	static void FramebufferResizeCallback(GLFWwindow* window, int width,
		int height) {
		auto app = reinterpret_cast<HelloTriangleApplication*>
			(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void CreateInstance() {
		instance = new VulkanInstance(enableValidationLayers);

		if (!instance->CreatedSuccesfully()) {
			std::cout << "Return value: " << instance->GetCreationResult() << std::endl;
			throw std::runtime_error("failed to create instance! ");
		}
	}

	void PickPhysicalDevice() {
		gfxDeviceManager = new GfxDeviceManager(instance->GetVkInstance(), surface,
			deviceExtensions);
	}

	void InitVulkan() {
		CreateInstance();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateDescriptorSetLayout();
		CreateCommandPool();

		resourceLoader = new ResourceLoader();
	
		CreateGameObjects();

		graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
			resourceLoader, surface, window, descriptorSetLayout, commandPool,
			resourceLoader->GetTexture(TEXTURE_PATH, gfxDeviceManager, logicalDeviceManager, commandPool),
			gameObjects);

		CreateSyncObjects();
	}

	void CreateSurface() {
		if (glfwCreateWindowSurface(instance->GetVkInstance(), window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void CreateLogicalDevice() {
		logicalDeviceManager = std::make_shared<LogicalDeviceManager>(gfxDeviceManager,
			instance, surface, deviceExtensions, enableValidationLayers);
	}

	void RecreateSwapChain() {
		int width = 0, height = 0;
		// in case window is minimized; wait for it
		// to come back up again
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logicalDeviceManager->GetDevice());

		delete graphicsEngine;
		graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
			resourceLoader, surface, window, descriptorSetLayout, commandPool,
			resourceLoader->GetTexture(TEXTURE_PATH, gfxDeviceManager, logicalDeviceManager, commandPool),
			gameObjects);
	}

	void CreateDescriptorSetLayout() {
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

		if (vkCreateDescriptorSetLayout(logicalDeviceManager->GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}
	}

	void CreateCommandPool() {
		GfxDeviceManager::QueueFamilyIndices queueFamilyIndices = gfxDeviceManager->
			FindQueueFamilies(surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = 0;

		if (vkCreateCommandPool(logicalDeviceManager->GetDevice(), &poolInfo, nullptr,
			&commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void CreateGameObjects() {
		std::shared_ptr<GameObject> houseObj = std::make_shared<GameObject>(resourceLoader->GetModel(MODEL_PATH),
										gfxDeviceManager, logicalDeviceManager,
										commandPool);
		
		std::shared_ptr<GameObject> cubeObj =
		std::make_shared<GameObject>(resourceLoader->GetModel(CUBE_MODEL_PATH),
									 gfxDeviceManager, logicalDeviceManager,
									 commandPool);
	
		gameObjects.push_back(houseObj);
		gameObjects.push_back(cubeObj);
	}

	void CreateSyncObjects() {
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(logicalDeviceManager->GetDevice(), &semaphoreInfo, nullptr,
				&imageAvailableSemaphores[i])
				!= VK_SUCCESS ||
				vkCreateSemaphore(logicalDeviceManager->GetDevice(), &semaphoreInfo, nullptr,
				&renderFinishedSemaphores[i])
				!= VK_SUCCESS ||
				vkCreateFence(logicalDeviceManager->GetDevice(), &fenceInfo, nullptr,
					&inFlightFences[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create a semaphore for a frame!");
			}
		}
	}

	void MainLoop() {
		lastFrameTime = glfwGetTime();
		float lastFrameReportTime = lastFrameTime;
		
		while (!glfwWindowShouldClose(window)) {
			float currentFrameTime = glfwGetTime();
			float deltaTime = currentFrameTime - lastFrameTime;
			lastFrameTime = currentFrameTime;
			HelloTriangleApplication::ProcessInput(window, deltaTime);
			
			uint32_t imageIndex;
			if (CanAcquireNextPresentableImageIndex(imageIndex)) {
				UpdateGameState(deltaTime, imageIndex);
				DrawFrame(imageIndex);
			}
			
			if ((currentFrameTime - lastFrameReportTime)
				> 3.0f) {
				lastFrameReportTime = currentFrameTime;
				std::cout << "Current FPS: " << 1.0f/deltaTime
					<< ".\n";
			}

			glfwPollEvents();
		}

		// wait for all operations to finish before cleaning up
		vkDeviceWaitIdle(logicalDeviceManager->GetDevice());
	}
	
	bool CanAcquireNextPresentableImageIndex(uint32_t& imageIndex) {
		vkWaitForFences(logicalDeviceManager->GetDevice(), 1, &inFlightFences[currentFrame], VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(logicalDeviceManager->GetDevice(),
			graphicsEngine->GetSwapChainManager()->GetSwapChain(), std::numeric_limits<uint64_t>::max(),
			imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swap chain image!");
		}
		
		return true;
	}
	

	// TODO: loop over models. per model do:
	// get model transform
	// get command buffers per model
	// wait on those buffers
	void UpdateGameState(float deltaTime, uint32_t imageIndex) {
		for (std::shared_ptr<GameObject>& gameObject : gameObjects) {
			gameObject->UpdateUniformBuffer(imageIndex,
											HelloTriangleApplication::mainCamera.ConstructViewMatrix(),
											graphicsEngine->GetSwapChainManager()->GetSwapChainExtent());
		}
	}

	void DrawFrame(uint32_t imageIndex) {
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &(graphicsEngine->GetCommandBufferModule()->GetCommandBuffers()[imageIndex]);

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(logicalDeviceManager->GetDevice(), 1, &inFlightFences[currentFrame]);

		if (vkQueueSubmit(logicalDeviceManager->GetGraphicsQueue(), 1, &submitInfo,
			inFlightFences[currentFrame]) !=
			VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { graphicsEngine->GetSwapChainManager()->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		presentInfo.pResults = nullptr;

		VkResult result = vkQueuePresentKHR(logicalDeviceManager->GetPresentQueue(), &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result ==
			VK_SUBOPTIMAL_KHR || framebufferResized) {
			framebufferResized = false;
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swap chain image!");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void CleanUp() {
		delete resourceLoader;

		delete graphicsEngine;
		
		// kill game objects before device manager is removed
		gameObjects.clear();

		vkDestroyDescriptorSetLayout(logicalDeviceManager->GetDevice(), descriptorSetLayout, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logicalDeviceManager->GetDevice(), renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(logicalDeviceManager->GetDevice(), imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(logicalDeviceManager->GetDevice(), inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(logicalDeviceManager->GetDevice(), commandPool, nullptr);

		logicalDeviceManager.reset();

		vkDestroySurfaceKHR(instance->GetVkInstance(), surface, nullptr);

		delete gfxDeviceManager;

		delete instance;

		glfwDestroyWindow(window);

		glfwTerminate();
	}
};

Camera HelloTriangleApplication::mainCamera = Camera(glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 0.0f,-1.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f, 4.5f,
	3.5f);
bool HelloTriangleApplication::firstMouse = false;
float HelloTriangleApplication::lastX = 0.0f;
float HelloTriangleApplication::lastY = 0.0f;
float HelloTriangleApplication::lastFrameTime = 0.0f;

void HelloTriangleApplication::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;
	
	float currentFrameTime = glfwGetTime();
	float deltaTime = currentFrameTime - lastFrameTime;
	HelloTriangleApplication::mainCamera.ProcessMouse(deltaTime*xoffset,
													  deltaTime*yoffset);
}

void HelloTriangleApplication::ProcessInput(GLFWwindow* window, float frameTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera.MoveForward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera.MoveBackward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera.MoveLeft(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera.MoveRight(frameTime);
	}
}

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
