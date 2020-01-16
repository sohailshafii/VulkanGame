#pragma once

// based on https://learnopengl.com/Getting-started/Camera

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	Camera(const glm::vec3& position,
		const glm::vec3& forwardVector, const glm::vec3& upVector,
		float yaw, float pitch, float movementSpeed,
		float mouseSensitivity);

	glm::mat4 constructViewMatrix() const;

	void moveForward(float deltaTime);
	void moveBackward(float deltaTime);
	void moveRight(float deltaTime);
	void moveLeft(float deltaTime);

	void processMouse(float mouseXMovement, float mouseYMovement);

private:
	void updateCoordinateSystem();

	glm::vec3 position;
	glm::vec3 forward, up, right;
	glm::vec3 worldUp;
	float yaw, pitch;
	float movementSpeed;
	float mouseSensitivity;
};
