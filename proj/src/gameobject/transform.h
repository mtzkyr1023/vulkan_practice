#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Transform {
public:
	Transform() :
		position_(0.0f, 0.0f, 0.0f),
		rotation_(1.0f, 0.0f, 0.0f, 0.0f),
		scale_(1.0f, 1.0f, 1.0f) {}
	Transform(
		const glm::vec3& position,
		const glm::quat& rotation,
		const glm::vec3& scale) :
		position_(position),
		rotation_(rotation),
		scale_(scale) {}
	~Transform() {}

	glm::vec3& position() { return position_; }
	glm::quat& rotation() { return rotation_; }
	glm::vec3& scale() { return scale_; }

	glm::mat4 world() {
		return glm::translate(glm::identity<glm::mat4>(), position_) *
			glm::mat4(rotation_) *
			glm::scale(glm::identity<glm::mat4>(), scale_);
	}

	glm::vec3 forward() {
		glm::mat4 world = glm::translate(glm::identity<glm::mat4>(), position_) *
			glm::mat4(rotation_) *
			glm::scale(glm::identity<glm::mat4>(), scale_);
		return glm::vec3(world[0][0], world[1][0], world[2][0]);
	}
	glm::vec3 right() {
		glm::mat4 world = glm::translate(glm::identity<glm::mat4>(), position_) *
			glm::mat4(rotation_) *
			glm::scale(glm::identity<glm::mat4>(), scale_);
		return glm::vec3(world[0][1], world[1][1], world[2][1]);
	}
	glm::vec3 up() {
		glm::mat4 world = glm::translate(glm::identity<glm::mat4>(), position_) *
			glm::mat4(rotation_) *
			glm::scale(glm::identity<glm::mat4>(), scale_);
		return glm::vec3(world[0][2], world[1][2], world[2][2]);
	}

protected:
	glm::vec3 position_;
	glm::quat rotation_;
	glm::vec3 scale_;
};

#endif