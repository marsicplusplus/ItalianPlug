#include "camera.hpp"

#include "glm/gtc/matrix_transform.hpp"
#include "input_handler.hpp"

Camera::Camera(glm::vec3 pos, glm::vec3 up, float fov, float yaw, float pitch, float roll) 
	: position(pos), front(glm::vec3(0.0f, 0.0f, -1.0f)), worldUp(up), fov(fov), yaw(yaw), roll(roll), pitch(pitch) {
	updateVectors();
}

Camera::~Camera() {}

glm::mat4 Camera::getViewMatrix() const {
	return glm::lookAt(position, position + front, up);
}

void Camera::updateVectors() {
	// The camera rotated and moved, new up and front vectors
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

float Camera::getFOV() const {
	return fov;
}

glm::vec3 Camera::getPosition() const {
	return position;
}

void Camera::setPosition(glm::vec3 position) {
	position = position;
	InputHandler::Instance()->scrollState(0.0);
	updateVectors();
}

void Camera::update(float dt) {
	float scroll = InputHandler::Instance()->getScrollState();
	if(scroll != 0){
		position += glm::vec3(.0f, .0f, (scroll > 0) ? -10.0f : 10.0f) * dt;
		InputHandler::Instance()->scrollState(0.0);
		updateVectors();
	}
}
