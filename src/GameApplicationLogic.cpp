#include "GameApplicationLogic.h"
#include <string>
#include "Camera.h"
#include "VulkanInstance.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "GraphicsEngine.h"
#include "ResourceLoader.h"
#include "GraphicsEngine.h"
#include "SwapChainManager.h"
#include "CommonBufferModule.h"
#include "SceneManagement/Scene.h"
#include "SceneManagement/SceneLoader.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Player/PlayerGameObjectBehavior.h"
#include "GameEngine.h"

#if __APPLE__
const std::string MODEL_PATH = "../../models/chalet.obj";
const std::string CUBE_MODEL_PATH = "../../models/cube.obj";
const std::string TEXTURE_PATH = "../../textures/chalet.jpg";
#else
const std::string MODEL_PATH = "../models/chalet.obj";
const std::string CUBE_MODEL_PATH = "../models/cube.obj";
const std::string TEXTURE_PATH = "../textures/chalet.jpg";
#endif

bool GameApplicationLogic::firstMouse = false;
float GameApplicationLogic::lastX = 0.0f;
float GameApplicationLogic::lastY = 0.0f;
float GameApplicationLogic::lastFrameTime = 0.0f;

GameEngine* GameApplicationLogic::gameEngine;

void GameApplicationLogic::Run() {
	InitWindow();
	InitVulkan();
	MainLoop();
	CleanUp();
}

void GameApplicationLogic::MouseCallback(GLFWwindow* window,
	double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top
	lastX = (float)xpos;
	lastY = (float)ypos;
	GameApplicationLogic::gameEngine->ProcessMouse(xpos, ypos, xoffset, yoffset);
}

void GameApplicationLogic::KeyCallback(GLFWwindow* window, int key, int scancode,
	int action, int mods) {
	GameApplicationLogic::gameEngine->ProcessKeyCallback(window, key, scancode,
		action, mods);
}

void GameApplicationLogic::ProcessInput(GLFWwindow* window,
	float currentFrameTime) {
	GameApplicationLogic::gameEngine->ProcessInput(window,
		currentFrameTime, lastFrameTime);
}

void GameApplicationLogic::InitWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "PlanetDeath", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);
}

void GameApplicationLogic::FramebufferResizeCallback(GLFWwindow* window,
	int width, int height) {
	auto app = reinterpret_cast<GameApplicationLogic*>
		(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

void GameApplicationLogic::CreateInstance() {
	instance = new VulkanInstance(enableValidationLayers);

	if (!instance->CreatedSuccesfully()) {
		std::cout << "Return value: " << instance->GetCreationResult() << std::endl;
		throw std::runtime_error("failed to create instance! ");
	}
}

void GameApplicationLogic::PickPhysicalDevice() {
	gfxDeviceManager = new GfxDeviceManager(instance->GetVkInstance(), surface,
		deviceExtensions);
}

void GameApplicationLogic::InitVulkan() {
	CreateInstance();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateCommandPool();

	resourceLoader = new ResourceLoader();

	gameEngine = new GameEngine(GameEngine::GameMode::Menu,
		gfxDeviceManager, logicalDeviceManager, resourceLoader,
		surface, window, commandPool, poolInfo);

	CreateSyncObjects();
}

void GameApplicationLogic::CreateSurface() {
	if (glfwCreateWindowSurface(instance->GetVkInstance(), window,
		nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

void GameApplicationLogic::CreateLogicalDevice() {
	logicalDeviceManager = std::make_shared<LogicalDeviceManager>(gfxDeviceManager,
		instance, surface, deviceExtensions, enableValidationLayers);
}

void GameApplicationLogic::RecreateSwapChain() {
	int width = 0, height = 0;
	// in case window is minimized; wait for it
	// to come back up again
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(logicalDeviceManager->GetDevice());

	gameEngine->RecreateGraphicsEngineForNewSwapchain(gfxDeviceManager,
		logicalDeviceManager, resourceLoader, surface, window, commandPool,
		poolInfo);
}

void GameApplicationLogic::CreateCommandPool() {
	GfxDeviceManager::QueueFamilyIndices queueFamilyIndices = gfxDeviceManager->
		FindQueueFamilies(surface);

	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	// need to be able to re-record at some point
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.pNext = nullptr;

	if (vkCreateCommandPool(logicalDeviceManager->GetDevice(), &poolInfo, nullptr,
		&commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

void GameApplicationLogic::CreateSyncObjects() {
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

void GameApplicationLogic::MainLoop() {
	lastFrameTime = (float)glfwGetTime();
	float lastFrameReportTime = lastFrameTime;

	while (!glfwWindowShouldClose(window)) {
		float currentFrameTime = (float)glfwGetTime();
		float deltaTime = currentFrameTime - lastFrameTime;
		GameApplicationLogic::ProcessInput(window, currentFrameTime);
		lastFrameTime = currentFrameTime;

		uint32_t imageIndex;
		if (CanAcquireNextPresentableImageIndex(imageIndex)) {
			UpdateGameState(currentFrameTime, deltaTime, imageIndex);
			DrawFrame(imageIndex);
		}

		if ((currentFrameTime - lastFrameReportTime) > 3.0f) {
			lastFrameReportTime = currentFrameTime;
			std::cout << "Current FPS: " << 1.0f / deltaTime
				<< ".\n";
		}

		glfwPollEvents();
	}

	// wait for all operations to finish before cleaning up
	vkDeviceWaitIdle(logicalDeviceManager->GetDevice());
}

bool GameApplicationLogic::CanAcquireNextPresentableImageIndex(
	uint32_t& imageIndex) {
	vkWaitForFences(logicalDeviceManager->GetDevice(), 1,
		&inFlightFences[currentFrame], VK_TRUE,
		std::numeric_limits<uint64_t>::max());

	VkResult result = vkAcquireNextImageKHR(logicalDeviceManager->GetDevice(),
		gameEngine->GetGraphicsEngine()->GetSwapChainManager()->GetSwapChain(),
		std::numeric_limits<uint64_t>::max(),
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

void GameApplicationLogic::UpdateGameState(float time,
	float deltaTime, uint32_t imageIndex) {
	gameEngine->UpdateFrame(time, deltaTime, imageIndex,
		gfxDeviceManager, resourceLoader, inFlightFences);
}

void GameApplicationLogic::DrawFrame(uint32_t imageIndex) {
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { 
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = 
		gameEngine->GetGraphicsEngine()->GetCommandBufferData(imageIndex);

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(logicalDeviceManager->GetDevice(), 1, &inFlightFences[currentFrame]);

	VkResult submitResult = vkQueueSubmit(logicalDeviceManager->GetGraphicsQueue(),
		1, &submitInfo, inFlightFences[currentFrame]);
	if (submitResult != VK_SUCCESS) {
		std::cerr << "Submit result: " << submitResult << std::endl;
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {
		gameEngine->GetGraphicsEngine()->GetSwapChainManager()->GetSwapChain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;

	VkResult result = vkQueuePresentKHR(logicalDeviceManager->GetPresentQueue(),
		&presentInfo);

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

void GameApplicationLogic::CleanUp() {
	delete resourceLoader;

	// delete game objects before destroying vulkan instance
	delete gameEngine;

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

