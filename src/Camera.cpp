#include "Camera.h"

Camera::Camera(const glm::vec3& position, 
	const glm::vec3& forwardVector, const glm::vec3& upVector,
	float yaw, float pitch, float movementSpeed,
	float mouseSensitivity) : position(position),
	forward(forwardVector), worldUp(upVector), yaw(yaw), pitch(pitch),
	movementSpeed(movementSpeed), mouseSensitivity(mouseSensitivity) {
	UpdateCoordinateSystem();
}

glm::mat4 Camera::ConstructViewMatrix() const {
	return glm::lookAt(position, position + forward, up);
}

void Camera::MoveForward(float deltaTime) {
	position += forward * movementSpeed * deltaTime;
}

void Camera::MoveBackward(float deltaTime) {
	position -= forward * movementSpeed * deltaTime;
}

void Camera::MoveRight(float deltaTime) {
	position += right * movementSpeed * deltaTime;
}

void Camera::MoveLeft(float deltaTime) {
	position -= right * movementSpeed * deltaTime;
}

void Camera::ProcessMouse(float mouseXMovement, float mouseYMovement) {
	mouseXMovement *= mouseSensitivity;
	mouseYMovement *= mouseSensitivity;

	yaw += mouseXMovement;
	pitch += mouseYMovement;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	UpdateCoordinateSystem();
}

void Camera::UpdateCoordinateSystem() {
	glm::vec3 newForward;
	newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newForward.y = sin(glm::radians(pitch));
	newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(newForward);
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}
