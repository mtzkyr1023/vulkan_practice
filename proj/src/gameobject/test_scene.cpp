
#include "test_scene.h"
#include "../render_engine.h"
#include "../defines.h"


void TestScene::initialize(RenderEngine* engine)
{
	bgMesh_ = std::make_shared<MeshObject>("models/sponza/gltf/", "sponza.gltf");
	bgMesh_->setup(engine);

	shadowCaster_.aspect() = 1.0f;
	shadowCaster_.range() = 1000.0f;
	shadowCaster_.farZ() = 2000.0f;
	shadowCaster_.isFps() = false;
	shadowCaster_.isOrtho() = true;
	shadowCaster_.width() = (float)kShadowMapWidth;
	shadowCaster_.height() = (float)kShadowMapHeight;

	camera_.aspect() = (float)kScreenWidth / (float)kScreenHeight;
	shadowCaster_.transform().position() = bgMesh_->mesh().center() * 0.25f;
}