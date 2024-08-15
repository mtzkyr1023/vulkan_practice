#ifndef _GAMEOBJECT_H_
#define _GAMEOBJECT_H_

#include "transform.h"

#include <vector>
#include <memory>

class GameObject {
public:
	GameObject() {}
	GameObject(
		const glm::vec3& position,
		const glm::quat& rotation,
		const glm::vec3& scale) :
		transform_(position, rotation, scale) {}
	GameObject(const Transform& transform) :
		transform_(transform) {}
	virtual ~GameObject() {}

	virtual void update(float deltaTime) {}

	Transform& transform() { return transform_; }

protected:
	Transform transform_;
};

class GameObjectManager {
public:
	GameObjectManager() {}
	~GameObjectManager() {}

	std::shared_ptr<class Camera> addCamera(class Camera initial);

	std::shared_ptr<class Camera> getCamera(uint32_t index);

private:
	std::vector<std::shared_ptr<GameObject>> objects_;

	std::vector<std::shared_ptr<class Camera>> cameras_;
};

#endif