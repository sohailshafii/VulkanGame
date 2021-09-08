
#include "GameEngine.h"
#include <stdexcept>
#include "GraphicsEngine.h"
#include "LogicalDeviceManager.h"
#include "ResourceLoader.h"
#include "Camera.h"
#include "SwapChainManager.h"
#include "SceneManagement/SceneLoader.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/MeshGameObject.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Player/PlayerGameObjectBehavior.h"
#include "GameObjects/Msc/StationaryGameObjectBehavior.h"
#include "GameObjects/FontObjects/MenuObject.h"
#include "GameObjects/FontObjects/FontTextureBuffer.h"
#include "GameObjects/FontObjects/MenuObject.h"
#include "GameObjects/FontObjects/MenuSelectorObjectBehavior.h"
#include "GameObjects/GameObjectCreationUtilFuncs.h"
#include "GameObjects/Mothership/MothershipBehavior.h"
#include "Resources/TextureCreator.h"
#include "Common.h"
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <iostream>
#include "nlohmann/json.hpp"

GameEngine::GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool, VkCommandPoolCreateInfo poolInfo) {
	sceneSettings =
		CreateSceneAndReturnSettings(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool, poolInfo, surface, window);
	this->window = window;
	mainCamera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 100.0f),
		0.0f, 0.0f, 0.5f, 0.035f);
	mainCamera->InitializeCameraSystem(sceneSettings.cameraPosition,
		sceneSettings.cameraYaw, sceneSettings.cameraPitch,
		sceneSettings.cameraMovementSpeed,
		sceneSettings.cameraMouseSensitivity);

	CreatePlayerGameObject(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool);
	auto& mainGameObjects = mainGameScene->GetGameObjects();
	for (auto& gameObject : mainGameObjects) {
		startingGameObjects.push_back(gameObject);
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
	// should auto-delete map of menu object

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
	menuObjects[MenuPart::Base].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Play,
		playMenuOptionText, glm::vec3(0.0f, 20.0f, 0.0f) + textOrigin, characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Base].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Difficulty,
		difficultyMenuOptionText, glm::vec3(0.0f, 10.0f, 0.0f) + textOrigin, characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Base].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::About,
		aboutMenuOptionText, glm::vec3(0.0f, 0.0, 0.0f) + textOrigin, characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Base].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Quit,
		quitMenuOptionText, glm::vec3(0.0f, -9.0, 0.0f) + textOrigin, characterScale, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	menuObjects[MenuPart::Difficulty].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Easy,
		easyMenuOptionText, glm::vec3(0.0f, 15.0f, 0.0f) + textOrigin, characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Difficulty].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Medium,
		mediumMenuOptionText, glm::vec3(0.0f, 5.0f, 0.0f) + textOrigin, characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Difficulty].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Hard,
		hardMenuOptionText, glm::vec3(0.0f, -5.0, 0.0f) + textOrigin, characterScaleDifficulty, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::Difficulty].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Back,
		backButtonText, glm::vec3(0.0f, -20.0, 0.0f) + textOrigin, characterScaleReduced, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	std::shared_ptr<Model> selectorModel = Model::CreateQuad(
		glm::vec3(-0.5f,-0.5f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f), true);
	nlohmann::json metadataNode = {
		{"tint_color",{1.0f, 1.0f, 1.0f, 1.0f }}
	};
	auto selectorMaterial = GameObjectCreator::CreateMaterial(
		DescriptorSetFunctions::MaterialType::UnlitColor,
		metadataNode);
	auto selectorBehaviorObj = std::make_shared<MenuSelectorObjectBehavior>(
		mainGameScene, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	difficultySelector =
		GameObjectCreator::CreateMeshGameObject(
			selectorMaterial, selectorModel, selectorBehaviorObj,
			glm::mat4(1.0f), resourceLoader, gfxDeviceManager,
			logicalDeviceManager, commandPool);
	difficultySelector->SetLocalTransform(glm::mat4(1.0f));

	const char* gameInfo =
		"This is a simple game developed using C++ and the Vulkan API,\n"
		"written by Sohail Shafii. Use WASD to move left-right-up-down,\n"
		"and the space bar to shoot at the mothership trying to kill you.\n"
		"During game mode, press escape to go back to the menu. I'm too\n"
		"lazy to write any more instructions.";
	menuObjects[MenuPart::About].push_back(std::make_shared<MenuObject>(
		MenuObject::MenuType::Play, gameInfo,
		glm::vec3(0.0f, 20.0f, 0.0f) + textOrigin, characterScaleParagraph, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));
	menuObjects[MenuPart::About].push_back(std::make_shared<MenuObject>(MenuObject::MenuType::Back,
		backButtonText, glm::vec3(0.0f, -10.0, 0.0f) + textOrigin, characterScaleReduced, true,
		fontTextureBuffer, textureSheetName, gfxDeviceManager, logicalDeviceManager,
		resourceLoader, commandPool));

	SetMenuSelectionIndices(MenuPart::Base, 0);
}

