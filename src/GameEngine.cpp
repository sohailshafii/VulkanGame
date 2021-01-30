
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
	currentDifficulty = Difficulty::Easy;
}

GameEngine::~GameEngine() {
	// delete game objects before destroying vulkan instance
	delete[] menuObjects;

	delete graphicsEngine;

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

	glm::vec3 characterScale = glm::vec3(0.4f, 0.4f, 0.4f);
	glm::vec3 characterScaleDifficulty = glm::vec3(0.3f, 0.3f, 0.3f);
	glm::vec3 characterScaleReduced = glm::vec3(0.25f, 0.25f, 0.25f);
	glm::vec3 characterScaleParagraph = glm::vec3(0.1f, 0.1f, 0.1f);
	// assemble menus here. only three of them
	menuObjects = new std::vector<std::shared_ptr<MenuObject>>[numMenus];
	menuObjects[0].push_back(std::make_shared<MenuObject>(playMenuOptionText,
		glm::vec3(0.0f, 20.0f, 80.0f), characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[0].push_back(std::make_shared<MenuObject>(difficultyMenuOptionText,
		glm::vec3(0.0f, 5.0f, 80.0f), characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[0].push_back(std::make_shared<MenuObject>(aboutMenuOptionText,
		glm::vec3(0.0f, -10.0, 80.0f), characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	menuObjects[1].push_back(std::make_shared<MenuObject>(easyMenuOptionText,
		glm::vec3(0.0f, 15.0f, 80.0f), characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[1].push_back(std::make_shared<MenuObject>(mediumMenuOptionText,
		glm::vec3(0.0f, 5.0f, 80.0f), characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[1].push_back(std::make_shared<MenuObject>(hardMenuOptionText,
		glm::vec3(0.0f, -5.0, 80.0f), characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[1].push_back(std::make_shared<MenuObject>(backButtonText,
		glm::vec3(0.0f, -20.0, 80.0f), characterScaleReduced, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	const char* gameInfo =
		"This is a simple game developed using C++ and the Vulkan API,\n"
		"written by Sohail Shafii. Use WASD to move left-right-up-down,\n"
		"and space bar to shoot at mothership trying to kill you. I'm too\n"
		"lazy to write any more instructions.";
	menuObjects[2].push_back(std::make_shared<MenuObject>(
		gameInfo,
		glm::vec3(0.0f, 20.0f, 80.0f), characterScaleParagraph, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[2].push_back(std::make_shared<MenuObject>(backButtonText,
		glm::vec3(0.0f, -10.0, 80.0f), characterScaleReduced, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	SetMenuSelectionIndices(0, 0);
}

void GameEngine::UpdateGameMode(GameMode newGameMode) {
	currentGameMode = newGameMode;
	if (currentGameMode == GameMode::Menu) {
		for (auto gameObject : normalGameObjects) {
			gameObject->SetMarkedForDeletionInScene(true);
		}
		AddMenuItems(0);
	}
	else {
		RemoveMenuItems(currentSubMenuIndex);

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
	bool pressAction = action == GLFW_PRESS;
	bool upPressed = pressAction &&
		(glfwGetKey(window, GLFW_KEY_UP)||
		glfwGetKey(window, GLFW_KEY_W));
	bool downPressed = pressAction &&
		(glfwGetKey(window, GLFW_KEY_DOWN) ||
		glfwGetKey(window, GLFW_KEY_S));
	bool enterPressed = pressAction &&
		glfwGetKey(window, GLFW_KEY_ENTER);

	if (enterPressed) {
		ActivateButtonInCurrentMenu();
	}
	else if (upPressed) {
		SelectNextMenuObject(false);
	}
	else if (downPressed) {
		SelectNextMenuObject(true);
	}
}

void GameEngine::ActivateButtonInCurrentMenu() {
	if (currentSubMenuIndex == 0) {
		ActivateButtonInInMainMenu();
	}
	else if (currentSubMenuIndex == 1) {
		ActivateButtonInInDifficultyMenu();
	}
	else {
		ActivateButtonInAboutMenu();
	}
}

void GameEngine::ActivateButtonInInMainMenu() {
	auto currentMenuObject =
		menuObjects[currentSubMenuIndex][currentSelectedMenuObject];
	auto currentMenuObjectText = currentMenuObject->GetText();
	
	if (currentMenuObjectText == playMenuOptionText) {
		UpdateGameMode(GameEngine::GameMode::Game);
	}
	else if (currentMenuObjectText == difficultyMenuOptionText) {
		AddMenuItems(1);
		SetMenuSelectionIndices(1, 0);

		RemoveMenuItems(0);
	}
	else if (currentMenuObjectText == aboutMenuOptionText) {
		AddMenuItems(2);
		SetMenuSelectionIndices(2, 0);

		RemoveMenuItems(0);
	}
}

void GameEngine::AddMenuItems(int menuIndex) {
	auto currenMenu = menuObjects[menuIndex];
	for (auto menuItem : currenMenu) {
		auto textGameObjects = menuItem->GetTextGameObjects();
		for (auto textGameObject : textGameObjects) {
			textGameObject->SetInitializedInEngine(false);
			mainGameScene->AddGameObject(textGameObject);
		}
	}
}

void GameEngine::RemoveMenuItems(int menuIndex) {
	auto currenMenu = menuObjects[menuIndex];
	for (auto menuItem : currenMenu) {
		auto textGameObjects = menuItem->GetTextGameObjects();
		for (auto textGameObject : textGameObjects) {
			textGameObject->SetMarkedForDeletionInScene(true);
		}
	}
}

void GameEngine::ActivateButtonInInDifficultyMenu() {
	auto currentMenuObject =
		menuObjects[currentSubMenuIndex][currentSelectedMenuObject];
	auto currentMenuObjectText = currentMenuObject->GetText();
	if (currentMenuObjectText == easyMenuOptionText) {
		currentDifficulty = Difficulty::Easy;
	}
	else if (currentMenuObjectText == mediumMenuOptionText) {
		currentDifficulty = Difficulty::Medium;
	}
	else if (currentMenuObjectText == hardMenuOptionText) {
		currentDifficulty = Difficulty::Hard;
	}
	else if (currentMenuObjectText == backButtonText) {
		AddMenuItems(0);
		RemoveMenuItems(1);
		SetMenuSelectionIndices(0, 0);
	}
}

void GameEngine::ActivateButtonInAboutMenu() {
	auto currentMenuObject =
		menuObjects[currentSubMenuIndex][currentSelectedMenuObject];
	auto currentMenuObjectText = currentMenuObject->GetText();
	if (currentMenuObjectText != backButtonText) {
		return;
	}
	AddMenuItems(0);
	RemoveMenuItems(2);
	SetMenuSelectionIndices(0, 0);
}

void GameEngine::SetMenuSelectionIndices(int subIndex, int menuItemIndex) {
	currentSubMenuIndex = subIndex;
	currentSelectedMenuObject = menuItemIndex;
	auto currentSubMenu = menuObjects[currentSubMenuIndex];
	auto currentSelectedMenuObj = currentSubMenu[currentSelectedMenuObject];
	for (auto menuObject : currentSubMenu) {
		menuObject->SetSelectState(currentSelectedMenuObj == menuObject);
	}
	
}

void GameEngine::SelectNextMenuObject(bool moveToNextElement) {
	menuObjects[currentSubMenuIndex][currentSelectedMenuObject]->SetSelectState(false);

	currentSelectedMenuObject = moveToNextElement ?
		currentSelectedMenuObject + 1 :
		currentSelectedMenuObject - 1;
	if (currentSelectedMenuObject < 0) {
		currentSelectedMenuObject = menuObjects[currentSubMenuIndex].size()-1;
	}
	currentSelectedMenuObject %= menuObjects[currentSubMenuIndex].size();

	menuObjects[currentSubMenuIndex][currentSelectedMenuObject]->SetSelectState(true);
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
