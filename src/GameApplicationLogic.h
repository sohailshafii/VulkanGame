#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "vulkan/vulkan.h"
#include <memory>
#include <vector>

class Scene;
class Camera;
class GameObject;
class VulkanInstance;
class GfxDeviceManager;
class LogicalDeviceManager;
class GraphicsEngine;
class ResourceLoader;

class GameApplicationLogic {
public:
	void Run();

	static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void ProcessInput(GLFWwindow* window, float frameTime);

private:
	GLFWwindow* window;
	const int WIDTH = 1366;
	const int HEIGHT = 768;
	const int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VulkanInstance* instance;
	GfxDeviceManager* gfxDeviceManager;
	std::shared_ptr<LogicalDeviceManager> logicalDeviceManager;

	VkSurfaceKHR surface;

	// TODO: have central place for command pool
	VkCommandPool commandPool;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	size_t currentFrame = 0;

	bool framebufferResized = false;

	ResourceLoader* resourceLoader;
	// this has to be static because we feed
	// camera inputs to it via static functions
	static class GameEngine* gameEngine;

	// need to be static for cursor callback function to work
	static bool firstMouse;
	static float lastX, lastY;
	static float lastFrameTime;

	static float lastFireTime;
	static float fireInterval;

	void InitWindow();
	static void FramebufferResizeCallback(GLFWwindow* window, int width,
		int height);

	void CreateInstance();
	void PickPhysicalDevice();
	void InitVulkan();

	void CreateSurface();
	void CreateLogicalDevice();

	void RecreateSwapChain();
	void CreateCommandPool();
	void CreateSyncObjects();
	void MainLoop();
	static void FireMainCannon();

	bool CanAcquireNextPresentableImageIndex(uint32_t& imageIndex);
	void UpdateGameState(float time, float deltaTime, uint32_t imageIndex);
	void DrawFrame(uint32_t imageIndex);
	void CleanUp();
};