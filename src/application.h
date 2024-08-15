#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "render_pipeine/impl/simple_pipeline.h"

class Application {
public:
	Application();
	~Application();

	void initialize(class RenderEngine* engine);
	void cleanup(class RenderEngine* engine);

	void render(class RenderEngine* engine);

private:
	SimplePipeline simplePipeline_;
};

#endif