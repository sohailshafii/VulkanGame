#pragma once

// based on https://learnopengl.com/Getting-started/Camera

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	Camera(const glm::vec3& position, float yaw, float pitch,
		float movementSpeed, float mouseSensitivity);

	void InitializeCameraSystem(const glm::vec3& position,
		float yaw, float pitch, float movementSpeed,
		float mouseSensitivity);

	void SetPosition(glm::vec3 const& position);

	void SetPositionYawPitch(const glm::vec3& position,
		float yaw, float pitch);

	glm::mat4 ConstructViewMatrix() const;

	glm::vec3 GetWorldPosition() const {
		return position;
	}

	glm::vec3 GetForwardDirection() const {
		return forward;
	}

	void MoveForward(float deltaTime);
	void MoveBackward(float deltaTime);
	void MoveRight(float deltaTime);
	void MoveLeft(float deltaTime);

	void ProcessMouse(float mouseXMovement, float mouseYMovement);

private:
	void UpdateCoordinateSystem();
	void UpdateAndClampPitch(float newValue);

	glm::vec3 position;
	glm::vec3 forward, up, right;
	glm::vec3 worldUp;
	float yaw, pitch;
	float movementSpeed;
	float mouseSensitivity;

	glm::vec3 boundsMin, boundsMax;
};
