#ifndef _IMGUI_WRAPPER_H_
#define _IMGUI_WRAPPER_H_

#include "vulkan/vulkan.hpp"
#include "Windows.h"

class ImGuiWrapper
{
public:

	void setup(class RenderEngine* engine, HWND hwnd);
	void cleanup(class RenderEngine* engine);

	void prepare();

	void render(class RenderEngine* engine, const vk::CommandBuffer& cb);

private:
	vk::DescriptorPool pool_;
};

#endif