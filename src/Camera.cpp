#include "Camera.h"
#include <iostream>

Camera::Camera(const glm::vec3& position, float yaw, float pitch,
	float movementSpeed, float mouseSensitivity) {
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	InitializeCameraSystem(position, yaw, pitch,
		movementSpeed, mouseSensitivity);
}

void Camera::SetPosition(glm::vec3 const& position) {
	this->position = position;
}

void Camera::InitializeCameraSystem(const glm::vec3& position,
	float yaw, float pitch, float movementSpeed,
	float mouseSensitivity) {
	this->position = position;
	this->yaw = yaw;
	UpdateAndClampPitch(pitch);
	this->movementSpeed = movementSpeed;
	this->mouseSensitivity = mouseSensitivity;
	UpdateCoordinateSystem();
}

void Camera::SetPositionYawPitch(const glm::vec3& position,
	float yaw, float pitch) {
	this->position = position;
	this->yaw = yaw;
	UpdateAndClampPitch(pitch);
	UpdateCoordinateSystem();
}

glm::mat4 Camera::ConstructViewMatrix() const {
	return glm::lookAt(position, position + forward, up);
}

void Camera::MoveForward(float deltaTime) {
	// prevent vertical movement
	position += glm::vec3(forward[0], 0.0f, forward[2]) * movementSpeed * deltaTime;
}

void Camera::MoveBackward(float deltaTime) {
	position -= glm::vec3(forward[0], 0.0f, forward[2]) * movementSpeed * deltaTime;
}

void Camera::MoveRight(float deltaTime) {
	position += glm::vec3(right[0], 0.0f, right[2]) * movementSpeed * deltaTime;
}

void Camera::MoveLeft(float deltaTime) {
	position -= glm::vec3(right[0], 0.0f, right[2]) * movementSpeed * deltaTime;
}

void Camera::ProcessMouse(float mouseXMovement, float mouseYMovement) {
	mouseXMovement *= mouseSensitivity;
	mouseYMovement *= mouseSensitivity;
	
	yaw += mouseXMovement;
	UpdateAndClampPitch(pitch + mouseYMovement);

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

void Camera::UpdateAndClampPitch(float newValue) {
	pitch = newValue;

	if (pitch > 80.0f) {
		pitch = 80.0f;
	}
	if (pitch < -80.0f) {
		pitch = -80.0f;
	}
}
