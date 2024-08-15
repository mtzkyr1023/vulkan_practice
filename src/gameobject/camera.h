#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "gameobject.h"


class Camera : public GameObject {
public:
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
		farZ_(farZ) {
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

protected:
	glm::mat4 viewMatrix_;
	glm::mat4 projMatrix_;

	float fov_;
	float aspect_;
	float nearZ_;
	float farZ_;
};

#endif