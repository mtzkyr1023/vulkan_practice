#include "gameobject.h"
#include "camera.h"


std::shared_ptr<Camera> GameObjectManager::addCamera(Camera initial) {
	std::shared_ptr<Camera> object = std::make_shared<Camera>(
		initial.transform(),
		initial.fov(),
		initial.aspect(),
		initial.nearZ(),
		initial.farZ());

	cameras_.push_back(object);

	return object;
}

std::shared_ptr<Camera> GameObjectManager::getCamera(uint32_t index) {
	if (cameras_.size() > (size_t)index) {
		return cameras_[index];
	}

	std::shared_ptr<Camera> object = std::make_shared<Camera>(
		Transform(glm::vec3(), glm::quat(1.0f, 0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
		glm::pi<float>() * 0.5f,
		1.0f,
		0.1f,
		1000.0f);

	cameras_.push_back(object);

	return object;
}