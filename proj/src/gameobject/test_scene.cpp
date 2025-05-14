
#include "test_scene.h"
#include "../render_engine.h"
#include "../defines.h"
#include "../util/input.h"


void TestScene::initialize(RenderEngine* engine)
{
	bgMesh_ = std::make_shared<MeshObject>("models/sponza/gltf/", "sponza.gltf");
	bgMesh_->setup(engine);

	camera_.width() = (float)kScreenWidth;
	camera_.height() = (float)kScreenHeight;

	shadowCaster_.aspect() = 1.0f;
	shadowCaster_.range() = 1000.0f;
	shadowCaster_.farZ() = 2000.0f;
	shadowCaster_.nearZ() = 1.0f;
	shadowCaster_.isFps() = false;
	shadowCaster_.isOrtho() = true;
	shadowCaster_.width() = (float)kShadowMapWidth * 4;
	shadowCaster_.height() = (float)kShadowMapHeight * 4;

	camera_.aspect() = (float)kScreenWidth / (float)kScreenHeight;
	shadowCaster_.transform().position() = bgMesh_->mesh().center() * 0.25f;
}

void TestScene::update(float deltaTime)
{

	if (Input::Instance().Push(DIK_W))
	{
		camera_.transform().position() += camera_.transform().forward() * 50.0f * deltaTime;
	}
	if (Input::Instance().Push(DIK_S))
	{
		camera_.transform().position() -= camera_.transform().forward() * 50.0f * deltaTime;
	}
	if (Input::Instance().Push(DIK_D))
	{
		camera_.transform().position() += camera_.transform().right() * 50.0f * deltaTime;
	}
	if (Input::Instance().Push(DIK_A))
	{
		camera_.transform().position() -= camera_.transform().right() * 50.0f * deltaTime;
	}
	if (Input::Instance().Push(DIK_E))
	{
		camera_.transform().position() += camera_.transform().up() * 50.0f * deltaTime;
	}
	if (Input::Instance().Push(DIK_Q))
	{
		camera_.transform().position() -= camera_.transform().up() * 50.0f * deltaTime;
	}

	camera_.transform().rotation() *=
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveYRightPushed() * deltaTime * -2.0f, camera_.transform().right()) *
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveXRightPushed() * deltaTime * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	shadowCaster_.transform().rotation() *=
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveYLeftPushed() * deltaTime * -2.0f, shadowCaster_.transform().right()) *
		glm::rotate(glm::identity<glm::quat>(), Input::Instance().GetMoveXLeftPushed() * deltaTime * 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

	camera_.update(deltaTime);
	shadowCaster_.update(deltaTime);
}
