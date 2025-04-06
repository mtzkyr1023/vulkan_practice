#include "camera.h"



void Camera::update(float deltaTime) {
	viewMatrix_ = glm::lookAt(
		transform_.position(),
		transform_.position() + transform_.forward(),
		-transform_.up());
	projMatrix_ = glm::perspective(
		fov_,
		aspect_,
		nearZ_,
		farZ_
	);
}