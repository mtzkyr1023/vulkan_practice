
#include <glm/gtc/reciprocal.hpp>

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
		float f = 1.0f / glm::tan(fov_ * 0.5f);
		float fn = 1.0f / (nearZ_ - farZ_);
		projMatrix_ = glm::zero<glm::mat4>();
		projMatrix_[0][0] = f / aspect_;
		projMatrix_[1][1] = f;
		projMatrix_[2][2] = farZ_ * fn;
		projMatrix_[2][3] = farZ_ * nearZ_ * fn;
		projMatrix_[3][2] = -1.0f;
	}
}