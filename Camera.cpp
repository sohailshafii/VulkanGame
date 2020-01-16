#include "Camera.h"

Camera::Camera(const glm::vec3& position, 
	const glm::vec3& forwardVector, const glm::vec3& upVector,
	float yaw, float pitch, float movementSpeed,
	float mouseSensitivity) : position(position),
	forward(forwardVector), worldUp(upVector), yaw(yaw), pitch(pitch),
	movementSpeed(movementSpeed), mouseSensitivity(mouseSensitivity) {
	updateCoordinateSystem();
}

glm::mat4 Camera::constructViewMatrix() const {
	return glm::lookAt(position, position + forward, up);
}

void Camera::moveForward(float deltaTime) {
	position += forward * movementSpeed * deltaTime;
}

void Camera::moveBackward(float deltaTime) {
	position -= forward * movementSpeed * deltaTime;
}

void Camera::moveRight(float deltaTime) {
	position += right * movementSpeed * deltaTime;
}

void Camera::moveLeft(float deltaTime) {
	position -= right * movementSpeed * deltaTime;
}

void Camera::processMouse(float mouseXMovement, float mouseYMovement) {
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

	updateCoordinateSystem();
}

void Camera::updateCoordinateSystem() {
	glm::vec3 newForward;
	newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newForward.y = sin(glm::radians(pitch));
	newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	forward = glm::normalize(newForward);
	right = glm::normalize(glm::cross(forward, worldUp));
	up = glm::normalize(glm::cross(right, forward));
}
