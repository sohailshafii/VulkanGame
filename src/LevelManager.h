#pragma once

// Responsible for loading either the main menu
// or the level(s) of the game.
class LevelManager {
public:
	LevelManager();
	~LevelManager();

	void UpdateFrame();

private:
	enum GameMode { MainMenu = 0, Level };

	GameMode currentGameMode;
};
