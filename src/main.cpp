#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <ctime>
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

#include "GameObjects/GameObject.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/PlayerGameObjectBehavior.h"
#include "SceneManagement/Scene.h"
#include "SceneManagement/SceneLoader.h"

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
	const int WIDTH = 1366;
	const int HEIGHT = 768;

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

	GraphicsEngine* graphicsEngine;

	VkCommandPool commandPool;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	ResourceLoader* resourceLoader;
	Scene* mainGameScene;

	// need to be static for cursor callback function to work
	static std::shared_ptr<Camera> mainCamera;
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
		CreateCommandPool();

		resourceLoader = new ResourceLoader();
	
		CreateGameObjects();

		graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
			resourceLoader, surface, window, commandPool,
			mainGameScene->GetGameObjects());

		CreateSyncObjects();
	}

	void CreatePlayerGameObject() {
		// add player game object; this is necessary because enemies
		// need to know where the player is
		std::shared_ptr<Material> gameObjectMaterial =
			GameObjectCreator::CreateMaterial(
				DescriptorSetFunctions::MaterialType::UnlitColor,
				"texture.jpg", resourceLoader, gfxDeviceManager,
				logicalDeviceManager, commandPool);
		std::shared_ptr gameObjectModel = GameObjectCreator::LoadModelFromName(
			"cube.obj", resourceLoader);
		glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
			glm::vec3(0.0f, 0.0f, 4.0f));
		std::shared_ptr<GameObject> newGameObject =
			GameObjectCreator::CreateGameObject(gameObjectMaterial,
				gameObjectModel,
				std::make_unique<PlayerGameObjectBehavior>(mainCamera),
				localToWorldTransform, resourceLoader, gfxDeviceManager,
				logicalDeviceManager, commandPool);
		mainGameScene->AddGameObject(newGameObject);
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
			resourceLoader, surface, window, commandPool, mainGameScene->GetGameObjects());
	}

	void CreateCommandPool() {
		GfxDeviceManager::QueueFamilyIndices queueFamilyIndices = gfxDeviceManager->
			FindQueueFamilies(surface);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		// need to be able to re-record at some point
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(logicalDeviceManager->GetDevice(), &poolInfo, nullptr,
			&commandPool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool!");
		}
	}

	void CreateGameObjects() {
#if __APPLE__
		std::string scenePath = "../../mainGameScene.json";
#else
		std::string scenePath = "../mainGameScene.json";
#endif
		mainGameScene = new Scene(resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
		SceneLoader::SceneSettings sceneSettings;

		SceneLoader::DeserializeJSONFileIntoScene(
			resourceLoader, gfxDeviceManager, logicalDeviceManager,
			commandPool, mainGameScene, sceneSettings, scenePath);

		mainCamera->InitializeCameraSystem(glm::vec3(0.0f, 2.0f, 100.0f),
			-90.0f, 0.0f, 14.5f, 0.035f);
		mainCamera->InitializeCameraSystem(sceneSettings.cameraPosition,
			sceneSettings.cameraYaw, sceneSettings.cameraPitch,
			sceneSettings.cameraMovementSpeed,
			sceneSettings.cameraMouseSensitivity);
		CreatePlayerGameObject();
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
		lastFrameTime = (float)glfwGetTime();
		float lastFrameReportTime = lastFrameTime;
		
		while (!glfwWindowShouldClose(window)) {
			float currentFrameTime = (float)glfwGetTime();
			float deltaTime = currentFrameTime - lastFrameTime;
			lastFrameTime = currentFrameTime;
			HelloTriangleApplication::ProcessInput(window, deltaTime);
			
			uint32_t imageIndex;
			if (CanAcquireNextPresentableImageIndex(imageIndex)) {
				UpdateGameState(currentFrameTime, deltaTime, imageIndex);
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
	
	void UpdateGameState(float time, float deltaTime, uint32_t imageIndex) {
		mainGameScene->Update(time, deltaTime, imageIndex,
			HelloTriangleApplication::mainCamera->ConstructViewMatrix(),
			graphicsEngine->GetSwapChainManager()->GetSwapChainExtent());

		// TODO: make this event driven to force decoupling
		auto& gameObjects = mainGameScene->GetGameObjects();
		std::vector<std::shared_ptr<GameObject>> gameObjectsToInit;
		for (auto& gameObject : gameObjects)
		{
			if (!gameObject->GetInitializedInEngine())
			{
				gameObjectsToInit.push_back(gameObject);
			}
		}
		if (gameObjectsToInit.size() > 0)
		{
			graphicsEngine->RecordCommandsForNewGameObjects(gfxDeviceManager,
				resourceLoader, inFlightFences, gameObjectsToInit, gameObjects);
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
		
		// delete game obejcts before main game scene is removed
		delete mainGameScene;

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

std::shared_ptr<Camera> HelloTriangleApplication::mainCamera = 
	std::make_shared<Camera>(glm::vec3(0.0f, 2.0f, 100.0f),
	-90.0f, 0.0f, 14.5f, 0.035f);
bool HelloTriangleApplication::firstMouse = false;
float HelloTriangleApplication::lastX = 0.0f;
float HelloTriangleApplication::lastY = 0.0f;
float HelloTriangleApplication::lastFrameTime = 0.0f;

void HelloTriangleApplication::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
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
	
	HelloTriangleApplication::mainCamera->ProcessMouse(xoffset, yoffset);
}

void HelloTriangleApplication::ProcessInput(GLFWwindow* window, float frameTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera->MoveForward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera->MoveBackward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera->MoveLeft(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		HelloTriangleApplication::mainCamera->MoveRight(frameTime);
	}
}

int main() {
	// in case we use rand anywhere, set up seed here
	srand(time(NULL));

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
