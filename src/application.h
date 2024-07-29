#ifndef _APPLICATION_H_
#define _APPLICATION_H_


#include "render_engine.h"


class Application {
public:
	Application();
	~Application();

	void initialize();
	void cleanup();

private:
	RenderEngine renderEngine_;

};

#endif