void GameEngine::UpdateGameMode(GameMode newGameMode) {
	currentGameMode = newGameMode;
	mainCamera->SetPositionYawPitch(sceneSettings.cameraPosition,
		sceneSettings.cameraYaw, sceneSettings.cameraPitch);
	if (currentGameMode == GameMode::Menu) {
		mainCamera->SetPosition(cameraMenuPos);
		auto& mainGameObjects = mainGameScene->GetGameObjects();
		for (auto gameObject : mainGameObjects) {
			gameObject->SetMarkedForDeletionInScene(true);
		}
		AddMenuItems(MenuPart::Base);
	}
	else {
		RemoveMenuItems(currentMenuPart);
		mainCamera->InitializeCameraSystem(sceneSettings.cameraPosition,
			sceneSettings.cameraYaw, sceneSettings.cameraPitch,
			sceneSettings.cameraMovementSpeed,
			sceneSettings.cameraMouseSensitivity);
		// reset everything to what they were before
		for (auto gameObject : startingGameObjects) {
			mainGameScene->AddGameObject(gameObject);
			gameObject->SetInitializedInEngine(false);
			MothershipBehavior* mothershipBehavior =
				dynamic_cast<MothershipBehavior*>(gameObject->GetGameObjectBehavior());
			if (mothershipBehavior) {
				mothershipBehavior->Reboot();
			}
		}
	}
}

void GameEngine::RecreateGraphicsEngineForNewSwapchain(
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
	VkCommandPool commandPool,
	VkCommandPoolCreateInfo poolCreateInfo) {
	delete graphicsEngine;
	graphicsEngine = new GraphicsEngine(gfxDeviceManager, logicalDeviceManager,
		resourceLoader, surface, window, commandPool, poolCreateInfo,
		mainGameScene->GetGameObjects());
}

void GameEngine::UpdateFrame(float time, float deltaTime, uint32_t imageIndex,
	GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
	std::vector<VkFence> const& inFlightFences) {
	mainGameScene->Update(time, deltaTime, imageIndex,
		mainCamera->ConstructViewMatrix(),
		graphicsEngine->GetSwapChainManager()->GetSwapChainExtent());

	auto& gameObjects = mainGameScene->GetGameObjects();
	bool atLeastOneUnitializedGameObject = false;
	std::vector<std::shared_ptr<GameObject>> gameObjectsToRemove;
	for (auto& gameObject : gameObjects) {
		if (!gameObject->GetInitializedInEngine() &&
			!gameObject->IsInvisible()) {
			atLeastOneUnitializedGameObject = true;
		}
		else if (gameObject->GetMarkedForDeletion()) {
			gameObjectsToRemove.push_back(gameObject);
		}
	}

	bool removedGameObjects = gameObjectsToRemove.size() > 0;
	if (removedGameObjects) {
		mainGameScene->RemoveGameObjects(gameObjectsToRemove);
		if (atLeastOneUnitializedGameObject) {
			graphicsEngine->RemoveGameObjectsAndReRecordCommandsForAddedGameObjects(
				gfxDeviceManager, resourceLoader, inFlightFences, gameObjectsToRemove,
				gameObjects);
		}
		else {
			graphicsEngine->RemoveGameObjectsAndRecordCommands(
				inFlightFences, gameObjectsToRemove,
				gameObjects);
		}
	}
	else if (atLeastOneUnitializedGameObject) {
		graphicsEngine->ReRecordCommandsForGameObjects(gfxDeviceManager,
			resourceLoader, inFlightFences, gameObjects);
	}

	graphicsEngine->Update(inFlightFences);
}

SceneLoader::SceneSettings GameEngine::CreateSceneAndReturnSettings(
	GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool,
	VkCommandPoolCreateInfo poolCreateInfo,
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
		commandPool, poolCreateInfo, mainGameScene->GetGameObjects());

	return sceneSettings;
}

void GameEngine::CreatePlayerGameObject(GfxDeviceManager* gfxDeviceManager,
	std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
	ResourceLoader* resourceLoader, VkCommandPool commandPool) {
	// add player game object; this is necessary because enemies
	// need to know where the player is
	glm::mat4 localToWorldTransform = glm::translate(glm::mat4(1.0f),
		glm::vec3(0.0f, 0.0f, 4.0f));

	std::shared_ptr<GameObject> newGameObject =
		std::make_shared<GameObject>(
			std::make_unique<PlayerGameObjectBehavior>(mainCamera));
	newGameObject->SetLocalTransform(localToWorldTransform);

	mainGameScene->AddGameObject(newGameObject);
}

