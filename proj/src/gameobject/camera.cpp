#include "camera.h"



void Camera::update(float deltaTime) {
	if (isFps_)
	{
		viewMatrix_ = glm::lookAt(
			transform_.position(),
			transform_.position() + transform_.forward(),
			-transform_.up());
	}
	else
	{
		viewMatrix_ = glm::lookAt(
			transform_.position() - transform_.forward() * range_,
			transform_.position(),
			-transform_.up());
	}
	if (isOrtho_)
	{
		projMatrix_ = glm::ortho(
			-width_ * 0.125f,
			width_ * 0.125f,
			height_ * 0.125f,
			-height_ * 0.125f,
			-farZ_,
			farZ_ 
		);
	}
	else
	{
		projMatrix_ = glm::perspective(
			fov_,
			aspect_,
			nearZ_,
			farZ_
		);
	}
}