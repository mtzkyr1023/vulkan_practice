
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
		float f = 1.0f / glm::tan(fov_ * 0.5f);
		float fn = 1.0f / (nearZ_ - farZ_);

		projMatrix_[0][0] = width_ * 0.0625f;
		projMatrix_[0][1] = 0.0f;
		projMatrix_[0][2] = 0.0f;
		projMatrix_[0][3] = 0.0f;

		projMatrix_[1][0] = 0.0f;
		projMatrix_[1][1] = height_ * 0.0625f;
		projMatrix_[1][2] = 0.0f;
		projMatrix_[1][3] = 0.0f;

		projMatrix_[2][0] = 0.0f;
		projMatrix_[2][1] = 0.0f;
		projMatrix_[2][2] = 0.0f;
		projMatrix_[2][3] = -1.0f;

		projMatrix_[3][0] = 0.0f;
		projMatrix_[3][1] = 0.0f;
		projMatrix_[3][2] = -nearZ_;
		projMatrix_[3][3] = 0.0f;
	}
	else
	{
		float f = 1.0f / glm::tan(fov_ * 0.5f);
		float fn = 1.0f / (nearZ_ - farZ_);
		
		projMatrix_[0][0] = f / aspect_;
		projMatrix_[0][1] = 0.0f;
		projMatrix_[0][2] = 0.0f;
		projMatrix_[0][3] = 0.0f;

		projMatrix_[1][0] = 0.0f;
		projMatrix_[1][1] = f;
		projMatrix_[1][2] = 0.0f;
		projMatrix_[1][3] = 0.0f;

		projMatrix_[2][0] = 0.0f;
		projMatrix_[2][1] = 0.0f;
		projMatrix_[2][2] = 0.0f;
		projMatrix_[2][3] = -1.0f;

		projMatrix_[3][0] = 0.0f;
		projMatrix_[3][1] = 0.0f;
		projMatrix_[3][2] = -nearZ_ * fn;
		projMatrix_[3][3] = 0.0f;
	}
}