void GameEngine::ProcessMouse(float xpos, float ypos,
	float xoffset, float yoffset) {
	mouseXPos = xpos;
	mouseYPos = ypos;
	// can't move around during menu mode
	if (currentGameMode == GameMode::Menu ||
		staticView) {
		return;
	}
	mainCamera->ProcessMouse(xoffset, yoffset);
}

void GameEngine::ProcessInput(GLFWwindow* window, float frameTime, float lastFrameTime) {
	// can't move around during menu mode
	if (currentGameMode == GameMode::Menu) {
		return;
	}

	HandleMainGameControls(window, frameTime, lastFrameTime);
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
	if (currentMenuPart == MenuPart::Base) {
		ActivateButtonInInMainMenu();
	}
	else if (currentMenuPart == MenuPart::Difficulty) {
		ActivateButtonInInDifficultyMenu();
	}
	else {
		ActivateButtonInAboutMenu();
	}
}

void GameEngine::ActivateButtonInInMainMenu() {
	auto currentMenuObject =
		menuObjects[currentMenuPart][currentSelectedMenuObject];
	auto currentMenuType = currentMenuObject->GetMenuType();
	
	if (currentMenuType == MenuObject::MenuType::Play) {
		UpdateGameMode(GameEngine::GameMode::Game);
	}
	else if (currentMenuType == MenuObject::MenuType::Difficulty) {
		AddMenuItems(MenuPart::Difficulty);
		SetMenuSelectionIndices(MenuPart::Difficulty, 0);
		PositionDifficultySelector();

		RemoveMenuItems(MenuPart::Base);
	}
	else if (currentMenuType == MenuObject::MenuType::About) {
		AddMenuItems(MenuPart::About);
		SetMenuSelectionIndices(MenuPart::About, 0);

		RemoveMenuItems(MenuPart::Base);
	}
	else if (currentMenuType == MenuObject::MenuType::Quit) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void GameEngine::AddMenuItems(MenuPart menuPart) {
	auto currentMenu = menuObjects[menuPart];
	for (auto menuItem : currentMenu) {
		menuItem->SetInitializedInEngine(false);
		mainGameScene->AddGameObject(menuItem);
	}

	if (menuPart == MenuPart::Difficulty) {
		difficultySelector->SetInitializedInEngine(false);
		mainGameScene->AddGameObject(difficultySelector);
	}
}

void GameEngine::RemoveMenuItems(MenuPart menuPart) {
	auto currenMenu = menuObjects[menuPart];
	for (auto menuItem : currenMenu) {
		menuItem->SetMarkedForDeletionInScene(true);
	}

	if (menuPart == MenuPart::Difficulty) {
		difficultySelector->SetMarkedForDeletionInScene(true);
	}
}

void GameEngine::ActivateButtonInInDifficultyMenu() {
	auto currentMenuObject =
		menuObjects[currentMenuPart][currentSelectedMenuObject];
	auto currentMenuType = currentMenuObject->GetMenuType();

	bool movedDifficultySelector = false;
	if (currentMenuType == MenuObject::MenuType::Easy) {
		currentDifficulty = Difficulty::Easy;
		PositionDifficultySelector();
	}
	else if (currentMenuType == MenuObject::MenuType::Medium) {
		currentDifficulty = Difficulty::Medium;
		PositionDifficultySelector();
	}
	else if (currentMenuType == MenuObject::MenuType::Hard) {
		currentDifficulty = Difficulty::Hard;
		PositionDifficultySelector();
	}
	else if (currentMenuType == MenuObject::MenuType::Back) {
		AddMenuItems(MenuPart::Base);
		RemoveMenuItems(MenuPart::Difficulty);
		SetMenuSelectionIndices(MenuPart::Base, 0);
	}
}

void GameEngine::PositionDifficultySelector() {
	auto currentMenuObject = menuObjects[MenuPart::Difficulty][0];

	if (currentDifficulty == Difficulty::Easy) {
		currentMenuObject = menuObjects[MenuPart::Difficulty][0];
	}
	else if (currentDifficulty == Difficulty::Medium) {
		currentMenuObject = menuObjects[MenuPart::Difficulty][1];
	}
	else {
		currentMenuObject = menuObjects[MenuPart::Difficulty][2];
	}

	auto difficultyWorldPos = currentMenuObject->GetWorldPosition();
	auto menuObjectScale = currentMenuObject->GetScale();
	difficultyWorldPos[0] -= 2.0f;
	difficultyWorldPos[1] += 0.5f * menuObjectScale[1];
	auto selectorTransform = glm::translate(glm::mat4(1.0f), difficultyWorldPos);
	selectorTransform = glm::scale(selectorTransform, glm::vec3(2.0f));
	difficultySelector->SetLocalTransform(selectorTransform);
}

void GameEngine::ActivateButtonInAboutMenu() {
	auto currentMenuObject =
		menuObjects[currentMenuPart][currentSelectedMenuObject];
	auto currentMenuType = currentMenuObject->GetMenuType();

	if (currentMenuType != MenuObject::MenuType::Back) {
		return;
	}
	AddMenuItems(MenuPart::Base);
	RemoveMenuItems(MenuPart::About);
	SetMenuSelectionIndices(MenuPart::Base, 0);
}

void GameEngine::SetMenuSelectionIndices(MenuPart newMenuPart, int menuItemIndex) {
	currentMenuPart = newMenuPart;
	currentSelectedMenuObject = menuItemIndex;
	auto currentSubMenu = menuObjects[currentMenuPart];
	auto currentSelectedMenuObj = currentSubMenu[currentSelectedMenuObject];
	for (auto menuObject : currentSubMenu) {
		menuObject->SetSelectState(currentSelectedMenuObj == menuObject);
	}
}

void GameEngine::SelectNextMenuObject(bool moveToNextElement) {
	menuObjects[currentMenuPart][currentSelectedMenuObject]->SetSelectState(false);

	currentSelectedMenuObject = moveToNextElement ?
		currentSelectedMenuObject + 1 :
		currentSelectedMenuObject - 1;
	if (currentSelectedMenuObject < 0) {
		currentSelectedMenuObject = (int)menuObjects[currentMenuPart].size() - 1;
	}
	currentSelectedMenuObject %= menuObjects[currentMenuPart].size();

	menuObjects[currentMenuPart][currentSelectedMenuObject]->SetSelectState(true);
}

void GameEngine::HandleMainGameControls(GLFWwindow* window, float frameTime,
	float lastFrameTime) {
	if (mobileCamera) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			mainCamera->MoveForward();
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			mainCamera->MoveBackward();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			mainCamera->MoveLeft();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			mainCamera->MoveRight();
		}
	}

	int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
		(oldMouseState == GLFW_PRESS && mouseState == GLFW_RELEASE )) {
		FireMainCannon(frameTime);
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		UpdateGameMode(GameMode::Menu);
	}

	oldMouseState = mouseState;
}

