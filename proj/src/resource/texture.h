#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <vulkan/vulkan.hpp>

class Texture
{
public:
	Texture() {}
	virtual ~Texture() {}

	void setupRenderTarget2d(class RenderEngine* engine, uint32_t width, uint32_t height, vk::Format format);
	void setupResource2d(class RenderEngine* engine, const char* filename);

	void release(class RenderEngine* engine);

	const std::shared_ptr<Memory>& memory() { return memory_; }

	const vk::Image& image() { return image_; }
	const vk::ImageView& view() { return view_; }

protected:
	std::shared_ptr<class Memory> memory_;

	vk::Image image_;
	vk::ImageView view_;
};

#endif