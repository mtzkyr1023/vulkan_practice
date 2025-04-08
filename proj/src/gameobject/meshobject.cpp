
#include "meshobject.h"
#include "../render_engine.h"

MeshObject::MeshObject(const std::string& foldername, const std::string& filename) :
	foldername_(foldername),
	filename_(filename)
{

}

MeshObject::~MeshObject()
{

}

void MeshObject::setup(RenderEngine* engine)
{
	mesh_.loadMesh(engine, foldername_.c_str(), filename_.c_str());
}

void MeshObject::cleanup(RenderEngine* engine)
{
	mesh_.release(engine);
}

