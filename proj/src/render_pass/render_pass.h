﻿#ifndef _RENDER_PASS_H_
#define _RENDER_PASS_H_

#include <vulkan/vulkan.hpp>

class RenderPass
{
public:


	RenderPass();
	~RenderPass();

	void setup(class RenderEngine* engine, const std::vector<class Texture*>& resources);
	void cleanup(class RenderEngine* engine);

	const vk::RenderPass& renderPass() { return renderPass_; }

	const vk::Framebuffer& framebuffer() { return framebuffer_; }

private:
	virtual void setupInternal(class RenderEngine* engine, const std::vector<class Texture*>& resources) = 0;

protected:
	vk::RenderPass renderPass_;

	vk::Framebuffer framebuffer_;
};

#endif