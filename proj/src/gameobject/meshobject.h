#ifndef _MESHOBJECT_H_
#define _MESHOBJECT_H_

#include "gameobject.h"
#include "../util/mesh.h"

#include <string>

class MeshObject : public GameObject
{
public:
	MeshObject(const std::string& foldername, const std::string& filename);
	~MeshObject();

	void setup(class RenderEngine* engine);
	void cleanup(class RenderEngine* engine);

	Mesh& mesh() { return mesh_; }

protected:
	Mesh mesh_;
	std::string foldername_;
	std::string filename_;
};

#endif