void GameEngine::FireMainCannon(float latestFrameTime) {
	if (latestFrameTime > (lastFireTime + fireInterval)) {
		lastFireTime = latestFrameTime;
		SpawnGameObject(Scene::SpawnType::Bullet, mainCamera->GetWorldPosition(),
			mainCamera->GetForwardDirection());
	}
}

void GameEngine::GetCurrentMouseWorldCoordAndDir(glm::vec3& mouseCoords,
	glm::vec3& direction) {
	glm::vec4 mousePosWithDepth(mouseXPos, mouseYPos, 0.98f, 1.0f);
	// restrict to normalized space, but keep in mind that y increases downwards
	// in screen space
	// go from that to clip space, which is restrict to [-1, 1]
	VkExtent2D extent2D = graphicsEngine->GetSwapChainManager()->GetSwapChainExtent();
	mousePosWithDepth[0] = ((mouseXPos + 0.5f) / extent2D.width) * 2.0f - 1.0f;
	// y is flipped in projection matrix
	mousePosWithDepth[1] = 2.0f*((mouseYPos + 0.5f) / extent2D.height) - 1.0f;

	glm::mat4 viewMatrix = mainCamera->ConstructViewMatrix();
	glm::mat4 projectionMatrix = Common::ConstructProjectionMatrix(extent2D.width,
		extent2D.height);
	glm::mat4 projectionViewInv = glm::inverse(projectionMatrix * viewMatrix);
	glm::vec4 worldSpaceMouseCoords = projectionViewInv * mousePosWithDepth;

	float wInv = 1.0f / worldSpaceMouseCoords[3];
	mouseCoords = glm::vec3(worldSpaceMouseCoords[0] * wInv,
		worldSpaceMouseCoords[1] * wInv, worldSpaceMouseCoords[2] * wInv);
	// the further we are from the center in clip space, the more the direction extends
	// out toward the periphery of the mother ship
	float xTValue = mousePosWithDepth[0];
	float yTValue = mousePosWithDepth[1];
	// map to unit circle
	// https://www.xarg.org/2017/07/how-to-map-a-square-to-a-circle/
	float mappedXExtentMotherShip = xTValue * sqrt(1.0f - yTValue * yTValue * 0.5f);
	float mappedYExtentMotherShip = yTValue * sqrt(1.0f - xTValue * xTValue * 0.5f);
	// multiply by radius
	mappedXExtentMotherShip *= 100.0f;
	mappedYExtentMotherShip *=-100.0f;
	direction = glm::vec3(mappedXExtentMotherShip, mappedYExtentMotherShip,
		-100.0f) - mouseCoords;
	direction = glm::normalize(direction);
}
