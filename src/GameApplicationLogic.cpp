#include "GameApplicationLogic.h"
#include <string>
#include "VulkanInstance.h"
#include "GfxDeviceManager.h"
#include "LogicalDeviceManager.h"
#include "GraphicsEngine.h"
#include "ResourceLoader.h"
#include "SceneManagement/Scene.h"
#include "Camera.h"
#include "GameObjects/GameObject.h"

#if __APPLE__
const std::string MODEL_PATH = "../../models/chalet.obj";
const std::string CUBE_MODEL_PATH = "../../models/cube.obj";
const std::string TEXTURE_PATH = "../../textures/chalet.jpg";
#else
const std::string MODEL_PATH = "../models/chalet.obj";
const std::string CUBE_MODEL_PATH = "../models/cube.obj";
const std::string TEXTURE_PATH = "../textures/chalet.jpg";
#endif

std::shared_ptr<Camera> GameApplicationLogic::mainCamera =
std::make_shared<Camera>(glm::vec3(0.0f, 2.0f, 100.0f),
	-90.0f, 0.0f, 14.5f, 0.035f);
Scene* GameApplicationLogic::mainGameScene = nullptr;
bool GameApplicationLogic::firstMouse = false;
float GameApplicationLogic::lastX = 0.0f;
float GameApplicationLogic::lastY = 0.0f;
float GameApplicationLogic::lastFrameTime = 0.0f;
float GameApplicationLogic::lastFireTime = -1.0f;
float GameApplicationLogic::fireInterval = 0.5f;

void GameApplicationLogic::Run() {
	// TODO
}

void GameApplicationLogic::MouseCallback(GLFWwindow* window,
	double xpos, double ypos) {
	// TODO
}

void GameApplicationLogic::ProcessInput(GLFWwindow* window,
	float frameTime) {
	// TODO
}

void GameApplicationLogic::InitWindow() {
	// TODO
}

void GameApplicationLogic::FramebufferResizeCallback(GLFWwindow* window,
	int width, int height) {
	// TODO
}

void GameApplicationLogic::CreateInstance() {
	// TODO
}

void GameApplicationLogic::PickPhysicalDevice() {
	// TODO
}

void GameApplicationLogic::InitVulkan() {
	// TODO
}

void GameApplicationLogic::CreatePlayerGameObject() {
	// TODO
}

void GameApplicationLogic::RemoveGameObjects(
	std::vector<GameObject*>& gameObjectsToRemove) {
	// TODO
}

void GameApplicationLogic::RemoveGameObjects(
	std::vector<std::shared_ptr<GameObject>> & gameObjectsToRemove) {
	// TODO
}

void GameApplicationLogic::CreateSurface() {
	// TODO
}

void GameApplicationLogic::CreateLogicalDevice() {
	// TODO
}

void GameApplicationLogic::RecreateSwapChain() {
	// TODO
}

void GameApplicationLogic::CreateCommandPool() {
	// TODO
}

void GameApplicationLogic::CreateGameObjects() {
	// TODO
}

void GameApplicationLogic::CreateSyncObjects() {
	// TODO
}

void GameApplicationLogic::MainLoop() {
	// TODO
}

void GameApplicationLogic::FireMainCannon() {
	// TODO
}

bool GameApplicationLogic::CanAcquireNextPresentableImageIndex(
	uint32_t& imageIndex) {
	return false; // TODO
}

void GameApplicationLogic::UpdateGameState(float time,
	float deltaTime, uint32_t imageIndex) {
	// TODO
}

void GameApplicationLogic::DrawFrame(uint32_t imageIndex) {
	// TODO
}

void GameApplicationLogic::CleanUp() {
	// TODO
}

