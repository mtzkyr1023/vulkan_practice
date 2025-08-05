
#include "scene.h"
#include "../render_engine.h"


void Scene::initialize(RenderEngine* engine)
{
}

void Scene::cleanup(RenderEngine* engine)
{
	for (const auto& ite : meshObjects_)
	{
		ite->cleanup(engine);
	}

	bgMesh_->cleanup(engine);
}