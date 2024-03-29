#pragma once

#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <vector>
#include <set>
#include "vulkan/vulkan.h"
#include <functional>
#include "SceneManagement/SceneLoader.h"
#include "SceneManagement/Scene.h"

class Scene;
class GraphicsEngine;
class GfxDeviceManager;
class LogicalDeviceManager;
class ResourceLoader;
class Camera;
class MenuObject;
class Model;
class Material;

class GameEngine {
public:
	enum class GameMode : char { Menu = 0, Game };
	enum class Difficulty : char { Easy = 0, Medium, Hard };

	enum class MenuPart : char { Base = 0, Difficulty, About };

	GameEngine(GameMode currentGameMode, GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const & logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
		VkCommandPool commandPool, VkCommandPoolCreateInfo poolInfo);
	~GameEngine();

	void CreateMenuObjects(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	void UpdateGameMode(GameMode newGameMode);
	void RecreateGraphicsEngineForNewSwapchain(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkSurfaceKHR surface, GLFWwindow* window,
		VkCommandPool commandPool,
		VkCommandPoolCreateInfo poolCreateInfo);

	void UpdateFrame(float time, float deltaTime, uint32_t imageIndex,
		GfxDeviceManager* gfxDeviceManager, ResourceLoader* resourceLoader,
		std::vector<VkFence> const & inFlightFences);

	GraphicsEngine* GetGraphicsEngine() {
		return graphicsEngine;
	}

	void SpawnGameObject(Scene::SpawnType spawnType,
		glm::vec3 const& spawnPosition,
		glm::vec3 const& forwardDir) {
		mainGameScene->SpawnGameObject(
			Scene::SpawnType::Bullet, spawnPosition,
			forwardDir);
	}

	void ProcessMouse(float xpos, float ypos,
		float xoffset, float yoffset);
	void ProcessInput(GLFWwindow* window,
		float frameTime, float lastFrameTime);
	void ProcessKeyCallback(GLFWwindow* window, int key,
		int scancode, int action, int mods);

private:
	GameMode currentGameMode;
	std::shared_ptr<Camera> mainCamera;
	Scene* mainGameScene;
	GraphicsEngine* graphicsEngine;

	float lastFireTime;
	float fireInterval;

	std::unordered_map<MenuPart, std::vector<std::shared_ptr<MenuObject>>> menuObjects;
	std::shared_ptr<GameObject> difficultySelector;

	int currentSelectedMenuObject;
	MenuPart currentMenuPart;
	int oldMouseState;
	std::vector<std::shared_ptr<GameObject>> startingGameObjects;
	std::shared_ptr<class TextureCreator> fontTextureSheet;
	class FontTextureBuffer* fontTextureBuffer;
	Difficulty currentDifficulty;
	GLFWwindow* window;
	SceneLoader::SceneSettings sceneSettings;
	float mouseXPos, mouseYPos;

	static constexpr int numMenus = 3;
	static inline const std::string playMenuOptionText = "Play";
	static inline const std::string aboutMenuOptionText = "About";
	static inline const std::string difficultyMenuOptionText = "Difficulty";
	static inline const std::string quitMenuOptionText = "Quit";
	static inline const std::string easyMenuOptionText = "Easy";
	static inline const std::string mediumMenuOptionText = "Medium";
	static inline const std::string hardMenuOptionText = "Hard";
	static inline const std::string backButtonText = "Back";
	static inline const glm::vec3 textOrigin = glm::vec3(0.0f, 0.0f, 0.0f);
	static inline const glm::vec3 cameraMenuPos = glm::vec3(0.0f, 0.0f, 80.0f);
	static const bool mobileCamera = true;
	static const bool staticView = false;
	// TODO: use somehow
	static const bool allowVerticalMovement = false;

	SceneLoader::SceneSettings CreateSceneAndReturnSettings(
		GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool,
		VkCommandPoolCreateInfo poolCreateInfo,
		VkSurfaceKHR surface, GLFWwindow* window);
	void CreatePlayerGameObject(GfxDeviceManager* gfxDeviceManager,
		std::shared_ptr<LogicalDeviceManager> const& logicalDeviceManager,
		ResourceLoader* resourceLoader, VkCommandPool commandPool);

	void HandleMainMenuControls(GLFWwindow* window, int key,
		int scancode, int action, int mods);
	void ActivateButtonInCurrentMenu();
	void AddMenuItems(MenuPart menuPart);
	void RemoveMenuItems(MenuPart menuPart);
	void ActivateButtonInInMainMenu();
	void ActivateButtonInInDifficultyMenu();
	void PositionDifficultySelector();
	void ActivateButtonInAboutMenu();
	void SetMenuSelectionIndices(MenuPart newMenuPart, int menuItemIndex);
	void SelectNextMenuObject(bool moveUp);

	void HandleMainGameControls(GLFWwindow* window, float frameTime, float lastFrameTime);
	void FireMainCannon(float latestFrameTime);

	void GetCurrentMouseWorldCoordAndDir(glm::vec3& mouseCoords,
		glm::vec3& direction);
};
