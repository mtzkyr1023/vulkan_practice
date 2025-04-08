#ifndef _SCENE_H_
#define _SCENE_H_

#include "gameobject.h"
#include "meshobject.h"
#include "camera.h"

#include "memory"
#include "vector"

class Scene
{
public:
	virtual void initialize(class RenderEngine* engine);
	virtual void cleanup(class RenderEngine* engine);

	std::vector<std::shared_ptr<MeshObject>>& meshObjects() { return meshObjects_; }

	std::shared_ptr<MeshObject>& meshObject(uint32_t index) { return meshObjects_[index]; }

	std::shared_ptr<MeshObject>& bgMesh() { return bgMesh_; }

protected:
	std::vector<std::shared_ptr<MeshObject>> meshObjects_;

	std::shared_ptr<MeshObject> bgMesh_;

	Camera camera_;

	Camera shadowCaster_;
};

#endif