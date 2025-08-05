#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "gameobject.h"


class Camera : public GameObject {
public:
	Camera() :
		fov_(glm::pi<float>() * 0.5f),
		aspect_(1.0f),
		nearZ_(0.1f),
		farZ_(1000.0f),
		isFps_(true),
		range_(1000.0f),
		width_(1.0f),
		height_(1.0f),
		isOrtho_(false) {
		viewMatrix_ = glm::lookAt(
			transform_.position(),
			transform_.forward(),
			transform_.up());
		projMatrix_ = glm::perspective(
			fov_,
			aspect_,
			nearZ_,
			farZ_
		);
	}
	Camera(
		const Transform& transform,
		float fov,
		float aspect,
		float nearZ,
		float farZ) :
		GameObject(transform),
		fov_(fov),
		aspect_(aspect),
		nearZ_(nearZ),
		farZ_(farZ),
		isFps_(true),
		range_(1000.0f),
		width_(1.0f),
		height_(1.0f),
		isOrtho_(false) {
		viewMatrix_ = glm::lookAt(
			transform_.position(),
			transform_.forward(),
			transform_.up());
		projMatrix_ = glm::perspective(
			fov,
			aspect,
			nearZ,
			farZ
		);
	}
	virtual ~Camera() {}

	virtual void update(float deltaTime) override;

	const glm::mat4& viewMatrix() { return viewMatrix_; }
	const glm::mat4& projMatrix() { return projMatrix_; }

	float& fov() { return fov_; }
	float& aspect() { return aspect_; }
	float& nearZ() { return nearZ_; }
	float& farZ() { return farZ_; }

	float& range() { return range_; }
	float& width() { return width_; }
	float& height() { return height_; }
	bool& isFps() { return isFps_; }
	bool& isOrtho() { return isOrtho_; }

	const glm::vec3 position();

protected:
	glm::mat4 viewMatrix_;
	glm::mat4 projMatrix_;

	float fov_;
	float aspect_;
	float nearZ_;
	float farZ_;

	float range_;

	float width_;
	float height_;

	bool isFps_;
	bool isOrtho_;
};

#endif