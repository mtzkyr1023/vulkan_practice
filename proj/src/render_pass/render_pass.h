#ifndef _RENDER_PASS_H_
#define _RENDER_PASS_H_

#include "vulkan/vulkan.hpp"

#include "../render_pipeine/resource.h"

class RenderPass
{
public:


	RenderPass();
	~RenderPass();

	void setup(class RenderEngine* engine);
	void cleanup(class RenderEngine* engine);

	const vk::RenderPass& renderPass() { return renderPass_; }

	const vk::Image& image(uint32_t index) { return images_[index]; }
	const vk::ImageView& imageView(uint32_t index) { return imageViews_[index]; }

	const vk::Framebuffer& framebuffer() { return framebuffer_; }

	const vk::DescriptorSet& descriptorSet(uint32_t index) { return descriptorSets_[index]; }

private:
	virtual void setupInternal(class RenderEngine* engine) = 0;

protected:
	vk::RenderPass renderPass_;

	std::vector<Memory> memories_;
	std::vector<vk::Image> images_;
	std::vector<vk::ImageView> imageViews_;

	std::vector<vk::DescriptorSet> descriptorSets_;

	vk::Framebuffer framebuffer_;
};

#endif