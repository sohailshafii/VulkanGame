
#include "GameEngine.h"
#include "GraphicsEngine.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "SceneManagement/Scene.h"

GameEngine::GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool) {

	this->currentGameMode = currentGameMode;
	CreateSceneAndGameObjects();

	graphicsEngine = new GraphicsEngine(gfxDeviceManager,
		logicalDeviceManager, resourceLoader, surface, window,
		commandPool, mainGameScene->GetGameObjects());
}

GameEngine::~GameEngine() {
	delete graphicsEngine;
	delete mainGameScene;
}

void GameEngine::UpdateGameMode(GameMode newGameMode) {
	currentGameMode = newGameMode;
	// TODO: switch pipelines
}

void GameEngine::CreateSceneAndGameObjects() {

}
