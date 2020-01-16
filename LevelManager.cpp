
#include "LevelManager.h"

LevelManager::LevelManager() {
	currentGameMode = LevelManager::GameMode::Level;
}

LevelManager::~LevelManager() {

}

// TODO: need representation of game object, which would be:
// 1. Mesh data, along with material to drive engine
// 2. transform
// 3. behavior class?
// need to collect all materials to generate pipelines


void LevelManager::UpdateFrame() {

}
