
#include "GameEngine.h"
#include "GraphicsEngine.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Camera.h"
#include "SwapChainManager.h"
#include "SceneManagement/SceneLoader.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/PlayerGameObjectBehavior.h"

GameEngine::GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool) {
	this->currentGameMode = currentGameMode;

	SceneLoader::SceneSettings sceneSettings =
		CreateSceneAndReturnSettings(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool, surface, window);

	mainCamera = std::make_shared<Camera>(glm::vec3(0.0f, 2.0f, 100.0f),
		-90.0f, 0.0f, 14.5f, 0.035f);
	mainCamera->InitializeCameraSystem(sceneSettings.cameraPosition,
		sceneSettings.cameraYaw, sceneSettings.cameraPitch,
		sceneSettings.cameraMovementSpeed,
		sceneSettings.cameraMouseSensitivity);

	CreatePlayerGameObject(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool);
}

GameEngine::~GameEngine() {
	delete graphicsEngine;
	// delete game objects before destroying vulkan instance

	delete mainGameScene;
}

void GameEngine::UpdateGameMode(GameMode newGameMode) {
	currentGameMode = newGameMode;
	// TODO: switch pipelines
}

void GameEngine::RecreateGraphicsEngineForNewSwapchain(
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool) {
	delete graphicsEngine;
	graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, surface, window, commandPool, mainGameScene->GetGameObjects());
}

void GameEngine::UpdateFrame(float time, float deltaTime, uint32_t imageIndex,
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const& inFlightFences) {
	mainGameScene->Update(time, deltaTime, imageIndex,
		mainCamera->ConstructViewMatrix(),
		graphicsEngine->GetSwapChainManager()->GetSwapChainExtent());

	// TODO: make this event driven to force decoupling
	// TODO: scene with menu objects
	auto& gameObjects = mainGameScene->GetGameObjects();
	std::vector<std::shared_ptr<GameObject>> gameObjectsToInit;
	std::vector<std::shared_ptr<GameObject>> gameObjectsToRemove;
	for (auto& gameObject : gameObjects) {
		if (!gameObject->GetInitializedInEngine()) {
			gameObjectsToInit.push_back(gameObject);
		}
		else if (gameObject->GetMarkedForDeletion()) {
			gameObjectsToRemove.push_back(gameObject);
		}
	}

	if (gameObjectsToInit.size() > 0) {
		graphicsEngine->RecordCommandsForNewGameObjects(gfxDeviceManager,
			resourceLoader, inFlightFences, gameObjectsToInit, gameObjects);
	}

	if (gameObjectsToRemove.size() > 0) {
		RemoveGameObjects(gameObjectsToRemove, inFlightFences);
	}
}

SceneLoader::SceneSettings GameEngine::CreateSceneAndReturnSettings(
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool,
	VkSurfaceKHR surface, GLFWwindow* window) {
#if __APPLE__
	std::string scenePath = "../../mainGameScene.json";
#else
	std::string scenePath = "../mainGameScene.json";
#endif
	mainGameScene = new Scene(resourceLoader,
		gfxDeviceManager, logicalDeviceManager, commandPool);
	SceneLoader::SceneSettings sceneSettings;

	SceneLoader::DeserializeJSONFileIntoScene(
		resourceLoader, gfxDeviceManager, logicalDeviceManager,
		commandPool, mainGameScene, sceneSettings, scenePath);

	graphicsEngine = new GraphicsEngine(gfxDeviceManager,
		logicalDeviceManager, resourceLoader, surface, window,
		commandPool, mainGameScene->GetGameObjects());

	return sceneSettings;
}

void GameEngine::CreatePlayerGameObject(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
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

void GameEngine::RemoveGameObjects(
	std::vector<GameObject*>& gameObjectsToRemove,
	std::vector<VkFence> const& inFlightFences) {
	mainGameScene->RemoveGameObjects(gameObjectsToRemove);
	auto& allGameObjects = mainGameScene->GetGameObjects();
	graphicsEngine->RemoveGameObjectsAndRecordCommands(
		inFlightFences, gameObjectsToRemove,
		allGameObjects);
}

void GameEngine::RemoveGameObjects(
	std::vector<std::shared_ptr<GameObject>>& gameObjectsToRemove,
	std::vector<VkFence> const& inFlightFences) {
	mainGameScene->RemoveGameObjects(gameObjectsToRemove);
	auto& allGameObjects = mainGameScene->GetGameObjects();
	graphicsEngine->RemoveGameObjectsAndRecordCommands(
		inFlightFences, gameObjectsToRemove,
		allGameObjects);
}