
#include "scene.h"

class TestScene : public Scene
{
public:
	virtual void initialize(class RenderEngine* engine) override;

	virtual void update(float deltaTime) override;
};