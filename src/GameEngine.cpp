
#include "GameEngine.h"
#include <stdexcept>
#include "GraphicsEngine.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Camera.h"
#include "SwapChainManager.h"
#include "SceneManagement/SceneLoader.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Player/PlayerGameObjectBehavior.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/FontObjects/MenuObject.h"
#include "GameObjects/FontObjects/FontTextureBuffer.h"
#include "Resources/TextureCreator.h"

GameEngine::GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool) {
	SceneLoader::SceneSettings sceneSettings =
		CreateSceneAndReturnSettings(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool, surface, window);

	mainCamera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 100.0f),
		0.0f, 0.0f, 14.5f, 0.035f);
	mainCamera->InitializeCameraSystem(sceneSettings.cameraPosition,
		sceneSettings.cameraYaw, sceneSettings.cameraPitch,
		sceneSettings.cameraMovementSpeed,
		sceneSettings.cameraMouseSensitivity);

	CreatePlayerGameObject(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool);
	auto& mainGameObjects = mainGameScene->GetGameObjects();
	for (auto& gameObject : mainGameObjects) {
		normalGameObjects.push_back(gameObject);
	}
	CreateMenuObjects(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool);

	UpdateGameMode(currentGameMode);

	lastFireTime = -1.0f;
	fireInterval = 0.5f;
}

GameEngine::~GameEngine() {
	delete graphicsEngine;
	// delete game objects before destroying vulkan instance

	delete mainGameScene;
}

void GameEngine::CreateMenuObjects(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	fontTextureBuffer = new FontTextureBuffer(
		std::string("oxanium/Oxanium-Medium.ttf"));
	if (fontTextureBuffer->GetBuffer() == nullptr) {
		throw std::runtime_error("Could not create texture buffer!");
	}
	std::string textureSheetName = "mainMenuTextureSheet";
	fontTextureSheet = resourceLoader->BuildRawTexture(textureSheetName,
		fontTextureBuffer->GetBuffer(), fontTextureBuffer->GetTextureWidth(),
		fontTextureBuffer->GetTextureHeight(), fontTextureBuffer->GetBytesPerPixel(),
		gfxDeviceManager, logicalDeviceManager, commandPool);

	menuMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::Text,
		textureSheetName, true, resourceLoader, gfxDeviceManager,
		logicalDeviceManager, commandPool);
	glm::vec3 characterScale = glm::vec3(0.5f, 0.5f, 0.5f);
	menuObjects.push_back(std::make_shared<MenuObject>("Play",
		glm::vec3(0.0f, 20.0f, 80.0f), characterScale, true,
		fontTextureBuffer, menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects.push_back(std::make_shared<MenuObject>("About",
		glm::vec3(0.0f, 5.0f, 80.0f), characterScale, true,
		fontTextureBuffer, menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects.push_back(std::make_shared<MenuObject>("Difficulty",
		glm::vec3(0.0f, -10.0, 80.0f), characterScale, true,
		fontTextureBuffer, menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	/*menuObjects.push_back(std::make_shared<MenuObject>("Easy",
		fontTextureBuffer,
		menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects.push_back(std::make_shared<MenuObject>("Medium",
		fontTextureBuffer,
		menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects.push_back(std::make_shared<MenuObject>("Hard",
		fontTextureBuffer,
		menuMaterial, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));*/

	menuObjects[0]->SetSelectState(true);
	currentSelectedMenuObject = 0;
}

void GameEngine::UpdateGameMode(GameMode newGameMode) {
	currentGameMode = newGameMode;
	if (currentGameMode == GameMode::Menu) {
		for (auto gameObject : normalGameObjects) {
			gameObject->SetMarkedForDeletion(true);
		}

		for (auto menuObject : menuObjects) {
			auto textGameObjects = menuObject->GetTextGameObjects();
			for (auto textGameObject : textGameObjects) {
				mainGameScene->AddGameObject(textGameObject);
			}
		}
	}
	else {
		for (auto menuObject : menuObjects) {
			auto textGameObjects = menuObject->GetTextGameObjects();
			for (auto textGameObject : textGameObjects) {
				textGameObject->SetMarkedForDeletion(false);
			}
		}

		for (auto gameObject : normalGameObjects) {
			mainGameScene->AddGameObject(gameObject);
		}
	}
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
			"texture.jpg", false, resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	std::shared_ptr<Model> gameObjectModel = GameObjectCreator::LoadModelFromName(
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

void GameEngine::ProcessMouse(float xoffset, float yoffset) {
	// can't move around during menu mode
	if (currentGameMode == GameMode::Menu) {
		return;
	}
	mainCamera->ProcessMouse(xoffset, yoffset);
}

void GameEngine::ProcessInput(GLFWwindow* window, float frameTime, float latestFrameTime) {
	// can't move around during menu mode
	if (currentGameMode == GameMode::Menu) {
		return;
	}

	HandleMainGameControls(window, frameTime, latestFrameTime);
}

void GameEngine::ProcessKeyCallback(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	if (currentGameMode == GameMode::Game) {
		return;
	}
	HandleMainMenuControls(window, key, scancode, action, mods);
}

void GameEngine::HandleMainMenuControls(GLFWwindow* window, int key,
	int scancode, int action, int mods) {
	bool upPressed = action == GLFW_PRESS &&
		(glfwGetKey(window, GLFW_KEY_UP)||
		glfwGetKey(window, GLFW_KEY_W));
	bool downPressed = action == GLFW_PRESS &&
		(glfwGetKey(window, GLFW_KEY_DOWN) ||
		glfwGetKey(window, GLFW_KEY_S));
	if (upPressed) {
		SelectNextMenuObject(false);
	}
	else if (downPressed) {
		SelectNextMenuObject(true);
	}
}

void GameEngine::SelectNextMenuObject(bool moveToNextElement) {
	menuObjects[currentSelectedMenuObject]->SetSelectState(false);

	currentSelectedMenuObject = moveToNextElement ?
		currentSelectedMenuObject + 1 :
		currentSelectedMenuObject - 1;
	if (currentSelectedMenuObject < 0) {
		currentSelectedMenuObject = 0;
	}
	currentSelectedMenuObject %= menuObjects.size();

	menuObjects[currentSelectedMenuObject]->SetSelectState(true);
}

void GameEngine::HandleMainGameControls(GLFWwindow* window, float frameTime, float latestFrameTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		mainCamera->MoveForward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		mainCamera->MoveBackward(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		mainCamera->MoveLeft(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		mainCamera->MoveRight(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		FireMainCannon(latestFrameTime);
	}
}

void GameEngine::FireMainCannon(float latestFrameTime) {
	if (latestFrameTime > (lastFireTime + fireInterval)) {
		lastFireTime = latestFrameTime;
		SpawnGameObject(Scene::SpawnType::Bullet, mainCamera->GetWorldPosition(),
			mainCamera->GetForwardDirection());
	}
}
