
#include "test_scene.h"
#include "../render_engine.h"


void TestScene::initialize(RenderEngine* engine)
{
	bgMesh_ = std::make_shared<MeshObject>("models/sponza/gltf/", "sponza.gltf");
}