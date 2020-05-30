#include "MothershipBehavior.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

void MothershipBehavior::UpdateSelf(float time, float deltaTime) {
	modelMatrix = glm::rotate(modelMatrix, 0.8f*deltaTime, axisOfRotation);
}